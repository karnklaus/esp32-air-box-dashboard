<template>
  <svg viewBox="0 0 960 460" xmlns="http://www.w3.org/2000/svg" class="pin-svg">

    <!-- ══ SENSOR BOX — OLED + AM2320 (I2C) ══ -->
    <rect x="8" y="48" width="220" height="168" rx="8" fill="#0f1623" stroke="#06b6d4" stroke-width="1.5"/>
    <rect x="8" y="48" width="220" height="28" rx="8" fill="#06b6d415"/>
    <rect x="8" y="62"  width="220" height="14" fill="#06b6d415"/>
    <text x="22" y="67"  font-size="9"    fill="#06b6d4" font-weight="600" letter-spacing="1.2" font-family="monospace">I2C · 0x3C / 0x5C</text>
    <text x="22" y="93"  font-size="13"   fill="#e2e8f0" font-weight="700">OLED + AM2320</text>
    <text x="22" y="108" font-size="9.5"  fill="#64748b">NFP1315-157B · 128×64 px</text>
    <line x1="22" y1="116" x2="218" y2="116" stroke="#1e2d3d" stroke-width="1"/>
    <text x="22" y="131" font-size="9.5" fill="#94a3b8" font-family="monospace">SDA  ←  GPIO22</text>
    <text x="22" y="147" font-size="9.5" fill="#94a3b8" font-family="monospace">SCL  ←  GPIO23</text>
    <text x="22" y="163" font-size="9.5" fill="#22c55e"  font-family="monospace">VCC  →  3.3V</text>
    <text x="22" y="179" font-size="9.5" fill="#475569"  font-family="monospace">GND  →  GND</text>
    <text x="22" y="204" font-size="9"   fill="#06b6d480" font-style="italic">OLED display · Temp / Hum sensor</text>

    <!-- ══ SENSOR BOX — INMP441 (I2S) ══ -->
    <rect x="8" y="248" width="220" height="148" rx="8" fill="#0f1623" stroke="#a855f7" stroke-width="1.5"/>
    <rect x="8" y="248" width="220" height="28" rx="8" fill="#a855f715"/>
    <rect x="8" y="262" width="220" height="14" fill="#a855f715"/>
    <text x="22" y="267" font-size="9"   fill="#a855f7" font-weight="600" letter-spacing="1.2" font-family="monospace">I2S · 16 kHz · 32-bit</text>
    <text x="22" y="293" font-size="13"  fill="#e2e8f0" font-weight="700">INMP441</text>
    <text x="22" y="308" font-size="9.5" fill="#64748b">MEMS Digital Microphone</text>
    <line x1="22" y1="316" x2="218" y2="316" stroke="#1e2d3d" stroke-width="1"/>
    <text x="22" y="331" font-size="9.5" fill="#94a3b8" font-family="monospace">BCLK ←  GPIO27</text>
    <text x="22" y="347" font-size="9.5" fill="#94a3b8" font-family="monospace">WS   ←  GPIO14</text>
    <text x="22" y="363" font-size="9.5" fill="#94a3b8" font-family="monospace">SD   →  GPIO26</text>
    <text x="22" y="384" font-size="9"   fill="#a855f780" font-style="italic">LOUD / QUIET detection</text>

    <!-- ══ SENSOR BOX — PMS5003 (UART) ══ -->
    <rect x="732" y="48" width="220" height="168" rx="8" fill="#0f1623" stroke="#f97316" stroke-width="1.5"/>
    <rect x="732" y="48"  width="220" height="28" rx="8" fill="#f9731615"/>
    <rect x="732" y="62"  width="220" height="14" fill="#f9731615"/>
    <text x="746" y="67"  font-size="9"   fill="#f97316" font-weight="600" letter-spacing="1.2" font-family="monospace">UART · 9600 baud</text>
    <text x="746" y="93"  font-size="13"  fill="#e2e8f0" font-weight="700">PMS5003</text>
    <text x="746" y="108" font-size="9.5" fill="#64748b">Laser Particle Counter</text>
    <line x1="746" y1="116" x2="942" y2="116" stroke="#1e2d3d" stroke-width="1"/>
    <text x="746" y="131" font-size="9.5" fill="#94a3b8" font-family="monospace">TXD  →  GPIO16 (RX)</text>
    <text x="746" y="147" font-size="9.5" fill="#94a3b8" font-family="monospace">RXD  ←  GPIO17 (TX)</text>
    <text x="746" y="163" font-size="9.5" fill="#eab308"  font-family="monospace">VCC  →  5V</text>
    <text x="746" y="179" font-size="9.5" fill="#475569"  font-family="monospace">GND  →  GND</text>
    <text x="746" y="204" font-size="9"   fill="#f9731680" font-style="italic">PM1.0 · PM2.5 · PM10</text>

    <!-- ══ SENSOR BOX — Red Button ══ -->
    <rect x="732" y="248" width="220" height="120" rx="8" fill="#0f1623" stroke="#ef4444" stroke-width="1.5"/>
    <rect x="732" y="248" width="220" height="28" rx="8" fill="#ef444415"/>
    <rect x="732" y="262" width="220" height="14" fill="#ef444415"/>
    <text x="746" y="267" font-size="9"   fill="#ef4444" font-weight="600" letter-spacing="1.2" font-family="monospace">GPIO · INPUT_PULLUP</text>
    <text x="746" y="293" font-size="13"  fill="#e2e8f0" font-weight="700">Red Button</text>
    <line x1="746" y1="302" x2="942" y2="302" stroke="#1e2d3d" stroke-width="1"/>
    <text x="746" y="317" font-size="9.5" fill="#94a3b8" font-family="monospace">SIG  →  GPIO33</text>
    <text x="746" y="333" font-size="9.5" fill="#475569"  font-family="monospace">GND  →  GND</text>
    <text x="746" y="356" font-size="9"   fill="#ef444480" font-style="italic">Hold 3s → Config Mode</text>

    <!-- ══ WIRES — Signal ══ -->
    <!-- I2C teal -->
    <path d="M345,152 H270 V128 H228" fill="none" stroke="#06b6d4" stroke-width="1.8" stroke-linecap="round"/>
    <path d="M345,177 H264 V156 H228" fill="none" stroke="#06b6d4" stroke-width="1.8" stroke-linecap="round"/>
    <!-- I2S purple -->
    <path d="M345,268 H258 V291 H228" fill="none" stroke="#a855f7" stroke-width="1.8" stroke-linecap="round"/>
    <path d="M345,293 H252 V320 H228" fill="none" stroke="#a855f7" stroke-width="1.8" stroke-linecap="round"/>
    <path d="M345,318 H246 V349 H228" fill="none" stroke="#a855f7" stroke-width="1.8" stroke-linecap="round"/>
    <!-- UART orange -->
    <path d="M615,152 H684 V125 H732" fill="none" stroke="#f97316" stroke-width="1.8" stroke-linecap="round"/>
    <path d="M615,177 H690 V153 H732" fill="none" stroke="#f97316" stroke-width="1.8" stroke-linecap="round"/>
    <!-- Button red -->
    <path d="M615,293 H684 V284 H732" fill="none" stroke="#ef4444" stroke-width="1.8" stroke-linecap="round"/>

    <!-- ══ WIRES — Power (dashed) ══ -->
    <path d="M345,368 H256 V173 H228" fill="none" stroke="#22c55e" stroke-width="1" stroke-dasharray="5 3" opacity="0.5" stroke-linecap="round"/>
    <path d="M345,368 H250 V380 H228" fill="none" stroke="#22c55e" stroke-width="1" stroke-dasharray="5 3" opacity="0.5" stroke-linecap="round"/>
    <path d="M615,368 H698 V172 H732" fill="none" stroke="#eab308" stroke-width="1" stroke-dasharray="5 3" opacity="0.5" stroke-linecap="round"/>

    <!-- ══ Wire endpoint dots ══ -->
    <circle cx="228" cy="128" r="3" fill="#06b6d4"/>
    <circle cx="228" cy="156" r="3" fill="#06b6d4"/>
    <circle cx="228" cy="291" r="3" fill="#a855f7"/>
    <circle cx="228" cy="320" r="3" fill="#a855f7"/>
    <circle cx="228" cy="349" r="3" fill="#a855f7"/>
    <circle cx="732" cy="125" r="3" fill="#f97316"/>
    <circle cx="732" cy="153" r="3" fill="#f97316"/>
    <circle cx="732" cy="284" r="3" fill="#ef4444"/>

    <!-- ══ ESP32 CHIP ══ -->
    <rect x="345" y="48" width="270" height="364" rx="10" fill="#0f1623" stroke="#2d3f52" stroke-width="2"/>
    <!-- top band -->
    <rect x="345" y="48" width="270" height="38" rx="10" fill="#162032"/>
    <rect x="345" y="72" width="270" height="14"          fill="#162032"/>
    <text x="480" y="73" text-anchor="middle" font-size="11" fill="#94a3b8" font-weight="700" letter-spacing="3" font-family="monospace">ESP32</text>
    <!-- inner die -->
    <rect x="385" y="106" width="190" height="260" rx="6" fill="#080c14" stroke="#1e2d3d" stroke-width="1"/>
    <text x="480" y="210" text-anchor="middle" font-size="9.5" fill="#1e2d3d" font-family="monospace">Xtensa LX6</text>
    <text x="480" y="226" text-anchor="middle" font-size="9"   fill="#1e2d3d" font-family="monospace">Dual-Core · 240 MHz</text>
    <text x="480" y="244" text-anchor="middle" font-size="9"   fill="#1e2d3d" font-family="monospace">4 MB Flash · 520 KB SRAM</text>
    <text x="480" y="262" text-anchor="middle" font-size="9"   fill="#1e2d3d" font-family="monospace">Wi-Fi 802.11 b/g/n</text>

    <!-- Left pins -->
    <text x="352" y="141" font-size="7.5" fill="#06b6d460" font-family="monospace" letter-spacing="1">I²C</text>
    <circle cx="345" cy="152" r="4.5" fill="#06b6d4"/>
    <text x="340" y="156" text-anchor="end" font-size="8.5" fill="#cbd5e1" font-family="monospace">22</text>
    <text x="349" y="156" font-size="8" fill="#06b6d4" font-family="monospace">SDA</text>
    <circle cx="345" cy="177" r="4.5" fill="#06b6d4"/>
    <text x="340" y="181" text-anchor="end" font-size="8.5" fill="#cbd5e1" font-family="monospace">23</text>
    <text x="349" y="181" font-size="8" fill="#06b6d4" font-family="monospace">SCL</text>

    <text x="352" y="256" font-size="7.5" fill="#a855f760" font-family="monospace" letter-spacing="1">I²S</text>
    <circle cx="345" cy="268" r="4.5" fill="#a855f7"/>
    <text x="340" y="272" text-anchor="end" font-size="8.5" fill="#cbd5e1" font-family="monospace">27</text>
    <text x="349" y="272" font-size="8" fill="#a855f7" font-family="monospace">BCLK</text>
    <circle cx="345" cy="293" r="4.5" fill="#a855f7"/>
    <text x="340" y="297" text-anchor="end" font-size="8.5" fill="#cbd5e1" font-family="monospace">14</text>
    <text x="349" y="297" font-size="8" fill="#a855f7" font-family="monospace">WS</text>
    <circle cx="345" cy="318" r="4.5" fill="#a855f7"/>
    <text x="340" y="322" text-anchor="end" font-size="8.5" fill="#cbd5e1" font-family="monospace">26</text>
    <text x="349" y="322" font-size="8" fill="#a855f7" font-family="monospace">SD</text>

    <circle cx="345" cy="355" r="4.5" fill="#22c55e"/>
    <text x="340" y="359" text-anchor="end" font-size="8.5" fill="#22c55e" font-family="monospace">3V3</text>
    <circle cx="345" cy="378" r="4.5" fill="#334155"/>
    <text x="340" y="382" text-anchor="end" font-size="8.5" fill="#475569" font-family="monospace">GND</text>

    <!-- Right pins -->
    <text x="609" y="141" text-anchor="end" font-size="7.5" fill="#f9731660" font-family="monospace" letter-spacing="1">UART</text>
    <circle cx="615" cy="152" r="4.5" fill="#f97316"/>
    <text x="620" y="156" font-size="8.5" fill="#cbd5e1" font-family="monospace">16</text>
    <text x="611" y="156" text-anchor="end" font-size="8" fill="#f97316" font-family="monospace">RX</text>
    <circle cx="615" cy="177" r="4.5" fill="#f97316"/>
    <text x="620" y="181" font-size="8.5" fill="#cbd5e1" font-family="monospace">17</text>
    <text x="611" y="181" text-anchor="end" font-size="8" fill="#f97316" font-family="monospace">TX</text>

    <text x="609" y="282" text-anchor="end" font-size="7.5" fill="#ef444460" font-family="monospace" letter-spacing="1">BTN</text>
    <circle cx="615" cy="293" r="4.5" fill="#ef4444"/>
    <text x="620" y="297" font-size="8.5" fill="#cbd5e1" font-family="monospace">33</text>
    <text x="611" y="297" text-anchor="end" font-size="8" fill="#ef4444" font-family="monospace">SIG</text>

    <circle cx="615" cy="355" r="4.5" fill="#eab308"/>
    <text x="620" y="359" font-size="8.5" fill="#eab308" font-family="monospace">5V</text>
    <circle cx="615" cy="378" r="4.5" fill="#334155"/>
    <text x="620" y="382" font-size="8.5" fill="#475569" font-family="monospace">GND</text>

  </svg>
</template>

<style scoped>
.pin-svg {
  display: block;
  width: 100%;
  min-width: 720px;
  height: auto;
}
</style>
