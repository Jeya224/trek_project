<template>
    <button v-if="!isConnected" type="button" @click="buttonClick()">connect to Bluetooth</button>
    <div v-else>
        <header>
            <span>Hello,</span>
            <span class="name">Project team trek2</span>
        </header>
        <div class="info">
            <h1>Indexes</h1>

            <div class="indexes">
            <div class="index">
                <div>
                <span>Steps per min</span>
                <svgo-activity/>
                </div>
                <p>
                35
                <small>SPM</small>
                </p>
            </div>
            <div class="index">
                <div>
                <span>Total steps</span>
                <svgo-activity/>
                </div>
                <p>
                4.1k
                <small>Steps</small>
                </p>
            </div>
            <div class="index">
                <div>
                <span>Stride length left</span>
                svgo
                </div>
                <p>
                0.72
                <small>Meters</small>
                </p>
            </div>
            <div class="index">
                <div>
                <span>Stride length right</span>
                svgo
                </div>
                <p>
                0.74
                <small>Meters</small>
                </p>
            </div>
            </div>
        </div>
    </div>
</template>

<script setup lang="ts">
const isConnected = ref(false)

const buttonClick = () => {
    isConnected.value = true
    console.log("trying to connect");
}
</script>

<style scoped>
.indexes {
    display: grid;
    grid-template-columns:repeat(2, 1fr);
    gap: 0.5rem;

    .index {
        display: flex;
        flex-direction: column;
        gap: 0.75rem;
        padding: 1rem;
        border-radius: 1.5rem;
        border: 1px solid #e8e8e8;
    }
}
</style>
