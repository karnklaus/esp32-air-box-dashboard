<script setup>
import HwDiagram from '../components/hw/HwDiagram.vue'
</script>

<template>
  <section class="section hw-page">
    <div class="container">
      <p class="section-label">Hardware</p>
      <h2 class="section-title">Pin Wiring Diagram</h2>
      <p class="section-desc">
        ESP32 ต่อสายกับ sensors ผ่าน 3 protocol: I²C · I²S · UART
        และรับสัญญาณปุ่มแดงผ่าน GPIO
      </p>

      <div class="diagram-wrap">
        <HwDiagram />
      </div>

      <div class="hw-legend">
        <span class="pill pill-teal"><i class="fa-solid fa-circle-nodes"></i> I²C — OLED + AM2320</span>
        <span class="pill pill-purple"><i class="fa-solid fa-wave-square"></i> I²S — INMP441</span>
        <span class="pill pill-orange"><i class="fa-solid fa-right-left"></i> UART — PMS5003</span>
        <span class="pill pill-red"><i class="fa-solid fa-circle-dot"></i> GPIO — Red Button</span>
        <span class="legend-power">
          <span class="dot" style="background:#22c55e"></span> 3.3V
          <span class="dot" style="background:#eab308; margin-left:10px"></span> 5V
          <span class="dot" style="background:#475569; margin-left:10px"></span> GND (dashed)
        </span>
      </div>
    </div>
  </section>
</template>

<style scoped>
.hw-page {
  background: radial-gradient(ellipse 60% 40% at 50% 30%, #a855f708 0%, transparent 70%), var(--bg);
}
.diagram-wrap {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 24px 16px;
  overflow-x: auto;
  margin-bottom: 24px;
}
.hw-legend {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 10px;
}
.legend-power {
  display: flex;
  align-items: center;
  gap: 4px;
  font-size: 0.78rem;
  color: var(--muted);
  margin-left: 6px;
}
.dot {
  display: inline-block;
  width: 9px; height: 9px;
  border-radius: 50%;
}
</style>
