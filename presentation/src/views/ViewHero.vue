<script setup>
const stats = [
  {
    icon: 'fa-solid fa-microchip',
    value: '1',
    label: 'Microcontroller',
    color: '#06b6d4',
    items: ['ESP32 (Xtensa LX6)', 'Wi-Fi 802.11 b/g/n', 'Flash storage'],
  },
  {
    icon: 'fa-solid fa-flask',
    value: '4',
    label: 'Sensors / Modules',
    color: '#a855f7',
    items: ['PMS5003 — PM1.0/2.5/10', 'INMP441 — Sound', 'AM2320 — Temp/Hum', 'OLED NFP1315-157B'],
  },
  {
    icon: 'fa-solid fa-tower-broadcast',
    value: '3',
    label: 'Protocols',
    color: '#f97316',
    items: ['UART — PMS5003', 'I2S — INMP441', 'I2C — OLED + AM2320'],
  },
]
</script>

<template>
  <section class="hero-page">
    <div class="container">
      <p class="section-label">Bangkok Air Grid</p>

      <h1 class="hero-title">
        ESP32<br>
        <span class="hero-accent">Air Box</span>
      </h1>

      <p class="hero-desc">
        ระบบวัดคุณภาพอากาศแบบ real-time ด้วย ESP32 — อ่านค่า PM, เสียง, อุณหภูมิ
        และส่งข้อมูลไปยัง Flask Server ผ่าน Wi-Fi ทุก 3 วินาที
      </p>

      <div class="hero-pills">
        <span class="pill pill-orange">UART</span>
        <span class="pill pill-purple">I2S</span>
        <span class="pill pill-teal">I2C</span>
        <span class="pill pill-teal">HTTP POST</span>
      </div>

      <div class="hero-stats">
        <div class="stat-card" v-for="stat in stats" :key="stat.label">
          <i :class="[stat.icon, 'stat-icon']" :style="{ color: stat.color }"></i>
          <div class="stat-value">{{ stat.value }}</div>
          <div class="stat-label">{{ stat.label }}</div>
          <ul class="stat-items">
            <li v-for="item in stat.items" :key="item">{{ item }}</li>
          </ul>
        </div>
      </div>
    </div>
  </section>
</template>

<style scoped>
.hero-page {
  min-height: calc(100dvh - var(--nav-h) - var(--arrows-h, 52px));
  display: flex;
  align-items: center;
  padding-block: 60px;
  position: relative;
  overflow: hidden;
}
.hero-page::before {
  content: '';
  position: absolute;
  inset: 0;
  background-image:
    linear-gradient(var(--border) 1px, transparent 1px),
    linear-gradient(90deg, var(--border) 1px, transparent 1px);
  background-size: 48px 48px;
  opacity: 0.3;
  pointer-events: none;
}
.hero-page::after {
  content: '';
  position: absolute;
  top: 20%; left: 50%;
  transform: translateX(-50%);
  width: 600px; height: 400px;
  background: radial-gradient(ellipse at center, #06b6d415 0%, transparent 70%);
  pointer-events: none;
}
.container { position: relative; z-index: 1; }

.hero-title {
  font-size: clamp(3rem, 8vw, 6rem);
  font-weight: 800;
  line-height: 1.05;
  letter-spacing: -0.03em;
  margin-bottom: 24px;
  color: var(--text);
}
.hero-accent { color: var(--teal); }

.hero-desc {
  font-size: 1.1rem;
  color: var(--dim);
  max-width: 560px;
  margin-bottom: 28px;
  line-height: 1.7;
}
.hero-pills {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
  margin-bottom: 48px;
}

.hero-stats {
  display: grid;
  grid-template-columns: repeat(3, 1fr);
  gap: 16px;
  max-width: 860px;
}
.stat-card {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 24px;
  transition: border-color 0.2s;
}
.stat-card:hover { border-color: var(--teal); }
.stat-icon { font-size: 1.3rem; margin-bottom: 12px; display: block; }
.stat-value {
  font-size: 2.4rem;
  font-weight: 800;
  line-height: 1;
  color: var(--text);
  margin-bottom: 4px;
}
.stat-label {
  font-size: 0.76rem;
  font-weight: 600;
  text-transform: uppercase;
  letter-spacing: 0.1em;
  color: var(--muted);
  margin-bottom: 14px;
}
.stat-items { list-style: none; display: flex; flex-direction: column; gap: 5px; }
.stat-items li {
  font-size: 0.82rem;
  color: var(--dim);
  font-family: 'JetBrains Mono', monospace;
  padding-left: 12px;
  position: relative;
}
.stat-items li::before { content: '·'; position: absolute; left: 0; color: var(--teal); }

@media (max-width: 680px) {
  .hero-stats { grid-template-columns: 1fr; }
}
</style>
