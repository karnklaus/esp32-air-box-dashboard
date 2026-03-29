from __future__ import annotations

import json
import os
import socket
import csv
import io
import threading
import time
import urllib.request
import urllib.error
from pathlib import Path
from datetime import datetime, timezone, timedelta

from flask import Flask, Response, jsonify, render_template, request


BASE_DIR = Path(__file__).resolve().parent
DATA_DIR = BASE_DIR / "data"
DATA_FILE = BASE_DIR / "data" / "air_boxes.json"
HISTORY_FILE = BASE_DIR / "data" / "air_box_history.json"
SERVER_SETTINGS_FILE = BASE_DIR / "data" / "server_settings.json"
ALERT_STATE_FILE = BASE_DIR / "data" / "alert_state.json"
MAX_HISTORY_PER_BOX = 240
LOUD_ALERT_COOLDOWN_SECONDS = 60
STATUS_MONITOR_INTERVAL_SECONDS = 5
ALERT_WARMUP_SECONDS = 10
ALERT_STATE_LOCK = threading.Lock()
APP_START_MONOTONIC = time.monotonic()

app = Flask(__name__)

BANGKOK_TZ = timezone(timedelta(hours=7))


DEFAULT_SERVER_SETTINGS = {
    "discord_webhook_url": "",
    "alert_pm1_threshold": 10,
    "alert_pm25_threshold": 20,
    "alert_pm10_threshold": 20,
    "alert_offline_seconds": 15,
    "alert_loud_cooldown_seconds": 60,
}


def detect_server_ip() -> str:
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    try:
        sock.connect(("10.255.255.255", 1))
        return sock.getsockname()[0]
    except OSError:
        return "127.0.0.1"
    finally:
        sock.close()


def load_air_boxes() -> list[dict]:
    if not DATA_FILE.exists():
        return []
    with DATA_FILE.open("r", encoding="utf-8") as f:
        return json.load(f)


def load_server_settings() -> dict:
    if not SERVER_SETTINGS_FILE.exists():
        return dict(DEFAULT_SERVER_SETTINGS)
    with SERVER_SETTINGS_FILE.open("r", encoding="utf-8") as f:
        data = json.load(f)
    merged = dict(DEFAULT_SERVER_SETTINGS)
    merged.update(data)
    return merged


def save_server_settings(settings: dict) -> None:
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    merged = dict(DEFAULT_SERVER_SETTINGS)
    merged.update(settings)
    with SERVER_SETTINGS_FILE.open("w", encoding="utf-8") as f:
        json.dump(merged, f, ensure_ascii=False, indent=2)


def load_alert_state() -> dict:
    if not ALERT_STATE_FILE.exists():
        return {}
    with ALERT_STATE_FILE.open("r", encoding="utf-8") as f:
        return json.load(f)


def save_alert_state(state: dict) -> None:
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    with ALERT_STATE_FILE.open("w", encoding="utf-8") as f:
        json.dump(state, f, ensure_ascii=False, indent=2)


def load_air_box_history() -> list[dict]:
    if not HISTORY_FILE.exists():
        return []
    with HISTORY_FILE.open("r", encoding="utf-8") as f:
        return json.load(f)


def save_air_boxes(air_boxes: list[dict]) -> None:
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    with DATA_FILE.open("w", encoding="utf-8") as f:
        json.dump(air_boxes, f, ensure_ascii=False, indent=2)


def save_air_box_history(history: list[dict]) -> None:
    DATA_DIR.mkdir(parents=True, exist_ok=True)
    with HISTORY_FILE.open("w", encoding="utf-8") as f:
        json.dump(history, f, ensure_ascii=False, indent=2)


def find_air_box(air_boxes: list[dict], box_id: str) -> dict | None:
    for box in air_boxes:
        if box["id"] == box_id:
            return box
    return None


