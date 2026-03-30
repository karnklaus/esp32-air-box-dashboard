<script setup>
import OledMock from '../components/config/OledMock.vue'

const steps = [
  {
    n: 1,
    icon: 'fa-solid fa-hand-pointer',
    color: 'var(--red)',
    title: 'Hold Red Button 3s',
    desc: 'กดปุ่มแดง (GPIO33) ค้างไว้ 3 วินาที — ESP32 ออกจาก normal mode',
  },
  {
    n: 2,
    icon: 'fa-solid fa-wifi',
    color: 'var(--teal)',
    title: 'ESP32 เปิด Wi-Fi AP',
    desc: 'สร้าง Access Point "AirBox-XXXXXX" พร้อม DNS captive portal ที่ 192.168.4.1',
  },
  {
    n: 3,
    icon: 'fa-solid fa-display',
    color: 'var(--purple)',
    title: 'OLED แสดง Config Info',
    desc: 'แสดง SSID, IP address, สถานะ — กดปุ่มสั้นเพื่อสลับเป็น QR code view',
  },
  {
    n: 4,
    icon: 'fa-solid fa-mobile-screen',
    color: 'var(--orange)',
    title: 'เชื่อมต่อด้วยมือถือ',
    desc: 'เชื่อมต่อ Wi-Fi "AirBox-XXXXXX" → captive portal เปิดอัตโนมัติ',
  },
  {
    n: 5,
    icon: 'fa-solid fa-floppy-disk',
    color: 'var(--green)',
    title: 'กรอก Config แล้ว Save',
    desc: 'ตั้งค่า Box ID, Wi-Fi, Server IP/Port → กด "Save and Reboot"',
  },
  {
    n: 6,
    icon: 'fa-solid fa-rotate-right',
    color: 'var(--teal)',
    title: 'ESP32 Reboot',
    desc: 'บันทึกลง Flash แล้ว reboot — เชื่อมต่อ Wi-Fi ใหม่และเริ่ม POST ข้อมูลไปยัง Server',
  },
]

const configFields = [
  { label: 'Box ID',        type: 'text',     hint: 'e.g. ABX-001' },
  { label: 'WiFi SSID',     type: 'select',   hint: 'Scanned networks dropdown' },
  { label: 'WiFi Password', type: 'password', hint: '••••••••' },
  { label: 'Server IP',     type: 'text',     hint: 'e.g. 192.168.1.100' },
  { label: 'Server Port',   type: 'number',   hint: '5000' },
]
</script>

<template>
  <section class="section cfg-page">
    <div class="container">
      <p class="section-label">Config Mode</p>
      <h2 class="section-title">On-Device Setup</h2>
      <p class="section-desc">
        กด Red Button ค้าง 3 วินาที เพื่อเปิด Captive Portal สำหรับตั้งค่า Wi-Fi และ Server
        โดยไม่ต้องแก้ไข firmware
      </p>

      <!-- Steps timeline -->
      <div class="steps">
        <div class="step" v-for="step in steps" :key="step.n">
          <div class="step-circle" :style="{ borderColor: step.color }">
            <i :class="step.icon" :style="{ color: step.color }"></i>
          </div>
          <div class="step-connector" v-if="step.n < steps.length"></div>
          <div class="step-body">
            <span class="step-num" :style="{ color: step.color }">{{ String(step.n).padStart(2,'0') }}</span>
            <strong class="step-title">{{ step.title }}</strong>
            <p class="step-desc">{{ step.desc }}</p>
          </div>
        </div>
      </div>

      <!-- OLED + Form mockups -->
      <div class="mockup-row">

        <!-- OLED panels -->
        <div class="mockup-col">
          <p class="mockup-label"><i class="fa-solid fa-display"></i> OLED Display</p>
          <div class="oled-pair">
            <div class="oled-item">
              <OledMock screen="config-info" />
              <span class="oled-caption">Info View</span>
            </div>
            <div class="oled-arrow"><i class="fa-solid fa-arrows-left-right"></i> กดปุ่มสลับ</div>
            <div class="oled-item">
              <OledMock screen="config-qr" />
              <span class="oled-caption">QR View</span>
            </div>
          </div>
          <div class="oled-item" style="margin-top:16px">
            <OledMock screen="connecting" />
            <span class="oled-caption">หลัง Save — COMPLETE</span>
          </div>
        </div>

        <!-- Captive portal form mock -->
        <div class="mockup-col">
          <p class="mockup-label"><i class="fa-solid fa-mobile-screen"></i> Captive Portal</p>
          <div class="portal-mock card">
            <div class="portal-header">
              <span class="portal-title">Air Box Config</span>
              <span class="portal-sub">192.168.4.1</span>
            </div>
            <p class="portal-hint">
              Connect with your phone, edit box id and WiFi, then save to reboot.
            </p>
            <div class="portal-form">
              <div class="portal-field" v-for="f in configFields" :key="f.label">
                <label>{{ f.label }}</label>
                <div class="portal-input" :class="{ select: f.type === 'select' }">
                  <span v-if="f.type === 'select'">
                    {{ f.hint }} <i class="fa-solid fa-chevron-down"></i>
                  </span>
                  <span v-else class="hint-text">{{ f.hint }}</span>
                </div>
              </div>
              <button class="portal-btn">
                <i class="fa-solid fa-floppy-disk"></i> Save and Reboot
              </button>
            </div>
          </div>
        </div>

      </div>

      <!-- Button behavior note -->
      <div class="btn-notes">
        <div class="btn-note card">
          <i class="fa-solid fa-circle-info" style="color:var(--teal)"></i>
          <div>
            <strong>Button Behavior (GPIO33 · INPUT_PULLUP)</strong>
            <ul>
              <li><kbd>Hold 3s</kbd> — เข้า / ออก Config Mode</li>
              <li><kbd>Short press</kbd> ใน Config Mode — สลับ OLED info ↔ QR</li>
              <li><kbd>Short press</kbd> ใน Normal Mode — สลับหน้า PM / Climate / Sound</li>
            </ul>
          </div>
        </div>
      </div>

    </div>
  </section>
