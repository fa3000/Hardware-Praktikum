/*
 * Exercise 04: Wearable Context Recognition with IMU
 * Author: Roozbeh Ghazavi
 * Year: 2026
 * Board: XIAO nRF52840 Sense
 * Sensor: LSM6DS3
 *
 * Implement ALL TODO sections.
 */

#include "LSM6DS3.h"
#include "Wire.h"
#include <Arduino.h>

// =====================
// Part E: BLE Library
// =====================
#include <ArduinoBLE.h>

LSM6DS3 myIMU(I2C_MODE, 0x6A);

// Constants
#define CONVERT_G_TO_MS2 9.80665f
#define FREQUENCY_HZ 50
#define INTERVAL_MS (1000 / (FREQUENCY_HZ + 1))
#define ORIENTATION_THRESHOLD 6.92964f // sin(45°) * 9,80
#define GESTURE_THRESHOLD 1
#define GYRO_THRESHOLD 100.0f

static unsigned long last_interval_ms = 0;

// =====================
// Part C: Accelerometer Buffer (Z-axis)
// =====================
#define WINDOW_SIZE 10

float az_buffer[WINDOW_SIZE];
int buffer_index = 0;
bool buffer_full = false;

// =====================
// Part C & D: Last Detected Gestures
// =====================
// TODO: Create variables to store last detected gestures
// - lastDetectedGesture: from accelerometer FSM
// - lastDynamicGesture: from gyroscope

String lastDetectedGesture = "NONE";
String lastDynamicGesture = "NONE";

// =====================
// Part E: BLE Objects
// =====================
// TODO: Create BLE service and characteristic
// Example:
BLEService imuService("180C");
BLECharacteristic imuCharacteristic("2A56", BLERead | BLENotify, 100);

// =====================
// Part B: Orientation Detection
// =====================
String detectOrientation(float ax, float ay, float az) {

  // TODO: Detect device orientation from accelerometer
  // Use thresholds on ay and az axes

  // If the device does not get additional acceleration from
  // outside, no two states can be reached at once.

  // Up between 45° and 135°
  if (az > ORIENTATION_THRESHOLD) {
    return "FACE UP";
  }
  // Left between 135° and 225°
  else if (ay > ORIENTATION_THRESHOLD) {
    return "LEFT";
  }
  // Down between 225° and 315°
  else if (az < -ORIENTATION_THRESHOLD) {
    return "FACE DOWN";
  }
  // Right between 315° and 45°
  else if (ay < -ORIENTATION_THRESHOLD) {
    return "RIGHT";
  }

  return "UNKNOWN";
}

// =====================
// Part C: Accelerometer-based Gesture Detection
// =====================
String detectGestureWindow() {

  // TODO: Detect SUPINATION/PRONATION from Z-axis acceleration buffer
  // Analyze min/max range and motion direction

  // Compute min and max values
  float min = az_buffer[0];
  float max = az_buffer[0];

  for (int i = 1; i < WINDOW_SIZE; i++) {
    if (az_buffer[i] > max) {
      max = az_buffer[i];
    }
    if (az_buffer[i] < min) {
      min = az_buffer[i];
    }
  }

  // Compute Differenz
  float diff = max - min;

  if (diff > GESTURE_THRESHOLD) {
    // Compare first and last value for direction
    // Ring buffer: Element 0 is the oldest value and WINDOW_SIZE-1 is the
    // newest
    if (az_buffer[WINDOW_SIZE - 1] < az_buffer[0]) {
      return "SUPINATION";
    }
    if (az_buffer[WINDOW_SIZE - 1] > az_buffer[0]) {
      return "PRONATION";
    }
  }
  return "NONE";
}

// =====================
// Part D: Gyroscope-based Dynamic Gesture Detection
// =====================
String detectDynamicGesture(float gyrX, float gyrY, float gyrZ) {

  // TODO: Detect 6 gestures using gyroscope
  // Gestures: TILT_LEFT, TILT_RIGHT, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT
  // Return strongest gesture (highest magnitude)

  char dominantAxis = ' ';

  // we need the magnitude
  float absX = abs(gyrX);
  float absY = abs(gyrY);
  float absZ = abs(gyrZ);

  // identify strongest gesture (can't use max(), because we need to keep the
  // sign) and check for threshold
  if (absX >= absY && absX >= absZ) {
    dominantAxis = 'X';
    if (absX < GYRO_THRESHOLD)
      return "NONE";
  } else if (absY >= absX && absY >= absZ) {
    dominantAxis = 'Y';
    if (absY < GYRO_THRESHOLD)
      return "NONE";
  } else if (absZ >= absY && absZ >= absX) {
    dominantAxis = 'Z';
    if (absZ < GYRO_THRESHOLD)
      return "NONE";
  }

  // check orientation
  switch (dominantAxis) {
  case 'X':
    if (gyrX >= 0) {
      return "TILT_LEFT";
    } else {
      return "TILT_RIGHT";
    }
    break;
  case 'Y':
    if (gyrY >= 0) {
      return "MOVE_UP";
    } else {
      return "MOVE_DOWN";
    }
    break;
  case 'Z':
    if (gyrZ >= 0) {
      return "MOVE_LEFT";
    } else {
      return "MOVE_RIGHT";
    }
    break;
  }
  return "NONE";
}

