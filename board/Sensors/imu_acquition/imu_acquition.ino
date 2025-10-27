/*
Readme:
Code version: 1
Code editor: Jeya
Organisation: OSLOMET
Group 1 EPS

This section describes the code layout and functions

Comments about sections will follow the following format:
//###CODENAME_what the section does

This is the code layout:
//###LIB_Libraries inclusion
//###SD_Definitions
//###SENID_DefineSensorID
//###VARINIT_Variables initialisation
//###FILTPARA_ Filtering parameters
//###SETUP_setup loop
//###WBUF_Write to buffer function
//###FBUF_Flush buffer function
//###MAINL_Main loop
*/

//###LIB_Libraries inclusion
#include "Nicla_System.h"    // Requirement for NICLA Sense ME
#include "Arduino_BHY2.h"    // Requirement for the Bosch sensor
#include <ArduinoBLE.h>      // Bluetooth via web
#include <math.h>            // To use math functions
#include <SPI.h>             // To use SPI connectivity
#include "SdFat.h"           // High-performance SD library
SdFat SD;

//###SD_Definitions
#define SD_CS_PIN SS
#define BUFFER_SIZE 16384    // Buffer size for SD writes
#define WRITE_INTERVAL 6000   // File flush interval in milliseconds

//###SENID_DefineSensorID
SensorXYZ gyroscope(SENSOR_ID_GYRO);
SensorXYZ accelerometer(SENSOR_ID_ACC);
SensorQuaternion quaternion(SENSOR_ID_RV);
SensorXYZ magnetometer(SENSOR_ID_MAG);

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

//###SETUP_setup loop
void setup() {
  Serial.begin(115200);
  Serial.println("Start");

  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(yellow);

  // Sensor Initialization
  BHY2.begin(NICLA_STANDALONE);
  gyroscope.begin();
  accelerometer.begin();
  quaternion.begin();
  magnetometer.begin();

  nicla::leds.setColor(green);
}

//###MAINL_Main loop
void loop() {
  BHY2.update();            // Update sensor readings
  nicla::leds.setColor(green);
  
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

  // Quaternion normalization
  float qw = quaternion.w(), qx = quaternion.x(), qy = quaternion.y(), qz = quaternion.z();
  float norm = sqrt(qw * qw + qx * qx + qy * qy + qz * qz);
  if (norm > 0) {
    qw /= norm; qx /= norm; qy /= norm; qz /= norm;
  }
  Xorientation = qx; Yorientation = qy; Zorientation = qz; Worientation = qw;

  // Linear acceleration
  float x = accelerometer.x() * 1 / ScaleFactor;
  float y = accelerometer.y() * 1 / ScaleFactor;
  float z = accelerometer.z() * 1 / ScaleFactor;

  // Apply low-pass filter
  x_filtered = alphaFilter * x + (1 - alphaFilter) * x_filtered;
  y_filtered = alphaFilter * y + (1 - alphaFilter) * y_filtered;
  z_filtered = alphaFilter * z + (1 - alphaFilter) * z_filtered;
  
  float accelerationMagnitude = sqrt(x_filtered * x_filtered + y_filtered * y_filtered + z_filtered * z_filtered);

  //--- Quaternion-Based Rotation ---
  float q_conjugate_x = -qx, q_conjugate_y = -qy, q_conjugate_z = -qz, q_conjugate_w = qw;
  float ax = x_filtered, ay = y_filtered, az = z_filtered;

  // First quaternion multiplication
  float qv0 = qw * 0 - qx * ax - qy * ay - qz * az;
  float qv1 = qw * ax + qy * az - qz * ay;
  float qv2 = qw * ay + qz * ax - qx * az;
  float qv3 = qw * az + qx * ay - qy * ax;

  // Second quaternion multiplication
  Gx = qv0 * q_conjugate_x + qv1 * q_conjugate_w + qv2 * q_conjugate_z - qv3 * q_conjugate_y;
  Gy = qv0 * q_conjugate_y - qv1 * q_conjugate_z + qv2 * q_conjugate_w + qv3 * q_conjugate_x;
  Gz = qv0 * q_conjugate_z + qv1 * q_conjugate_y - qv2 * q_conjugate_x + qv3 * q_conjugate_w;

  // Update velocities using trapezoidal integration
  vX += ((GxO + Gx) / 2.0) * DeltaTime;
  vY += ((GyO + Gy) / 2.0) * DeltaTime;
  vZ += ((GzO + Gz) / 2.0) * DeltaTime;

  // Update displacements
  dX += vX * DeltaTime;
  dY += vY * DeltaTime;
  dZ += vZ * DeltaTime;

  // Prepare data for logging

  //Set integrated variable to 0 when the device is still (see the parameter in >FILTPARA)
  if (accelerationMagnitude < accelerationThreshold) {
    // Reset velocities and displacements
    vX = vY = vZ = 0.0;
    dX = dY = dZ = 0.0;
  }

  // Affichage des valeurs
  // Serial.print("GyroX: "); Serial.print(GyroX);
  // Serial.print(" GyroY: "); Serial.print(GyroY);
  // Serial.print(" GyroZ: "); Serial.println(GyroZ);

  // Serial.print("MagX: "); Serial.print(Magx);
  // Serial.print(" MagY: "); Serial.print(Magy);
  // Serial.print(" MagZ: "); Serial.println(Magz);

  // Serial.print("AccX: "); Serial.print(x);
  // Serial.print(" AccY: "); Serial.print(y);
  // Serial.print(" AccZ: "); Serial.println(z);

  // Affichage compatible avec Serial Plotter
  // Serial.print(GyroX); Serial.print(" ");
  // Serial.print(GyroY); Serial.print(" ");
  // Serial.print(GyroZ); Serial.print(" ");
  // Serial.print(Magx);  Serial.print(" ");
  // Serial.print(Magy);  Serial.print(" ");
  // Serial.print(Magz);  Serial.print(" ");
  // Serial.print(x);     Serial.print(" ");
  // Serial.print(y);     Serial.print(" ");
  // Serial.println(z);

  // Serial.print(GyroX); Serial.print(" ");
  // Serial.print(GyroY); Serial.print(" ");
  // Serial.println(GyroZ);

  // Serial.print(Magx);  Serial.print(" ");
  // Serial.print(Magy);  Serial.print(" ");
  // Serial.println(Magz);

  // Serial.print(x+0.05);     Serial.print(" ");
  // Serial.print(y+0.05);     Serial.print(" ");
  // Serial.println(z-1);

  Serial.print(x); Serial.print(" ");
  Serial.print(y); Serial.print(" ");
  Serial.println(z);

  delay(10);
}
