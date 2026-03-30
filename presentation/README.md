# ESP32 Air Box — Presentation App

Presentation web app อธิบายระบบ **ESP32 Air Box Dashboard** แบบ interactive
พร้อม diagram, animation และ live simulation

> Built with **Vue 3 + Vite**, Vue Router 4, Chart.js, Font Awesome

---

## Requirements

| Tool | Version |
|------|---------|
| Node.js | ≥ 18 |
| npm | ≥ 9 |

ตรวจสอบ version ที่ติดตั้งอยู่

```bash
node -v
npm -v
```

---

## Installation

```bash
# 1. เข้า folder
cd presentation

# 2. ติดตั้ง dependencies
npm install
```

---

## Run (Development)

```bash
npm run dev
```

เปิดเบราว์เซอร์ไปที่ → **http://localhost:5173**

> กด `q` ใน terminal เพื่อหยุด server

---

## Build (Production)

```bash
# สร้าง static files ไว้ใน dist/
npm run build

# Preview build ที่ได้
npm run preview
```

---

## โครงสร้างไฟล์

```
presentation/
├── index.html
├── package.json
├── vite.config.js
└── src/
    ├── main.js               # entry point — mount Vue app
    ├── App.vue               # shell: AppNav + RouterView + NavArrows
    ├── style.css             # design tokens + global classes
    ├── router/
    │   └── index.js          # SLIDES array + Vue Router config
    ├── components/
    │   ├── AppNav.vue        # fixed top navigation bar
    │   ├── NavArrows.vue     # fixed bottom prev/next arrows + progress bar
    │   ├── arch/
    │   │   └── ArchDiagram.vue       # SVG system architecture
    │   ├── config/
    │   │   └── OledMock.vue          # SVG OLED display mock
    │   ├── dashboard/
    │   │   └── BangkokMap.vue        # SVG Bangkok map + air box pins
    │   ├── dataflow/
    │   │   └── DataFlowDiagram.vue   # SVG data flow diagram
    │   └── hw/
    │       └── HwDiagram.vue         # SVG hardware wiring diagram
    └── views/
        ├── ViewHero.vue          # T1 — Intro / landing
        ├── ViewHardware.vue      # T2 — Hardware & wiring
        ├── ViewArchitecture.vue  # T3 — System architecture
        ├── ViewConfigMode.vue    # T4 — Config mode flow
        ├── ViewDataFlow.vue      # T5 — Data flow & storage
        ├── ViewDashboard.vue     # T6 — Dashboard preview mock
        └── ViewLiveSim.vue       # T7 — Live PM2.5 simulation
```

---

## Dependencies

| Package | ใช้ทำอะไร |
|---------|-----------|
| `vue` | UI framework |
| `vue-router` | routing แบบ SPA (hash mode) |
| `chart.js` | กราฟ PM2.5 real-time ใน Live Simulation |
| `@fortawesome/fontawesome-free` | icons ทั่ว app |

---

## Navigation

- **Top nav** — คลิก icon/label เพื่อข้ามไปหน้าใดก็ได้
- **Bottom arrows** — ← / → เพื่อไป section ถัดไป/ก่อนหน้า
- **Progress bar** — แถบสีฟ้าด้านบน arrows แสดง % ที่ผ่านไปแล้ว

---

## Sections

| # | Route | เนื้อหา |
|---|-------|---------|
| 1 | `/hero` | แนะนำระบบ, protocol ที่ใช้, specs |
| 2 | `/hardware` | วงจรเชื่อมต่อ sensor → ESP32 (SVG) |
| 3 | `/architecture` | ESP32 ↔ Flask ↔ Browser (SVG) |
| 4 | `/config` | ขั้นตอน Config Mode + captive portal |
| 5 | `/dataflow` | การไหลของข้อมูล + JSON schema (SVG) |
| 6 | `/dashboard` | ตัวอย่าง Dashboard จริง + Bangkok map |
| 7 | `/simulation` | Chart.js live simulation + alert engine |
