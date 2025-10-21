import asyncio
from bleak import BleakClient, BleakScanner
import struct

IMU_SERVICE_UUID = "180F"
IMU_CHAR_UUID = "2A19"

async def run():
    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover()
    for i, d in enumerate(devices):
        print(i, d.name, d.address)

    address = input("Enter the device address: ")

    async with BleakClient(address) as client:
        print("Connected!")

        def handle_imu(sender, data):
            x, y, z = struct.unpack('fff', data)
            print(f"Accel: x={x:.2f}, y={y:.2f}, z={z:.2f}")

        await client.start_notify(IMU_CHAR_UUID, handle_imu)

        print("Receiving notifications... Press Ctrl+C to exit")
        while True:
            await asyncio.sleep(1)

asyncio.run(run())
