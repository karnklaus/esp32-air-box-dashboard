<script setup>
import { ref, onMounted, onUnmounted, computed } from 'vue'
import {
  Chart, LineController, LineElement, PointElement, LinearScale,
  CategoryScale, Filler, Tooltip, Legend,
} from 'chart.js'

Chart.register(LineController, LineElement, PointElement, LinearScale,
               CategoryScale, Filler, Tooltip, Legend)

/* ─── Config ─────────────────────────────────────── */
const THRESHOLD = 35      // µg/m³ — AQI moderate/unhealthy boundary
const MAX_POINTS = 40
const TICK_MS = 800

const BOXES = [
  { id: 'ABX-001', label: 'Silom',    color: '#06b6d4', base: 18, noise: 8  },
  { id: 'ABX-002', label: 'Victory',  color: '#a855f7', base: 26, noise: 14 },
  { id: 'ABX-003', label: 'Chatuchak',color: '#22c55e', base: 9,  noise: 6  },
]

/* ─── State ──────────────────────────────────────── */
const canvasRef = ref(null)
const running   = ref(false)
const alerts    = ref([])       // { id, label, color, value, time }
const ticks     = ref(0)
const latestPm  = ref(BOXES.map(b => ({ ...b, current: b.base })))

let chart    = null
let timerId  = null

/* ─── Seeded smooth random walk ─────────────────── */
function nextPm(box) {
  const drift  = (Math.random() - 0.48) * 3
  const jitter = (Math.random() - 0.5) * box.noise
  const raw    = (latestPm.value.find(b => b.id === box.id)?.current ?? box.base) + drift + jitter
  return Math.max(2, Math.min(80, raw))
}

/* ─── Alert helpers ──────────────────────────────── */
function pushAlert(box, value) {
  alerts.value.unshift({
    id: box.id, label: box.label, color: box.color,
    value: Math.round(value),
    time: new Date().toLocaleTimeString('th-TH', { hour: '2-digit', minute: '2-digit', second: '2-digit' }),
  })
  if (alerts.value.length > 5) alerts.value.pop()
}

/* ─── Chart bootstrap ────────────────────────────── */
function buildChart() {
  const labels = Array.from({ length: MAX_POINTS }, () => '')
  chart = new Chart(canvasRef.value, {
    type: 'line',
    data: {
      labels,
      datasets: BOXES.map(box => ({
        label: box.label,
        data: Array(MAX_POINTS).fill(null),
        borderColor: box.color,
        backgroundColor: box.color + '18',
        borderWidth: 2,
        pointRadius: 0,
        pointHoverRadius: 4,
        tension: 0.4,
        fill: false,
      })),
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      animation: { duration: 200 },
      scales: {
        x: {
          grid: { color: '#1e2d3d' },
          ticks: { display: false },
        },
        y: {
          min: 0, max: 80,
          grid: { color: '#1e2d3d' },
          ticks: {
            color: '#64748b',
            font: { family: 'monospace', size: 10 },
            callback: v => v + ' µg',
          },
        },
      },
      plugins: {
        legend: {
          labels: {
            color: '#94a3b8',
            font: { family: 'monospace', size: 11 },
            boxWidth: 12,
            padding: 16,
          },
        },
        tooltip: {
          backgroundColor: '#0f1623',
          borderColor: '#1e2d3d',
          borderWidth: 1,
          titleColor: '#e2e8f0',
          bodyColor: '#94a3b8',
          callbacks: {
            label: ctx => ` ${ctx.dataset.label}: ${ctx.parsed.y?.toFixed(1)} µg/m³`,
          },
        },
      },
    },
    plugins: [{
      id: 'threshold-line',
      afterDraw(ch) {
        const { ctx, chartArea: { left, right }, scales: { y } } = ch
        const yPos = y.getPixelForValue(THRESHOLD)
        ctx.save()
        ctx.setLineDash([5, 4])
        ctx.strokeStyle = '#ef444470'
        ctx.lineWidth = 1.5
        ctx.beginPath()
        ctx.moveTo(left, yPos)
        ctx.lineTo(right, yPos)
        ctx.stroke()
        ctx.setLineDash([])
        ctx.fillStyle = '#ef4444aa'
        ctx.font = '9px monospace'
        ctx.fillText('Alert threshold 35 µg/m³', left + 6, yPos - 4)
        ctx.restore()
      },
    }],
  })
}

