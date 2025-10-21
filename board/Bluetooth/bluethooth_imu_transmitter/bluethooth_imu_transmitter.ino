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

//###SD_Definitions
#define SD_CS_PIN SS
#define BUFFER_SIZE 16384    // Buffer size for SD writes
#define WRITE_INTERVAL 6000   // File flush interval in milliseconds

const int DATA_SIZE = 12;

//###SENID_DefineSensorID
SensorXYZ gyroscope(SENSOR_ID_GYRO);
SensorXYZ accelerometer(SENSOR_ID_ACC);
SensorQuaternion quaternion(SENSOR_ID_RV);
SensorXYZ magnetometer(SENSOR_ID_MAG);

BLEService imuService("180F"); // Service BLE custom
BLECharacteristic imuCharacteristic("2A19", BLERead | BLENotify, DATA_SIZE); // 12 bytes: X,Y,Z float

//###VARINIT_Variables initialisation
static unsigned long prevTime = 0;  // To track time between measurements
unsigned long lastFlushTime = 0;   // Last time data was flushed
char dataBuffer[BUFFER_SIZE];
int bufferIndex = 0;
File myFile;

float Time, DeltaTime;
float LaccX, LaccY, LaccZ;
float Xorientation, Yorientation, Zorientation, Worientation;
float GyroX, GyroY, GyroZ;
float vX = 0, vY = 0, vZ = 0;      // Velocity components
float dX = 0, dY = 0, dZ = 0;      // Displacement components
float Gx = 0.0, Gy = 0.0, Gz = 0.0;
float GxO = 0.0, GyO = 0.0, GzO = 0.0;
float Magx, Magy, Magz;
float x_filtered = 0.0, y_filtered = 0.0, z_filtered = 0.0;
int Pnum = 0;
float ScaleFactor = 4096.0;

//###FILTPARA_ Filtering parameters
const float accelerationThreshold = 0.5; //threshold for setting velocity and displacment to 0 when the device is still
const float alphaFilter = 1;     // Low-pass filter coefficient

void setup() {
  Serial.begin(115200);
  while (!Serial);

  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(yellow);

  // Sensor Initialization
  BHY2.begin(NICLA_STANDALONE);
  gyroscope.begin();
  accelerometer.begin();
  quaternion.begin();
  magnetometer.begin();

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
  nicla::leds.setColor(green);

  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());
    //digitalWrite(LED_BUILTIN, HIGH);
    nicla::leds.setColor(blue);
    while (central.connected()) {
      BHY2.update();            // Update sensor readings
  
      // Calculate timing
      unsigned long currentTime = millis();
      Time = currentTime / 1000.0; // Time in seconds
      DeltaTime = (currentTime - prevTime) / 1000.0; // Delta time in seconds
      prevTime = currentTime;

      //--- Sensor Data Acquisition ---
      GyroX = gyroscope.x()/ ScaleFactor;
      GyroY = gyroscope.y()/ScaleFactor;
      GyroZ = gyroscope.z()/ScaleFactor;
      Magx = magnetometer.x()/ScaleFactor;
      Magy = magnetometer.y()/ScaleFactor;
      Magz = magnetometer.z()/ScaleFactor;

      // Linear acceleration
      float x = accelerometer.x() * 1 / ScaleFactor;
      float y = accelerometer.y() * 1 / ScaleFactor;
      float z = accelerometer.z() * 1 / ScaleFactor;

      // Convertir les 9 floats en tableau de bytes
      uint8_t data[DATA_SIZE];
      // memcpy(data, &GyroX, 4);
      // memcpy(data + 4, &GyroY, 4);
      // memcpy(data + 8, &GyroZ, 4);
      // memcpy(data + 12, &Magx, 4);
      // memcpy(data + 16, &Magy, 4);
      // memcpy(data + 20, &Magz, 4);
      // memcpy(data + 24, &x, 4);
      // memcpy(data + 28, &y, 4);
      // memcpy(data + 32, &z, 4);
      memcpy(data, &x, 4);
      memcpy(data + 4, &y, 4);
      memcpy(data + 8, &z, 4);

      imuCharacteristic.writeValue(data, DATA_SIZE);
      delay(100);
    }

    Serial.print("Disconnected from central: ");
    //digitalWrite(LED_BUILTIN, LOW);
    Serial.println(central.address());
  }
}

