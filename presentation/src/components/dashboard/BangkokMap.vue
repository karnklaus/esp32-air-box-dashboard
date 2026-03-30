<!-- Simplified Bangkok map with air box markers -->
<script setup>
import { computed } from 'vue'

const props = defineProps({
  boxes: { type: Array, default: () => [] },
})

/* Bangkok bounds used to project lat/lng → SVG coords */
const BKK = { minLng: 100.45, maxLng: 100.65, minLat: 13.65, maxLat: 13.90 }
const W = 310, H = 260

function pmColor(pm25) {
  if (pm25 <= 12) return '#22c55e'
  if (pm25 <= 35) return '#f97316'
  return '#ef4444'
}

const projectedBoxes = computed(() =>
  props.boxes.map(box => ({
    ...box,
    x: ((box.lng - BKK.minLng) / (BKK.maxLng - BKK.minLng)) * W,
    y: ((BKK.maxLat - box.lat) / (BKK.maxLat - BKK.minLat)) * H,
  }))
)
</script>

<template>
  <svg :viewBox="`0 0 ${W} ${H}`" xmlns="http://www.w3.org/2000/svg" class="bkk-map">
    <!-- tile background -->
    <rect width="310" height="260" fill="#1a2535"/>

    <!-- grid (tile seams) -->
    <line v-for="x in [62,124,186,248]" :key="'gx'+x" :x1="x" y1="0" :x2="x" y2="260" stroke="#212f42" stroke-width="0.5"/>
    <line v-for="y in [52,104,156,208]" :key="'gy'+y" x1="0" :y1="y" x2="310" :y2="y" stroke="#212f42" stroke-width="0.5"/>

    <!-- Chao Phraya river (west side) -->
    <path d="M40,0 Q30,40 45,80 Q35,120 42,160 Q30,200 38,260"
          fill="none" stroke="#1e3f5a" stroke-width="8" stroke-linecap="round"/>

    <!-- Major roads -->
    <!-- Sukhumvit (horizontal, central) -->
    <line x1="0" y1="148" x2="310" y2="148" stroke="#2a3f58" stroke-width="3"/>
    <!-- Rama IV -->
    <line x1="0" y1="185" x2="310" y2="180" stroke="#2a3f58" stroke-width="2.5"/>
    <!-- Phahon Yothin (vertical) -->
    <line x1="148" y1="0" x2="148" y2="260" stroke="#2a3f58" stroke-width="2.5"/>
    <!-- Silom Rd -->
    <line x1="50" y1="210" x2="200" y2="200" stroke="#2a3f58" stroke-width="2"/>
    <!-- expressway diagonal -->
    <line x1="0" y1="100" x2="310" y2="160" stroke="#243448" stroke-width="4"/>
    <!-- local roads -->
    <line x1="100" y1="0"   x2="100" y2="260" stroke="#243040" stroke-width="1.5"/>
    <line x1="200" y1="0"   x2="200" y2="260" stroke="#243040" stroke-width="1.5"/>
    <line x1="0"   y1="80"  x2="310" y2="80"  stroke="#243040" stroke-width="1.5"/>
    <line x1="0"   y1="220" x2="310" y2="220" stroke="#243040" stroke-width="1.5"/>

    <!-- Parks (green blobs) -->
    <ellipse cx="148" cy="100" rx="18" ry="12" fill="#1a3a20" opacity="0.7"/>
    <ellipse cx="240" cy="60"  rx="14" ry="10" fill="#1a3a20" opacity="0.6"/>

    <!-- Air box markers -->
    <g v-for="box in projectedBoxes" :key="box.id">
      <!-- shadow -->
      <circle :cx="box.x" :cy="box.y + 2" r="9" fill="#00000040"/>
      <!-- pin body -->
      <circle :cx="box.x" :cy="box.y" r="9"
              :fill="box.online ? pmColor(box.pm25) : '#475569'"
              :stroke="box.online ? '#ffffff30' : '#33415530'"
              stroke-width="1.5"/>
      <!-- offline dim overlay -->
      <circle v-if="!box.online" :cx="box.x" :cy="box.y" r="9" fill="#00000050"/>
      <!-- pm label inside pin -->
      <text :x="box.x" :y="box.y + 3.5" text-anchor="middle"
            font-size="7" font-weight="700" fill="white" font-family="monospace">
        {{ box.pm25 }}
      </text>
      <!-- box name label -->
      <text :x="box.x + 12" :y="box.y + 4" font-size="7.5"
            fill="#e2e8f0" font-family="monospace" font-weight="600">
        {{ box.name.split(' ')[0] }}
      </text>
    </g>

    <!-- compass -->
    <text x="290" y="20" text-anchor="middle" font-size="9" fill="#475569" font-family="monospace">N</text>
    <line x1="290" y1="24" x2="290" y2="36" stroke="#475569" stroke-width="1"/>
    <polygon points="290,22 287,30 290,28 293,30" fill="#475569"/>

    <!-- attribution -->
    <text x="308" y="257" text-anchor="end" font-size="6" fill="#2a3f58">© OSM</text>
  </svg>
</template>

<style scoped>
.bkk-map {
  display: block;
  width: 100%;
  height: 100%;
  border-radius: 4px;
}
</style>
