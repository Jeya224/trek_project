#include <Arduino.h>
#include <Wire.h>
#include <ArduinoBLE.h>
#include <Nicla_System.h>

BLEService imuService("180F"); // Service BLE custom
BLECharacteristic imuCharacteristic("2A19", BLERead | BLENotify, 12); // 12 bytes: X,Y,Z float

void setup() {
  Serial.begin(115200);
  while (!Serial);



  // Initialiser BLE
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  BLE.setLocalName("NiclaBLE");
  BLE.setAdvertisedService(imuService);
  imuService.addCharacteristic(imuCharacteristic);
  BLE.addService(imuService);
  BLE.advertise();

  Serial.println("BLE device active, waiting for connections...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      float x, y, z;
  

      // Convertir les floats en bytes
      uint8_t data[12];
      memcpy(data, &x, 4);
      memcpy(data + 4, &y, 4);
      memcpy(data + 8, &z, 4);

      imuCharacteristic.writeValue(data, 12);

      delay(100); // 10 Hz
    }

    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
}

