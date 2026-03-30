<script setup>
import BangkokMap from '../components/dashboard/BangkokMap.vue'

const boxes = [
  {
    id: 'ABX-001',
    name: 'Silom Office',
    lat: 13.726, lng: 100.527,
    online: true,
    pm1: 11, pm25: 18, pm10: 22,
    temp: 29.4, humidity: 71,
    db: 38, loud: false,
  },
  {
    id: 'ABX-002',
    name: 'Victory Monument',
    lat: 13.765, lng: 100.537,
    online: true,
    pm1: 16, pm25: 24, pm10: 31,
    temp: 31.1, humidity: 66,
    db: 62, loud: true,
  },
  {
    id: 'ABX-003',
    name: 'Chatuchak Park',
    lat: 13.800, lng: 100.555,
    online: false,
    pm1: 5, pm25: 8, pm10: 11,
    temp: 28.7, humidity: 74,
    db: 41, loud: false,
  },
]

function pmColor(v) {
  if (v <= 12) return '#22c55e'
  if (v <= 35) return '#f97316'
  return '#ef4444'
}
function pmLabel(v) {
  if (v <= 12) return 'Good'
  if (v <= 35) return 'Moderate'
  return 'Unhealthy'
}
</script>

<template>
  <section class="section db-page">
    <div class="container">
      <p class="section-label">Dashboard</p>
      <h2 class="section-title">Live Air Quality Dashboard</h2>
      <p class="section-desc">
        Browser dashboard ดึงข้อมูลจาก Flask API ทุก 5 วินาที แสดงสถานะ Air Box
        แบบ real-time พร้อมแผนที่ Bangkok และ PM2.5 color coding
      </p>

      <!-- Browser chrome mock -->
      <div class="browser-mock">
        <!-- title bar -->
        <div class="browser-bar">
          <span class="dot dot-red"></span>
          <span class="dot dot-yellow"></span>
          <span class="dot dot-green"></span>
          <div class="browser-url">
            <i class="fa-solid fa-lock" style="font-size:0.65rem; color:#22c55e"></i>
            <span>192.168.1.100:5000</span>
          </div>
          <span class="browser-tag">Air Box Dashboard</span>
        </div>

        <!-- dashboard content inside browser -->
        <div class="dash-body">

          <!-- top header bar -->
          <div class="dash-header">
            <div class="dash-logo">
              <i class="fa-solid fa-wind" style="color:var(--teal)"></i>
              <span>Bangkok Air Monitor</span>
            </div>
            <div class="dash-status">
              <span class="status-pill">
                <span class="pulse"></span> Live · updating every 5s
              </span>
              <span class="dash-count">{{ boxes.filter(b => b.online).length }}/{{ boxes.length }} online</span>
            </div>
          </div>

          <!-- map + cards -->
          <div class="dash-grid">

            <!-- map panel -->
            <div class="map-panel">
              <p class="panel-label"><i class="fa-solid fa-map-location-dot"></i> Bangkok</p>
              <BangkokMap :boxes="boxes" />
              <!-- legend -->
              <div class="map-legend">
                <span class="leg-item"><span class="leg-dot" style="background:#22c55e"></span> Good (≤12)</span>
                <span class="leg-item"><span class="leg-dot" style="background:#f97316"></span> Moderate (≤35)</span>
                <span class="leg-item"><span class="leg-dot" style="background:#ef4444"></span> Unhealthy (>35)</span>
                <span class="leg-item"><span class="leg-dot" style="background:#475569"></span> Offline</span>
              </div>
            </div>

            <!-- cards panel -->
            <div class="cards-panel">
              <p class="panel-label"><i class="fa-solid fa-list"></i> Air Boxes</p>
              <div class="box-card" v-for="box in boxes" :key="box.id"
                   :class="{ offline: !box.online }">

                <!-- card header -->
                <div class="card-head">
                  <div>
                    <span class="card-name">{{ box.name }}</span>
                    <span class="card-id">{{ box.id }}</span>
                  </div>
                  <span class="online-badge" :class="box.online ? 'on' : 'off'">
                    <i :class="box.online ? 'fa-solid fa-circle-check' : 'fa-solid fa-circle-xmark'"></i>
                    {{ box.online ? 'Online' : 'Offline' }}
                  </span>
                </div>

                <!-- PM chips row -->
                <div class="pm-row">
                  <div class="pm-chip" v-for="(val, key) in { 'PM1.0': box.pm1, 'PM2.5': box.pm25, 'PM10': box.pm10 }" :key="key">
                    <span class="pm-label">{{ key }}</span>
                    <span class="pm-val" :style="{ color: pmColor(val) }">{{ val }}</span>
                    <span class="pm-unit">µg/m³</span>
                  </div>
                  <div class="pm-status-chip" :style="{ background: pmColor(box.pm25) + '20', color: pmColor(box.pm25), borderColor: pmColor(box.pm25) + '50' }">
                    {{ pmLabel(box.pm25) }}
                  </div>
                </div>

                <!-- climate + sound row -->
                <div class="meta-row">
                  <span class="meta-item">
                    <i class="fa-solid fa-temperature-half" style="color:#f97316"></i>
                    {{ box.temp }}°C
                  </span>
                  <span class="meta-item">
                    <i class="fa-solid fa-droplet" style="color:#06b6d4"></i>
                    {{ box.humidity }}%
                  </span>
                  <span class="sound-pill" :class="box.loud ? 'loud' : 'quiet'">
                    <i :class="box.loud ? 'fa-solid fa-volume-high' : 'fa-solid fa-volume-low'"></i>
                    {{ box.loud ? 'LOUD' : 'QUIET' }}
                    <span class="db-val">{{ box.db }} dB</span>
                  </span>
                </div>

              </div>
            </div>

          </div>
        </div>
      </div>

    </div>
  </section>
