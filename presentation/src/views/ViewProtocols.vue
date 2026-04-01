<script setup>
import ProtoI2C  from '../components/protocol/ProtoI2C.vue'
import ProtoUART from '../components/protocol/ProtoUART.vue'
import ProtoI2S  from '../components/protocol/ProtoI2S.vue'
import ProtoGPIO from '../components/protocol/ProtoGPIO.vue'

const tabs = [
  { id: 'i2c',  label: 'I2C',  icon: 'fa-solid fa-circle-nodes',  color: '#06b6d4' },
  { id: 'uart', label: 'UART', icon: 'fa-solid fa-arrows-left-right', color: '#f97316' },
  { id: 'i2s',  label: 'I2S',  icon: 'fa-solid fa-wave-square',   color: '#a855f7' },
  { id: 'gpio', label: 'GPIO', icon: 'fa-solid fa-toggle-on',     color: '#ef4444' },
]

import { ref } from 'vue'
const active = ref('i2c')
</script>

<template>
  <section class="section proto-page">
    <div class="container">
      <p class="section-label">Protocols</p>
      <h2 class="section-title">Communication Protocols</h2>
      <p class="section-desc">
        ทุก sensor ใช้ protocol ที่ต่างกัน — แสดง frame-level จาก code จริงใน
        <code>testing_function.ino</code>
      </p>

      <!-- Tab bar -->
      <div class="tab-bar">
        <button
          v-for="t in tabs" :key="t.id"
          class="tab-btn"
          :class="{ active: active === t.id }"
          :style="active === t.id ? `--tc:${t.color}` : ''"
          @click="active = t.id"
        >
          <i :class="t.icon" :style="active === t.id ? `color:${t.color}` : ''"></i>
          {{ t.label }}
        </button>
      </div>

      <!-- Tab content -->
      <div class="tab-content">
        <Transition name="tab-fade" mode="out-in">
          <div :key="active">
            <ProtoI2C  v-if="active === 'i2c'"  />
            <ProtoUART v-if="active === 'uart'" />
            <ProtoI2S  v-if="active === 'i2s'"  />
            <ProtoGPIO v-if="active === 'gpio'" />
          </div>
        </Transition>
      </div>

    </div>
  </section>
</template>

<style scoped>
.proto-page {
  background: radial-gradient(ellipse 70% 35% at 50% 15%, #06b6d405 0%, transparent 70%), var(--bg);
}

.tab-bar {
  display: flex;
  gap: 6px;
  margin-bottom: 28px;
  flex-wrap: wrap;
}
.tab-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  padding: 9px 22px;
  border-radius: 8px;
  border: 1px solid var(--border);
  background: var(--surface);
  color: var(--muted);
  font-size: 0.85rem;
  font-weight: 700;
  cursor: pointer;
  transition: color 0.15s, background 0.15s, border-color 0.15s;
}
.tab-btn:hover { color: var(--dim); }
.tab-btn.active {
  background: color-mix(in srgb, var(--tc) 12%, var(--surface));
  border-color: color-mix(in srgb, var(--tc) 40%, transparent);
  color: var(--text);
}

.tab-content {
  min-height: 300px;
}

.tab-fade-enter-active,
.tab-fade-leave-active { transition: opacity 0.15s, transform 0.15s; }
.tab-fade-enter-from   { opacity: 0; transform: translateY(4px); }
.tab-fade-leave-to     { opacity: 0; transform: translateY(-4px); }

code {
  font-family: 'JetBrains Mono', monospace;
  font-size: 0.82rem;
  color: var(--teal);
  background: #06b6d410;
  padding: 1px 6px;
  border-radius: 3px;
}
</style>
