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
float vX = 0, vY = 0, vZ = 0;      // Velocity components
float dX = 0, dY = 0, dZ = 0;      // Displacement components
float Gx = 0.0, Gy = 0.0, Gz = 0.0;
float GxO = 0.0, GyO = 0.0, GzO = 0.0;
float x_filtered = 0.0, y_filtered = 0.0, z_filtered = 0.0;
int Pnum = 0;
float ScaleFactor = 4096.0;

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

// Parameters for testing
float r = 1.0;           // rayon de la spirale
float PosZ = -2.0;          // position Z initiale
float dPosZ = 0.01;         // incrément Z à chaque point
float theta = 0.0;       // angle initial
float dtheta = 0.1;      // incrément d'angle

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

void processSensors(float* outX, float* outY, float* outZ) {
  BHY2.update();            // Update sensor readings
  
  // Calculate timing
  unsigned long currentTime = millis();
  Time = currentTime / 1000.0; // Time in seconds
  DeltaTime = (currentTime - prevTime) / 1000.0; // Delta time in seconds
  prevTime = currentTime;

  // Quaternion normalization
  float qw = quaternion.w(), qx = quaternion.x(), qy = quaternion.y(), qz = quaternion.z();
  float norm = sqrt(qw * qw + qx * qx + qy * qy + qz * qz);
  if (norm > 0) {
    qw /= norm; qx /= norm; qy /= norm; qz /= norm;
  }
  Xorientation = qx; Yorientation = qy; Zorientation = qz; Worientation = qw;

  // Linear acceleration
  float ax = accelerometer.x() * 1 / ScaleFactor;
  float ay = accelerometer.y() * 1 / ScaleFactor;
  float az = accelerometer.z() * 1 / ScaleFactor;

  // // Apply low-pass filter
  // x_filtered = alphaFilter * ax + (1 - alphaFilter) * x_filtered;
  // y_filtered = alphaFilter * ay + (1 - alphaFilter) * y_filtered;
  // z_filtered = alphaFilter * az + (1 - alphaFilter) * z_filtered;
  
  // Compute acceleration magnitude and its derivative 
  float accMagnitude = sqrt(ax * ax + ay * ay + az * az);
  float deriv = (accMagnitude - prevAccMag) / DeltaTime;

  // EMA smoothig
  emaAccMag = alpha * accMagnitude + (1 - alpha) * emaAccMag;
  emaDeriv = alpha_deriv * deriv + (1 - alpha_deriv) * emaDeriv;

  //--- Quaternion-Based Rotation ---
  float q_conjugate_x = -qx, q_conjugate_y = -qy, q_conjugate_z = -qz, q_conjugate_w = qw;

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

  // Check if device is still
  bool near_ref = fabs(emaAccMag - accMagnitudeRef) < accMagnitudeTolerance;
  bool slow = fabs(emaDeriv) < derivAccMagnitudeTolerance;
  // bool isReset = false;

  //Set integrated variable to 0 when the device is still
  if (near_ref && slow) {
    // Reset velocities
    vX = vY = vZ = 0.0;
    // isReset = true;
    
  }
  // Serial.print(isReset); Serial.print(" ");

  // Update displacements
  dX += vX * DeltaTime;
  dY += vY * DeltaTime;
  dZ += vZ * DeltaTime;

  // Affichage des valeurs
  // Serial.print(qx); Serial.print(" ");
  // Serial.print(qy); Serial.print(" ");
  // Serial.print(qz); Serial.print(" ");
  // Serial.println(qw);

  // Serial.print(near_ref-1.1); Serial.print(" ");
  // Serial.print(slow-2.2); Serial.print(" ");
  // Serial.println(accMagnitude);

  // Serial.print(vX); Serial.print(" ");
  // Serial.print(vY); Serial.print(" ");
  // Serial.print(vY); Serial.print(" ");
  // Serial.println(accMagnitude);

  // Serial.print(dX); Serial.print(" ");
  // Serial.print(dY); Serial.print(" ");
  // Serial.println(dZ);

  // Simulating real time pts
  float PosX = r * sin(theta);
  float PosY = r * cos(theta);

  *outX = PosX;
  *outY = PosY;
  *outZ = PosZ;

  // incrémente theta et z pour le prochain point
  theta += dtheta;
  PosZ += dPosZ;
}

//###MAINL_Main loop
void loop() {
  nicla::leds.setColor(green);
  float x, y, z;
  processSensors(&x, &y, &z);

  // envoie x, y, z séparés par un espace
  Serial.print(x, 4); Serial.print(" ");
  Serial.print(y, 4); Serial.print(" ");
  Serial.println(z, 4);

  delay(10);
}
