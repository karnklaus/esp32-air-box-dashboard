<script setup>
import ArchDiagram from '../components/arch/ArchDiagram.vue'
</script>

<template>
  <section class="section arch-page">
    <div class="container">
      <p class="section-label">Architecture</p>
      <h2 class="section-title">System Overview</h2>
      <p class="section-desc">
        ESP32 รายงานข้อมูลไปยัง Flask Server ทุก 3 วินาที ผ่าน HTTP POST —
        Browser ดึงข้อมูลล่าสุดจาก REST API แบบ polling
      </p>

      <div class="diagram-wrap">
        <ArchDiagram />
      </div>

      <div class="arch-legend">
        <span class="pill pill-orange">
          <i class="fa-solid fa-arrow-right"></i> POST — ESP32 → Server
        </span>
        <span class="pill pill-teal">
          <i class="fa-solid fa-arrow-right"></i> GET — Server → Browser
        </span>
        <span class="legend-note">
          <i class="fa-solid fa-circle-info"></i>
          No WebSocket — Browser polls <code>/api/air-boxes</code> ทุก 10s
        </span>
      </div>
    </div>
  </section>
</template>

<style scoped>
.arch-page {
  background: radial-gradient(ellipse 70% 50% at 50% 20%, #22c55e06 0%, transparent 70%), var(--bg);
}
.diagram-wrap {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 24px 16px;
  overflow-x: auto;
  margin-bottom: 24px;
}
.arch-legend {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 12px;
}
.legend-note {
  display: flex;
  align-items: center;
  gap: 7px;
  font-size: 0.82rem;
  color: var(--muted);
}
.legend-note i { color: var(--teal); }
.legend-note code {
  font-family: 'JetBrains Mono', monospace;
  font-size: 0.78rem;
  color: var(--teal);
  background: var(--teal-dim);
  padding: 1px 6px;
  border-radius: 4px;
}
</style>
