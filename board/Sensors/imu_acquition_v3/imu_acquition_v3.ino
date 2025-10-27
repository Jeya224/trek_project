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
SensorXYZ linearAcc(SENSOR_ID_LACC);

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
float dx = 0, dy = 0, dz = 0;      // Displacement components
float Gx = 0.0, Gy = 0.0, Gz = 0.0;
float prevGx = 0.0, prevGy = 0.0, prevGz = 0.0;
float Magx, Magy, Magz;
float x_filtered = 0.0, y_filtered = 0.0, z_filtered = 0.0;
int Pnum = 0;
float ScaleFactor = 4096.0;

float prev_ax = 0.0, prev_ay = 0.0, prev_az = 0.0;
float prev_vx = 0.0, prev_vy = 0.0, prev_vz = 0.0;

// Filtering parameters
const float alphaFilter = 1;     // Low-pass filter coefficient

// Parameters for still detection
const float accMagnitudeRef = 1.0;            // reference acceleration magnitude when device is still
const float accMagnitudeTolerance = 0.2;      // tolerance around reference magnitude
const float derivAccMagnitudeTolerance = 0.15; // threshold for derivative Magnitude
const float alpha = 0.2;                      // smoothing factor for acceleration magnitude
const float alpha_deriv = 0.3;                // smoothing factor for derivative

// EMA (Exponential Moving Average) variables
float emaAccMag = 1.0;       // EMA of acceleration magnitude
float emaDeriv = 0.0;        // EMA of derivative
float prevAccMag = 1.0;      // previous acceleration magnitude

//###SETUP_setup loop
void setup() {
  Serial.begin(115200);
  Serial.println("Start");

  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(yellow);

  // Sensor Initialization
  BHY2.begin(NICLA_STANDALONE);
  linearAcc.begin();

  nicla::leds.setColor(green);
}

//###MAINL_Main loop
void loop() {
  BHY2.update();            // Update sensor readings
  nicla::leds.setColor(green);


  unsigned long currentTime = millis();
  DeltaTime = (currentTime - prevTime) / 1000.0; // Delta time in seconds
  prevTime = currentTime;

  float ax = linearAcc.x() * 16.0 / 65536.0 * 9.81;
  float ay = linearAcc.y() * 16.0 / 65536.0 * 9.81;
  float az = linearAcc.z() * 16.0 / 65536.0 * 9.81;

  // Serial.print(ax); Serial.print(" ");
  // Serial.print(ay); Serial.print(" ");
  // Serial.println(az);

  // intégration trapézoïdale pour vitesse
  vX += 0.5 * (ax + prev_ax) * DeltaTime;
  vY += 0.5 * (ay + prev_ay) * DeltaTime;
  vZ += 0.5 * (az + prev_az) * DeltaTime;

  // intégration trapézoïdale pour position
  dx += 0.5 * (vX + prev_vx) * DeltaTime;
  dy += 0.5 * (vY + prev_vy) * DeltaTime;
  dz += 0.5 * (vZ + prev_vz) * DeltaTime;

  // mémoriser anciennes valeurs
  prev_ax = ax;
  prev_ay = ay;
  prev_az = az;

  prev_vx = vX;
  prev_vy = vY;
  prev_vz = vZ;

  Serial.print(dx); Serial.print(" ");
  Serial.print(dy); Serial.print(" ");
  Serial.println(dz);
}