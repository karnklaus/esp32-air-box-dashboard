import { createRouter, createWebHashHistory } from 'vue-router'

export const SLIDES = [
  { path: '/hero',         name: 'hero',         label: 'Intro',        icon: 'fa-solid fa-house',              component: () => import('../views/ViewHero.vue')         },
  { path: '/hardware',     name: 'hardware',     label: 'Hardware',     icon: 'fa-solid fa-microchip',          component: () => import('../views/ViewHardware.vue')     },
  { path: '/architecture', name: 'architecture', label: 'Architecture', icon: 'fa-solid fa-diagram-project',    component: () => import('../views/ViewArchitecture.vue') },
  { path: '/config',       name: 'config',       label: 'Config Mode',  icon: 'fa-solid fa-sliders',            component: () => import('../views/ViewConfigMode.vue')   },
  { path: '/dataflow',     name: 'dataflow',     label: 'Data Flow',    icon: 'fa-solid fa-arrow-right-arrow-left', component: () => import('../views/ViewDataFlow.vue')  },
  { path: '/dashboard',    name: 'dashboard',    label: 'Dashboard',    icon: 'fa-solid fa-gauge-high',         component: () => import('../views/ViewDashboard.vue')    },
  { path: '/simulation',   name: 'simulation',   label: 'Simulation',   icon: 'fa-solid fa-chart-line',         component: () => import('../views/ViewLiveSim.vue')      },
]

const routes = [
  { path: '/', redirect: '/hero' },
  ...SLIDES.map(({ path, name, component }) => ({ path, name, component })),
]

export default createRouter({
  history: createWebHashHistory(),
  routes,
})
