<!-- Pixel-accurate mockup of the 128×64 OLED display -->
<script setup>
defineProps({
  /** 'config-info' | 'config-qr' | 'pm' | 'connecting' */
  screen: { type: String, default: 'config-info' },
})
</script>

<template>
  <div class="oled-frame">
    <svg viewBox="0 0 128 64" class="oled-screen" xmlns="http://www.w3.org/2000/svg">

      <!-- Outer border (renderHeader draws rect 0,0 128,64) -->
      <rect x="0" y="0" width="128" height="64" fill="#06100a" stroke="#1a3020" stroke-width="0.5"/>
      <rect x="1" y="1" width="126" height="62" fill="none" stroke="#2ecc5030" stroke-width="0.5"/>

      <!-- ── CONFIG INFO screen ── -->
      <template v-if="screen === 'config-info'">
        <text x="6"  y="12" class="px">CONFIG MODE</text>
        <line x1="4" y1="15" x2="124" y2="15" stroke="#2ecc5040" stroke-width="0.5"/>
        <text x="6"  y="24" class="px dim">SSID</text>
        <text x="30" y="24" class="px">AIRBOX-3C4E5F</text>
        <text x="6"  y="34" class="px dim">WEB </text>
        <text x="30" y="34" class="px">192.168.4.1</text>
        <line x1="4" y1="41" x2="124" y2="41" stroke="#2ecc5040" stroke-width="0.5"/>
        <text x="6"  y="50" class="px dim">STATUS</text>
        <text x="6"  y="59" class="px hi">READY</text>
      </template>

      <!-- ── CONFIG QR screen ── -->
      <template v-else-if="screen === 'config-qr'">
        <!-- QR finder: top-left 7×7 modules at 2px each, origin (4,4) -->
        <rect x="4"  y="4"  width="14" height="14" fill="#c8f0c8"/>
        <rect x="6"  y="6"  width="10" height="10" fill="#06100a"/>
        <rect x="8"  y="8"  width="6"  height="6"  fill="#c8f0c8"/>
        <!-- QR finder: bottom-left at (4,46) -->
        <rect x="4"  y="46" width="14" height="14" fill="#c8f0c8"/>
        <rect x="6"  y="48" width="10" height="10" fill="#06100a"/>
        <rect x="8"  y="50" width="6"  height="6"  fill="#c8f0c8"/>
        <!-- QR finder: top-right at (46,4) (roughly) -->
        <rect x="46" y="4"  width="14" height="14" fill="#c8f0c8"/>
        <rect x="48" y="6"  width="10" height="10" fill="#06100a"/>
        <rect x="50" y="8"  width="6"  height="6"  fill="#c8f0c8"/>
        <!-- timing pattern H -->
        <rect x="18" y="10" width="2" height="2" fill="#c8f0c8"/>
        <rect x="22" y="10" width="2" height="2" fill="#c8f0c8"/>
        <rect x="26" y="10" width="2" height="2" fill="#c8f0c8"/>
        <rect x="30" y="10" width="2" height="2" fill="#c8f0c8"/>
        <rect x="34" y="10" width="2" height="2" fill="#c8f0c8"/>
        <rect x="38" y="10" width="2" height="2" fill="#c8f0c8"/>
        <rect x="42" y="10" width="2" height="2" fill="#c8f0c8"/>
        <!-- timing pattern V -->
        <rect x="10" y="18" width="2" height="2" fill="#c8f0c8"/>
        <rect x="10" y="22" width="2" height="2" fill="#c8f0c8"/>
        <rect x="10" y="26" width="2" height="2" fill="#c8f0c8"/>
        <rect x="10" y="30" width="2" height="2" fill="#c8f0c8"/>
        <rect x="10" y="34" width="2" height="2" fill="#c8f0c8"/>
        <rect x="10" y="38" width="2" height="2" fill="#c8f0c8"/>
        <rect x="10" y="42" width="2" height="2" fill="#c8f0c8"/>
        <!-- data modules (random fill to look authentic) -->
        <rect x="18" y="18" width="2" height="2" fill="#c8f0c8"/>
        <rect x="22" y="20" width="2" height="2" fill="#c8f0c8"/>
        <rect x="26" y="18" width="2" height="2" fill="#c8f0c8"/>
        <rect x="30" y="22" width="2" height="2" fill="#c8f0c8"/>
        <rect x="34" y="20" width="2" height="2" fill="#c8f0c8"/>
        <rect x="38" y="18" width="2" height="2" fill="#c8f0c8"/>
        <rect x="42" y="22" width="2" height="2" fill="#c8f0c8"/>
        <rect x="18" y="26" width="2" height="2" fill="#c8f0c8"/>
        <rect x="24" y="28" width="2" height="2" fill="#c8f0c8"/>
        <rect x="28" y="26" width="2" height="2" fill="#c8f0c8"/>
        <rect x="32" y="30" width="2" height="2" fill="#c8f0c8"/>
        <rect x="36" y="28" width="2" height="2" fill="#c8f0c8"/>
        <rect x="40" y="26" width="2" height="2" fill="#c8f0c8"/>
        <rect x="22" y="34" width="2" height="2" fill="#c8f0c8"/>
        <rect x="26" y="36" width="2" height="2" fill="#c8f0c8"/>
        <rect x="30" y="34" width="2" height="2" fill="#c8f0c8"/>
        <rect x="34" y="38" width="2" height="2" fill="#c8f0c8"/>
        <rect x="40" y="36" width="2" height="2" fill="#c8f0c8"/>
        <rect x="20" y="40" width="2" height="2" fill="#c8f0c8"/>
        <rect x="28" y="42" width="2" height="2" fill="#c8f0c8"/>
        <rect x="36" y="40" width="2" height="2" fill="#c8f0c8"/>
        <rect x="44" y="42" width="2" height="2" fill="#c8f0c8"/>
        <rect x="18" y="50" width="2" height="2" fill="#c8f0c8"/>
        <rect x="22" y="52" width="2" height="2" fill="#c8f0c8"/>
        <rect x="30" y="50" width="2" height="2" fill="#c8f0c8"/>
        <rect x="36" y="52" width="2" height="2" fill="#c8f0c8"/>
        <rect x="42" y="50" width="2" height="2" fill="#c8f0c8"/>
        <rect x="46" y="54" width="2" height="2" fill="#c8f0c8"/>
        <!-- format module (alignment pattern near bottom-right of qr area) -->
        <rect x="48" y="48" width="6"  height="6"  fill="#c8f0c8"/>
        <rect x="50" y="50" width="2"  height="2"  fill="#06100a"/>
        <!-- right side labels -->
        <text x="68" y="20" class="px dim">WIFI</text>
        <text x="68" y="30" class="px">AIRBOX</text>
        <text x="68" y="40" class="px">3C4E5F</text>
        <line x1="66" y1="44" x2="126" y2="44" stroke="#2ecc5040" stroke-width="0.5"/>
        <text x="68" y="52" class="px dim">TAP BTN</text>
        <text x="68" y="60" class="px dim">FOR INFO</text>
      </template>

      <!-- ── CONNECTING screen ── -->
      <template v-else-if="screen === 'connecting'">
        <text x="6" y="12" class="px">CONFIG MODE</text>
        <line x1="4" y1="15" x2="124" y2="15" stroke="#2ecc5040" stroke-width="0.5"/>
        <text x="6" y="24" class="px dim">SSID</text>
        <text x="30" y="24" class="px">HOME-WIFI</text>
        <text x="6" y="34" class="px dim">WEB </text>
        <text x="30" y="34" class="px">192.168.4.1</text>
        <line x1="4" y1="41" x2="124" y2="41" stroke="#2ecc5040" stroke-width="0.5"/>
        <text x="6" y="50" class="px dim">STATUS</text>
        <text x="6" y="59" class="px blink">COMPLETE</text>
      </template>

      <!-- ── PM screen (normal mode) ── -->
      <template v-else-if="screen === 'pm'">
        <text x="6" y="12" class="px">PM DETECTOR</text>
        <line x1="4" y1="15" x2="124" y2="15" stroke="#2ecc5040" stroke-width="0.5"/>
        <!-- PM1.0 card -->
        <rect x="4"  y="20" width="38" height="30" fill="none" stroke="#2ecc5050" stroke-width="0.5"/>
        <text x="7"  y="28" class="px dim">PM1.0</text>
        <text x="10" y="43" class="px val">12</text>
        <!-- PM2.5 card -->
        <rect x="45" y="20" width="38" height="30" fill="none" stroke="#2ecc5050" stroke-width="0.5"/>
        <text x="48" y="28" class="px dim">PM2.5</text>
        <text x="53" y="43" class="px val">18</text>
        <!-- PM10 card -->
        <rect x="86" y="20" width="38" height="30" fill="none" stroke="#2ecc5050" stroke-width="0.5"/>
        <text x="89" y="28" class="px dim">PM10 </text>
        <text x="95" y="43" class="px val">20</text>
      </template>

    </svg>
  </div>
</template>

<style scoped>
.oled-frame {
  background: #020806;
  border: 3px solid #1a2a20;
  border-radius: 6px;
  padding: 10px;
  box-shadow:
    0 0 0 1px #0a1a10,
    0 0 20px #00ff4408,
    inset 0 0 10px #00000060;
  display: inline-block;
}

.oled-screen {
  display: block;
  width: 256px;   /* 128 * 2 — displayed at 2× */
  height: 128px;
  image-rendering: pixelated;
}

/* pixel text classes */
.px   { font-family: monospace; font-size: 6px; fill: #c8f0c8; }
.dim  { font-family: monospace; font-size: 6px; fill: #4a8060; }
.hi   { font-family: monospace; font-size: 6px; fill: #6effaa; }
.val  { font-family: monospace; font-size: 10px; font-weight: bold; fill: #c8f0c8; }
</style>
