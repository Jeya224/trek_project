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

  accel.begin();

  BHY2.begin();
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
      float x, y, z;
      x = accel.x() ;
      y = accel.y() ;
      z = accel.z() ;

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