</template>

<style scoped>
.db-page {
  background: radial-gradient(ellipse 70% 40% at 50% 20%, #06b6d406 0%, transparent 70%), var(--bg);
}

/* ─── Browser mock ───────────────────────────────── */
.browser-mock {
  border: 1px solid var(--border);
  border-radius: 10px;
  overflow: hidden;
  background: var(--surface);
  box-shadow: 0 8px 40px #00000060;
}
.browser-bar {
  background: #141c28;
  border-bottom: 1px solid var(--border);
  padding: 8px 14px;
  display: flex;
  align-items: center;
  gap: 10px;
}
.dot {
  width: 11px; height: 11px;
  border-radius: 50%;
  flex-shrink: 0;
}
.dot-red    { background: #ef4444; }
.dot-yellow { background: #eab308; }
.dot-green  { background: #22c55e; }
.browser-url {
  flex: 1;
  background: var(--bg);
  border: 1px solid var(--border);
  border-radius: 5px;
  padding: 4px 10px;
  font-size: 0.75rem;
  font-family: 'JetBrains Mono', monospace;
  color: var(--dim);
  display: flex;
  align-items: center;
  gap: 6px;
  max-width: 260px;
}
.browser-tag {
  font-size: 0.72rem;
  color: var(--muted);
  margin-left: auto;
  white-space: nowrap;
}

/* ─── Dashboard body ─────────────────────────────── */
.dash-body {
  padding: 16px;
}
.dash-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  margin-bottom: 16px;
  flex-wrap: wrap;
  gap: 8px;
}
.dash-logo {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 0.95rem;
  font-weight: 700;
  color: var(--text);
}
.dash-status {
  display: flex;
  align-items: center;
  gap: 12px;
}
.status-pill {
  display: flex;
  align-items: center;
  gap: 6px;
  font-size: 0.75rem;
  color: var(--teal);
  background: #06b6d410;
  border: 1px solid #06b6d430;
  padding: 3px 10px;
  border-radius: 20px;
}
.pulse {
  width: 7px; height: 7px;
  border-radius: 50%;
  background: var(--teal);
  animation: pulse 1.5s infinite;
}
@keyframes pulse {
  0%, 100% { opacity: 1; }
  50%       { opacity: 0.3; }
}
.dash-count {
  font-size: 0.75rem;
  color: var(--muted);
}

/* ─── Grid ───────────────────────────────────────── */
.dash-grid {
  display: grid;
  grid-template-columns: auto 1fr;
  gap: 20px;
  align-items: start;
}

/* ─── Map panel ──────────────────────────────────── */
.panel-label {
  font-size: 0.72rem;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.1em;
  color: var(--muted);
  margin-bottom: 10px;
  display: flex;
  align-items: center;
  gap: 6px;
}
.map-legend {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin-top: 10px;
}
.leg-item {
  display: flex;
  align-items: center;
  gap: 5px;
  font-size: 0.68rem;
  color: var(--muted);
}
.leg-dot {
  width: 8px; height: 8px;
  border-radius: 50%;
  flex-shrink: 0;
}

/* ─── Cards panel ────────────────────────────────── */
.cards-panel {
  display: flex;
  flex-direction: column;
}
.box-card {
  background: var(--bg);
  border: 1px solid var(--border);
  border-radius: 8px;
  padding: 12px 14px;
  margin-bottom: 10px;
  transition: opacity 0.2s;
}
.box-card.offline {
  opacity: 0.55;
}
.card-head {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  margin-bottom: 10px;
  gap: 8px;
}
.card-name {
  display: block;
  font-size: 0.9rem;
  font-weight: 700;
  color: var(--text);
}
.card-id {
  display: block;
  font-size: 0.7rem;
  color: var(--muted);
  font-family: 'JetBrains Mono', monospace;
}
.online-badge {
  font-size: 0.7rem;
  font-weight: 600;
  padding: 2px 8px;
  border-radius: 20px;
  display: flex;
  align-items: center;
  gap: 4px;
  white-space: nowrap;
  flex-shrink: 0;
}
.online-badge.on { background: #22c55e15; color: #22c55e; border: 1px solid #22c55e30; }
.online-badge.off { background: #47556915; color: #475569; border: 1px solid #47556930; }

/* ─── PM row ─────────────────────────────────────── */
.pm-row {
  display: flex;
  align-items: center;
  gap: 10px;
  flex-wrap: wrap;
  margin-bottom: 8px;
}
.pm-chip {
  display: flex;
  flex-direction: column;
  align-items: center;
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: 6px;
  padding: 5px 10px;
  min-width: 54px;
}
.pm-label {
  font-size: 0.62rem;
  color: var(--muted);
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.05em;
}
.pm-val {
  font-size: 1rem;
  font-weight: 800;
  font-family: 'JetBrains Mono', monospace;
  line-height: 1.2;
}
.pm-unit {
  font-size: 0.58rem;
  color: var(--muted);
}
.pm-status-chip {
  font-size: 0.72rem;
  font-weight: 700;
  padding: 3px 10px;
  border-radius: 20px;
  border: 1px solid;
}

/* ─── Meta row ───────────────────────────────────── */
.meta-row {
  display: flex;
  align-items: center;
  gap: 12px;
  flex-wrap: wrap;
}
.meta-item {
  font-size: 0.78rem;
  color: var(--dim);
  display: flex;
  align-items: center;
  gap: 5px;
}
.sound-pill {
  font-size: 0.7rem;
  font-weight: 700;
  padding: 2px 8px;
  border-radius: 20px;
  display: flex;
  align-items: center;
  gap: 5px;
  margin-left: auto;
}
.sound-pill.loud  { background: #ef444415; color: #ef4444; border: 1px solid #ef444430; }
.sound-pill.quiet { background: #22c55e15; color: #22c55e; border: 1px solid #22c55e30; }
.db-val {
  font-weight: 400;
  opacity: 0.7;
}

@media (max-width: 700px) {
  .dash-grid { grid-template-columns: 1fr; }
}
</style>
