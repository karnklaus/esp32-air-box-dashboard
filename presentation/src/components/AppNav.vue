<script setup>
import { useRoute } from 'vue-router'
import { SLIDES } from '../router/index.js'

const route = useRoute()
</script>

<template>
  <nav class="app-nav">
    <div class="app-nav-inner">
      <RouterLink class="app-nav-logo" to="/hero">
        <i class="fa-solid fa-wind"></i> AirBox
      </RouterLink>
      <ul class="app-nav-links">
        <li v-for="slide in SLIDES" :key="slide.name">
          <RouterLink
            :to="slide.path"
            :class="['nav-link', { active: route.name === slide.name }]"
            :title="slide.label"
          >
            <i :class="slide.icon"></i>
            <span class="nav-label">{{ slide.label }}</span>
          </RouterLink>
        </li>
      </ul>
    </div>
  </nav>
</template>

<style scoped>
.app-nav {
  position: fixed;
  top: 0; left: 0; right: 0;
  z-index: 100;
  height: var(--nav-h);
  background: #080c14cc;
  backdrop-filter: blur(12px);
  -webkit-backdrop-filter: blur(12px);
  border-bottom: 1px solid var(--border);
}

.app-nav-inner {
  max-width: var(--max-w);
  margin-inline: auto;
  padding-inline: 24px;
  height: 100%;
  display: flex;
  align-items: center;
  gap: 24px;
}

.app-nav-logo {
  font-size: 0.88rem;
  font-weight: 700;
  letter-spacing: 0.08em;
  text-transform: uppercase;
  color: var(--teal);
  text-decoration: none;
  white-space: nowrap;
  display: flex;
  align-items: center;
  gap: 7px;
  transition: opacity 0.15s;
}
.app-nav-logo:hover { opacity: 0.8; }

.app-nav-links {
  list-style: none;
  display: flex;
  gap: 2px;
  flex-wrap: wrap;
}

.nav-link {
  display: flex;
  align-items: center;
  gap: 6px;
  color: var(--muted);
  text-decoration: none;
  font-size: 0.8rem;
  font-weight: 500;
  padding: 5px 10px;
  border-radius: 6px;
  transition: color 0.15s, background 0.15s;
}
.nav-link i { font-size: 0.75rem; }
.nav-link:hover { color: var(--text); background: var(--surface); }
.nav-link.active { color: var(--teal); background: var(--teal-dim); }

@media (max-width: 768px) {
  .nav-label { display: none; }
  .nav-link { padding: 6px 8px; }
}
</style>
