<template>
  <div class="proto-block">
    <h3 class="proto-title">
      <span class="proto-badge" style="background:#ef444420;color:#ef4444;border-color:#ef444440">GPIO</span>
      Digital Input · GPIO33 · Internal Pull-up · Active LOW
    </h3>
    <p class="proto-note">Red button ต่อ GPIO33 → GND โดยตรง — ใช้ internal pull-up ของ ESP32 ไม่ต้องต่อ resistor ภายนอก</p>

    <!-- Timing diagram SVG -->
    <p class="frame-label">GPIO33 Timing — Short tap (toggle screen) vs Long hold 3s (Config Mode)</p>
    <div class="svg-wrap">
      <svg viewBox="0 0 760 160" xmlns="http://www.w3.org/2000/svg" style="width:100%;height:auto">
        <!-- GPIO33 label -->
        <text x="4" y="52" font-family="'JetBrains Mono',monospace" font-size="11" font-weight="700" fill="#94a3b8">GPIO33</text>

        <!-- IDLE HIGH -->
        <line x1="60" y1="30" x2="160" y2="30" stroke="#22c55e" stroke-width="2.5"/>
        <!-- Short press: LOW from 160..240 -->
        <line x1="160" y1="30" x2="160" y2="62" stroke="#22c55e" stroke-width="2"/>
        <line x1="160" y1="62" x2="240" y2="62" stroke="#ef4444" stroke-width="2.5"/>
        <line x1="240" y1="62" x2="240" y2="30" stroke="#22c55e" stroke-width="2"/>
        <!-- back HIGH 240..360 -->
        <line x1="240" y1="30" x2="360" y2="30" stroke="#22c55e" stroke-width="2.5"/>
        <!-- Long hold: LOW from 360..640 (280px → represents 3000ms) -->
        <line x1="360" y1="30" x2="360" y2="62" stroke="#22c55e" stroke-width="2"/>
        <line x1="360" y1="62" x2="640" y2="62" stroke="#ef4444" stroke-width="2.5" stroke-dasharray="0"/>
        <line x1="640" y1="62" x2="640" y2="30" stroke="#22c55e" stroke-width="2"/>
        <!-- IDLE HIGH after -->
        <line x1="640" y1="30" x2="740" y2="30" stroke="#22c55e" stroke-width="2.5"/>

        <!-- HIGH/LOW labels -->
        <text x="65" y="25" font-family="monospace" font-size="9" font-weight="700" fill="#22c55e">HIGH (3.3V) — idle, pull-up</text>
        <text x="175" y="78" font-family="monospace" font-size="9" font-weight="700" fill="#ef4444">LOW (0V) — pressed</text>

        <!-- Short tap annotation -->
        <line x1="160" y1="90" x2="240" y2="90" stroke="#f97316" stroke-width="1"/>
        <line x1="160" y1="86" x2="160" y2="94" stroke="#f97316" stroke-width="1"/>
        <line x1="240" y1="86" x2="240" y2="94" stroke="#f97316" stroke-width="1"/>
        <text x="200" y="103" text-anchor="middle" font-family="monospace" font-size="9" fill="#f97316">Short tap</text>
        <text x="200" y="113" text-anchor="middle" font-family="monospace" font-size="9" fill="#f97316">&lt; 3000ms</text>
        <text x="200" y="123" text-anchor="middle" font-family="monospace" font-size="9" fill="#64748b">toggle screen</text>

        <!-- Long hold annotation -->
        <line x1="360" y1="90" x2="640" y2="90" stroke="#a855f7" stroke-width="1"/>
        <line x1="360" y1="86" x2="360" y2="94" stroke="#a855f7" stroke-width="1"/>
        <line x1="640" y1="86" x2="640" y2="94" stroke="#a855f7" stroke-width="1"/>
        <text x="500" y="103" text-anchor="middle" font-family="monospace" font-size="9" fill="#a855f7">Long hold ≥ 3000ms</text>
        <text x="500" y="113" text-anchor="middle" font-family="monospace" font-size="9" fill="#a855f7">(CONFIG_HOLD_MS)</text>
        <text x="500" y="123" text-anchor="middle" font-family="monospace" font-size="9" fill="#64748b">→ Enter Config Mode</text>

        <!-- Debounce note -->
        <rect x="148" y="130" width="104" height="16" fill="#f9731610" rx="3"/>
        <text x="200" y="141" text-anchor="middle" font-family="monospace" font-size="8" fill="#f97316">debounce 1ms (CONFIG_TAP_DEBOUNCE_MS)</text>

        <!-- Time axis -->
        <line x1="60" y1="150" x2="740" y2="150" stroke="#1e2d3d" stroke-width="1"/>
        <text x="160" y="148" text-anchor="middle" font-family="monospace" font-size="9" fill="#64748b">t₀</text>
        <text x="240" y="148" text-anchor="middle" font-family="monospace" font-size="9" fill="#64748b">t₁</text>
        <text x="360" y="148" text-anchor="middle" font-family="monospace" font-size="9" fill="#64748b">t₂</text>
        <text x="640" y="148" text-anchor="middle" font-family="monospace" font-size="9" fill="#64748b">t₂+3s</text>
        <text x="740" y="158" text-anchor="end" font-family="monospace" font-size="9" fill="#64748b">time →</text>
      </svg>
    </div>

    <!-- State machine table -->
    <div class="state-grid">
      <div class="st-row header"><span>State</span><span>GPIO33</span><span>Duration</span><span>Action</span><span>Code</span></div>
      <div class="st-row">
        <span class="st-label idle">IDLE</span>
        <span class="mono hi">HIGH</span>
        <span>—</span>
        <span>ไม่มีการกด</span>
        <span class="mono code-s">digitalRead(33) == HIGH</span>
      </div>
      <div class="st-row">
        <span class="st-label press">PRESS</span>
        <span class="mono lo">LOW</span>
        <span>0 → detect</span>
        <span>บันทึกเวลา press</span>
        <span class="mono code-s">configButtonPressMs = millis()</span>
      </div>
      <div class="st-row">
        <span class="st-label tap">TAP</span>
        <span class="mono hi">HIGH</span>
        <span>&lt; 3000ms</span>
        <span>toggle normalScreenIndex</span>
        <span class="mono code-s">normalScreenTapHandled = true</span>
      </div>
      <div class="st-row">
        <span class="st-label hold">HOLD</span>
        <span class="mono lo">LOW</span>
        <span>≥ 3000ms</span>
        <span>set configMode = true → reboot</span>
        <span class="mono code-s">configButtonLatched = true</span>
      </div>
    </div>
  </div>