def default_air_box(box_id: str) -> dict:
    return {
        "id": box_id,
        "name": "NONE",
        "district": "",
        "lat": 13.7563,
        "lng": 100.5018,
        "pm1_0": 0,
        "pm2_5": 0,
        "pm10": 0,
        "temperature_c": None,
        "humidity_rh": None,
        "sound_state": "QUIET",
        "last_seen": None,
    }


def now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


def server_config_context() -> dict:
    port = 5000
    server_ip = detect_server_ip()
    settings = load_server_settings()
    return {
        "server_ip": server_ip,
        "server_port": port,
        "report_url": f"http://{server_ip}:{port}/api/air-boxes/report",
        "discord_webhook_url": settings.get("discord_webhook_url", ""),
        "alert_pm1_threshold": settings.get("alert_pm1_threshold", 10),
        "alert_pm25_threshold": settings.get("alert_pm25_threshold", 20),
        "alert_pm10_threshold": settings.get("alert_pm10_threshold", 20),
        "alert_offline_seconds": settings.get("alert_offline_seconds", 15),
        "alert_loud_cooldown_seconds": settings.get("alert_loud_cooldown_seconds", 60),
    }


def parse_iso_timestamp(value: str | None) -> datetime | None:
    if not value:
        return None
    try:
        return datetime.fromisoformat(value)
    except ValueError:
        return None


def is_online(box: dict, now: datetime | None = None) -> bool:
    now = now or datetime.now(timezone.utc)
    seen = parse_iso_timestamp(box.get("last_seen"))
    if seen is None:
        return False
    return (now - seen).total_seconds() <= 10


def discord_alert_enabled() -> bool:
    settings = load_server_settings()
    return bool(str(settings.get("discord_webhook_url", "")).strip())


def alerts_enabled() -> bool:
    return (time.monotonic() - APP_START_MONOTONIC) >= ALERT_WARMUP_SECONDS


def send_discord_message(message: str) -> bool:
    settings = load_server_settings()
    webhook_url = str(settings.get("discord_webhook_url", "")).strip()
    if not webhook_url:
        print("[discord] skipped: webhook URL not set")
        return False

    payload = json.dumps({"content": message}).encode("utf-8")
    req = urllib.request.Request(
        webhook_url,
        data=payload,
        headers={
            "Content-Type": "application/json",
            "Accept": "application/json",
            "User-Agent": "BangkokAirBox/1.0 (+Flask Discord Webhook Client)",
        },
        method="POST",
    )
    try:
        with urllib.request.urlopen(req, timeout=5) as response:
            ok = 200 <= response.status < 300
            print(f"[discord] response status={response.status} ok={ok}")
            return ok
    except urllib.error.HTTPError as exc:
        try:
            error_body = exc.read().decode("utf-8", errors="replace")
        except Exception:
            error_body = "<unable to read body>"
        print(f"[discord] HTTPError status={exc.code} body={error_body}")
        return False
    except (urllib.error.URLError, TimeoutError) as exc:
        print(f"[discord] send failed: {exc}")
        return False


def get_alert_flag(state: dict, key: str) -> bool | None:
    if key not in state:
        return None
    return bool(state.get(key))


def set_alert_flag(state: dict, key: str, value: bool) -> None:
    state[key] = value


def get_alert_timestamp(state: dict, key: str) -> datetime | None:
    return parse_iso_timestamp(state.get(key))


def set_alert_timestamp(state: dict, key: str, value: datetime) -> None:
    state[key] = value.isoformat()


def box_label(box: dict) -> str:
    name = str(box.get("name", "NONE")).strip() or "NONE"
    box_id = str(box.get("id", "")).strip()
    district = str(box.get("district", "")).strip()
    if district:
        return f"{name} ({box_id}, {district})"
    return f"{name} ({box_id})"


def format_local_time(value: datetime | None = None) -> str:
    value = value or datetime.now(timezone.utc)
    return value.astimezone(BANGKOK_TZ).strftime("%d/%m/%Y %H:%M:%S")


