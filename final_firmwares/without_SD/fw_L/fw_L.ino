#include <Arduino_BHY2.h>    // Library to access the Nicla Sense ME sensors
#include <ArduinoBLE.h>      // Library to handle Bluetooth Low Energy (BLE) communication
#include "Nicla_System.h"    // Provides system-level functions for Nicla boards
#include <math.h>            // To use math functions

// Parameters initialisation
const char* bleDeviceName = "NiclaBLE1";
const int BLE_FLOAT_COUNT_TX = 2;
const unsigned long bleWaitDuration = 20000; //20sec
const unsigned long bleReWaitDuration = 200000; //200sec

// Define SensorsID
SensorXYZ linearAcc(SENSOR_ID_LACC);
SensorXYZ gyroscope(SENSOR_ID_GYRO);

// Define a custom BLE service and characteristic
const int bufferSize = BLE_FLOAT_COUNT_TX * 4;
BLEService imuService("12345678-1234-1234-1234-123456781234");
BLECharacteristic imuCharacteristic("12345679-1234-1234-1234-123456781234", BLERead | BLENotify, bufferSize);
BLECharacteristic cmdCharacteristic("12345671-1234-1234-1234-123456781234", BLEWrite | BLEWriteWithoutResponse, 1);

// UI mode enum
enum UIMode : uint8_t {
  MINIMAL = 0,
  DEBUG = 1,
  RESET = 2
};

// Variables initialisation
bool bleConnected = false;
UIMode ui_mode = MINIMAL;; // 0 = minimal mode, 1 = debug, 2 = reset
unsigned long connectStartTime = 0;
bool reconnecting = false;

static unsigned long prevTime = 0;
float accScale = 1/4096.0;
float gyroScale = 2.0 * 3.14159265 / 360.0 * (2000.0 / 32768.0);
const float g = 9.81;

int count = 0;
float prevAccMag = 0.0;
float emaAlpha = 0.2;
float emaDerivAlpha = 0.3;
float emaAccMag = 0.0;       // EMA of acceleration magnitude
float emaDeriv = 0.0;        // EMA of derivative
float accMagTol = 0.1;
float derivMagTol = 2.0;

bool prevSwingState = false;
bool startCalc = false;

float minGz = 0.0;
int idMinGz = 0;

float prevFiltAy = 0.0;
float vy = 0.0;
float py = 0.0;
float strideLength = -1.0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // Initialize BLE module
  if (!BLE.begin()) {
    Serial.println(" BLE init failed!");
    while (1);
  }

  //
  nicla::begin();
  nicla::leds.begin();
  nicla::leds.setColor(green);

  // Initialize the sensor hub (BHI260) and sensors
  BHY2.begin(NICLA_STANDALONE);
  linearAcc.begin();
  gyroscope.begin();

  // Configure BLE settings
  BLE.setLocalName(bleDeviceName);
  BLE.setAdvertisedService(imuService);
  imuService.addCharacteristic(imuCharacteristic);
  imuService.addCharacteristic(cmdCharacteristic);
  BLE.addService(imuService);

  // Set event handler for UI mode command
  cmdCharacteristic.setEventHandler(BLEWritten, receiveUIModeCommand);

  BLE.advertise();
  Serial.println("Advertising...");

  // Independent BLE check phase
  connectStartTime = millis();
  while (millis() - connectStartTime < bleWaitDuration) {
    BLEDevice central = BLE.central();
    if (central) {
      Serial.println("Central connected during initial phase!");
      bleConnected = true;
      nicla::leds.setColor(blue);
      break; // connexion établie, sortir de la boucle d'attente
    }
  }

  if (!bleConnected) {
    BLE.stopAdvertise();
    nicla::leds.setColor(red);
    Serial.println("No connection after " + String(bleWaitDuration / 1000) + " sec, offline mode");
  }

  Serial.println("Starting main loop...");
}

void loop() {
  BLEDevice central = BLE.central();
  if (central && central.connected()) {
    if(!bleConnected) {
      Serial.println("Reconnected..");
      nicla::leds.setColor(blue);
      bleConnected = true;
      reconnecting = false;

      count = 0;
      initParams();
    }
  } else {
    if(bleConnected) {
      Serial.println("Disconnected..");
      nicla::leds.setColor(red);
      bleConnected = false;
      ui_mode = MINIMAL;

      // Start reconnection attempt
      reconnecting = true;
      connectStartTime = millis();
      BLE.advertise();
    }
  }

  // Stop reconnect phase if timeout exceeded
  if (reconnecting && (millis() - connectStartTime >= bleReWaitDuration)) {
    reconnecting = false;
    BLE.stopAdvertise();
    Serial.println("Reconnect timeout, continuing offline");
  }

  // Handle tasks depending on UI mode
  switch (ui_mode) {
    case MINIMAL: {
      computeStrideLength();
      delay(20);
      break;
    }

    case DEBUG:
      // debug mode tasks
      Serial.println("debug");
      nicla::leds.setColor(white);
      break;

    case RESET:
      // reset mode tasks
      Serial.println("reset");
      nicla::leds.setColor(red);
      delay(100);
      break;
  }
  
}

void receiveUIModeCommand(BLEDevice central, BLECharacteristic characteristic) {
  if (characteristic.valueLength() > 0) {
    ui_mode = static_cast<UIMode>(*(characteristic.value())); // read single byte and cast to enum
    Serial.print("UI mode updated to: ");
    Serial.println(ui_mode);

    // Update LED color once when UI mode changes via BLE command
    switch (ui_mode) {
      case MINIMAL: nicla::leds.setColor(blue); Serial.println("ble minimal-----");break;
      case DEBUG: nicla::leds.setColor(white); break;
    }
  }
}

void sendMessage(float x, float y) {
  uint8_t data[bufferSize];
  memcpy(data, &x, 4);
  memcpy(data + 4, &y, 4);
  imuCharacteristic.writeValue(data, bufferSize);
}

void computeStrideLength() {
  BHY2.update();

  unsigned long currentTime = millis();
  float deltaTime = (currentTime - prevTime) / 1000.0; // Delta time in seconds
  prevTime = currentTime;

  float ax = linearAcc.x() * accScale;
  float ay = linearAcc.y() * accScale;
  float gz = gyroscope.z() * gyroScale;

  //
  float accMag = sqrt(ax * ax + ay * ay);
  float derivMag = (accMag - prevAccMag) / deltaTime;
  prevAccMag = accMag;

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

    minGz = gz;
    idMinGz = 0;
    strideLength = -1.0;
  }

  if (startCalc){
    float filtAy = ay*g;
    vy += ((filtAy + prevFiltAy) / 2.0) * deltaTime;
    py += vy*deltaTime;
    prevFiltAy = filtAy;

    if (gz<minGz) {
      minGz = gz;
      idMinGz = count;
      strideLength = py;
    }
  }
  
  if (prevSwingState && !swingState) {
    startCalc = false;
    if(strideLength > 0.01){
      strideLength = strideLength*100;
      strideLength = (strideLength + 16.4)/0.682;
      if (bleConnected) {
        sendMessage(strideLength, ay);
      }
    }
    //
    initParams();
  }

  //à la fin
  prevSwingState = swingState;
  count += 1;
} 

void initParams() {
  prevFiltAy = 0.0;
  vy = 0.0;
  py = 0.0;
}