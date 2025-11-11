#include <Arduino_BHY2.h>
#include <ArduinoBLE.h>
#include "Nicla_System.h"

SensorXYZ accel(SENSOR_ID_ACC);

BLEService imuService("12345678-1234-1234-1234-123456781234"); 
BLECharacteristic imuCharacteristic("12345679-1234-1234-1234-123456781234", BLERead | BLENotify, 16);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println(" BLE init failed!");
    while (1);
  }

  BHY2.begin();
  accel.begin();

  BLE.setLocalName("NiclaBLE");
  BLE.setAdvertisedService(imuService);

  imuService.addCharacteristic(imuCharacteristic);
  BLE.addService(imuService);
  BLE.advertise();

  Serial.println(" Nicla BLE activated and in ad...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to ");
    Serial.println(central.address());

    while (central.connected()) {
      // Très important : rafraîchir les capteurs à chaque cycle
      BHY2.update();

      float x = accel.x() / 1000.0;
      float y = accel.y() / 1000.0;
      float z = accel.z() / 1000.0;
      uint32_t t = millis();

      uint8_t data[16];
      memcpy(data, &x, 4);
      memcpy(data + 4, &y, 4);
      memcpy(data + 8, &z, 4);
      memcpy(data + 12, &t, 4);

      imuCharacteristic.writeValue(data, 16);

      //  Affichage série pour contrôle
      Serial.print("t="); Serial.print(t);
      Serial.print("  x="); Serial.print(x, 3);
      Serial.print("  y="); Serial.print(y, 3);
      Serial.print("  z="); Serial.println(z, 3);

      delay(100);
    }

    Serial.println("Disconnected.");
  }
}