/* ─── Tick ───────────────────────────────────────── */
function tick() {
  ticks.value++
  const label = `+${ticks.value * (TICK_MS / 1000)}s`

  chart.data.labels.shift()
  chart.data.labels.push(label)

  BOXES.forEach((box, i) => {
    const val = nextPm(box)
    latestPm.value[i].current = val
    chart.data.datasets[i].data.shift()
    chart.data.datasets[i].data.push(val)
    // edge-triggered alert: only on crossing upward
    const prev = chart.data.datasets[i].data.at(-2)
    if (prev !== null && prev <= THRESHOLD && val > THRESHOLD) {
      pushAlert(box, val)
    }
  })
  chart.update()
}

/* ─── Controls ───────────────────────────────────── */
function start() {
  if (running.value) return
  running.value = true
  timerId = setInterval(tick, TICK_MS)
}
function stop() {
  running.value = false
  clearInterval(timerId)
}
function reset() {
  stop()
  ticks.value = 0
  alerts.value = []
  latestPm.value = BOXES.map(b => ({ ...b, current: b.base }))
  chart.data.labels = Array.from({ length: MAX_POINTS }, () => '')
  chart.data.datasets.forEach(ds => { ds.data = Array(MAX_POINTS).fill(null) })
  chart.update()
}

const pmBadgeColor = (v) => {
  if (v <= 12) return '#22c55e'
  if (v <= 35) return '#f97316'
  return '#ef4444'
}

/* ─── Lifecycle ──────────────────────────────────── */
onMounted(buildChart)
onUnmounted(() => { clearInterval(timerId); chart?.destroy() })
</script>

