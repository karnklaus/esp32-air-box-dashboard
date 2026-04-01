<template>
  <div class="proto-block">
    <h3 class="proto-title">
      <span class="proto-badge" style="background:#f9731620;color:#f97316;border-color:#f9731640">UART</span>
      9600 baud · 8N1 · GPIO16 (RX←PMS) / GPIO17 (TX→PMS)
    </h3>
    <p class="proto-note">PMS5003 ส่ง frame อัตโนมัติตาม duty cycle ของตัวเอง — ESP32 อ่านฝั่งเดียว (passive)</p>

    <!-- 8N1 single byte -->
    <p class="frame-label">UART 8N1 — โครงสร้างแต่ละ byte (1 start bit + 8 data bits + 1 stop bit)</p>
    <div class="frame-row">
      <div class="f-field idle" style="--fc:#475569;flex:2">
        <span class="f-name">IDLE</span>
        <span class="f-val">HIGH</span>
        <span class="f-sub">line at rest</span>
      </div>
      <div class="f-field" style="--fc:#22c55e">
        <span class="f-name">START</span>
        <span class="f-val">LOW</span>
        <span class="f-sub">1 bit</span>
      </div>
      <div class="f-field" style="--fc:#f97316;flex:2">
        <span class="f-name">D0 (LSB)</span>
        <span class="f-val">bit</span>
      </div>
      <div class="f-field" style="--fc:#f97316;flex:2">
        <span class="f-name">D1</span>
        <span class="f-val">bit</span>
      </div>
      <div class="f-field" style="--fc:#f97316;flex:2">
        <span class="f-name">D2</span>
        <span class="f-val">bit</span>
      </div>
      <div class="f-field" style="--fc:#f97316;flex:2">
        <span class="f-name">D3</span>
        <span class="f-val">bit</span>
      </div>
      <div class="f-field" style="--fc:#f97316;flex:2">
        <span class="f-name">D4</span>
        <span class="f-val">bit</span>
      </div>
      <div class="f-field" style="--fc:#f97316;flex:2">
        <span class="f-name">D5</span>
        <span class="f-val">bit</span>
      </div>
      <div class="f-field" style="--fc:#f97316;flex:2">
        <span class="f-name">D6</span>
        <span class="f-val">bit</span>
      </div>
      <div class="f-field" style="--fc:#f97316;flex:2">
        <span class="f-name">D7 (MSB)</span>
        <span class="f-val">bit</span>
      </div>
      <div class="f-field" style="--fc:#22c55e">
        <span class="f-name">STOP</span>
        <span class="f-val">HIGH</span>
        <span class="f-sub">1 bit</span>
      </div>
      <div class="f-field idle" style="--fc:#475569;flex:2">
        <span class="f-name">IDLE</span>
        <span class="f-val">HIGH</span>
      </div>
    </div>
    <div class="timing-note">
      1 bit = 1/9600 ≈ <strong>104 µs</strong> &nbsp;·&nbsp; 1 byte (10 bits) ≈ <strong>1.04 ms</strong> &nbsp;·&nbsp; 32-byte frame ≈ <strong>33.3 ms</strong>
    </div>

    <!-- PMS5003 32-byte packet -->
    <p class="frame-label">PMS5003 — 32-byte data frame (Big-endian uint16, checksum = sum byte[0..29])</p>
    <div class="packet-grid">
      <div class="pk-row header">
        <span>Offset</span><span>Bytes</span><span>Field</span><span>Code variable</span><span>Description</span>
      </div>
      <div class="pk-row" v-for="f in pmsFields" :key="f.off" :style="`--rc:${f.color}`">
        <span class="mono">{{ f.off }}</span>
        <span class="mono">{{ f.bytes }}</span>
        <span class="pk-field" :style="`background:${f.color}20;border-color:${f.color}40;color:${f.color}`">{{ f.name }}</span>
        <span class="mono pk-code">{{ f.code }}</span>
        <span class="pk-desc">{{ f.desc }}</span>
      </div>
    </div>

    <!-- Code snippet -->
    <div class="code-note">
      <span class="cn-label">readPMSdata() — sync on start byte 0x42, verify checksum</span>
      <code>if (s-&gt;peek() != 0x42) &#123; s-&gt;read(); return false; &#125;  // รอ start byte</code>
      <code>if (s-&gt;available() &lt; 32) return false;              // รอให้ครบ 32 bytes</code>
      <code>for (i = 0; i &lt; 30; i++) sum += buffer[i];          // checksum</code>
      <code>if (sum != candidate.checksum) return false;         // validate</code>
    </div>
  </div>
</template>