</template>

<style scoped>
.cfg-page {
  background: radial-gradient(ellipse 60% 40% at 50% 20%, #ef444406 0%, transparent 70%), var(--bg);
}

/* ─── Steps ───────────────────────────────────────── */
.steps {
  display: flex;
  flex-wrap: wrap;
  gap: 0;
  margin-bottom: 52px;
}
.step {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 0;
  flex: 0 0 auto;
}
.step-circle {
  width: 44px; height: 44px;
  border-radius: 50%;
  border: 2px solid;
  background: var(--surface);
  display: flex; align-items: center; justify-content: center;
  font-size: 1rem;
  flex-shrink: 0;
  position: relative;
  z-index: 1;
}
.step-connector {
  width: 2px;
  height: 28px;
  background: var(--border);
  margin: 2px 0;
}
.step-body {
  background: var(--surface);
  border: 1px solid var(--border);
  border-radius: var(--radius);
  padding: 12px 16px;
  margin: 6px 0 12px;
  max-width: 440px;
}
.step-num {
  font-size: 0.7rem;
  font-weight: 700;
  font-family: monospace;
  letter-spacing: 0.1em;
  display: block;
  margin-bottom: 2px;
}
.step-title {
  font-size: 0.95rem;
  color: var(--text);
  display: block;
  margin-bottom: 4px;
}
.step-desc {
  font-size: 0.83rem;
  color: var(--muted);
  line-height: 1.5;
  margin: 0;
}

/* ─── Mockup row ─────────────────────────────────── */
.mockup-row {
  display: grid;
  grid-template-columns: auto 1fr;
  gap: 40px;
  align-items: start;
  margin-bottom: 32px;
}
.mockup-label {
  font-size: 0.8rem;
  font-weight: 600;
  color: var(--muted);
  text-transform: uppercase;
  letter-spacing: 0.1em;
  margin-bottom: 14px;
  display: flex;
  align-items: center;
  gap: 7px;
}
.oled-pair {
  display: flex;
  align-items: center;
  gap: 16px;
  flex-wrap: wrap;
}
.oled-item {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 8px;
}
.oled-caption {
  font-size: 0.75rem;
  color: var(--muted);
}
.oled-arrow {
  font-size: 0.75rem;
  color: var(--muted);
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 4px;
}

/* ─── Portal mock ────────────────────────────────── */
.portal-mock {
  max-width: 360px;
}
.portal-header {
  margin-bottom: 8px;
}
.portal-title {
  font-size: 1.1rem;
  font-weight: 700;
  color: var(--text);
  display: block;
}
.portal-sub {
  font-size: 0.75rem;
  color: var(--muted);
  font-family: monospace;
}
.portal-hint {
  font-size: 0.8rem;
  color: var(--muted);
  margin-bottom: 16px;
  line-height: 1.5;
}
.portal-form {
  display: flex;
  flex-direction: column;
  gap: 10px;
}
.portal-field label {
  display: block;
  font-size: 0.7rem;
  font-weight: 700;
  text-transform: uppercase;
  letter-spacing: 0.08em;
  color: var(--muted);
  margin-bottom: 4px;
}
.portal-input {
  background: var(--bg);
  border: 1px solid var(--border);
  border-radius: 5px;
  padding: 8px 12px;
  font-size: 0.82rem;
  color: var(--dim);
  display: flex;
  justify-content: space-between;
  align-items: center;
  font-family: monospace;
}
.hint-text { opacity: 0.5; }
.portal-btn {
  margin-top: 4px;
  padding: 10px;
  background: var(--teal);
  color: #000;
  border: none;
  border-radius: 6px;
  font-weight: 700;
  font-size: 0.85rem;
  cursor: default;
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 8px;
}

/* ─── Button notes ───────────────────────────────── */
.btn-notes { margin-top: 8px; }
.btn-note {
  display: flex;
  gap: 14px;
  align-items: flex-start;
  max-width: 560px;
}
.btn-note strong {
  display: block;
  margin-bottom: 8px;
  font-size: 0.9rem;
}
.btn-note ul {
  list-style: none;
  display: flex;
  flex-direction: column;
  gap: 6px;
  padding: 0;
  margin: 0;
}
.btn-note li {
  font-size: 0.83rem;
  color: var(--dim);
}
kbd {
  background: var(--surface2);
  border: 1px solid var(--border);
  border-radius: 4px;
  padding: 1px 7px;
  font-family: monospace;
  font-size: 0.78rem;
  color: var(--text);
}

@media (max-width: 700px) {
  .mockup-row { grid-template-columns: 1fr; }
}
</style>
