<template>

  <template v-if="false">
    <div class="connect">
      <div>
        <button type="button" @click="requestDevice()">
          <svg width="100%" height="100%" viewBox="0 0 24 24" fill="none" xmlns="http://www.w3.org/2000/svg">
            <path d="M6 7L18 17L12 22V2L18 7L6 17" stroke="currentColor" stroke-width="2" stroke-linecap="round"
              stroke-linejoin="round" />
          </svg>
        </button>
        <h1>Connect to bluetooth</h1>
        <span v-if="isGettingData" class="loader" />
      </div>
    </div>
  </template>

  <template v-else-if="isSupported">
    <pre>{{ accel }}</pre>
    <!-- <button v-if="!isConnected" type="button" @click="requestDevice()">connect to Bluetooth</button> -->
    <header>
      <span class="header">Hello,</span>
      <span class="name">Project team trek2</span>
    </header>

    <div class="modes">
      <button type="button" :class="{ 'active': currentMode === 'minimal' }" @click="switchMode('minimal')">
        Minimal mode
      </button>
      <button type="button" :class="{ 'active': currentMode === 'detail' }" @click="switchMode('detail')">
        Detailed mode
      </button>
    </div>

    <div v-if="currentMode === 'minimal'">
      <div class="time-frames">
        <button type="button" :class="{ 'active': selectedTimeFrame === 30 }" @click="selectTimeFrame(30)">
          30s
        </button>
        <button type="button" :class="{ 'active': selectedTimeFrame === 60 }" @click="selectTimeFrame(60)">
          1m
        </button>
        <button type="button" :class="{ 'active': selectedTimeFrame === 300 }" @click="selectTimeFrame(300)">
          5m
        </button>
        <button type="button" :class="{ 'active': selectedTimeFrame === 600 }" @click="selectTimeFrame(600)">
          10m
        </button>
        <button type="button" :class="{ 'active': selectedTimeFrame === 1800 }" @click="selectTimeFrame(1800)">
          30m
        </button>
      </div>

      <div class="info">
        <h1>{{ selectedTimeFrame }}</h1>

        <div class="indexes">
          <div class="index">
            <div>
              <span>Stride length left</span>
              <svgo-person />
            </div>
            <p>
              0.72
              <small>Meters</small>
            </p>
          </div>
          <div class="index">
            <div>
              <span>Stride length right</span>
              <svgo-person />
            </div>
            <p>
              0.74
              <small>Meters</small>
            </p>
          </div>
        </div>

        <!-- ✅ Added parabolic graph below indexes -->
        <div class="minimal-graph">
          <StepDeviationSVG />
        </div>
      </div>
    </div>


    <div v-if="currentMode === 'detail'">

      <div class="time-frames">
        <button type="button" :class="{ 'active': selectedTimeFrame === 300 }" @click="selectTimeFrame(300)">
          5m
        </button>
        <button type="button" :class="{ 'active': selectedTimeFrame === 900 }" @click="selectTimeFrame(900)">
          15m
        </button>
        <button type="button" :class="{ 'active': selectedTimeFrame === 1800 }" @click="selectTimeFrame(1800)">
          30m
        </button>
        <button type="button" :class="{ 'active': selectedTimeFrame === 3600 }" @click="selectTimeFrame(3600)">
          1h
        </button>
        <button type="button" :class="{ 'active': selectedTimeFrame === -1 }" @click="selectTimeFrame(-1)">
          all
        </button>
      </div>

      <div class="info">
        <h1>Indexes</h1>

        <div class="indexes">
          <div class="index">
            <div>
              <span>Steps per min</span>
              <svgo-activity />
            </div>
            <p>
              35
              <small>SPM</small>
            </p>
          </div>
          <div class="index">
            <div>
              <span>Total steps</span>
              <svgo-steps />
            </div>
            <p>
              4.1k
              <small>Steps</small>
            </p>
          </div>
          <div class="index">
            <div>
              <span>Stride length left</span>
              <svgo-person />
            </div>
            <p>
              0.72
              <small>Meters</small>
            </p>
          </div>
          <div class="index">
            <div>
              <span>Stride length right</span>
              <svgo-person />
            </div>
            <p>
              0.74
              <small>Meters</small>
            </p>
          </div>
        </div>
      </div>
      <div>
        <div>
          <StepDeviationSVG />
        </div>

      </div>
    </div>
  </template>
