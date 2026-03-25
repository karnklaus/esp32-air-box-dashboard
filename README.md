# Bangkok Air Box

ESP32 air quality dashboard project with:
- `PMS5003` for `PM1.0 / PM2.5 / PM10`
- `INMP441` for sound detection
- `NFP1315-157B OLED` for local display
- Flask web dashboard for map, overview, history, and config

## Main Files

- `app.py`
  Flask server for dashboard, overview, config page, APIs, history, and CSV export.

- `testing_function/testing_function.ino`
  Main ESP32 firmware.
  Features:
  - reads `PMS5003`
  - reads `INMP441`
  - shows values on OLED
  - has Wi-Fi config mode with captive portal
  - stores `Box ID`, Wi-Fi, server IP, and port in flash
  - sends sensor data to Flask every 3 seconds by HTTP POST

- `templates/`
  Flask HTML pages:
  - `index.html` = dashboard
  - `overview.html` = analytics/overview
  - `config.html` = server config page

- `static/`
  Frontend assets:
  - `dashboard.js`
  - `overview.js`
  - `dashboard.css`

## Sensor Test Sketches

These folders are kept on purpose for separate sensor testing:
- `PMS5003_TEST/`
- `INMP441_TEST/`
- `RC522_TEST/`
- `NFP1315_157B_TEST/`

## Running The Server

Install Flask:

```bash
pip install flask
```

Run:

```bash
python app.py
```

Open:

```text
http://127.0.0.1:5000
```

## ESP32 Config Mode

In firmware config mode you can set:
- `Box ID`
- `WiFi SSID`
- `WiFi Password`
- `Server IP`
- `Server Port`

Red button behavior:
- hold `5 seconds` to enter config mode
- hold `5 seconds` again to exit config mode
- short press while in config mode toggles info/QR view on OLED

## Data Storage

Runtime data is stored locally in `data/`:
- `air_boxes.json`
- `air_box_history.json`

These files are ignored by Git on purpose.
If they do not exist, `app.py` will recreate what it needs automatically.

## Notes For GitHub

Ignored from Git:
- local VS Code settings
- runtime JSON/database files in `data/`
- Python cache files

This keeps the repository clean while preserving the test sketch folders.
