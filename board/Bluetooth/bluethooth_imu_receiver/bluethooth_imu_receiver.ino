//###LIB_Libraries inclusion
#include "Nicla_System.h"    // Requirement for NICLA Sense ME
#include "Arduino_BHY2.h"    // Requirement for the Bosch sensor
#include <ArduinoBLE.h>      // Bluetooth via web
#include <Arduino.h>
#include <Wire.h>
#include <math.h>            // To use math functions
#include <SPI.h>             // To use SPI connectivity
#include "SdFat.h"           // High-performance SD library
SdFat SD;

BLEDevice peripheral;
BLECharacteristic imuChar;
const int DATA_SIZE = 12;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(yellow);

  // Initialiser BLE
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }
  Serial.println("BLE Central ready — scanning for peripherals...");
  // Démarrer le scan BLE
  BLE.scan();
}

void loop() {
  nicla::leds.setColor(green);

  // Vérifie si un périphérique BLE est trouvé
  peripheral = BLE.available();

  if (peripheral) {
    // Serial.print("Found device: ");
    // Serial.println(peripheral.address());
    // Serial.print("Local name: ");
    // Serial.println(peripheral.localName());

    // On se connecte uniquement si c'est notre Nicla
    if (peripheral.localName() == "NiclaBLE") {
      BLE.stopScan();

      Serial.println("Connecting ...");
      if (peripheral.connect()) {
        Serial.println("Connected to NiclaBLE!");
      } else {
        Serial.println("Connection failed!");
        BLE.scan();
        return;
      }

      // Découvrir les services et caractéristiques
      if (peripheral.discoverAttributes()) {
        imuChar = peripheral.characteristic("2A19");
      } else {
        Serial.println("Attribute discovery failed!");
        peripheral.disconnect();
        BLE.scan();
        return;
      }

      // Vérifier que la caractéristique existe et supporte les notifications
      if (!imuChar || !imuChar.canSubscribe()) {
        Serial.println("IMU characteristic not found or not notifiable!");
        peripheral.disconnect();
        BLE.scan();
        return;
      }

      // S’abonner aux notifications
      imuChar.subscribe();
      Serial.println("Subscribed to IMU notifications!");

      
      // Lire les données en continu
      while (peripheral.connected()) {
        nicla::leds.setColor(blue);
        if (imuChar.valueUpdated()) {
          uint8_t data[DATA_SIZE];
          imuChar.readValue(data, DATA_SIZE);

          float GyroX, GyroY, GyroZ, MagX, MagY, MagZ, x, y, z;
          // memcpy(&GyroX, data, 4);
          // memcpy(&GyroY, data + 4, 4);
          // memcpy(&GyroZ, data + 8, 4);

          // memcpy(&MagX, data + 12, 4);
          // memcpy(&MagY, data + 16, 4);
          // memcpy(&MagZ, data + 20, 4);

          // memcpy(&x, data + 24, 4);
          // memcpy(&y, data + 28, 4);
          // memcpy(&z, data + 32, 4);

          memcpy(&x, data, 4);
          memcpy(&y, data + 4, 4);
          memcpy(&z, data + 8, 4);

          // Affichage pour Serial Plotter (espaces entre les variables)
          // Serial.print(GyroX); Serial.print(" ");
          // Serial.print(GyroY); Serial.print(" ");
          // Serial.print(GyroZ); Serial.print(" ");
          // Serial.print(MagX); Serial.print(" ");
          // Serial.print(MagY); Serial.print(" ");
          // Serial.print(MagZ); Serial.print(" ");
          Serial.print(x); Serial.print(" ");
          Serial.print(y); Serial.print(" ");
          Serial.println(z);
        }
        delay(50);
      }
      Serial.println("Disconnected. Restarting scan...");
      BLE.scan();
    }
  }
}

