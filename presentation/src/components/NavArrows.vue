<script setup>
import { computed } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { SLIDES } from '../router/index.js'

const route  = useRoute()
const router = useRouter()

const currentIndex = computed(() => SLIDES.findIndex(s => s.name === route.name))
const prev = computed(() => currentIndex.value > 0 ? SLIDES[currentIndex.value - 1] : null)
const next = computed(() => currentIndex.value < SLIDES.length - 1 ? SLIDES[currentIndex.value + 1] : null)

const progress = computed(() => ((currentIndex.value + 1) / SLIDES.length) * 100)
</script>

<template>
  <footer class="nav-arrows">
    <!-- progress bar -->
    <div class="nav-progress">
      <div class="nav-progress-bar" :style="{ width: progress + '%' }"></div>
    </div>

    <div class="nav-arrows-inner">
      <!-- prev -->
      <RouterLink v-if="prev" :to="prev.path" class="arrow-btn arrow-prev">
        <i class="fa-solid fa-arrow-left"></i>
        <span>{{ prev.label }}</span>
      </RouterLink>
      <span v-else class="arrow-btn arrow-placeholder"></span>

      <!-- slide counter -->
      <span class="slide-counter">
        {{ currentIndex + 1 }} / {{ SLIDES.length }}
      </span>

      <!-- next -->
      <RouterLink v-if="next" :to="next.path" class="arrow-btn arrow-next">
        <span>{{ next.label }}</span>
        <i class="fa-solid fa-arrow-right"></i>
      </RouterLink>
      <span v-else class="arrow-btn arrow-placeholder"></span>
    </div>
  </footer>
</template>

<style scoped>
:root { --arrows-h: 52px; }

.nav-arrows {
  position: fixed;
  bottom: 0; left: 0; right: 0;
  z-index: 100;
  height: var(--arrows-h);
  background: #080c14ee;
  backdrop-filter: blur(12px);
  -webkit-backdrop-filter: blur(12px);
  border-top: 1px solid var(--border);
}

.nav-progress {
  height: 2px;
  background: var(--border);
}
.nav-progress-bar {
  height: 100%;
  background: var(--teal);
  transition: width 0.3s ease;
}

.nav-arrows-inner {
  max-width: var(--max-w);
  margin-inline: auto;
  padding-inline: 24px;
  height: calc(var(--arrows-h) - 2px);
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.arrow-btn {
  display: flex;
  align-items: center;
  gap: 8px;
  font-size: 0.82rem;
  font-weight: 500;
  color: var(--muted);
  text-decoration: none;
  padding: 6px 12px;
  border-radius: 6px;
  transition: color 0.15s, background 0.15s;
  min-width: 120px;
}
.arrow-btn:hover { color: var(--text); background: var(--surface); }
.arrow-next { justify-content: flex-end; }
.arrow-placeholder { min-width: 120px; }

.slide-counter {
  font-size: 0.78rem;
  color: var(--muted);
  font-family: 'JetBrains Mono', monospace;
}
</style>