def build_alert_message(alert_type: str, box: dict, now: datetime, extra_lines: list[str] | None = None) -> str:
    lines = [
        "====================",
        "**AIR BOX ALERT**",
        "--------------------",
        f"Time: {format_local_time(now)}",
        f"Type: {alert_type}",
        "",
        f"Name: {str(box.get('name', 'NONE')).strip() or 'NONE'}",
        f"Box ID: {str(box.get('id', '-')).strip() or '-'}",
        f"District: {str(box.get('district', '-')).strip() or '-'}",
        "",
        f"PM1.0: {box.get('pm1_0', 0)}",
        f"PM2.5: {box.get('pm2_5', 0)}",
        f"PM10: {box.get('pm10', 0)}",
    ]
    if extra_lines:
        lines.append("")
        lines.extend(extra_lines)
    lines.append("====================")
    return "\n".join(lines)


def evaluate_live_alerts(box: dict) -> None:
    if not alerts_enabled():
        return

    settings = load_server_settings()
    webhook_url = str(settings.get("discord_webhook_url", "")).strip()
    if not webhook_url:
        return

    pm1_threshold = int(settings.get("alert_pm1_threshold", 10))
    pm25_threshold = int(settings.get("alert_pm25_threshold", 20))
    pm10_threshold = int(settings.get("alert_pm10_threshold", 20))
    loud_cooldown_seconds = int(settings.get("alert_loud_cooldown_seconds", LOUD_ALERT_COOLDOWN_SECONDS))
    now = datetime.now(timezone.utc)
    with ALERT_STATE_LOCK:
        state = load_alert_state()
        changed = False

        pm_rules = [
            ("pm1_0", pm1_threshold, "PM1.0 HIGH"),
            ("pm2_5", pm25_threshold, "PM2.5 HIGH"),
            ("pm10", pm10_threshold, "PM10 HIGH"),
        ]

        for field, threshold, alert_name in pm_rules:
            key = f"status:{box['id']}:{field}_high"
            current_high = int(box.get(field, 0)) > threshold
            previous_high = get_alert_flag(state, key)

            if previous_high is None:
                set_alert_flag(state, key, current_high)
                changed = True
                continue

            if current_high and not previous_high:
                print(f"[alert] {alert_name} triggered for {box['id']} value={box.get(field, 0)}")
                sent = send_discord_message(
                    build_alert_message(
                        alert_name,
                        box,
                        now,
                        [f"Threshold: {threshold}", f"Reading: {box.get(field, 0)}"],
                    )
                )
                if sent:
                    print(f"[alert] {alert_name} sent for {box['id']}")
                else:
                    print(f"[alert] {alert_name} failed for {box['id']}")

            if previous_high != current_high:
                set_alert_flag(state, key, current_high)
                changed = True

        loud_key = f"status:{box['id']}:loud"
        current_loud = normalize_sound_filter(box.get("sound_state")) == "LOUD"
        previous_loud = get_alert_flag(state, loud_key)
        loud_sent_key = f"time:{box['id']}:loud_sent"
        last_loud_sent = get_alert_timestamp(state, loud_sent_key)

        if previous_loud is None:
            set_alert_flag(state, loud_key, current_loud)
            changed = True
        else:
            should_send_loud = current_loud and (
                last_loud_sent is None or
                (now - last_loud_sent).total_seconds() >= loud_cooldown_seconds
            )

            if should_send_loud:
                print(f"[alert] LOUD triggered for {box['id']}")
                sent = send_discord_message(
                    build_alert_message(
                        "LOUD DETECTED",
                        box,
                        now,
                        ["Status: LOUD"],
                    )
                )
                if sent:
                    print(f"[alert] LOUD sent for {box['id']}")
                else:
                    print(f"[alert] LOUD failed for {box['id']}")
                set_alert_timestamp(state, loud_sent_key, now)
                changed = True

            if previous_loud != current_loud:
                set_alert_flag(state, loud_key, current_loud)
                changed = True

        if changed:
            save_alert_state(state)


