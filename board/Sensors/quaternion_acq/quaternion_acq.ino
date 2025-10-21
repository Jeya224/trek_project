//###LIB_Libraries inclusion
#include "Nicla_System.h"    // Requirement for NICLA Sense ME
#include "Arduino_BHY2.h"    // Requirement for the Bosch sensor
#include <math.h>            // To use math functions

//###SENID_DefineSensorID
SensorXYZ gyroscope(SENSOR_ID_GYRO);
SensorXYZ accelerometer(SENSOR_ID_ACC);
SensorQuaternion quaternion(SENSOR_ID_RV);
SensorXYZ magnetometer(SENSOR_ID_MAG);

float Xorientation, Yorientation, Zorientation, Worientation;
float ScaleFactor = 4096.0;

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

}

//###MAINL_Main loop
void loop() {
  BHY2.update();            // Update sensor readings
  nicla::leds.setColor(green);
  
  // Quaternion normalization
  float qw = quaternion.w(), qx = quaternion.x(), qy = quaternion.y(), qz = quaternion.z();
  float norm = sqrt(qw * qw + qx * qx + qy * qy + qz * qz);
  if (norm > 0) {
    qw /= norm; qx /= norm; qy /= norm; qz /= norm;
  }

  // Send quaternion via serial in CSV
  Serial.print(qw); Serial.print(",");
  Serial.print(qx); Serial.print(",");
  Serial.print(qy); Serial.print(",");
  Serial.println(qz);

  delay(10); // 100 Hz
}
