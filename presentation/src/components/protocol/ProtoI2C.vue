<template>
  <div class="proto-block">
    <h3 class="proto-title">
      <span class="proto-badge" style="background:#06b6d420;color:var(--teal);border-color:#06b6d440">I2C</span>
      50 kHz · shared bus GPIO22 (SDA) / GPIO23 (SCL)
    </h3>
    <p class="proto-note">OLED (0x3C) และ AM2320 (0x5C) แชร์บัสเดียวกัน — ไม่ชนกันเพราะ address ต่างกัน</p>

    <!-- OLED Command -->
    <p class="frame-label">OLED SSD1306 — ส่ง 1 คำสั่ง (เช่น 0xAF = Display ON)</p>
    <div class="frame-row">
      <div class="f-field" style="--fc:#22c55e">
        <span class="f-name">S</span>
        <span class="f-val">START</span>
      </div>
      <div class="f-field" style="--fc:#06b6d4;flex:2">
        <span class="f-name">ADDR 7-bit</span>
        <span class="f-val">0x3C</span>
        <span class="f-sub">OLED</span>
      </div>
      <div class="f-field" style="--fc:#a855f7">
        <span class="f-name">R/W̄</span>
        <span class="f-val">0</span>
        <span class="f-sub">Write</span>
      </div>
      <div class="f-field ack" style="--fc:#f97316">
        <span class="f-name">ACK</span>
        <span class="f-val">↓</span>
      </div>
      <div class="f-field" style="--fc:#eab308;flex:2">
        <span class="f-name">Control Byte</span>
        <span class="f-val">0x00</span>
        <span class="f-sub">Co=0 D/C̄=0 → CMD</span>
      </div>
      <div class="f-field ack" style="--fc:#f97316">
        <span class="f-name">ACK</span>
        <span class="f-val">↓</span>
      </div>
      <div class="f-field" style="--fc:#e2e8f0;flex:2">
        <span class="f-name">Command</span>
        <span class="f-val">0xAF</span>
        <span class="f-sub">Display ON</span>
      </div>
      <div class="f-field ack" style="--fc:#f97316">
        <span class="f-name">ACK</span>
        <span class="f-val">↓</span>
      </div>
      <div class="f-field" style="--fc:#22c55e">
        <span class="f-name">P</span>
        <span class="f-val">STOP</span>
      </div>
    </div>

    <!-- OLED Data flush -->
    <p class="frame-label">OLED SSD1306 — ส่ง pixel data (16 bytes ต่อครั้ง ×64 รอบต่อ frame)</p>
    <div class="frame-row">
      <div class="f-field" style="--fc:#22c55e"><span class="f-name">S</span><span class="f-val">START</span></div>
      <div class="f-field" style="--fc:#06b6d4;flex:2"><span class="f-name">ADDR</span><span class="f-val">0x3C</span></div>
      <div class="f-field" style="--fc:#a855f7"><span class="f-name">R/W̄</span><span class="f-val">0</span><span class="f-sub">Write</span></div>
      <div class="f-field ack" style="--fc:#f97316"><span class="f-name">ACK</span><span class="f-val">↓</span></div>
      <div class="f-field" style="--fc:#eab308;flex:2">
        <span class="f-name">Control Byte</span>
        <span class="f-val">0x40</span>
        <span class="f-sub">Co=0 D/C̄=1 → DATA</span>
      </div>
      <div class="f-field ack" style="--fc:#f97316"><span class="f-name">ACK</span><span class="f-val">↓</span></div>
      <div class="f-field" style="--fc:#64748b;flex:3">
        <span class="f-name">Data[0..15]</span>
        <span class="f-val">D0 D1 … D15</span>
        <span class="f-sub">16 pixel bytes + ACK each</span>
      </div>
      <div class="f-field" style="--fc:#22c55e"><span class="f-name">P</span><span class="f-val">STOP</span></div>
    </div>

    <!-- AM2320 Wake -->
    <p class="frame-label">AM2320 — Wake-up (sensor ประหยัดพลังงาน ต้อง wake ก่อนทุกครั้ง)</p>
    <div class="frame-row">
      <div class="f-field" style="--fc:#22c55e"><span class="f-name">S</span><span class="f-val">START</span></div>
      <div class="f-field" style="--fc:#06b6d4;flex:2"><span class="f-name">ADDR 7-bit</span><span class="f-val">0x5C</span><span class="f-sub">AM2320</span></div>
      <div class="f-field" style="--fc:#a855f7"><span class="f-name">R/W̄</span><span class="f-val">0</span><span class="f-sub">Write</span></div>
      <div class="f-field nack" style="--fc:#ef4444"><span class="f-name">NACK</span><span class="f-val">✕</span><span class="f-sub">ปกติ</span></div>
      <div class="f-field" style="--fc:#22c55e"><span class="f-name">P</span><span class="f-val">STOP</span></div>
      <div class="f-gap">delay 10ms</div>
    </div>

    <!-- AM2320 Write request -->
    <p class="frame-label">AM2320 — Write Request: อ่าน 4 registers เริ่มจาก 0x00 (humidity + temperature)</p>
    <div class="frame-row">
      <div class="f-field" style="--fc:#22c55e"><span class="f-name">S</span><span class="f-val">START</span></div>
      <div class="f-field" style="--fc:#06b6d4;flex:2"><span class="f-name">ADDR</span><span class="f-val">0x5C</span></div>
      <div class="f-field" style="--fc:#a855f7"><span class="f-name">R/W̄</span><span class="f-val">0</span><span class="f-sub">Write</span></div>
      <div class="f-field ack" style="--fc:#f97316"><span class="f-name">ACK</span><span class="f-val">↓</span></div>
      <div class="f-field" style="--fc:#e2e8f0;flex:2">
        <span class="f-name">Function</span>
        <span class="f-val">0x03</span>
        <span class="f-sub">Read Regs</span>
      </div>
      <div class="f-field ack" style="--fc:#f97316"><span class="f-name">ACK</span><span class="f-val">↓</span></div>
      <div class="f-field" style="--fc:#e2e8f0;flex:2">
        <span class="f-name">Start Reg</span>
        <span class="f-val">0x00</span>
        <span class="f-sub">Humidity H</span>
      </div>
      <div class="f-field ack" style="--fc:#f97316"><span class="f-name">ACK</span><span class="f-val">↓</span></div>
      <div class="f-field" style="--fc:#e2e8f0;flex:2">
        <span class="f-name">Count</span>
        <span class="f-val">0x04</span>
        <span class="f-sub">4 registers</span>
      </div>
      <div class="f-field ack" style="--fc:#f97316"><span class="f-name">ACK</span><span class="f-val">↓</span></div>
      <div class="f-field" style="--fc:#22c55e"><span class="f-name">P</span><span class="f-val">STOP</span></div>
      <div class="f-gap">delay 10ms</div>
    </div>

    <!-- AM2320 Read response -->
    <p class="frame-label">AM2320 — Read Response: 8 bytes (func + count + 4 data + 2 CRC Modbus)</p>
    <div class="frame-row">
      <div class="f-field" style="--fc:#22c55e"><span class="f-name">S</span><span class="f-val">START</span></div>
      <div class="f-field" style="--fc:#06b6d4;flex:2"><span class="f-name">ADDR</span><span class="f-val">0x5C</span></div>
      <div class="f-field" style="--fc:#a855f7"><span class="f-name">R/W̄</span><span class="f-val">1</span><span class="f-sub">Read</span></div>
      <div class="f-field ack" style="--fc:#f97316"><span class="f-name">ACK</span><span class="f-val">↓</span></div>
      <div class="f-field" style="--fc:#64748b;flex:1.5"><span class="f-name">buf[0]</span><span class="f-val">0x03</span><span class="f-sub">func echo</span></div>
      <div class="f-field" style="--fc:#64748b;flex:1.5"><span class="f-name">buf[1]</span><span class="f-val">0x04</span><span class="f-sub">byte count</span></div>
      <div class="f-field" style="--fc:#06b6d4;flex:3">
        <span class="f-name">buf[2..3]</span>
        <span class="f-val">HUM_H · HUM_L</span>
        <span class="f-sub">÷10 = %RH</span>
      </div>
      <div class="f-field" style="--fc:#f97316;flex:3">
        <span class="f-name">buf[4..5]</span>
        <span class="f-val">TEMP_H · TEMP_L</span>
        <span class="f-sub">bit15=sign ÷10 = °C</span>
      </div>
      <div class="f-field" style="--fc:#a855f7;flex:3">
        <span class="f-name">buf[6..7]</span>
        <span class="f-val">CRC_L · CRC_H</span>
        <span class="f-sub">CRC16-Modbus little-endian</span>
      </div>
      <div class="f-field nack" style="--fc:#ef4444"><span class="f-name">NAK</span><span class="f-val">✕</span></div>
      <div class="f-field" style="--fc:#22c55e"><span class="f-name">P</span><span class="f-val">STOP</span></div>
    </div>

    <!-- CRC note -->
    <div class="code-note">
      <span class="cn-label">CRC verify (code line 578-579)</span>
      <code>const uint16_t crcExpected = (buffer[7] &lt;&lt; 8) | buffer[6]; // little-endian</code>
      <code>const uint16_t crcActual = crc16Modbus(buffer, 6);          // over 6 bytes</code>
    </div>
  </div>
