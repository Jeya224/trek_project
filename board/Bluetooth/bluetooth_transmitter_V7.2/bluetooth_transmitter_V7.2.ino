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

  BLE.setLocalName("NiclaBLE2");
  BLE.setAdvertisedService(imuService);

  imuService.addCharacteristic(imuCharacteristic);
  BLE.addService(imuService);
  BLE.advertise();

  Serial.println(" Nicla BLE active et en publicité...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connecté à ");
    Serial.println(central.address());

    while (central.connected()) {
      // Très important : rafraîchir les capteurs à chaque cycle
      BHY2.update();

      float x2 = accel.x() / 1000.0;
      float y2 = accel.y() / 1000.0;
      float z2 = accel.z() / 1000.0;
      uint32_t t2 = millis();

      uint8_t data[16];
      memcpy(data, &x2, 4);
      memcpy(data + 4, &y2, 4);
      memcpy(data + 8, &z2, 4);
      memcpy(data + 12, &t2, 4);
    

      imuCharacteristic.writeValue(data, 16);

      //  Affichage série pour contrôle
      Serial.print("t2="); Serial.print(t2);
      Serial.print("  x2="); Serial.print(x2, 3);
      Serial.print("  y2="); Serial.print(y2, 3);
      Serial.print("  z2="); Serial.println(z2, 3);

      delay(100);
    }

    Serial.println("Déconnecté.");
  }
}