def evaluate_status_change_alerts(air_boxes: list[dict]) -> None:
    if not alerts_enabled():
        return

    settings = load_server_settings()
    webhook_url = str(settings.get("discord_webhook_url", "")).strip()
    if not webhook_url:
        return

    offline_seconds = int(settings.get("alert_offline_seconds", 15))
    now = datetime.now(timezone.utc)
    with ALERT_STATE_LOCK:
        state = load_alert_state()
        changed = False

        for box in air_boxes:
            seen = parse_iso_timestamp(box.get("last_seen"))
            current_online = False if seen is None else (now - seen).total_seconds() <= offline_seconds
            key = f"status:{box['id']}:online"
            previous_online = get_alert_flag(state, key)

            if previous_online is None:
                set_alert_flag(state, key, current_online)
                changed = True
                continue

            if previous_online != current_online:
                alert_type = "DEVICE ONLINE" if current_online else "DEVICE OFFLINE"
                extra_lines = ["Status: ONLINE"] if current_online else [
                    "Status: OFFLINE",
                    f"Last seen: {box.get('last_seen', '-')}",
                ]
                print(f"[alert] {alert_type} triggered for {box['id']}")
                sent = send_discord_message(build_alert_message(alert_type, box, now, extra_lines))
                if sent:
                    print(f"[alert] {alert_type} sent for {box['id']}")
                else:
                    print(f"[alert] {alert_type} failed for {box['id']}")
                set_alert_flag(state, key, current_online)
                changed = True

        if changed:
            save_alert_state(state)


def background_status_monitor() -> None:
    while True:
        try:
            evaluate_status_change_alerts(load_air_boxes())
        except Exception as exc:
            print(f"[status-monitor] error: {exc}")
        time.sleep(STATUS_MONITOR_INTERVAL_SECONDS)


def start_background_status_monitor() -> None:
    print(f"[status-monitor] started interval={STATUS_MONITOR_INTERVAL_SECONDS}s")
    worker = threading.Thread(target=background_status_monitor, daemon=True, name="airbox-status-monitor")
    worker.start()


def normalize_sound_filter(value: str | None) -> str:
    return str(value or "").strip().upper()


def filter_air_boxes(
    air_boxes: list[dict],
    district: str = "",
    online: str = "",
    loud: str = "",
) -> list[dict]:
    district_value = district.strip().casefold()
    online_value = online.strip().lower()
    loud_value = loud.strip().upper()
    now = datetime.now(timezone.utc)

    filtered = []
    for box in air_boxes:
        if district_value and str(box.get("district", "")).strip().casefold() != district_value:
            continue
        if online_value == "online" and not is_online(box, now):
            continue
        if online_value == "offline" and is_online(box, now):
            continue
        if loud_value == "LOUD" and normalize_sound_filter(box.get("sound_state")) != "LOUD":
            continue
        if loud_value == "QUIET" and normalize_sound_filter(box.get("sound_state")) != "QUIET":
            continue
        filtered.append(box)
    return filtered


def append_history_entry(history: list[dict], box: dict) -> list[dict]:
    entry = {
        "id": box["id"],
        "timestamp": box.get("last_seen"),
        "pm1_0": int(box.get("pm1_0", 0)),
        "pm2_5": int(box.get("pm2_5", 0)),
        "pm10": int(box.get("pm10", 0)),
        "temperature_c": box.get("temperature_c"),
        "humidity_rh": box.get("humidity_rh"),
        "sound_state": str(box.get("sound_state", "QUIET")),
    }
    history.append(entry)

    per_box_counts: dict[str, int] = {}
    trimmed: list[dict] = []
    for item in reversed(history):
        box_id = str(item.get("id", ""))
        count = per_box_counts.get(box_id, 0)
        if count >= MAX_HISTORY_PER_BOX:
            continue
        per_box_counts[box_id] = count + 1
        trimmed.append(item)
    trimmed.reverse()
    return trimmed


