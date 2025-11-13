#include <Arduino_BHY2.h>    // Library to access the Nicla Sense ME sensors
#include <ArduinoBLE.h>      // Library to handle Bluetooth Low Energy (BLE) communication
#include "Nicla_System.h"    // Provides system-level functions for Nicla boards
#include <math.h>            // To use math functions

// Define SensorsID
SensorXYZ linearAcc(SENSOR_ID_LACC);
SensorXYZ gyroscope(SENSOR_ID_GYRO);
SensorQuaternion quaternion(SENSOR_ID_RV);

// Define a custom BLE service and characteristic
BLEService imuService("12345678-1234-1234-1234-123456781234"); 
BLECharacteristic imuCharacteristic("12345679-1234-1234-1234-123456781234", BLERead | BLENotify, 16);

// Parameters initialisation

// Variables initialisation
static unsigned long prevTime = 0;
float accScale = 1/4096.0;
float lin_accScale = 16.0 / 65536.0;
float gyroScale = 2.0 * 3.14159265 / 360.0 * (2000.0 / 32768.0);
const float g = 9.81;

const int N = 200;
float tabPx[N], tabPy[N];
float tabDeltaTime[N];

int count = 0;
int swingEnd = 0;

float prevAccMag = 0.0;
float emaAlpha = 0.2;
float emaDerivAlpha = 0.3;
float emaAccMag = 0.0;       // EMA of acceleration magnitude
float emaDeriv = 0.0;        // EMA of derivative
float accMagTol = 0.1;
float derivMagTol = 2.0;

bool prevSwingState = false;
bool startCalc = false;

float maxGz = 0.0;
int idMaxGz = 0;

float lpAlpha = 0.4;
float filtAx = 0.0;
float filtAy = 0.0;
float prevFiltAx = 0.0;
float prevFiltAy = 0.0;

float vx = 0.0;
float vy = 0.0;
float px = 0.0;
float py = 0.0;
float new_px = 0.0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize BLE module
  if (!BLE.begin()) {
    Serial.println(" BLE init failed!");
    while (1);
  }

  // Initialize the sensor hub (BHI260) and sensors
  BHY2.begin(NICLA_STANDALONE);
  linearAcc.begin();
  gyroscope.begin();
  quaternion.begin();

  // Configure BLE settings
  BLE.setLocalName("NiclaBLE1");
  BLE.setAdvertisedService(imuService);

  imuService.addCharacteristic(imuCharacteristic);
  BLE.addService(imuService);
  BLE.advertise();

  Serial.println(" Nicla BLE active et en publicité...");
}

void loop() {
  ble_loop();
}

// Loop function to define characteristics and behavior when a BLE connection is established
void ble_loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connecté à ");
    Serial.println(central.address());

    count = 0;

    while (central.connected()) {
      // Très important : rafraîchir les capteurs à chaque cycle
      BHY2.update();

      unsigned long currentTime = millis();
      float deltaTime = (currentTime - prevTime) / 1000.0; // Delta time in seconds
      prevTime = currentTime;

      float ax = linearAcc.x() * accScale;
      float ay = linearAcc.y() * accScale;
      float az = linearAcc.z() * accScale;

      // float gx = gyroscope.x() * gyroScale;
      // float gy = gyroscope.y() * gyroScale;
      float gz = gyroscope.z() * gyroScale;

      // float qw = quaternion.w();
      // float qx = quaternion.x();
      // float qy = quaternion.y();
      // float qz = quaternion.z();

      //
      float accMag = sqrt(ax * ax + ay * ay + az * az);
      float derivMag = (accMag - prevAccMag) / deltaTime;

      // EMA smoothig
      emaAccMag = emaAlpha * accMag + (1 - emaAlpha) * emaAccMag;
      emaDeriv = emaDerivAlpha * derivMag + (1 - emaDerivAlpha) * emaDeriv;

      //
      bool magNearRef = abs(emaAccMag) < accMagTol;
      bool magSlow = abs(emaDeriv) < derivMagTol;

      bool swingState = !(magNearRef && magSlow);

      if (!prevSwingState && swingState) {
        count = 0;
        startCalc = true;

        maxGz = gz;
        idMaxGz = 0;

        prevFiltAx = 0.0; 
        prevFiltAy = 0.0;
        vx = 0.0;
        vy = 0.0;
        px = 0.0;
        py = 0.0;
        new_px = 0.0;
      } 

      if (startCalc){
        if (gz>maxGz) {
          maxGz = gz;
          idMaxGz = count;
        }

        // Low-pass filter
        if (count == 0) {
          filtAx = ax;
          filtAy = ay;
        } else {
          filtAx = lpAlpha * ax + (1-lpAlpha) * prevFiltAx;
          filtAy = lpAlpha * ay + (1-lpAlpha) * prevFiltAy;
        }

        // g to m/s^2
        filtAx *= g;
        filtAy *= g;

        //
        vx += ((filtAx + prevFiltAx) / 2.0) * deltaTime;
        vy += ((filtAy + prevFiltAy) / 2.0) * deltaTime;

        //
        px += vx*deltaTime;
        py += vy*deltaTime;

        //
        prevFiltAx = filtAx;
        prevFiltAy = filtAy;

        //
        if (px>0) {
          new_px = 0.0;
        } else {
          new_px = -px;
        }

        //
        tabPx[count] = new_px;
        tabPy[count] = py;
      }

      
      if (prevSwingState && !swingState) {
        startCalc = false;
        swingEnd = idMaxGz; //swingStart is always 0
        float dataSize = count+1;

        // //copier la phase swing
        // memcpy(prevTabPx, tabPx, (swingEnd+1) * sizeof(float));
        // memcpy(prevTabPy, tabPy, (swingEnd+1) * sizeof(float));

        // //copier la partie cste
        // for (int i = swingEnd; i < prevDataSize; i++) {
        //   prevTabPx[i] = 0.0;
        //   prevTabPy[i] = tabPy[swingEnd];
        // }

        float strideLength = tabPy[swingEnd];
        sendStrideLength(strideLength);
      }

      //à la fin
      prevSwingState = swingState;
      prevAccMag = accMag;
      count += 1;

      //
      // sendPos(new_px, y);

    }

    Serial.println("Déconnecté.");
  }
}

void sendMessage(float x, float y) {
  uint8_t data[8];
  memcpy(data, &x, 4);
  memcpy(data + 4, &y, 4);
  imuCharacteristic.writeValue(data, 8);
}

void sendStrideLength(float x) {
  uint8_t data[4];
  memcpy(data, &x, 4);
  imuCharacteristic.writeValue(data, 4);
}