</template>

<script setup lang="ts">
import { ref, watch } from 'vue'
import { useBluetooth, useEventListener, watchPausable } from '@vueuse/core'

const SERVICE_UUID = "19B10000-E8F2-537E-4F6C-D104768A1214"
const CHARACTERISTIC_UUID = "19B10001-E8F2-537E-4F6C-D104768A1214"

const { isSupported, isConnected, device, requestDevice, server } = useBluetooth({
  acceptAllDevices: true,
})

const isGettingData = ref(false)
const accel = ref<Record<string, number>>({ x: 0, y: 0, z: 0 })
const currentMode = ref<"minimal" | "detail">("minimal")
const selectedTimeFrame = ref<number>(30)

/* === INDEX VALUES === */
const indexes = ref({
  strideLeft: 0.72,
  strideRight: 0.74,
  stepsPerMin: 35,
  totalSteps: 4100,
})

/* === Random data generator === */
const randomFloat = (min: number, max: number, decimals = 2) => {
  const val = Math.random() * (max - min) + min
  return parseFloat(val.toFixed(decimals))
}

const randomInt = (min: number, max: number) => {
  return Math.floor(Math.random() * (max - min + 1)) + min
}

/* === Update indexes === */
const updateIndexes = () => {
  indexes.value = {
    strideLeft: randomFloat(0.6, 0.9),
    strideRight: randomFloat(0.6, 0.9),
    stepsPerMin: randomInt(20, 120),
    totalSteps: randomInt(500, 10000),
  }
}

/* === MODE & TIMEFRAME === */
const switchMode = (mode: "minimal" | "detail") => {
  if (currentMode.value !== mode) {
    currentMode.value = mode
    updateIndexes()
  }
}

const selectTimeFrame = (timeFrame: number) => {
  if (selectedTimeFrame.value !== timeFrame) {
    selectedTimeFrame.value = timeFrame
    updateIndexes()
  }
}

/* === BLUETOOTH LOGIC (unchanged, optional) === */
const getBleData = async () => {
  console.log("Searching for BLE device...")
  isGettingData.value = true
  const service = await server.getPrimaryService(SERVICE_UUID)
  const characteristic = await service.getCharacteristic(CHARACTERISTIC_UUID)
  console.log("Connected to device!")

  await characteristic.startNotifications()
  useEventListener(
    characteristic,
    "characteristicvaluechanged",
    (event) => {
      const value = event.target.value
      const x = value.getFloat32(0, true)
      const y = value.getFloat32(4, true)
      const z = value.getFloat32(8, true)
      accel.value = { x, y, z }
      console.log(`Accel: X=${x.toFixed(2)}, Y=${y.toFixed(2)}, Z=${z.toFixed(2)}`)
    },
    { passive: true }
  )
  const characteristicData = await characteristic.readValue()
  accel.value = {
    x: characteristicData.getFloat32(0, true),
    y: characteristicData.getFloat32(4, true),
    z: characteristicData.getFloat32(8, true),
  }
}

const { stop } = watchPausable(isConnected, (newIsConnected) => {
  if (!newIsConnected || !server.value || isGettingData.value) return
  getBleData()
  stop()
})

// Initialize random data on load
updateIndexes()
</script>


<style scoped>
/* GENERAL LAYOUT */
header {
  display: flex;
  flex-direction: column;
  align-items: center;
  margin: 2rem 0 1.5rem;
  text-align: center;
}

.header {
  font-size: 1.25rem;
  font-weight: 500;
  color: #222;
}

.name {
  font-size: 1.75rem;
  font-weight: 700;
  color: #0077b6;
}

/* MODE TOGGLE */
.modes {
  display: flex;
  justify-content: center;
  gap: 1rem;
  margin-bottom: 2rem;
}