<script setup>
const pmsFields = [
  { off: '0x00', bytes: '1', name: '0x42',       color: '#22c55e', code: '—',                   desc: 'Start byte 1 ("B")' },
  { off: '0x01', bytes: '1', name: '0x4D',       color: '#22c55e', code: '—',                   desc: 'Start byte 2 ("M")' },
  { off: '0x02', bytes: '2', name: 'Frame Len',  color: '#64748b', code: 'framelen',             desc: 'Remaining length = 28' },
  { off: '0x04', bytes: '2', name: 'PM1.0 CF',   color: '#06b6d4', code: 'pm10_standard',       desc: 'PM1.0 standard (µg/m³)' },
  { off: '0x06', bytes: '2', name: 'PM2.5 CF',   color: '#06b6d4', code: 'pm25_standard',       desc: 'PM2.5 standard (µg/m³)' },
  { off: '0x08', bytes: '2', name: 'PM10 CF',    color: '#06b6d4', code: 'pm100_standard',      desc: 'PM10 standard (µg/m³)' },
  { off: '0x0A', bytes: '2', name: 'PM1.0 ATM',  color: '#f97316', code: 'pm10_env',            desc: 'PM1.0 atmospheric (µg/m³)' },
  { off: '0x0C', bytes: '2', name: 'PM2.5 ATM',  color: '#f97316', code: 'pm25_env',            desc: 'PM2.5 atmospheric (µg/m³) ← ใช้ใน report' },
  { off: '0x0E', bytes: '2', name: 'PM10 ATM',   color: '#f97316', code: 'pm100_env',           desc: 'PM10 atmospheric (µg/m³)' },
  { off: '0x10', bytes: '2', name: '>0.3µm',     color: '#a855f7', code: 'particles_03um',      desc: 'Particle count per 0.1L' },
  { off: '0x12', bytes: '2', name: '>0.5µm',     color: '#a855f7', code: 'particles_05um',      desc: '' },
  { off: '0x14', bytes: '2', name: '>1.0µm',     color: '#a855f7', code: 'particles_10um',      desc: '' },
  { off: '0x16', bytes: '2', name: '>2.5µm',     color: '#a855f7', code: 'particles_25um',      desc: '' },
  { off: '0x18', bytes: '2', name: '>5.0µm',     color: '#a855f7', code: 'particles_50um',      desc: '' },
  { off: '0x1A', bytes: '2', name: '>10µm',      color: '#a855f7', code: 'particles_100um',     desc: '' },
  { off: '0x1C', bytes: '2', name: 'Reserved',   color: '#334155', code: 'unused',              desc: 'Reserved / version' },
  { off: '0x1E', bytes: '2', name: 'Checksum',   color: '#ef4444', code: 'checksum',            desc: 'Sum of bytes[0..29]' },
]
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

.frame-row {
  display: flex; align-items: stretch; gap: 2px;
  margin-bottom: 10px; flex-wrap: wrap;
}
.f-field {
  display: flex; flex-direction: column; align-items: center; justify-content: center;
  flex: 1; min-width: 40px; padding: 6px 4px;
  background: color-mix(in srgb, var(--fc) 12%, var(--surface));
  border: 1px solid color-mix(in srgb, var(--fc) 35%, transparent);
  border-radius: 4px; gap: 2px; text-align: center;
}
.f-name { font-size: 0.6rem; font-weight: 700; text-transform: uppercase; letter-spacing: 0.06em; color: color-mix(in srgb, var(--fc) 80%, var(--muted)); }
.f-val { font-size: 0.78rem; font-weight: 800; font-family: 'JetBrains Mono', monospace; color: var(--fc); }
.f-sub { font-size: 0.58rem; color: var(--muted); }

.timing-note {
  font-size: 0.75rem; color: var(--muted); margin-bottom: 24px;
  padding: 6px 12px; background: var(--bg); border: 1px solid var(--border); border-radius: 5px;
}
.timing-note strong { color: #f97316; }

/* Packet grid */
.packet-grid {
  border: 1px solid var(--border);
  border-radius: 8px;
  overflow: hidden;
  margin-bottom: 16px;
  font-size: 0.78rem;
}
.pk-row {
  display: grid;
  grid-template-columns: 52px 40px 1fr 1fr 2fr;
  gap: 0;
  border-bottom: 1px solid var(--border);
  align-items: center;
}
.pk-row:last-child { border-bottom: none; }
.pk-row.header {
  background: var(--bg);
  font-size: 0.65rem;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.08em;
  color: var(--muted);
}
.pk-row > span { padding: 6px 10px; border-right: 1px solid var(--border); }
.pk-row > span:last-child { border-right: none; }
.mono { font-family: 'JetBrains Mono', monospace; color: var(--dim); }
.pk-field {
  font-size: 0.7rem; font-weight: 700;
  padding: 3px 8px; border-radius: 3px; border: 1px solid;
  text-align: center; white-space: nowrap;
}
.pk-code { font-size: 0.7rem; color: var(--teal); }
.pk-desc { color: var(--muted); font-size: 0.72rem; }

.code-note {
  background: var(--bg); border: 1px solid var(--border); border-radius: 6px;
  padding: 10px 14px; display: flex; flex-direction: column; gap: 4px;
}
.cn-label {
  font-size: 0.65rem; font-weight: 700; color: var(--muted);
  text-transform: uppercase; letter-spacing: 0.08em;
}
code { font-family: 'JetBrains Mono', monospace; font-size: 0.75rem; color: var(--teal); }
</style>
