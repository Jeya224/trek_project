#include <Arduino_BHY2.h>

#include <ArduinoBLE.h>
#include "Nicla_System.h"

SensorXYZ accel(SENSOR_ID_ACC);

BLEService imuService("12345678-1234-1234-1234-123456781234"); 
BLECharacteristic imuCharacteristic("12345679-1234-1234-1234-123456781234", BLERead | BLENotify, 12);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("BLE init failed!");
    while (1);
  }

  

  BHY2.begin();
  accel.begin();
  BLE.setLocalName("NiclaBLE");
  BLE.setAdvertisedService(imuService);

  imuService.addCharacteristic(imuCharacteristic);
  BLE.addService(imuService);
  BLE.advertise();


  Serial.println("Nicla BLE active et en publicité...");
}

void loop() {
  BLEDevice central = BLE.central();

  BHY2.update();

  if (central) {
    Serial.print("Connecté à ");
    Serial.println(central.address());
    
    while (central.connected()) {
      float x = accel.x() / 1000 * 10; // amplification 10x juste pour voir le graphe
      float y = accel.y() / 1000 * 10;
      float z = accel.z() / 1000 * 10;


      Serial.print("acc_X:");
      Serial.print(accel.x());
      Serial.print(",");
      Serial.print("acc_Y:");
      Serial.print(accel.y());
      Serial.print(",");
      Serial.print("acc_Z:");
      Serial.println(accel.z());

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

