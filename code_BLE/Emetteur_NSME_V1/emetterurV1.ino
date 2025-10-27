#include <ArduinoBLE.h>
#include <Nicla_System.h>
#include <Nicla_IMU.h>

BLEService imuService("19B10000-E8F2-537E-4F6C-D104768A1214"); 
BLECharacteristic imuCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 12);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("BLE init failed!");
    while (1);
  }

  NiclaIMU.begin();
  BLE.setLocalName("NiclaBLE");
  BLE.setAdvertisedService(imuService);

  imuService.addCharacteristic(imuCharacteristic);
  BLE.addService(imuService);
  BLE.advertise();

  Serial.println("Nicla BLE active et en publicité...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connecté à ");
    Serial.println(central.address());

    while (central.connected()) {
      float x, y, z;
      NiclaIMU.readAccelerometer(x, y, z);

      uint8_t data[12];
      memcpy(data, &x, 4);
      memcpy(data + 4, &y, 4);
      memcpy(data + 8, &z, 4);at x, y, z;
      NiclaIMU.readAccelerometer(x, y, z);

      uint8_t data[12];
      memcpy(data, &x, 4);
      memcpy(data + 4, &y, 4);
      memcpy(data + 8, &z, 4);

      imuCharacteristic.writeValue(data, 12);
      delay(100);
    }

    Serial.println("Déconnecté.");
  }
}
