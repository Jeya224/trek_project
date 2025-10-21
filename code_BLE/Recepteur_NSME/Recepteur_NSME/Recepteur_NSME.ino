#include <Arduino.h>
#include <ArduinoBLE.h>

BLEDevice peripheral;
BLECharacteristic imuChar;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  Serial.println("BLE Central ready — scanning for peripherals...");

  // Démarrer le scan BLE
  BLE.scan();
}

void loop() {
  // Vérifie si un périphérique BLE est trouvé
  peripheral = BLE.available();

  if (peripheral) {
    Serial.print("Found device: ");
    Serial.println(peripheral.address());
    Serial.print("Local name: ");
    Serial.println(peripheral.localName());

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
        if (imuChar.valueUpdated()) {
          uint8_t data[12];
          imuChar.readValue(data, 12);

          float x, y, z;
          memcpy(&x, data, 4);
          memcpy(&y, data + 4, 4);
          memcpy(&z, data + 8, 4);

          Serial.print("Accel: ");
          Serial.print(x, 2);
          Serial.print(", ");
          Serial.print(y, 2);
          Serial.print(", ");
          Serial.println(z, 2);
        }
        delay(50);
      }

      Serial.println("Disconnected. Restarting scan...");
      BLE.scan();
    }
  }
}

