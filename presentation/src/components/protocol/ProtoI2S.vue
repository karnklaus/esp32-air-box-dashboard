<template>
  <div class="proto-block">
    <h3 class="proto-title">
      <span class="proto-badge" style="background:#a855f720;color:#a855f7;border-color:#a855f740">I2S</span>
      16 kHz · 32-bit · BCLK=GPIO27 · WS=GPIO14 · SD=GPIO26
    </h3>
    <p class="proto-note">INMP441 digital MEMS mic ส่งข้อมูลแบบ I2S standard — ESP32 เป็น Master RX, ใช้ DMA buffer 4×128 samples</p>

    <!-- I2S Frame timing diagram (SVG) -->
    <p class="frame-label">I2S Frame — 1 stereo frame = 64 BCLK cycles (32 left + 32 right)</p>
    <div class="svg-wrap">
      <svg viewBox="0 0 760 210" xmlns="http://www.w3.org/2000/svg" style="width:100%;height:auto">
        <!-- grid lines -->
        <line v-for="x in gridLines" :key="x" :x1="x" y1="10" :x2="x" y2="200" stroke="#1e2d3d" stroke-width="0.5"/>

        <!-- ── WS signal ── -->
        <text x="4" y="42" font-family="'JetBrains Mono',monospace" font-size="11" font-weight="700" fill="#94a3b8">WS</text>
        <polyline points="60,25 60,45 380,45 380,25 740,25" fill="none" stroke="#06b6d4" stroke-width="2"/>
        <text x="80" y="22" font-family="monospace" font-size="9" fill="#06b6d4">LEFT channel (WS=LOW)</text>
        <text x="430" y="22" font-family="monospace" font-size="9" fill="#64748b">RIGHT channel (WS=HIGH) — unused</text>

        <!-- ── BCLK signal ── -->
        <text x="4" y="90" font-family="'JetBrains Mono',monospace" font-size="11" font-weight="700" fill="#94a3b8">BCLK</text>
        <polyline :points="bclkPoints" fill="none" stroke="#a855f7" stroke-width="1.5"/>

        <!-- ── SD data signal ── -->
        <text x="4" y="148" font-family="'JetBrains Mono',monospace" font-size="11" font-weight="700" fill="#94a3b8">SD</text>
        <!-- MSB slot -->
        <rect x="61" y="127" width="38" height="28" fill="#a855f720" stroke="#a855f760" stroke-width="1" rx="2"/>
        <text x="80" y="143" text-anchor="middle" font-family="'JetBrains Mono',monospace" font-size="9" font-weight="600" fill="#a855f7">MSB</text>
        <text x="80" y="153" text-anchor="middle" font-family="monospace" font-size="8" fill="#64748b">bit31</text>
        <!-- middle bits -->
        <rect x="99" y="127" width="76" height="28" fill="#64748b20" stroke="#64748b40" stroke-width="1" rx="2"/>
        <text x="137" y="143" text-anchor="middle" font-family="'JetBrains Mono',monospace" font-size="9" font-weight="600" fill="#64748b">bit30 … bit09</text>
        <!-- LSB -->
        <rect x="175" y="127" width="38" height="28" fill="#a855f720" stroke="#a855f760" stroke-width="1" rx="2"/>
        <text x="194" y="143" text-anchor="middle" font-family="'JetBrains Mono',monospace" font-size="9" font-weight="600" fill="#a855f7">LSB</text>
        <text x="194" y="153" text-anchor="middle" font-family="monospace" font-size="8" fill="#64748b">bit08</text>
        <!-- padding 8 bits -->
        <rect x="213" y="127" width="76" height="28" fill="#ef444415" stroke="#ef444430" stroke-width="1" rx="2" stroke-dasharray="3,2"/>
        <text x="251" y="139" text-anchor="middle" font-family="'JetBrains Mono',monospace" font-size="9" font-weight="600" fill="#ef4444">padding</text>
        <text x="251" y="150" text-anchor="middle" font-family="monospace" font-size="8" fill="#64748b">bit07..00 = 0</text>
        <!-- right channel (unused) -->
        <rect x="381" y="127" width="172" height="28" fill="#33415520" stroke="#47556940" stroke-width="1" rx="2" stroke-dasharray="4,2"/>
        <text x="467" y="143" text-anchor="middle" font-family="'JetBrains Mono',monospace" font-size="9" font-weight="600" fill="#475569">RIGHT channel — not used</text>
        <text x="467" y="153" text-anchor="middle" font-family="monospace" font-size="8" fill="#475569">(L/R pin = GND → LEFT only)</text>

        <!-- ── Annotations ── -->
        <line x1="61" y1="175" x2="379" y2="175" stroke="#06b6d4" stroke-width="1" stroke-dasharray="3,2"/>
        <text x="220" y="172" text-anchor="middle" font-family="monospace" font-size="9" fill="#06b6d4">32 BCLK cycles = 1 left-channel sample</text>
        <line x1="381" y1="175" x2="740" y2="175" stroke="#475569" stroke-width="1" stroke-dasharray="3,2"/>
        <text x="560" y="172" text-anchor="middle" font-family="monospace" font-size="9" fill="#475569">32 BCLK cycles = right (ignored)</text>

        <!-- shift annotation -->
        <text x="80" y="192" text-anchor="middle" font-family="monospace" font-size="9" font-weight="600" fill="#f97316">code: sample = raw32 &gt;&gt; 8  (24-bit effective)</text>

        <!-- sample rate annotation -->
        <text x="740" y="195" text-anchor="end" font-family="monospace" font-size="9" fill="#64748b">1 frame = 1/16000 s ≈ 62.5 µs</text>
        <text x="740" y="207" text-anchor="end" font-family="monospace" font-size="9" fill="#64748b">1 BCLK = 62.5/64 ≈ 0.98 µs</text>
      </svg>
    </div>

    <!-- Config table -->
    <div class="cfg-grid">
      <div class="cfg-row header"><span>Parameter</span><span>Value</span><span>Code constant</span></div>
      <div class="cfg-row"><span>Sample rate</span><span class="v-teal">16,000 Hz</span><span class="mono">MIC_SAMPLE_RATE</span></div>
      <div class="cfg-row"><span>Bit depth</span><span class="v-teal">32 bits/sample</span><span class="mono">I2S_BITS_PER_SAMPLE_32BIT</span></div>
      <div class="cfg-row"><span>Effective resolution</span><span class="v-teal">24 bits</span><span class="mono">micSamples[i] >> 8</span></div>
      <div class="cfg-row"><span>Channel</span><span class="v-teal">Left only</span><span class="mono">I2S_CHANNEL_FMT_ONLY_LEFT</span></div>
      <div class="cfg-row"><span>DMA buffers</span><span class="v-teal">4 × 128 samples</span><span class="mono">dma_buf_count=4, dma_buf_len=128</span></div>
      <div class="cfg-row"><span>Block read</span><span class="v-teal">256 samples</span><span class="mono">MIC_BLOCK_SAMPLES</span></div>
      <div class="cfg-row"><span>Loud threshold</span><span class="v-teal">peak > 100,000</span><span class="mono">SOUND_THRESHOLD</span></div>
      <div class="cfg-row"><span>Envelope decay</span><span class="v-teal">fast: ×0.60 slow: ×0.90</span><span class="mono">micEnvelope blend</span></div>
    </div>
  </div>
