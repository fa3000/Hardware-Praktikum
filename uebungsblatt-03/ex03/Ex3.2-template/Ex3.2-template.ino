// Task 2: Robust Temperature and Humidity Monitoring

#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <DHT.h>
#include <math.h>

// --- Configuration ---
// define sensor pin and type (DHT11)
const uint8_t DHT_PIN = 7;
#define DHTTYPE DHT11

// --- Objects ---
// create DHT sensor instance
DHT dht(DHT_PIN, DHTTYPE);

// --- Timing ---
unsigned long lastSample = 0;
// define sampling interval (2 seconds)
const unsigned long SAMPLING_INTERVAL = 2000;

// --- State Variables ---
// store last valid temperature and humidity
float lastValidTemp = 0.0;
float lastValidHum = 0.0;
// maintain a failure counter
uint8_t failureCounter = 0;
const uint8_t FAILURE_THRESHOLD = 5;

// --- Computation ---
float computeDewPoint(float tempC, float relHum) {
  // implement Magnus formula using natural logarithm
  // Magnus formula constants
  const float a = 17.62;
  const float b = 243.12;

  // Natural logarithm of relative humidity (0.0 to 1.0) + temperature term
  float alpha = log(relHum / 100.0) + (a * tempC) / (b + tempC);
  float dewPoint = (b * alpha) / (a - alpha);

  return dewPoint;
}

void setup() {
  Serial.begin(115200);
  while (!Serial && millis() < 3000)
    ;

  // initialize sensor
  dht.begin();
}

void loop() {
  unsigned long now = millis();

  // implement non-blocking sampling (2 s)
  if (now - lastSample >= SAMPLING_INTERVAL) {
    lastSample = now;

    // read temperature and humidity
    float currentTemp = dht.readTemperature();
    float currentHum = dht.readHumidity();

    // handle invalid readings (NaN)
    if (isnan(currentTemp) || isnan(currentHum)) {
      // Increment failure counter on error
      failureCounter++;
    } else {
      // Reset on Success
      failureCounter = 0;
      // Update last valid values
      lastValidTemp = currentTemp;
      lastValidHum = currentHum;
    }

    // compute dew point, print formatted output. If failure count exceeds
    // threshold print a warning.
    float dewPoint = computeDewPoint(lastValidTemp, lastValidHum);

    // Warning system: display message if failure counter exceeds threshold
    if (failureCounter > FAILURE_THRESHOLD) {
      Serial.print("[WARN] DHT failures: ");
      Serial.println(failureCounter);
    }

    // Print formatted output with one decimal place
    Serial.print("T=");
    Serial.print(lastValidTemp, 1);
    Serial.print("C RH=");
    Serial.print(lastValidHum, 1);
    Serial.print("% dewPoint =");
    Serial.print(dewPoint, 1);
    Serial.println("C");

    // Note: If we are using cached values because of a failure
    if (failureCounter > 0) {
      Serial.println("(Using last valid)");
    }
  }
}
