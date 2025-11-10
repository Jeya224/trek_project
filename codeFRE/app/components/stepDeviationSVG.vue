<template>
  <div class="chart-wrap">
    <!-- ===== HEADER (optional in minimal) ===== -->
    <div v-if="mode === 'detail'" class="chart-header">
      <h3>Step deviation (meters)</h3>
      <div class="controls">
        <button @click="regenerate">Regenerate fake data</button>
        <label><input type="range" min="5" max="60" v-model.number="numSteps" /> Steps: {{ numSteps }}</label>
      </div>
    </div>
    <div v-else class="chart-header minimal-header">
      <h3>Step deviation (meters)</h3>
    </div>

    <!-- ===== PARABOLIC (LEFT/RIGHT) CHART ===== -->
    <svg :width="width" :height="height" viewBox="0 0 700 360" class="svg-chart"
      :class="{ 'minimal-svg': mode === 'minimal' }" role="img" aria-label="Step deviation chart">
      <!-- Background -->
      <rect x="0" y="0" :width="width" :height="height" fill="white" rx="8" />

      <!-- axes -->
      <line :x1="marginLeft" :y1="height - marginBottom" :x2="width - marginRight" :y2="height - marginBottom"
        stroke="#eee" />
      <line :x1="marginLeft" :y1="marginTop" :x2="marginLeft" :y2="height - marginBottom" stroke="#eee" />

      <!-- Right leg (blue) -->
      <path :d="pathFor(rightData)" fill="none" stroke="#007bff" stroke-width="3" stroke-linejoin="round"
        stroke-linecap="round" />

      <!-- Left leg (red) -->
      <path :d="pathFor(leftData)" fill="none" stroke="#ff4d4d" stroke-width="3" stroke-linejoin="round"
        stroke-linecap="round" />

      <!-- Smooth gradient fill for visual polish -->
      <defs>
        <linearGradient id="gradLeft" x1="0" y1="0" x2="0" y2="1">
          <stop offset="0%" stop-color="#ff4d4d" stop-opacity="0.25" />
          <stop offset="100%" stop-color="#ff4d4d" stop-opacity="0" />
        </linearGradient>
        <linearGradient id="gradRight" x1="0" y1="0" x2="0" y2="1">
          <stop offset="0%" stop-color="#007bff" stop-opacity="0.25" />
          <stop offset="100%" stop-color="#007bff" stop-opacity="0" />
        </linearGradient>
      </defs>

      <!-- Points (small dots) -->
      <g v-for="(p, i) in leftData" :key="'lp' + i">
        <circle :cx="xToPx(i + 1)" :cy="yToPx(p)" r="2" fill="#ff4d4d" opacity="0.8" />
      </g>
      <g v-for="(p, i) in rightData" :key="'rp' + i">
        <circle :cx="xToPx(i + 1)" :cy="yToPx(p)" r="2" fill="#007bff" opacity="0.8" />
      </g>

      <!-- legend (show always, small in minimal) -->
      <g :transform="`translate(${width - marginRight - 150}, ${marginTop})`">
        <rect x="0" y="-12" width="150" height="44" rx="8" fill="rgba(255,255,255,0.85)" stroke="#f0f0f0" />
        <g transform="translate(10,0)">
          <line x1="0" y1="4" x2="20" y2="4" stroke="#ff4d4d" stroke-width="3" />
          <text x="28" y="8" font-size="12" fill="#222">Left leg</text>

          <line x1="0" y1="20" x2="20" y2="20" stroke="#007bff" stroke-width="3" />
          <text x="28" y="24" font-size="12" fill="#222">Right leg</text>
        </g>
      </g>
    </svg>

    <!-- ===== BAR CHART (only in detail mode) ===== -->
    <template v-if="mode === 'detail'">
      <div class="chart-header" style="margin-top:20px">
        <h3>Total Steps Over Time (30-minute intervals)</h3>
        <div class="controls">
          <button @click="generateStepsData">Regenerate step timeline</button>
        </div>
      </div>

      <svg :width="width" :height="stepHeight" viewBox="0 0 700 260" class="svg-chart" role="img"
        aria-label="Total steps over time">
        <rect x="0" y="0" :width="width" :height="stepHeight" fill="white" rx="8" />

        <line :x1="s_marginLeft" :y1="stepHeight - s_marginBottom" :x2="width - s_marginRight"
          :y2="stepHeight - s_marginBottom" stroke="#ddd" />
        <line :x1="s_marginLeft" :y1="s_marginTop" :x2="s_marginLeft" :y2="stepHeight - s_marginBottom" stroke="#ddd" />

        <!-- steps line -->
        <path :d="s_pathFor(stepsData)" fill="none" stroke="#2eb5fa" stroke-width="2.5" stroke-linejoin="round"
          stroke-linecap="round" />

        <g v-for="(p, i) in stepsData" :key="'sp' + i">
          <circle :cx="s_xToPx(i)" :cy="s_yToPx(p)" r="2.5" fill="#2eb5fa" />
        </g>
      </svg>
    </template>
  </div>