<template>
  <section class="section sim-page">
    <div class="container">
      <p class="section-label">Live Simulation</p>
      <h2 class="section-title">PM2.5 Live Simulation</h2>
      <p class="section-desc">
        จำลองการทำงานของ Dashboard แบบ real-time — ค่า PM2.5 เปลี่ยนแปลงทุก 0.8 วินาที
        พร้อม alert engine ที่ triggered เมื่อข้ามเกณฑ์ 35 µg/m³ (AQI Moderate → Unhealthy)
      </p>

      <!-- Controls -->
      <div class="controls">
        <button class="ctrl-btn btn-start" :disabled="running" @click="start">
          <i class="fa-solid fa-play"></i> Start
        </button>
        <button class="ctrl-btn btn-stop" :disabled="!running" @click="stop">
          <i class="fa-solid fa-pause"></i> Pause
        </button>
        <button class="ctrl-btn btn-reset" @click="reset">
          <i class="fa-solid fa-rotate-left"></i> Reset
        </button>
        <span class="tick-counter">
          <i class="fa-solid fa-clock" style="color:var(--muted)"></i>
          {{ (ticks * TICK_MS / 1000).toFixed(1) }}s elapsed
        </span>
        <span class="live-indicator" :class="{ active: running }">
          <span class="pulse-dot"></span>
          {{ running ? 'LIVE' : 'PAUSED' }}
        </span>
      </div>

      <!-- Chart + sidebar -->
      <div class="sim-grid">

        <!-- Chart -->
        <div class="chart-wrap card">
          <div class="chart-inner">
            <canvas ref="canvasRef"></canvas>
          </div>
        </div>

        <!-- Right sidebar -->
        <div class="sidebar">

          <!-- Live readings -->
          <div class="reading-block card">
            <p class="block-label"><i class="fa-solid fa-gauge-high"></i> Live PM2.5</p>
            <div class="reading-row" v-for="box in latestPm" :key="box.id">
              <span class="reading-dot" :style="{ background: box.color }"></span>
              <span class="reading-name">{{ box.label }}</span>
              <span class="reading-val" :style="{ color: pmBadgeColor(box.current) }">
                {{ Math.round(box.current) }}
              </span>
              <span class="reading-unit">µg/m³</span>
              <!-- mini bar -->
              <div class="mini-bar-track">
                <div class="mini-bar-fill"
                     :style="{ width: Math.min(100, box.current / 80 * 100) + '%', background: pmBadgeColor(box.current) }">
                </div>
              </div>
            </div>
          </div>

          <!-- Alert log -->
          <div class="alert-block card">
            <p class="block-label">
              <i class="fa-brands fa-discord" style="color:#5865f2"></i>
              Alert Log
              <span v-if="alerts.length" class="alert-count">{{ alerts.length }}</span>
            </p>
            <div v-if="!alerts.length" class="no-alerts">
              <i class="fa-solid fa-circle-check" style="color:#22c55e"></i>
              No alerts yet
            </div>
            <transition-group name="alert" tag="div">
              <div class="alert-entry" v-for="(al, i) in alerts" :key="al.time + al.id + i">
                <span class="alert-dot" :style="{ background: al.color }"></span>
                <div class="alert-body">
                  <span class="alert-label">{{ al.label }}</span>
                  <span class="alert-pm" style="color:#ef4444">{{ al.value }} µg/m³</span>
                </div>
                <span class="alert-time">{{ al.time }}</span>
              </div>
            </transition-group>
          </div>

        </div>
      </div>

      <!-- Feature notes -->
      <div class="feature-row">
        <div class="feat-card card">
          <i class="fa-solid fa-arrows-rotate" style="color:var(--teal)"></i>
          <div>
            <strong>5s Poll cycle</strong>
            <p>Browser ดึงข้อมูลจาก <code>GET /api/air-boxes</code> ทุก 5 วินาที</p>
          </div>
        </div>
        <div class="feat-card card">
          <i class="fa-solid fa-bolt" style="color:#eab308"></i>
          <div>
            <strong>Edge-triggered Alert</strong>
            <p>Discord webhook ส่งเมื่อ PM2.5 ข้ามเกณฑ์ขึ้น — ไม่ spam ขณะยังสูงอยู่</p>
          </div>
        </div>
        <div class="feat-card card">
          <i class="fa-solid fa-clock-rotate-left" style="color:var(--purple)"></i>
          <div>
            <strong>240-reading FIFO</strong>
            <p>Server เก็บประวัติ 240 จุดต่อ box → กราฟ trend ย้อนหลัง ~20 นาที</p>
          </div>
        </div>
      </div>

    </div>
  </section>
</template>

