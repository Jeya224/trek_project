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

  // Linear acceleration
  float x = accelerometer.x() * 1 / ScaleFactor;
  float y = accelerometer.y() * 1 / ScaleFactor;
  float z = accelerometer.z() * 1 / ScaleFactor;

  // Apply low-pass filter
  x_filtered = alphaFilter * x + (1 - alphaFilter) * x_filtered;
  y_filtered = alphaFilter * y + (1 - alphaFilter) * y_filtered;
  z_filtered = alphaFilter * z + (1 - alphaFilter) * z_filtered;
  
  float accelerationMagnitude = sqrt(x_filtered * x_filtered + y_filtered * y_filtered + z_filtered * z_filtered);

  Serial.print(x);     Serial.print(" ");
  Serial.print(y);     Serial.print(" ");
  Serial.print(z);     Serial.print(" ");
  Serial.println(accelerationMagnitude);

  delay(10);
}