.modes button {
  padding: 0.75rem 1.5rem;
  border-radius: 12px;
  border: none;
  cursor: pointer;
  background-color: #e0f7f4;
  color: #004d40;
  font-weight: 500;
  transition: all 0.2s ease-in-out;
}

.modes button:hover {
  background-color: #c6ede8;
}

.modes button.active {
  background-color: #2eb5fa;
  color: white;
  box-shadow: 0 4px 12px rgba(46, 181, 250, 0.3);
}

/* TIME FRAME SELECTOR */
.time-frames {
  display: flex;
  flex-wrap: wrap;
  justify-content: center;
  gap: 0.75rem;
  margin-bottom: 2rem;
}

.time-frames button {
  background: #f1f9fc;
  border: 1px solid #d6edf8;
  color: #0077b6;
  padding: 0.5rem 1rem;
  border-radius: 8px;
  font-weight: 500;
  transition: all 0.2s ease-in-out;
}

.time-frames button:hover {
  background-color: #e0f4fb;
}

.time-frames button.active {
  background-color: #2eb5fa;
  color: white;
  border-color: #2eb5fa;
}

/* INFO AREA */
.info {
  display: flex;
  flex-direction: column;
  align-items: center;
  gap: 1.25rem;
  padding: 1.5rem;
  border-radius: 16px;
  background-color: #ffffff;
  box-shadow: 0 4px 12px rgba(0, 0, 0, 0.08);
  max-width: 700px;
  margin: 0 auto;
}

.info h1 {
  color: #004d40;
  font-size: 1.5rem;
  font-weight: 600;
}

/* INDEX CARDS */
.indexes {
  display: grid;
  grid-template-columns: repeat(auto-fit, minmax(260px, 1fr));
  gap: 1rem;
  width: 100%;
}

.index {
  display: flex;
  flex-direction: column;
  justify-content: space-between;
  background-color: #f8fcff;
  padding: 1.25rem 1.5rem;
  border-radius: 16px;
  box-shadow: 0 2px 8px rgba(46, 181, 250, 0.1);
  transition: transform 0.15s ease, box-shadow 0.15s ease;
}

.index:hover {
  transform: translateY(-2px);
  box-shadow: 0 4px 12px rgba(46, 181, 250, 0.15);
}

.index div {
  display: flex;
  align-items: center;
  justify-content: space-between;
}

.index span {
  font-weight: 500;
  color: #0077b6;
}

.index p {
  font-size: 1.75rem;
  font-weight: 600;
  color: #004d40;
  margin: 0;
}

.index small {
  display: block;
  font-size: 0.875rem;
  color: #6c757d;
}

/* CONNECT SCREEN (unchanged, just kept for context) */
.connect {
  width: 100%;
  height: 100dvh;
  background-color: #2eb5fa;
  display: flex;
  place-items: center;
  justify-content: center;
}

.connect div {
  display: grid;
  justify-items: center;
  gap: 24px;
}

.connect button {
  width: fit-content;
  padding: 48px;
  border-radius: 50%;
  background-color: white;
  border: none;
  cursor: pointer;
}

.connect button:hover {
  background-color: #e4e4e4;
}

.connect svg {
  color: #2eb5fa;
  width: 48px;
}

.connect h1 {
  color: white;
}

.connect .loader {
  width: 40px;
  aspect-ratio: 2;
  --_g: no-repeat radial-gradient(circle closest-side, #fff 90%, #fff0);
  background: var(--_g) 0% 50%, var(--_g) 50% 50%, var(--_g) 100% 50%;
  background-size: calc(100% / 3) 50%;
  animation: loading 1s infinite linear;
}

@keyframes loading {
  20% {
    background-position: 0% 0%, 50% 50%, 100% 50%;
  }

  40% {
    background-position: 0% 100%, 50% 0%, 100% 50%;
  }

  60% {
    background-position: 0% 50%, 50% 100%, 100% 0%;
  }

  80% {
    background-position: 0% 50%, 50% 50%, 100% 100%;
  }
}
</style>
