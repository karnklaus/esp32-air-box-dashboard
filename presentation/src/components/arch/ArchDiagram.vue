<template>
  <svg viewBox="0 0 940 430" xmlns="http://www.w3.org/2000/svg" class="arch-svg">
    <defs>
      <marker id="arr-orange" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto">
        <path d="M0,0 L0,6 L7,3 z" fill="#f97316"/>
      </marker>
      <marker id="arr-teal" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto">
        <path d="M0,0 L0,6 L7,3 z" fill="#06b6d4"/>
      </marker>
      <marker id="arr-muted" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto">
        <path d="M0,0 L0,6 L7,3 z" fill="#475569"/>
      </marker>
    </defs>

    <!-- ══════════════════════════════════════════════
         ESP32 AIR BOX  (left column)
    ══════════════════════════════════════════════ -->

    <!-- stack shadow (multiple devices hint) -->
    <rect x="22" y="57" width="200" height="302" rx="9" fill="#0a111c" stroke="#1e3a4a" stroke-width="1" opacity="0.7"/>

    <!-- main card -->
    <rect x="10" y="44" width="200" height="302" rx="9" fill="#0f1623" stroke="#06b6d4" stroke-width="1.5"/>
    <!-- header band -->
    <rect x="10" y="44" width="200" height="34" rx="9" fill="#06b6d420"/>
    <rect x="10" y="64"  width="200" height="14" fill="#06b6d420"/>
    <text x="110" y="66" text-anchor="middle" font-size="11" fill="#06b6d4" font-weight="700" letter-spacing="1" font-family="monospace">ESP32 Air Box</text>

    <!-- sensor list -->
    <text x="28" y="103" font-size="9" fill="#64748b" font-weight="600" letter-spacing="1" font-family="monospace">SENSORS</text>
    <line x1="28" y1="108" x2="200" y2="108" stroke="#1e2d3d" stroke-width="1"/>

    <!-- PMS5003 -->
    <rect x="26" y="115" width="176" height="28" rx="5" fill="#f9731610" stroke="#f9731630" stroke-width="1"/>
    <text x="38" y="127" font-size="9" fill="#f97316" font-family="monospace" font-weight="600">PMS5003</text>
    <text x="38" y="138" font-size="8.5" fill="#64748b" font-family="monospace">PM1.0 · PM2.5 · PM10</text>
    <text x="192" y="132" text-anchor="end" font-size="7.5" fill="#f9731680" font-family="monospace">UART</text>

    <!-- INMP441 -->
    <rect x="26" y="148" width="176" height="28" rx="5" fill="#a855f710" stroke="#a855f730" stroke-width="1"/>
    <text x="38" y="160" font-size="9" fill="#a855f7" font-family="monospace" font-weight="600">INMP441</text>
    <text x="38" y="171" font-size="8.5" fill="#64748b" font-family="monospace">LOUD / QUIET</text>
    <text x="192" y="165" text-anchor="end" font-size="7.5" fill="#a855f780" font-family="monospace">I2S</text>

    <!-- AM2320 + OLED -->
    <rect x="26" y="181" width="176" height="28" rx="5" fill="#06b6d410" stroke="#06b6d430" stroke-width="1"/>
    <text x="38" y="193" font-size="9" fill="#06b6d4" font-family="monospace" font-weight="600">OLED + AM2320</text>
    <text x="38" y="204" font-size="8.5" fill="#64748b" font-family="monospace">Display · Temp/Hum</text>
    <text x="192" y="198" text-anchor="end" font-size="7.5" fill="#06b6d480" font-family="monospace">I2C</text>

    <!-- Red Button -->
    <rect x="26" y="214" width="176" height="22" rx="5" fill="#ef444410" stroke="#ef444430" stroke-width="1"/>
    <text x="38" y="229" font-size="9" fill="#ef4444" font-family="monospace" font-weight="600">Red Button (GPIO33)</text>

    <!-- report interval -->
    <line x1="26" y1="246" x2="200" y2="246" stroke="#1e2d3d" stroke-width="1"/>
    <text x="28" y="261" font-size="9" fill="#64748b" font-weight="600" letter-spacing="1" font-family="monospace">REPORTS</text>
    <text x="28" y="278" font-size="9" fill="#94a3b8" font-family="monospace">POST  every  3 seconds</text>
    <text x="28" y="294" font-size="9" fill="#64748b" font-family="monospace">id · pm1_0 · pm2_5</text>
    <text x="28" y="309" font-size="9" fill="#64748b" font-family="monospace">pm10 · sound_state</text>

    <!-- multiple devices badge -->
    <rect x="26" y="322" width="176" height="16" rx="4" fill="#06b6d415"/>
    <text x="114" y="334" text-anchor="middle" font-size="8.5" fill="#06b6d4" font-family="monospace">× Multiple Air Boxes supported</text>

    <!-- ══════════════════════════════════════════════
         ARROWS: ESP32 → Flask
    ══════════════════════════════════════════════ -->

    <!-- POST arrow -->
    <line x1="216" y1="160" x2="314" y2="160"
          stroke="#f97316" stroke-width="2" marker-end="url(#arr-orange)"/>
    <text x="265" y="153" text-anchor="middle" font-size="8.5" fill="#f97316" font-family="monospace" font-weight="600">POST /report</text>
    <text x="265" y="175" text-anchor="middle" font-size="8" fill="#f9731680" font-family="monospace">every 3s</text>

    <!-- Config pull (reverse, dashed) -->
    <line x1="314" y1="245" x2="216" y2="245"
          stroke="#475569" stroke-width="1.2" stroke-dasharray="5 3" marker-end="url(#arr-muted)"/>
    <text x="265" y="240" text-anchor="middle" font-size="7.5" fill="#475569" font-family="monospace">Config Mode AP</text>

    <!-- ══════════════════════════════════════════════
         FLASK SERVER  (center column)
    ══════════════════════════════════════════════ -->

    <rect x="320" y="24" width="268" height="382" rx="9" fill="#0f1623" stroke="#22c55e" stroke-width="1.5"/>
    <!-- header -->
    <rect x="320" y="24" width="268" height="34" rx="9" fill="#22c55e20"/>
    <rect x="320" y="44"  width="268" height="14" fill="#22c55e20"/>
    <text x="454" y="46" text-anchor="middle" font-size="11" fill="#22c55e" font-weight="700" letter-spacing="1" font-family="monospace">Flask Server</text>
    <text x="454" y="63" text-anchor="middle" font-size="8.5" fill="#22c55e80" font-family="monospace">Python · app.py · port 5000</text>

    <!-- Routes section -->
    <text x="338" y="82" font-size="9" fill="#64748b" font-weight="600" letter-spacing="1" font-family="monospace">ROUTES</text>
    <line x1="338" y1="87" x2="576" y2="87" stroke="#1e2d3d" stroke-width="1"/>

    <text x="338" y="102" font-size="9.5" fill="#94a3b8" font-family="monospace">GET  /dashboard</text>
    <text x="338" y="117" font-size="9.5" fill="#94a3b8" font-family="monospace">GET  /overview</text>
    <text x="338" y="132" font-size="9.5" fill="#94a3b8" font-family="monospace">GET  /config</text>
    <text x="338" y="147" font-size="9.5" fill="#22c55e" font-family="monospace">POST /api/air-boxes/report</text>
    <text x="338" y="162" font-size="9.5" fill="#94a3b8" font-family="monospace">GET  /api/air-boxes</text>
    <text x="338" y="177" font-size="9.5" fill="#94a3b8" font-family="monospace">GET  /api/overview/history</text>
    <text x="338" y="192" font-size="9.5" fill="#94a3b8" font-family="monospace">GET  /api/overview/export.csv</text>
    <text x="338" y="207" font-size="9.5" fill="#94a3b8" font-family="monospace">PATCH/DELETE /api/air-boxes/:id</text>

    <!-- Data section -->
    <line x1="338" y1="220" x2="576" y2="220" stroke="#1e2d3d" stroke-width="1"/>
    <text x="338" y="234" font-size="9" fill="#64748b" font-weight="600" letter-spacing="1" font-family="monospace">DATA  (data/)</text>
    <line x1="338" y1="239" x2="576" y2="239" stroke="#1e2d3d" stroke-width="1"/>

    <rect x="336" y="246" width="244" height="22" rx="4" fill="#16202f"/>
    <text x="348" y="261" font-size="9" fill="#94a3b8" font-family="monospace">air_boxes.json</text>
    <text x="574" y="261" text-anchor="end" font-size="8" fill="#64748b" font-family="monospace">latest snapshot</text>

    <rect x="336" y="272" width="244" height="22" rx="4" fill="#16202f"/>
    <text x="348" y="287" font-size="9" fill="#94a3b8" font-family="monospace">air_box_history.json</text>
    <text x="574" y="287" text-anchor="end" font-size="8" fill="#64748b" font-family="monospace">240 records/box</text>

    <rect x="336" y="298" width="244" height="22" rx="4" fill="#16202f"/>
    <text x="348" y="313" font-size="9" fill="#94a3b8" font-family="monospace">server_settings.json</text>
    <text x="574" y="313" text-anchor="end" font-size="8" fill="#64748b" font-family="monospace">alerts · thresholds</text>

    <!-- Background monitor section -->
    <line x1="338" y1="332" x2="576" y2="332" stroke="#1e2d3d" stroke-width="1"/>
    <text x="338" y="346" font-size="9" fill="#64748b" font-weight="600" letter-spacing="1" font-family="monospace">BACKGROUND THREAD</text>
    <line x1="338" y1="351" x2="576" y2="351" stroke="#1e2d3d" stroke-width="1"/>
    <rect x="336" y="358" width="244" height="38" rx="4" fill="#16202f" stroke="#1e2d3d" stroke-width="1"/>
    <text x="348" y="373" font-size="9" fill="#94a3b8" font-family="monospace">Status monitor (every 5s)</text>
    <text x="348" y="388" font-size="9" fill="#94a3b8" font-family="monospace">Discord webhook alerts</text>

    <!-- ══════════════════════════════════════════════
         ARROWS: Flask ↔ Browser
    ══════════════════════════════════════════════ -->

    <!-- GET pages (Flask → Browser) -->
    <line x1="594" y1="125" x2="660" y2="125"
          stroke="#06b6d4" stroke-width="2" marker-end="url(#arr-teal)"/>
    <text x="627" y="118" text-anchor="middle" font-size="8" fill="#06b6d4" font-family="monospace">HTML pages</text>

    <!-- GET api poll (Browser → Flask) -->
    <line x1="660" y1="158" x2="594" y2="158"
          stroke="#475569" stroke-width="1.5" marker-end="url(#arr-muted)"/>
    <text x="627" y="173" text-anchor="middle" font-size="8" fill="#475569" font-family="monospace">GET /api  (poll 10s)</text>

    <!-- ══════════════════════════════════════════════
         BROWSER  (right column)
    ══════════════════════════════════════════════ -->

    <rect x="666" y="68" width="264" height="278" rx="9" fill="#0f1623" stroke="#8b5cf6" stroke-width="1.5"/>
    <!-- header -->
    <rect x="666" y="68" width="264" height="34" rx="9" fill="#8b5cf620"/>
    <rect x="666" y="88"  width="264" height="14" fill="#8b5cf620"/>
    <text x="798" y="90" text-anchor="middle" font-size="11" fill="#8b5cf6" font-weight="700" letter-spacing="1" font-family="monospace">Web Browser</text>

    <!-- pages -->
    <text x="684" y="122" font-size="9" fill="#64748b" font-weight="600" letter-spacing="1" font-family="monospace">PAGES</text>
    <line x1="684" y1="127" x2="918" y2="127" stroke="#1e2d3d" stroke-width="1"/>

    <rect x="682" y="134" width="230" height="40" rx="5" fill="#16202f" stroke="#1e2d3d" stroke-width="1"/>
    <text x="694" y="149" font-size="9.5" fill="#e2e8f0" font-weight="600">/dashboard</text>
    <text x="694" y="165" font-size="8.5" fill="#64748b">Bangkok Map · Air Box cards · Live PM</text>

    <rect x="682" y="178" width="230" height="40" rx="5" fill="#16202f" stroke="#1e2d3d" stroke-width="1"/>
    <text x="694" y="193" font-size="9.5" fill="#e2e8f0" font-weight="600">/overview</text>
    <text x="694" y="209" font-size="8.5" fill="#64748b">PM charts · History trend · CSV export</text>

    <rect x="682" y="222" width="230" height="40" rx="5" fill="#16202f" stroke="#1e2d3d" stroke-width="1"/>
    <text x="694" y="237" font-size="9.5" fill="#e2e8f0" font-weight="600">/config</text>
    <text x="694" y="253" font-size="8.5" fill="#64748b">Thresholds · Discord webhook</text>

    <!-- polling note -->
    <line x1="684" y1="272" x2="918" y2="272" stroke="#1e2d3d" stroke-width="1"/>
    <text x="684" y="286" font-size="8.5" fill="#94a3b8" font-family="monospace">JS polls /api/air-boxes every 10s</text>
    <text x="684" y="300" font-size="8.5" fill="#64748b" font-family="monospace">Leaflet.js · Chart.js · SVG</text>
    <text x="684" y="335" font-size="8.5" fill="#64748b" font-family="monospace">No WebSocket — pure HTTP</text>

  </svg>
</template>

<style scoped>
.arch-svg {
  display: block;
  width: 100%;
  min-width: 760px;
  height: auto;
}
</style>
