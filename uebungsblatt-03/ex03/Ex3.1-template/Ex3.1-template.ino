// Task 1: Environmental Light Categorization
// Complete the implementation according to the task description.

#include <Adafruit_TinyUSB.h>
#include <Arduino.h>

// define necessary constants (e.g., calibration range, thresholds, timing)
// min and max values for the calibrated range (ii)
const uint16_t RANGE_MIN = 50;
const uint16_t RANGE_MAX = 3500;

// thresholds for categorize function
const uint8_t CAT_LOW = 30;
const uint8_t CAT_HIGH = 70;

// timing
const uint16_t TIMING_MS = 500;

// Pin
const uint8_t SENSOR_PIN = A0;

// implement a function that categorizes the normalized value
const char *categorize(int normalized) {
  // return "LOW", "MEDIUM", or "HIGH"
  if (normalized < CAT_LOW) {
    return "LOW";
  } else if (normalized >= CAT_HIGH) {
    return "HIGH";
  }
  return "MEDIUM";
}

void setup() {
  // initialize Serial communication
  Serial.begin(115200);

  while (!Serial)
    ; // Wait for USB Serial connection

  // configure ADC resolution to 12-bit
  analogReadResolution(12);
}

uint32_t lastMillis = 0;

void loop() {
  // implement non-blocking timing using millis()
  // Current Time
  uint32_t currentMillis = millis();

  // The system should sample every 500 ms
  if (currentMillis - lastMillis >= TIMING_MS) {
    // set time
    lastMillis = currentMillis;

    // read raw value from light sensor (12Bit ADC = 0-4095)
    uint16_t rawValue = analogRead(SENSOR_PIN);
    // clamp the raw value to a calibrated range
    uint32_t clampValue = constrain(rawValue, RANGE_MIN, RANGE_MAX);
    // TODO: normalize the value to a 0–100 scale
    uint32_t normValue =
        (clampValue - RANGE_MIN) * 100 / (RANGE_MAX - RANGE_MIN);
    // determine the category using the categorize() function
    const char *catValue = categorize(normValue);
    // print raw value, normalized value, and category to Serial
    Serial.print("Raw: ");
    Serial.println(rawValue);
    Serial.print("Normalized: ");
    Serial.print(normValue);
    Serial.println("%");
    Serial.print("Category: ");
    Serial.println(catValue);
    Serial.println("_______________");
  }
}
