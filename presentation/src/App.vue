<script setup>
import AppNav from './components/AppNav.vue'
import NavArrows from './components/NavArrows.vue'
</script>

<template>
  <AppNav />
  <main class="main-content">
    <RouterView v-slot="{ Component }">
      <Transition name="fade" mode="out-in">
        <component :is="Component" />
      </Transition>
    </RouterView>
  </main>
  <NavArrows />
</template>

<style>
.main-content {
  padding-top: var(--nav-h);
  padding-bottom: var(--arrows-h);
  min-height: 100dvh;
}

/* Page transition */
.fade-enter-active,
.fade-leave-active {
  transition: opacity 0.2s ease;
}
.fade-enter-from,
.fade-leave-to {
  opacity: 0;
}
</style>
