<script setup>
import DataFlowDiagram from '../components/dataflow/DataFlowDiagram.vue'
</script>

<template>
  <section class="section df-page">
    <div class="container">
      <p class="section-label">Data Flow</p>
      <h2 class="section-title">Data Lifecycle</h2>
      <p class="section-desc">
        ติดตามข้อมูลตั้งแต่ถูกบันทึกใน Flash ของ ESP32 ผ่าน HTTP POST
        ไปจนถึงการเก็บบน Server และส่งต่อให้ Browser กับ Discord
      </p>

      <div class="diagram-wrap">
        <DataFlowDiagram />
      </div>

      <div class="df-legend">
        <span class="pill pill-teal">
          <i class="fa-solid fa-database"></i> Flash — ไม่ส่งออก
        </span>
        <span class="pill pill-orange">
          <i class="fa-solid fa-arrow-right"></i> POST payload — 7 fields
        </span>
        <span class="pill" style="background:#22c55e15; color:#22c55e; border:1px solid #22c55e30">
          <i class="fa-solid fa-server"></i> Server storage — snapshot + history
        </span>
        <span class="pill pill-red">
          <i class="fa-brands fa-discord"></i> Alert — edge-triggered + cooldown
        </span>
        <div class="df-note">
          <i class="fa-solid fa-circle-info" style="color:var(--teal)"></i>
          <span>
            metadata (name, district, lat/lng) ถูก lock หลัง POST แรก
            — แก้ไขได้ผ่าน <code>PATCH /api/air-boxes/:id</code>
          </span>
        </div>
      </div>
    </div>
  </section>
</template>

<style scoped>
.df-page {
  background: radial-gradient(ellipse 70% 40% at 50% 20%, #22c55e06 0%, transparent 70%), var(--bg);
}
.diagram-wrap {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 24px 16px;
  overflow-x: auto;
  margin-bottom: 24px;
}
.df-legend {
  display: flex;
  flex-wrap: wrap;
  align-items: center;
  gap: 10px;
}
.df-note {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 0.82rem;
  color: var(--muted);
  width: 100%;
  margin-top: 4px;
}
.df-note code {
  font-family: 'JetBrains Mono', monospace;
  font-size: 0.78rem;
  color: var(--teal);
  background: var(--teal-dim);
  padding: 1px 6px;
  border-radius: 4px;
}
</style>
