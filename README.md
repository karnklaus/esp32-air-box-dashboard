# Bangkok Air Box Dashboard

ESP32-based air monitoring system for Bangkok with a Flask dashboard, live map, overview analytics, local OLED display, and Wi-Fi device configuration.

## What This Project Does

- Reads particulate matter from `PMS5003`
  - `PM1.0`
  - `PM2.5`
  - `PM10`
- Reads sound activity from `INMP441`
- Shows local status on an `NFP1315-157B` OLED
- Sends live sensor data from ESP32 to a Flask server over Wi-Fi
- Displays multiple air boxes on a Bangkok map
- Provides overview analytics, history, and CSV export
- Supports on-device config mode with captive portal + OLED QR screen

## System Overview

```text
ESP32 Air Box
  |- PMS5003
  |- INMP441
  |- OLED
  |- Wi-Fi config mode
  `- HTTP POST every 3s
         |
         v
Flask Server
  |- Dashboard
  |- Overview
  |- Config
  |- Device metadata
  `- History + CSV export
```

## Main Components

### Server

- [app.py](/c:/Users/Ancie/Desktop/embeded/app.py)
  - Flask app
  - Dashboard routes
  - Overview routes
  - Config page
  - Device report API
  - History and CSV export

### ESP32 Firmware

- [testing_function.ino](/c:/Users/Ancie/Desktop/embeded/testing_function/testing_function.ino)
  - Main production firmware
  - Reads sensors
  - Drives OLED
  - Handles config mode
  - Stores settings in flash
  - Sends live data to Flask

### Frontend

- [index.html](/c:/Users/Ancie/Desktop/embeded/templates/index.html) - dashboard
- [overview.html](/c:/Users/Ancie/Desktop/embeded/templates/overview.html) - analytics page
- [config.html](/c:/Users/Ancie/Desktop/embeded/templates/config.html) - server config page
- [dashboard.js](/c:/Users/Ancie/Desktop/embeded/static/dashboard.js)
- [overview.js](/c:/Users/Ancie/Desktop/embeded/static/overview.js)
- [dashboard.css](/c:/Users/Ancie/Desktop/embeded/static/dashboard.css)

## Sensor Test Sketches

These folders are intentionally kept in the repository for isolated hardware testing:

- [PMS5003_TEST](/c:/Users/Ancie/Desktop/embeded/PMS5003_TEST/PMS5003_TEST.ino)
- [INMP441_TEST](/c:/Users/Ancie/Desktop/embeded/INMP441_TEST/INMP441_TEST.ino)
- [RC522_TEST](/c:/Users/Ancie/Desktop/embeded/RC522_TEST/RC522_TEST.ino)
- [NFP1315_157B_TEST](/c:/Users/Ancie/Desktop/embeded/NFP1315_157B_TEST/NFP1315_157B_TEST.ino)

## Quick Start

### 1. Install Python dependencies

```bash
pip install -r requirements.txt
```

### 2. Run the Flask server

```bash
python app.py
```

### 3. Open the web UI

```text
http://127.0.0.1:5000
```

Main pages:

- `/dashboard`
- `/overview`
- `/config`

## ESP32 Config Mode

The ESP32 firmware includes a local config portal.

Config fields:

- `Box ID`
- `WiFi SSID`
- `WiFi Password`
- `Server IP`
- `Server Port`

Red button behavior:

- Hold `5 seconds` to enter config mode
- Hold `5 seconds` again to exit config mode
- Short press in config mode to switch OLED info/QR view

Config mode features:

- Captive portal
- Wi-Fi SSID dropdown from scanned networks
- OLED config info
- QR screen for quick phone access

## Data Flow

### Stored on ESP32

- `Box ID`
- `WiFi SSID`
- `WiFi Password`
- `Server IP`
- `Server Port`

### Stored on Server

- `name`
- `district`
- `lat`
- `lng`
- latest sensor snapshot
- history records

### Sent from ESP32 to Server

Every `3 seconds`, the ESP32 reports:

- `id`
- `pm1_0`
- `pm2_5`
- `pm10`
- `sound_state`

## Runtime Data

Runtime files are stored under `data/` and ignored by Git:

- `air_boxes.json`
- `air_box_history.json`
- other local runtime files if generated later

If missing, the server recreates what it needs automatically.

## Repository Layout

```text
.
|- app.py
|- requirements.txt
|- README.md
|- testing_function/
|  `- testing_function.ino
|- templates/
|  |- index.html
|  |- overview.html
|  `- config.html
|- static/
|  |- dashboard.css
|  |- dashboard.js
|  `- overview.js
|- data/
`- *_TEST/
```

## Requirements

Server:

- Python 3.10+
- Flask

Hardware:

- ESP32
- PMS5003
- INMP441
- NFP1315-157B OLED

## Notes

- This repository keeps the hardware test sketches on purpose.
- Runtime data is excluded from Git for a cleaner repo.
- The dashboard is designed for multiple air boxes, not only one device.

## License

No license has been added yet.