@app.route("/")
@app.route("/dashboard")
def index():
    air_boxes = load_air_boxes()
    return render_template("index.html", air_boxes=air_boxes, server_config=server_config_context())


@app.route("/overview")
def overview_page():
    air_boxes = load_air_boxes()
    return render_template("overview.html", air_boxes=air_boxes, server_config=server_config_context())


@app.route("/config", methods=["GET", "POST"])
def config_page():
    save_status = ""
    if request.method == "POST":
        current = load_server_settings()
        action = str(request.form.get("action", "save")).strip().lower()
        if action == "test_discord":
            test_box = {
                "name": "TEST BOX",
                "id": "ABX-TEST",
                "district": "Bangkok",
                "pm1_0": 8,
                "pm2_5": 16,
                "pm10": 18,
            }
            test_message = build_alert_message(
                "TEST MESSAGE",
                test_box,
                datetime.now(timezone.utc),
                ["Status: Discord webhook is working"],
            )
            if send_discord_message(test_message):
                save_status = "Discord test sent"
            else:
                save_status = "Discord test failed"
        else:
            current["discord_webhook_url"] = str(request.form.get("discord_webhook_url", "")).strip()
            try:
                current["alert_pm1_threshold"] = int(request.form.get("alert_pm1_threshold", current["alert_pm1_threshold"]))
                current["alert_pm25_threshold"] = int(request.form.get("alert_pm25_threshold", current["alert_pm25_threshold"]))
                current["alert_pm10_threshold"] = int(request.form.get("alert_pm10_threshold", current["alert_pm10_threshold"]))
                current["alert_offline_seconds"] = int(request.form.get("alert_offline_seconds", current["alert_offline_seconds"]))
                current["alert_loud_cooldown_seconds"] = int(
                    request.form.get("alert_loud_cooldown_seconds", current["alert_loud_cooldown_seconds"])
                )
            except ValueError:
                save_status = "Invalid alert settings"
            else:
                save_server_settings(current)
                save_status = "Saved"

    ctx = server_config_context()
    ctx["save_status"] = save_status
    return render_template("config.html", server_config=ctx)


@app.route("/api/air-boxes")
def air_boxes_api():
    air_boxes = load_air_boxes()
    return jsonify(air_boxes)


@app.route("/api/overview/history")
def overview_history_api():
    history = load_air_box_history()
    box_id = str(request.args.get("box_id", "")).strip().upper()
    limit = request.args.get("limit", default=120, type=int)

    if box_id:
        history = [entry for entry in history if str(entry.get("id", "")).upper() == box_id]

    if limit and limit > 0:
        history = history[-limit:]

    return jsonify(history)


@app.route("/api/overview/export.csv")
def overview_export_csv():
    air_boxes = load_air_boxes()
    filtered = filter_air_boxes(
        air_boxes,
        district=str(request.args.get("district", "")),
        online=str(request.args.get("online", "")),
        loud=str(request.args.get("loud", "")),
    )

    output = io.StringIO()
    writer = csv.writer(output)
    writer.writerow([
        "name",
        "id",
        "district",
        "lat",
        "lng",
        "pm1_0",
        "pm2_5",
        "pm10",
        "temperature_c",
        "humidity_rh",
        "sound_state",
        "last_seen",
    ])
    for box in filtered:
        writer.writerow([
            box.get("name", "NONE"),
            box.get("id", ""),
            box.get("district", ""),
            box.get("lat", ""),
            box.get("lng", ""),
            box.get("pm1_0", 0),
            box.get("pm2_5", 0),
            box.get("pm10", 0),
            box.get("temperature_c", ""),
            box.get("humidity_rh", ""),
            box.get("sound_state", "QUIET"),
            box.get("last_seen", ""),
        ])

    csv_data = output.getvalue()
    output.close()
    return Response(
        csv_data,
        mimetype="text/csv",
        headers={"Content-Disposition": "attachment; filename=air_boxes_export.csv"},
    )