// =====================
// Part C (Bonus): Gesture State Machine (FSM)
// =====================
enum State { IDLE, MOVING, DETECTED };

State currentState = IDLE;

String detectGestureFSM() {

  // TODO: Implement FSM for robust gesture detection
  // Transitions: IDLE → MOVING → DETECTED → IDLE
  // Store detected gesture in lastDetectedGesture

  return "NONE";
}

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  if (myIMU.begin() != 0) {
    Serial.println("IMU initialization failed!");
    while (1)
      ;
  }

  Serial.println("IMU initialized.");

  // =====================
  // Part E: BLE Setup
  // =====================
  // DONE: Initialize BLE, add service/characteristic, start advertising
  if (!BLE.begin()) {
    Serial.println("BLE init failed!");
    while (1)
      ;
  }
  BLE.setLocalName("XIAO-IMU");
  BLE.setAdvertisedService(imuService);
  imuService.addCharacteristic(imuCharacteristic);
  BLE.addService(imuService);
  BLE.advertise();
  Serial.println("BLE initialized.");
}

void loop() {
  BLE.poll();
  if (millis() > last_interval_ms + INTERVAL_MS) {
    last_interval_ms = millis();

    // =====================
    // Part A: IMU Data Acquisition
    // =====================
    // DONE: Read accelerometer (ax, ay, az) from myIMU
    // DONE: Convert accelerometer from G to m/s² using CONVERT_G_TO_MS2
    // DONE: Read gyroscope (gyrX, gyrY, gyrZ) from myIMU

    float ax = myIMU.readFloatAccelX() * CONVERT_G_TO_MS2; // TODO
    float ay = myIMU.readFloatAccelY() * CONVERT_G_TO_MS2; // TODO
    float az = myIMU.readFloatAccelZ() * CONVERT_G_TO_MS2; // TODO

    float gyrX = myIMU.readFloatGyroX(); // TODO
    float gyrY = myIMU.readFloatGyroY(); // TODO
    float gyrZ = myIMU.readFloatGyroZ(); // TODO

    // =====================
    // Part C: Accelerometer Buffer Management
    // =====================
    az_buffer[buffer_index] = az;
    buffer_index++;

    if (buffer_index >= WINDOW_SIZE) {
      buffer_index = 0;
      buffer_full = true;
    }

    // =====================
    // Part B: Orientation Detection
    // =====================
    String orientation = detectOrientation(ax, ay, az);

    // =====================
    // Part C: Accelerometer-based Gesture Detection
    // =====================
    // DONE: Call detectGestureWindow() or detectGestureFSM()
    // DONE: If gesture detected (not "NONE"), store in lastDetectedGesture

    if (buffer_full) {
      String detectedGesture =
          detectGestureWindow(); // DONE: Replace with actual detection
      if (detectedGesture != "NONE") {
        lastDetectedGesture = detectedGesture;
      }
      // Wait until buffer is full again before next gesture detection
      buffer_full = false;
      buffer_index = 0;
    }

    // =====================
    // Part D: Gyroscope-based Dynamic Gesture Detection
    // =====================
    // DONE: Call detectDynamicGesture()
    String dynamicGesture = detectDynamicGesture(gyrX, gyrY, gyrZ);

    // DONE: If gesture detected (not "NONE"), store in lastDynamicGesture
    if (dynamicGesture != "NONE") {
      lastDynamicGesture = dynamicGesture;
    }

    // =====================
    // Serial Output (USB)
    // =====================
    Serial.print("ax: ");
    Serial.print(ax);
    Serial.print(" | ay: ");
    Serial.print(ay);
    Serial.print(" | az: ");
    Serial.print(az);

    Serial.print(" | gyrX: ");
    Serial.print(gyrX);
    Serial.print(" | gyrY: ");
    Serial.print(gyrY);
    Serial.print(" | gyrZ: ");
    Serial.print(gyrZ);

    Serial.print(" | Orientation: ");
    Serial.print(orientation);

    Serial.print(" | Accelerometer Gesture: ");
    Serial.print(lastDetectedGesture);

    Serial.println(" | Gyro Gesture: ");
    Serial.println(lastDynamicGesture);

    // =====================
    // Part E: Bluetooth Low Energy (BLE) Communication
    // =====================
    // DONE: Format and send data via BLE:
    // - Accelerometer readings (ax, ay, az)
    // - Gyroscope readings (gyrX, gyrY, gyrZ)
    // - Orientation detection result
    // - Gesture detection results (FSM and Gyro)

    // Construct the formatted data string (matches Serial output format)
    String bleData = "ax: " + String(ax, 2) + " | ay: " + String(ay, 2) +
                     " | az: " + String(az, 2) + " | gyrX: " + String(gyrX, 2) +
                     " | gyrY: " + String(gyrY, 2) +
                     " | gyrZ: " + String(gyrZ, 2) + "\n" +
                     "Orientation: " + orientation +
                     " | FSM Gesture: " + lastDetectedGesture +
                     " | Gyro Gesture: " + lastDynamicGesture;

    // Send the data string wirelessly via BLE
    imuCharacteristic.writeValue(bleData.c_str());
  }
}