</template>

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
  border-radius: 8px; padding: 12px; margin-bottom: 20px; overflow-x: auto;
}

/* State table */
.state-grid {
  border: 1px solid var(--border); border-radius: 8px;
  overflow: hidden; font-size: 0.78rem;
}
.st-row {
  display: grid;
  grid-template-columns: 90px 60px 90px 1fr 1.6fr;
  border-bottom: 1px solid var(--border); align-items: center;
}
.st-row:last-child { border-bottom: none; }
.st-row.header {
  background: var(--bg); font-size: 0.65rem; font-weight: 700;
  text-transform: uppercase; letter-spacing: 0.08em; color: var(--muted);
}
.st-row > span { padding: 7px 12px; border-right: 1px solid var(--border); }
.st-row > span:last-child { border-right: none; }

.st-label {
  font-size: 0.7rem; font-weight: 800;
  padding: 2px 8px !important; border-radius: 20px;
  text-align: center; letter-spacing: 0.06em;
}
.st-label.idle  { background: #47556920; color: #94a3b8; border: 1px solid #47556940; }
.st-label.press { background: #ef444420; color: #ef4444; border: 1px solid #ef444440; }
.st-label.tap   { background: #f9731620; color: #f97316; border: 1px solid #f9731640; }
.st-label.hold  { background: #a855f720; color: #a855f7; border: 1px solid #a855f740; }
.mono { font-family: 'JetBrains Mono', monospace; font-size: 0.75rem; }
.hi { color: #22c55e; font-weight: 700; }
.lo { color: #ef4444; font-weight: 700; }
.code-s { color: var(--teal); font-size: 0.68rem; }
</style>
