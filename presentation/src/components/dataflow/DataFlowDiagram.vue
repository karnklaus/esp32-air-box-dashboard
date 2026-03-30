<template>
  <svg viewBox="0 0 960 510" xmlns="http://www.w3.org/2000/svg" class="df-svg">
    <defs>
      <marker id="df-arr-orange" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto">
        <path d="M0,0 L0,6 L7,3 z" fill="#f97316"/>
      </marker>
      <marker id="df-arr-teal" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto">
        <path d="M0,0 L0,6 L7,3 z" fill="#06b6d4"/>
      </marker>
      <marker id="df-arr-red" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto">
        <path d="M0,0 L0,6 L7,3 z" fill="#ef4444"/>
      </marker>
      <marker id="df-arr-muted" markerWidth="8" markerHeight="8" refX="7" refY="3" orient="auto">
        <path d="M0,0 L0,6 L7,3 z" fill="#475569"/>
      </marker>
    </defs>

    <!-- ══════════════════════════════════════
         ZONE BACKGROUNDS
    ══════════════════════════════════════ -->
    <!-- ESP32 zone -->
    <rect x="4"   y="8" width="263" height="495" rx="10" fill="#06b6d406" stroke="#06b6d420" stroke-width="1"/>
    <text x="135" y="498" text-anchor="middle" font-size="8" fill="#06b6d440" font-family="monospace" letter-spacing="1">ESP32 DEVICE</text>

    <!-- Server zone -->
    <rect x="274" y="8" width="406" height="495" rx="10" fill="#22c55e06" stroke="#22c55e20" stroke-width="1"/>
    <text x="477" y="498" text-anchor="middle" font-size="8" fill="#22c55e40" font-family="monospace" letter-spacing="1">FLASK SERVER</text>

    <!-- Output zone -->
    <rect x="687" y="8" width="269" height="495" rx="10" fill="#8b5cf606" stroke="#8b5cf620" stroke-width="1"/>
    <text x="821" y="498" text-anchor="middle" font-size="8" fill="#8b5cf640" font-family="monospace" letter-spacing="1">OUTPUT</text>

    <!-- ══════════════════════════════════════
         ESP32 — Flash Storage box
    ══════════════════════════════════════ -->
    <rect x="16" y="22" width="240" height="172" rx="8" fill="#0f1623" stroke="#06b6d4" stroke-width="1.5"/>
    <rect x="16" y="22" width="240" height="30" rx="8" fill="#06b6d420"/>
    <rect x="16" y="38" width="240" height="14" fill="#06b6d420"/>
    <text x="30" y="41" font-size="9" fill="#06b6d4" font-weight="600" letter-spacing="1" font-family="monospace">FLASH STORAGE  (Preferences)</text>
    <text x="30" y="60" font-size="8.5" fill="#64748b" font-family="monospace">Persists across reboots</text>
    <line x1="30" y1="65" x2="246" y2="65" stroke="#1e2d3d" stroke-width="1"/>
    <!-- fields -->
    <text x="30" y="80"  font-size="9.5" fill="#94a3b8" font-family="monospace">box_id        string</text>
    <text x="30" y="96"  font-size="9.5" fill="#94a3b8" font-family="monospace">wifi_ssid     string</text>
    <text x="30" y="112" font-size="9.5" fill="#94a3b8" font-family="monospace">wifi_password string</text>
    <text x="30" y="128" font-size="9.5" fill="#94a3b8" font-family="monospace">server_ip     string</text>
    <text x="30" y="144" font-size="9.5" fill="#94a3b8" font-family="monospace">server_port   uint16</text>
    <line x1="30" y1="153" x2="246" y2="153" stroke="#1e2d3d" stroke-width="1"/>
    <text x="30" y="165" font-size="8.5" fill="#64748b" font-family="monospace" font-style="italic">Editable via Config Mode portal</text>
    <text x="30" y="181" font-size="8.5" fill="#06b6d480" font-family="monospace">→ not sent to server</text>

    <!-- ══════════════════════════════════════
         ESP32 — POST Payload box
    ══════════════════════════════════════ -->
    <rect x="16" y="210" width="240" height="218" rx="8" fill="#0f1623" stroke="#f97316" stroke-width="1.5"/>
    <rect x="16" y="210" width="240" height="30" rx="8" fill="#f9731620"/>
    <rect x="16" y="226" width="240" height="14" fill="#f9731620"/>
    <text x="30" y="229" font-size="9" fill="#f97316" font-weight="600" letter-spacing="1" font-family="monospace">POST PAYLOAD  (every 3s)</text>
    <text x="30" y="252" font-size="8.5" fill="#64748b" font-family="monospace">/api/air-boxes/report</text>
    <line x1="30" y1="257" x2="246" y2="257" stroke="#1e2d3d" stroke-width="1"/>
    <!-- fields with types -->
    <text x="30"  y="272" font-size="9.5" fill="#e2e8f0" font-family="monospace">id</text>
    <text x="246" y="272" text-anchor="end" font-size="8.5" fill="#64748b" font-family="monospace">string</text>
    <text x="30"  y="288" font-size="9.5" fill="#e2e8f0" font-family="monospace">pm1_0</text>
    <text x="246" y="288" text-anchor="end" font-size="8.5" fill="#64748b" font-family="monospace">int</text>
    <text x="30"  y="304" font-size="9.5" fill="#e2e8f0" font-family="monospace">pm2_5</text>
    <text x="246" y="304" text-anchor="end" font-size="8.5" fill="#64748b" font-family="monospace">int</text>
    <text x="30"  y="320" font-size="9.5" fill="#e2e8f0" font-family="monospace">pm10</text>
    <text x="246" y="320" text-anchor="end" font-size="8.5" fill="#64748b" font-family="monospace">int</text>
    <text x="30"  y="336" font-size="9.5" fill="#94a3b8" font-family="monospace">temperature_c</text>
    <text x="246" y="336" text-anchor="end" font-size="8.5" fill="#64748b" font-family="monospace">float | null</text>
    <text x="30"  y="352" font-size="9.5" fill="#94a3b8" font-family="monospace">humidity_rh</text>
    <text x="246" y="352" text-anchor="end" font-size="8.5" fill="#64748b" font-family="monospace">float | null</text>
    <text x="30"  y="368" font-size="9.5" fill="#e2e8f0" font-family="monospace">sound_state</text>
    <text x="246" y="368" text-anchor="end" font-size="8.5" fill="#64748b" font-family="monospace">"LOUD"|"QUIET"</text>
    <line x1="30" y1="376" x2="246" y2="376" stroke="#1e2d3d" stroke-width="1"/>
    <text x="30"  y="390" font-size="8.5" fill="#f9731680" font-family="monospace" font-style="italic">max in 3s window, then reset</text>
    <text x="30"  y="403" font-size="8.5" fill="#f9731680" font-family="monospace" font-style="italic">temperature/humidity via AM2320</text>

    <!-- ══════════════════════════════════════
         MAIN POST ARROW
    ══════════════════════════════════════ -->
    <line x1="260" y1="318" x2="366" y2="318"
          stroke="#f97316" stroke-width="2.5" marker-end="url(#df-arr-orange)"/>
    <text x="313" y="312" text-anchor="middle" font-size="8" fill="#f97316" font-family="monospace" font-weight="600">POST</text>
    <text x="313" y="330" text-anchor="middle" font-size="7.5" fill="#f9731670" font-family="monospace">JSON</text>

    <!-- ══════════════════════════════════════
         SERVER — air_boxes.json
    ══════════════════════════════════════ -->
    <rect x="282" y="22" width="396" height="230" rx="8" fill="#0f1623" stroke="#22c55e" stroke-width="1.5"/>
    <rect x="282" y="22" width="396" height="30" rx="8" fill="#22c55e20"/>
    <rect x="282" y="38" width="396" height="14" fill="#22c55e20"/>
    <text x="296" y="41" font-size="9" fill="#22c55e" font-weight="600" letter-spacing="1" font-family="monospace">air_boxes.json  (live snapshot)</text>
    <text x="296" y="60" font-size="8.5" fill="#64748b" font-family="monospace">1 entry per box · updated on every POST</text>
    <line x1="296" y1="65" x2="668" y2="65" stroke="#1e2d3d" stroke-width="1"/>

    <!-- two column layout for fields -->
    <!-- col left -->
    <text x="296" y="81"  font-size="9.5" fill="#e2e8f0" font-family="monospace">id</text>
    <text x="296" y="97"  font-size="9.5" fill="#f97316" font-family="monospace">pm1_0, pm2_5, pm10</text>
    <text x="296" y="113" font-size="9.5" fill="#94a3b8" font-family="monospace">temperature_c</text>
    <text x="296" y="129" font-size="9.5" fill="#94a3b8" font-family="monospace">humidity_rh</text>
    <text x="296" y="145" font-size="9.5" fill="#f97316" font-family="monospace">sound_state</text>
    <text x="296" y="161" font-size="9.5" fill="#22c55e" font-family="monospace">last_seen</text>
    <!-- col right -->
    <text x="490" y="81"  font-size="9.5" fill="#64748b" font-family="monospace">from device</text>
    <text x="490" y="97"  font-size="9.5" fill="#64748b" font-family="monospace">from POST · updated</text>
    <text x="490" y="113" font-size="9.5" fill="#64748b" font-family="monospace">from POST · nullable</text>
    <text x="490" y="129" font-size="9.5" fill="#64748b" font-family="monospace">from POST · nullable</text>
    <text x="490" y="145" font-size="9.5" fill="#64748b" font-family="monospace">from POST · updated</text>
    <text x="490" y="161" font-size="9.5" fill="#22c55e" font-family="monospace">UTC ISO · server sets</text>
    <!-- server-only fields -->
    <line x1="296" y1="169" x2="668" y2="169" stroke="#1e2d3d" stroke-width="0.5" stroke-dasharray="4 3"/>
    <text x="296" y="182" font-size="9.5" fill="#475569" font-family="monospace">name, district</text>
    <text x="490" y="182" font-size="9.5" fill="#475569" font-family="monospace">server-configured only</text>
    <text x="296" y="198" font-size="9.5" fill="#475569" font-family="monospace">lat, lng</text>
    <text x="490" y="198" font-size="9.5" fill="#475569" font-family="monospace">server-configured only</text>
    <line x1="296" y1="206" x2="668" y2="206" stroke="#1e2d3d" stroke-width="1"/>
    <text x="296" y="220" font-size="8.5" fill="#22c55e80" font-family="monospace" font-style="italic">metadata locked after first POST — edit via PATCH /api/air-boxes/:id</text>
    <text x="296" y="235" font-size="8.5" fill="#22c55e80" font-family="monospace" font-style="italic">online = last_seen within 10s</text>

    <!-- ══════════════════════════════════════
         SERVER — air_box_history.json
    ══════════════════════════════════════ -->
    <rect x="282" y="265" width="396" height="155" rx="8" fill="#0f1623" stroke="#22c55e" stroke-width="1.5" stroke-dasharray="6 3"/>
    <rect x="282" y="265" width="396" height="30" rx="8" fill="#22c55e15"/>
    <rect x="282" y="281" width="396" height="14" fill="#22c55e15"/>
    <text x="296" y="284" font-size="9" fill="#22c55e" font-weight="600" letter-spacing="1" font-family="monospace">air_box_history.json  (time series)</text>
    <text x="296" y="305" font-size="8.5" fill="#64748b" font-family="monospace">appended each POST · max 240 entries per box (FIFO)</text>
    <line x1="296" y1="310" x2="668" y2="310" stroke="#1e2d3d" stroke-width="1"/>
    <text x="296" y="325" font-size="9.5" fill="#94a3b8" font-family="monospace">id, timestamp</text>
    <text x="296" y="341" font-size="9.5" fill="#94a3b8" font-family="monospace">pm1_0, pm2_5, pm10</text>
    <text x="296" y="357" font-size="9.5" fill="#94a3b8" font-family="monospace">temperature_c, humidity_rh</text>
    <text x="296" y="373" font-size="9.5" fill="#94a3b8" font-family="monospace">sound_state</text>
    <line x1="296" y1="381" x2="668" y2="381" stroke="#1e2d3d" stroke-width="1"/>
    <text x="296" y="395" font-size="8.5" fill="#22c55e80" font-family="monospace" font-style="italic">exported via GET /api/overview/export.csv</text>
    <text x="296" y="409" font-size="8.5" fill="#22c55e80" font-family="monospace" font-style="italic">queried via GET /api/overview/history?box_id=&amp;limit=</text>

    <!-- ══════════════════════════════════════
         SERVER — server_settings.json
    ══════════════════════════════════════ -->
    <rect x="282" y="434" width="396" height="58" rx="8" fill="#0f1623" stroke="#64748b" stroke-width="1"/>
    <text x="296" y="452" font-size="9" fill="#64748b" font-weight="600" letter-spacing="1" font-family="monospace">server_settings.json</text>
    <text x="296" y="468" font-size="9" fill="#475569" font-family="monospace">alert_pm1/pm25/pm10_threshold · alert_offline_seconds</text>
    <text x="296" y="481" font-size="9" fill="#475569" font-family="monospace">alert_loud_cooldown_seconds · discord_webhook_url</text>

    <!-- ══════════════════════════════════════
         SIDE ARROWS (server → output zone)
    ══════════════════════════════════════ -->
    <!-- air_boxes → Alert Engine -->
    <line x1="682" y1="120" x2="760" y2="120"
          stroke="#ef4444" stroke-width="1.5" marker-end="url(#df-arr-red)"/>
    <text x="721" y="113" text-anchor="middle" font-size="7.5" fill="#ef4444" font-family="monospace">triggers</text>

    <!-- history → Browser -->
    <line x1="682" y1="340" x2="760" y2="340"
          stroke="#06b6d4" stroke-width="1.5" marker-end="url(#df-arr-teal)"/>
    <text x="721" y="333" text-anchor="middle" font-size="7.5" fill="#06b6d4" font-family="monospace">serves</text>

    <!-- ══════════════════════════════════════
         OUTPUT — Alert Engine
    ══════════════════════════════════════ -->
    <rect x="696" y="22" width="255" height="230" rx="8" fill="#0f1623" stroke="#ef4444" stroke-width="1.5"/>
    <rect x="696" y="22" width="255" height="30" rx="8" fill="#ef444420"/>
    <rect x="696" y="38" width="255" height="14" fill="#ef444420"/>
    <text x="710" y="41" font-size="9" fill="#ef4444" font-weight="600" letter-spacing="1" font-family="monospace">ALERT ENGINE</text>

    <text x="710" y="62" font-size="9" fill="#64748b" font-weight="600" letter-spacing="1" font-family="monospace">LIVE (per POST)</text>
    <line x1="710" y1="66" x2="941" y2="66" stroke="#1e2d3d" stroke-width="1"/>
    <text x="710" y="80"  font-size="9" fill="#94a3b8" font-family="monospace">PM HIGH    rise-edge only</text>
    <text x="710" y="95"  font-size="9" fill="#94a3b8" font-family="monospace">LOUD       cooldown 60s</text>

    <text x="710" y="114" font-size="9" fill="#64748b" font-weight="600" letter-spacing="1" font-family="monospace">BACKGROUND  (every 5s)</text>
    <line x1="710" y1="118" x2="941" y2="118" stroke="#1e2d3d" stroke-width="1"/>
    <text x="710" y="132" font-size="9" fill="#94a3b8" font-family="monospace">OFFLINE    last_seen &gt; threshold</text>
    <text x="710" y="147" font-size="9" fill="#94a3b8" font-family="monospace">ONLINE     recovery detect</text>

    <line x1="710" y1="157" x2="941" y2="157" stroke="#1e2d3d" stroke-width="1"/>
    <text x="710" y="171" font-size="9" fill="#64748b" font-weight="600" letter-spacing="1" font-family="monospace">OUTPUT</text>
    <line x1="710" y1="175" x2="941" y2="175" stroke="#1e2d3d" stroke-width="1"/>
    <!-- Discord pill -->
    <rect x="710" y="181" width="230" height="28" rx="5" fill="#5865f210" stroke="#5865f240" stroke-width="1"/>
    <text x="722" y="196" font-size="9.5" fill="#7289da" font-family="monospace" font-weight="600">Discord Webhook POST</text>
    <text x="722" y="207" font-size="8" fill="#475569" font-family="monospace">box label · type · PM values</text>
    <!-- warmup note -->
    <line x1="710" y1="218" x2="941" y2="218" stroke="#1e2d3d" stroke-width="1"/>
    <text x="710" y="231" font-size="8" fill="#475569" font-family="monospace" font-style="italic">10s warmup on server start</text>
    <text x="710" y="243" font-size="8" fill="#475569" font-family="monospace" font-style="italic">alert_state.json tracks edge state</text>

    <!-- ══════════════════════════════════════
         OUTPUT — Browser / API
    ══════════════════════════════════════ -->
    <rect x="696" y="268" width="255" height="158" rx="8" fill="#0f1623" stroke="#8b5cf6" stroke-width="1.5"/>
    <rect x="696" y="268" width="255" height="30" rx="8" fill="#8b5cf620"/>
    <rect x="696" y="284" width="255" height="14" fill="#8b5cf620"/>
    <text x="710" y="287" font-size="9" fill="#8b5cf6" font-weight="600" letter-spacing="1" font-family="monospace">BROWSER / API</text>
    <line x1="710" y1="302" x2="941" y2="302" stroke="#1e2d3d" stroke-width="1"/>
    <text x="710" y="316" font-size="9"   fill="#94a3b8" font-family="monospace">GET /api/air-boxes</text>
    <text x="938" y="316" text-anchor="end" font-size="8" fill="#475569" font-family="monospace">poll 10s</text>
    <text x="710" y="332" font-size="9"   fill="#94a3b8" font-family="monospace">GET /api/overview/history</text>
    <text x="938" y="332" text-anchor="end" font-size="8" fill="#475569" font-family="monospace">on demand</text>
    <text x="710" y="348" font-size="9"   fill="#94a3b8" font-family="monospace">GET /api/overview/export.csv</text>
    <text x="938" y="348" text-anchor="end" font-size="8" fill="#475569" font-family="monospace">download</text>
    <text x="710" y="364" font-size="9"   fill="#94a3b8" font-family="monospace">PATCH /api/air-boxes/:id</text>
    <text x="938" y="364" text-anchor="end" font-size="8" fill="#475569" font-family="monospace">edit meta</text>
    <line x1="710" y1="372" x2="941" y2="372" stroke="#1e2d3d" stroke-width="1"/>
    <text x="710" y="386" font-size="8.5" fill="#8b5cf680" font-family="monospace" font-style="italic">No WebSocket — pure HTTP polling</text>
    <text x="710" y="400" font-size="8.5" fill="#8b5cf680" font-family="monospace" font-style="italic">Leaflet.js · SVG charts</text>

    <!-- browser ← server (poll arrow back) -->
    <line x1="760" y1="395" x2="682" y2="395"
          stroke="#475569" stroke-width="1.2" stroke-dasharray="5 3" marker-end="url(#df-arr-muted)"/>
    <text x="721" y="407" text-anchor="middle" font-size="7.5" fill="#475569" font-family="monospace">poll ←</text>

  </svg>
</template>

<style scoped>
.df-svg {
  display: block;
  width: 100%;
  min-width: 800px;
  height: auto;
}
</style>