</template>

<script setup>
import { computed } from 'vue'

/* Generate BCLK pulses for 32 cycles (abbreviated to 16 visible, scaled) */
const bclkPoints = computed(() => {
  const startX = 60
  const endX = 380
  const totalCycles = 16   // show 16 pulses representing 32
  const cycleW = (endX - startX) / totalCycles
  const high = 70, low = 90
  let pts = [`${startX},${high}`]
  for (let i = 0; i < totalCycles; i++) {
    const x0 = startX + i * cycleW
    const x1 = x0 + cycleW / 2
    const x2 = x0 + cycleW
    pts.push(`${x0},${low}`)
    pts.push(`${x1},${low}`)
    pts.push(`${x1},${high}`)
    pts.push(`${x2},${high}`)
  }
  // right channel (same pattern but muted)
  const rStart = 381, rEnd = 740
  const rCycleW = (rEnd - rStart) / totalCycles
  for (let i = 0; i < totalCycles; i++) {
    const x0 = rStart + i * rCycleW
    const x1 = x0 + rCycleW / 2
    const x2 = x0 + rCycleW
    pts.push(`${x0},${low}`)
    pts.push(`${x1},${low}`)
    pts.push(`${x1},${high}`)
    pts.push(`${x2},${high}`)
  }
  return pts.join(' ')
})

const gridLines = [60, 100, 140, 180, 220, 260, 300, 340, 380, 420, 460, 500, 540, 580, 620, 660, 700, 740]
</script>

<style scoped>
.proto-block { margin-bottom: 40px; }
.proto-title {
  font-size: 0.95rem; font-weight: 700; color: var(--text);
  display: flex; align-items: center; gap: 10px; margin-bottom: 6px;
}
.proto-badge {
  font-size: 0.72rem; font-weight: 800; padding: 2px 10px;
  border-radius: 20px; border: 1px solid; letter-spacing: 0.08em;
}
.proto-note { font-size: 0.8rem; color: var(--muted); margin-bottom: 20px; }
.frame-label { font-size: 0.75rem; color: var(--dim); margin-bottom: 6px; font-weight: 600; }
.svg-wrap {
  background: var(--surface); border: 1px solid var(--border);
  border-radius: 8px; padding: 12px; margin-bottom: 20px;
  overflow-x: auto;
}

/* Config table */
.cfg-grid {
  border: 1px solid var(--border); border-radius: 8px; overflow: hidden;
  font-size: 0.78rem; margin-bottom: 16px;
}
.cfg-row {
  display: grid; grid-template-columns: 1.5fr 1.5fr 2fr;
  border-bottom: 1px solid var(--border); align-items: center;
}
.cfg-row:last-child { border-bottom: none; }
.cfg-row.header {
  background: var(--bg); font-size: 0.65rem; font-weight: 700;
  text-transform: uppercase; letter-spacing: 0.08em; color: var(--muted);
}
.cfg-row > span { padding: 7px 12px; border-right: 1px solid var(--border); }
.cfg-row > span:last-child { border-right: none; }
.v-teal { color: var(--teal); font-weight: 700; font-family: monospace; }
.mono { font-family: 'JetBrains Mono', monospace; color: var(--dim); font-size: 0.72rem; }
</style>