</template>

<script setup lang="ts">
import { ref, computed, watch } from 'vue'

// ✅ Accept mode prop (default: 'detail')
const props = defineProps({
  mode: {
    type: String as () => 'minimal' | 'detail',
    default: 'detail'
  }
})

/* --- same data + chart logic as before --- */
const width = 700
const height = 360
const marginLeft = 50
const marginRight = 30
const marginTop = 30
const marginBottom = 60
const numSteps = ref(20)

/* Generate parabolic chart data */
function generateParabolicData(n: number) {
  const mid = (n + 1) / 2
  const left: number[] = []
  const right: number[] = []
  for (let i = 1; i <= n; i++) {
    const dx = i - mid
    const baseLeft = 0.0025 * dx * dx + 0.02
    const baseRight = 0.0020 * dx * dx + 0.018
    const noiseL = (Math.random() - 0.5) * 0.03
    const noiseR = (Math.random() - 0.5) * 0.03
    left.push(Number((baseLeft + noiseL).toFixed(4)))
    right.push(Number((baseRight + noiseR).toFixed(4)))
  }
  return { left, right }
}

const { left: leftDataInit, right: rightDataInit } = generateParabolicData(numSteps.value)
const leftData = ref<number[]>(leftDataInit)
const rightData = ref<number[]>(rightDataInit)

function regenerate() {
  const { left, right } = generateParabolicData(numSteps.value)
  leftData.value = left
  rightData.value = right
  generateStepsData()
}

watch(numSteps, (n) => {
  const { left, right } = generateParabolicData(n)
  leftData.value = left
  rightData.value = right
})

const yMin = computed(() => Math.max(0, Math.min(...leftData.value, ...rightData.value) - 0.02))
const yMax = computed(() => Math.max(...leftData.value, ...rightData.value) + 0.02)

function xToPx(stepIndex: number) {
  const usable = width - marginLeft - marginRight
  return marginLeft + ((stepIndex - 1) / (numSteps.value - 1 || 1)) * usable
}
function yToPx(value: number) {
  const top = marginTop
  const bottom = height - marginBottom
  const ratio = (value - yMin.value) / (yMax.value - yMin.value)
  return bottom - ratio * (bottom - top)
}
function pathFor(data: number[]) {
  if (!data.length) return ''
  return `M ${data.map((v, i) => `${xToPx(i + 1)},${yToPx(v)}`).join(' L ')}`
}

/* Steps chart logic (same as before) */
const stepHeight = 260
const s_marginLeft = 50
const s_marginRight = 30
const s_marginTop = 30
const s_marginBottom = 60
const stepsData = ref<number[]>([])
function generateStepsData() {
  let total = 0
  const arr: number[] = []
  for (let i = 0; i < 48; i++) {
    const inc = Math.round(Math.max(0, Math.random() * 500 - 50))
    total += inc
    arr.push(total)
  }
  stepsData.value = arr
}
generateStepsData()

function s_xToPx(i: number) {
  const usable = width - s_marginLeft - s_marginRight
  return s_marginLeft + (i / 47) * usable
}
function s_yToPx(v: number) {
  const bottom = stepHeight - s_marginBottom
  const top = s_marginTop
  const maxv = Math.max(...stepsData.value, 1000)
  const ratio = v / maxv
  return bottom - ratio * (bottom - top)
}
function s_pathFor(data: number[]) {
  if (!data.length) return ''
  return `M ${data.map((v, i) => `${s_xToPx(i)},${s_yToPx(v)}`).join(' L ')}`
}
</script>

<style scoped>
.chart-wrap {
  max-width: 740px;
  margin: 1.25rem auto;
  font-family: system-ui, -apple-system, "Segoe UI", Roboto, "Helvetica Neue", Arial;
}

.chart-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  gap: 12px;
  margin-bottom: 8px;
}

.chart-header.minimal-header {
  justify-content: center;
  margin-bottom: 1rem;
}

.chart-header h3 {
  margin: 0;
  font-size: 1.1rem;
  color: #003a63;
  font-weight: 600;
}

.controls {
  display: flex;
  align-items: center;
  gap: 10px;
  font-size: 13px;
}

.controls button {
  padding: 6px 10px;
  border-radius: 8px;
  border: 1px solid #d0d7de;
  background: #fff;
  cursor: pointer;
  transition: 0.2s;
}

.controls button:hover {
  background-color: #eef8ff;
}

.svg-chart {
  width: 100%;
  height: auto;
  border-radius: 12px;
  box-shadow: 0 6px 18px rgba(2, 6, 23, 0.06);
  background: white;
}

.minimal-svg {
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.08);
  border: 1px solid #f1f1f1;
}
</style>