<style scoped>
.sim-page {
  background: radial-gradient(ellipse 70% 40% at 50% 20%, #a855f706 0%, transparent 70%), var(--bg);
}

/* ─── Controls ───────────────────────────────────── */
.controls {
  display: flex;
  align-items: center;
  gap: 10px;
  flex-wrap: wrap;
  margin-bottom: 20px;
}
.ctrl-btn {
  display: flex;
  align-items: center;
  gap: 7px;
  padding: 8px 18px;
  border-radius: 7px;
  border: none;
  font-size: 0.85rem;
  font-weight: 700;
  cursor: pointer;
  transition: opacity 0.15s, transform 0.1s;
}
.ctrl-btn:disabled { opacity: 0.35; cursor: default; }
.ctrl-btn:not(:disabled):active { transform: scale(0.97); }
.btn-start { background: var(--teal);   color: #000; }
.btn-stop  { background: #f97316;       color: #000; }
.btn-reset { background: var(--surface); color: var(--dim); border: 1px solid var(--border); }
.tick-counter {
  font-size: 0.78rem;
  color: var(--muted);
  font-family: 'JetBrains Mono', monospace;
  margin-left: 4px;
  display: flex;
  align-items: center;
  gap: 6px;
}
.live-indicator {
  margin-left: auto;
  font-size: 0.72rem;
  font-weight: 700;
  letter-spacing: 0.08em;
  color: #475569;
  display: flex;
  align-items: center;
  gap: 6px;
  border: 1px solid var(--border);
  padding: 3px 10px;
  border-radius: 20px;
}
.live-indicator.active { color: var(--teal); border-color: #06b6d430; background: #06b6d410; }
.pulse-dot {
  width: 7px; height: 7px;
  border-radius: 50%;
  background: currentColor;
}
.live-indicator.active .pulse-dot { animation: pulse 1.2s infinite; }
@keyframes pulse { 0%,100%{opacity:1} 50%{opacity:0.25} }

/* ─── Grid ───────────────────────────────────────── */
.sim-grid {
  display: grid;
  grid-template-columns: 1fr 260px;
  gap: 16px;
  align-items: start;
  margin-bottom: 24px;
}

/* ─── Chart ──────────────────────────────────────── */
.chart-wrap { padding: 16px; }
.chart-inner { height: 300px; position: relative; }

/* ─── Sidebar ────────────────────────────────────── */
.block-label {
  font-size: 0.72rem;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.1em;
  color: var(--muted);
  margin-bottom: 12px;
  display: flex;
  align-items: center;
  gap: 7px;
}
.alert-count {
  margin-left: auto;
  background: #ef444420;
  color: #ef4444;
  border: 1px solid #ef444440;
  border-radius: 20px;
  padding: 0 6px;
  font-size: 0.65rem;
}
.reading-block, .alert-block { padding: 14px; margin-bottom: 14px; }

/* Live readings */
.reading-row {
  display: grid;
  grid-template-columns: 8px 60px 36px 34px 1fr;
  align-items: center;
  gap: 6px;
  margin-bottom: 10px;
}
.reading-dot {
  width: 8px; height: 8px;
  border-radius: 50%;
  flex-shrink: 0;
}
.reading-name { font-size: 0.78rem; color: var(--dim); }
.reading-val {
  font-size: 0.95rem;
  font-weight: 800;
  font-family: 'JetBrains Mono', monospace;
  text-align: right;
}
.reading-unit { font-size: 0.6rem; color: var(--muted); }
.mini-bar-track {
  height: 4px;
  background: var(--bg);
  border-radius: 2px;
  overflow: hidden;
}
.mini-bar-fill {
  height: 100%;
  border-radius: 2px;
  transition: width 0.3s ease, background 0.3s ease;
}

/* Alert log */
.no-alerts {
  font-size: 0.78rem;
  color: var(--muted);
  display: flex;
  align-items: center;
  gap: 7px;
  padding: 8px 0;
}
.alert-entry {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 7px 0;
  border-bottom: 1px solid var(--border);
}
.alert-entry:last-child { border-bottom: none; }
.alert-dot {
  width: 8px; height: 8px;
  border-radius: 50%;
  flex-shrink: 0;
}
.alert-body {
  display: flex;
  flex-direction: column;
  gap: 1px;
  flex: 1;
}
.alert-label { font-size: 0.78rem; font-weight: 700; color: var(--text); }
.alert-pm    { font-size: 0.7rem; font-family: monospace; }
.alert-time  { font-size: 0.65rem; color: var(--muted); white-space: nowrap; font-family: monospace; }

/* alert transition */
.alert-enter-active { transition: all 0.25s ease; }
.alert-enter-from   { opacity: 0; transform: translateX(-8px); }

/* ─── Feature row ────────────────────────────────── */
.feature-row {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 14px;
}
.feat-card {
  display: flex;
  align-items: flex-start;
  gap: 12px;
  padding: 14px 16px;
  font-size: 1.1rem;
}
.feat-card strong { display: block; font-size: 0.85rem; color: var(--text); margin-bottom: 4px; }
.feat-card p { font-size: 0.78rem; color: var(--muted); line-height: 1.5; margin: 0; }
.feat-card code {
  font-family: 'JetBrains Mono', monospace;
  font-size: 0.73rem;
  color: var(--teal);
  background: var(--teal-dim);
  padding: 1px 5px;
  border-radius: 3px;
}

@media (max-width: 700px) {
  .sim-grid      { grid-template-columns: 1fr; }
  .feature-row   { grid-template-columns: 1fr; }
}
</style>
