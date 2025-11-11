//###LIB_Libraries inclusion
#include "Nicla_System.h"    // Requirement for NICLA Sense ME
#include "Arduino_BHY2.h"    // Requirement for the Bosch sensor
#include <math.h>            // To use math functions
#include <SPI.h>             // To use SPI connectivity


//###SENID_DefineSensorID
SensorXYZ gyroscope(SENSOR_ID_GYRO);
SensorXYZ accelerometer(SENSOR_ID_ACC);
SensorQuaternion quaternion(SENSOR_ID_RV);
SensorXYZ magnetometer(SENSOR_ID_MAG);
SensorXYZ linearAcc(SENSOR_ID_LACC);

//###VARINIT_Variables initialisation
static unsigned long prevTime = 0;  // To track time between measurements

float Time, DeltaTime;
float accScale = 1/4096.0;
float gyroScale = 2.0 * 3.14159265 / 360.0 * (2000.0 / 32768.0);
float laccScale = 16.0 / 65536.0;

//###SETUP_setup loop
void setup() {
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Start");

  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(yellow);

  // Sensor Initialization
  BHY2.begin(NICLA_STANDALONE);
  gyroscope.begin();
  accelerometer.begin();
  quaternion.begin();
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
  
  float ax = accelerometer.x() * accScale;
  float ay = accelerometer.y() * accScale;
  float az = accelerometer.z() * accScale;

  float gx = gyroscope.x() * gyroScale;
  float gy = gyroscope.y() * gyroScale;
  float gz = gyroscope.z() * gyroScale;

  float qw = quaternion.w();
  float qx = quaternion.x();
  float qy = quaternion.y();
  float qz = quaternion.z();

  float lin_ax = linearAcc.x() * accScale;
  float lin_ay = linearAcc.y() * accScale;
  float lin_az = linearAcc.z() * accScale;

  Serial.print(DeltaTime); Serial.print(" ");
  Serial.print(ax); Serial.print(" ");
  Serial.print(ay); Serial.print(" ");
  Serial.print(az); Serial.print(" ");
  Serial.print(gx); Serial.print(" ");
  Serial.print(gy); Serial.print(" ");
  Serial.print(gz); Serial.print(" ");
  Serial.print(qw); Serial.print(" ");
  Serial.print(qx); Serial.print(" ");
  Serial.print(qy); Serial.print(" ");
  Serial.print(qz); Serial.print(" ");
  Serial.print(lin_ax); Serial.print(" ");
  Serial.print(lin_ay); Serial.print(" ");
  Serial.println(lin_az);

  delay(10);
}