@app.route("/api/air-boxes/<box_id>", methods=["PATCH"])
def update_air_box(box_id: str):
    payload = request.get_json(silent=True) or {}
    air_boxes = load_air_boxes()
    box = find_air_box(air_boxes, box_id)
    if box is None:
        return jsonify({"error": "Air box not found"}), 404

    editable_fields = {
        "name": str,
        "district": str,
        "lat": float,
        "lng": float,
        "pm1_0": int,
        "pm2_5": int,
        "pm10": int,
        "sound_state": str,
    }

    for field, caster in editable_fields.items():
        if field not in payload:
            continue
        try:
            value = caster(payload[field])
        except (TypeError, ValueError):
            return jsonify({"error": f"Invalid value for {field}"}), 400
        box[field] = value

    if "name" not in box or not str(box["name"]).strip():
        box["name"] = "NONE"

    save_air_boxes(air_boxes)
    return jsonify(box)


@app.route("/api/air-boxes/<box_id>", methods=["DELETE"])
def delete_air_box(box_id: str):
    air_boxes = load_air_boxes()
    box = find_air_box(air_boxes, box_id)
    if box is None:
        return jsonify({"error": "Air box not found"}), 404

    air_boxes = [item for item in air_boxes if item["id"] != box_id]
    save_air_boxes(air_boxes)
    history = [item for item in load_air_box_history() if item.get("id") != box_id]
    save_air_box_history(history)
    alert_state = load_alert_state()
    cleaned_state = {
        key: value
        for key, value in alert_state.items()
        if f":{box_id}:" not in key and not key.startswith(f"status:{box_id}:")
    }
    save_alert_state(cleaned_state)
    return jsonify({"deleted": box_id})


@app.route("/api/air-boxes/report", methods=["POST"])
def report_air_box():
    payload = request.get_json(silent=True) or {}
    box_id = str(payload.get("id", "")).strip().upper()
    if not box_id:
        return jsonify({"error": "Missing air box id"}), 400

    air_boxes = load_air_boxes()
    box = find_air_box(air_boxes, box_id)
    is_new_box = box is None
    if box is None:
        box = default_air_box(box_id)
        air_boxes.append(box)

    live_fields = {
        "pm1_0": int,
        "pm2_5": int,
        "pm10": int,
        "temperature_c": float,
        "humidity_rh": float,
        "sound_state": str,
    }
    metadata_fields = {
        "lat": float,
        "lng": float,
        "district": str,
        "name": str,
    }

    for field, caster in live_fields.items():
        if field not in payload:
            continue
        try:
            box[field] = caster(payload[field])
        except (TypeError, ValueError):
            return jsonify({"error": f"Invalid value for {field}"}), 400

    # Persist metadata per box ID on this machine.
    # Device reports may include location/name hints, but once a box already exists
    # we keep the locally configured metadata instead of overwriting it on every report.
    for field, caster in metadata_fields.items():
        if field not in payload:
            continue
        if not is_new_box:
            continue
        try:
            box[field] = caster(payload[field])
        except (TypeError, ValueError):
            return jsonify({"error": f"Invalid value for {field}"}), 400

    if not str(box.get("name", "")).strip():
        box["name"] = "NONE"

    box["last_seen"] = now_iso()

    save_air_boxes(air_boxes)
    history = append_history_entry(load_air_box_history(), box)
    save_air_box_history(history)
    evaluate_live_alerts(box)
    return jsonify(box), 201


if __name__ == "__main__":
    debug_mode = True
    if os.environ.get("WERKZEUG_RUN_MAIN") == "true" or not debug_mode:
        start_background_status_monitor()
    app.run(host="0.0.0.0", port=5000, debug=debug_mode)
