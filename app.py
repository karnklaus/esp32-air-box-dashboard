from __future__ import annotations

import json
import socket
import csv
import io
from pathlib import Path
from datetime import datetime, timezone

from flask import Flask, Response, jsonify, render_template, request


BASE_DIR = Path(__file__).resolve().parent
DATA_DIR = BASE_DIR / "data"
DATA_FILE = BASE_DIR / "data" / "air_boxes.json"
HISTORY_FILE = BASE_DIR / "data" / "air_box_history.json"
MAX_HISTORY_PER_BOX = 240

app = Flask(__name__)


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
        "sound_state": "QUIET",
        "last_seen": None,
    }


def now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


def server_config_context() -> dict:
    port = 5000
    server_ip = detect_server_ip()
    return {
        "server_ip": server_ip,
        "server_port": port,
        "report_url": f"http://{server_ip}:{port}/api/air-boxes/report",
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


@app.route("/config")
def config_page():
    return render_template("config.html", server_config=server_config_context())


@app.route("/api/air-boxes")
def air_boxes_api():
    return jsonify(load_air_boxes())


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
    writer.writerow(["name", "id", "district", "lat", "lng", "pm1_0", "pm2_5", "pm10", "sound_state", "last_seen"])
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
    return jsonify(box), 201


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=True)