</template>

<style scoped>
.proto-block { margin-bottom: 40px; }
.proto-title {
  font-size: 0.95rem;
  font-weight: 700;
  color: var(--text);
  display: flex;
  align-items: center;
  gap: 10px;
  margin-bottom: 6px;
}
.proto-badge {
  font-size: 0.72rem;
  font-weight: 800;
  padding: 2px 10px;
  border-radius: 20px;
  border: 1px solid;
  letter-spacing: 0.08em;
}
.proto-note {
  font-size: 0.8rem;
  color: var(--muted);
  margin-bottom: 20px;
}
.frame-label {
  font-size: 0.75rem;
  color: var(--dim);
  margin-bottom: 6px;
  font-weight: 600;
}
.frame-row {
  display: flex;
  align-items: stretch;
  gap: 2px;
  margin-bottom: 20px;
  flex-wrap: wrap;
}
.f-field {
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  flex: 1;
  min-width: 44px;
  padding: 6px 4px;
  background: color-mix(in srgb, var(--fc) 12%, var(--surface));
  border: 1px solid color-mix(in srgb, var(--fc) 35%, transparent);
  border-radius: 4px;
  gap: 2px;
  text-align: center;
}
.f-name {
  font-size: 0.6rem;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.06em;
  color: color-mix(in srgb, var(--fc) 80%, var(--muted));
}
.f-val {
  font-size: 0.78rem;
  font-weight: 800;
  font-family: 'JetBrains Mono', monospace;
  color: var(--fc);
}
.f-sub {
  font-size: 0.58rem;
  color: var(--muted);
  text-align: center;
  line-height: 1.3;
}
.f-gap {
  display: flex;
  align-items: center;
  padding: 0 8px;
  font-size: 0.65rem;
  color: var(--muted);
  font-style: italic;
  white-space: nowrap;
}
.code-note {
  background: var(--bg);
  border: 1px solid var(--border);
  border-radius: 6px;
  padding: 10px 14px;
  display: flex;
  flex-direction: column;
  gap: 4px;
  margin-top: -10px;
  margin-bottom: 20px;
}
.cn-label {
  font-size: 0.65rem;
  font-weight: 700;
  color: var(--muted);
  text-transform: uppercase;
  letter-spacing: 0.08em;
}
code {
  font-family: 'JetBrains Mono', monospace;
  font-size: 0.75rem;
  color: var(--teal);
}
</style>
