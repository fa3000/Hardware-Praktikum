// Task 4: Smart Plant Monitoring Station

#include <ArduinoBLE.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <U8g2lib.h>
#include <DHT.h>

// --- Hardware Configuration ---
// TODO: define pins (DHT, light sensor, LED, buzzer)

// DHT
const uint8_t DHT_PIN = D7; 
const uint8_t DHTTYPE = DHT11;
// light sensor
const uint8_t SENSOR_PIN_light = A0;
// LED
const uint8_t LED_PIN = LEDR;
// buzzer
const uint8_t BUZZER_PIN = D3;


// --- System Constants ---
// TODO: define timing constants for:
// - sensor sampling
// - display refresh (~2 Hz)
// - BLE transmission (1 Hz)
// - warm-up duration (30 s)

const unsigned long updateTimeDisplay = 500;
const unsigned long updateTimeBLE = 1000;
const unsigned long updateTimeWarmUP = 30000;

const unsigned long updateTimeLight = 500;
const unsigned long updateTimeTemperature = 2000;
const unsigned long updateTimeAir = 1000;

const unsigned long updateTimeLEDAttention = 500;
const unsigned long updateTimeLEDStressed = 250;

const unsigned long warmUpDuration = 30000;

const unsigned long updateHealthScoring = 1000;

const unsigned long buzzerIntervall = 500; // Activate buzzer for 500ms


// --- Objects ---
// TODO: initialize display, sensors, BLE service and characteristic

//display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// Air Quality Sensor
Adafruit_SGP30 sgp;  
 // temperature and humidity Sensor     
DHT dht(DHT_PIN, DHTTYPE);
// BLE service and characteristic
BLEService plantService("fff0");
BLEStringCharacteristic plantCharacteristic("fff1", BLERead | BLENotify, 50); // string with max 50 symbols


// --- FSM ---
enum SystemState {
    STATE_INIT,
    STATE_HEALTHY,
    STATE_ATTENTION,
    STATE_STRESSED
};

SystemState currentState = STATE_INIT;


// TODO: define timing variables for asynchronous operation
unsigned long LastSampleDisplay = 0;
unsigned long LastSampleBLE = 0;
unsigned long LastSampleWarmUp = 0;

unsigned long LastSampleLight = 0;
unsigned long LastSampleTemperatur = 0;
unsigned long LastSampleAir = 0;

unsigned long LastHealthScoring = 0;

//LED
unsigned long LastSampleLED = 0;

// Buzzer
unsigned long LastBuzzerActivation = 0;


// TODO: define variables for sensor data storage

float lastValidTemperature = 0.0;
float lastValidHumidity = 0.0;
uint16_t lastValidECO2 = 0;
uint16_t lastValidLight = 0;

float healthScore = 0;

//system start time
unsigned long systemStartTime = 0;

//LED flag
bool LED_ON = false;

// TODO: helper function(s), e.g.:
// - state to string conversion

String stateToString(SystemState state){
    switch (state){
        case STATE_INIT: 
            return "INIT";
        case STATE_HEALTHY:
            return "HEALTHY";
        case STATE_ATTENTION:
            return "ATTENTIOM";
        case STATE_STRESSED:
            return "STRESSED";
        default:
            return "UNKNOWN";
    }
}

// display helper function
void display_values(SystemState state,
                    float temperature,
                    float humidity,
                    uint16_t lightLevel,
                    uint16_t eCo2
                    ) 
                    {

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);
  // show state 
  u8g2.setCursor(0, 11);
  u8g2.print("STATE: ");
  u8g2.print(stateToString(state));
  // show temperatur 
  u8g2.setCursor(0,22);
  u8g2.print("Temperature: ");
  u8g2.print(temperature, 1);
  u8g2.print((char)176); // Gradzeichen
  u8g2.print("C");
  // show humidity
  u8g2.setCursor(0, 33);
  u8g2.print("Humidity: ");
  u8g2.print(humidity, 1);
  u8g2.print("%");
  // show light level
  u8g2.setCursor(0, 44);
  u8g2.print("Light: ");
  u8g2.print(lightLevel);
  u8g2.print("%");
  // show CO2 Levels
  u8g2.setCursor(0, 55);
  u8g2.print("eCO2: ");
  u8g2.print(eCo2);
  u8g2.print("ppm");
  u8g2.sendBuffer();
}

//LED alert
// switches LED on / off
void SetLED(boolean ledON) {
  if (ledON) {
    digitalWrite(LED_PIN, LOW);
  } else {
    digitalWrite(LED_PIN, HIGH);
  }
}

bool buzzerIsActive = false;

void AlertSystem(SystemState state, unsigned long now){
    // LED Logic
    switch (state){
        case STATE_INIT:
            SetLED(false);
            LED_ON = false; 
            break;

        case STATE_HEALTHY:
            SetLED(true);
            LED_ON = true;
            break;

        case STATE_ATTENTION:
            if(now - LastSampleLED >= updateTimeLEDAttention){
                LED_ON = !LED_ON;
                SetLED(LED_ON);
                LastSampleLED = now;
            }
            break;

        case STATE_STRESSED:
            if(now - LastSampleLED >= updateTimeLEDStressed){
                LED_ON = !LED_ON;
                SetLED(LED_ON);
                LastSampleLED = now;
            }
            break;
        }
    // Buzzer Logic
    if(lastValidECO2 > 2200 && state != STATE_INIT){
        if (now - LastBuzzerActivation >=  2 * buzzerIntervall){ // Activate buzzer every 1s (500ms on / 500ms off)
            tone(BUZZER_PIN, 1000, buzzerIntervall); // Activate buzzer_pin with frequency of 1000hz for 500ms
            LastBuzzerActivation = now;
        }
    }
}

// - value normalization (light)
const uint16_t RANGE_MIN = 50;
const uint16_t RANGE_MAX = 3500;

uint32_t normalize(uint32_t rawValue){
    // clamp the raw value to a calibrated range
    uint32_t clampValue = constrain(rawValue, RANGE_MIN, RANGE_MAX);
    //normalize the value to a 0–100 scale
    uint32_t normValue =(clampValue - RANGE_MIN) * 100 / (RANGE_MAX - RANGE_MIN);
    return normValue;
}

void setup() {
    Serial.begin(115200);

    // TODO: initialize hardware (pins, display, sensors)
    u8g2.begin();
    dht.begin();
    pinMode(LED_PIN, OUTPUT);          // LED
    pinMode(BUZZER_PIN, OUTPUT);       // Buzzer
    analogReadResolution(12);          // Set ADC Resolution to 12 Bit

    if (!sgp.begin()) {
        Serial.println("SGP30 init failed");
        while (true);
    }
    
    // TODO: initialize BLE and start advertising

    // BLE code is inspired from the adruino github example for BLE with notify
    // https://github.com/arduino-libraries/ArduinoBLE/blob/master/examples/Peripheral/BatteryMonitor/BatteryMonitor.ino

    if (!BLE.begin()) {
        Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1); // Stop here in case of error
    }

    BLE.setLocalName("PlantMonitor"); // Set name
    BLE.setAdvertisedService(plantService); // add the service UUID
    plantService.addCharacteristic(plantCharacteristic); // add the sensor data characteristic
    BLE.addService(plantService); // Add the plant service
    plantCharacteristic.writeValue("Warming up..."); // set initial value for this characteristic

    // start advertising
    BLE.advertise();

    Serial.println("Bluetooth® device active, waiting for connections...");

    // TODO: store system start time (for warm-up)
    systemStartTime = millis(); 
}


void loop() {
    unsigned long now = millis();

    // TODO: maintain BLE stack (if required)
    BLE.poll(); // activate BLE

    // In case of lost connection re-advertise
    if (!BLE.connected()) {
        BLE.advertise(); 
    }


    // i) TODO: asynchronous sensor acquisition (light, DHT, SGP30)
    // lightSensor
    if (now - LastSampleLight >= updateTimeLight) {
        // set time
        LastSampleLight = now;
        // read raw value from light sensor (12Bit ADC = 0-4095)
        uint16_t rawLight = analogRead(SENSOR_PIN_light);
        lastValidLight = normalize(rawLight);
    }

    // AirSensor
    if(now - LastSampleAir >= updateTimeAir){
        // set time
        LastSampleAir = now;
        //perform a measurement and handle failure cases
        bool measure = sgp.IAQmeasure();
        if(!measure){
            Serial.println("eCO2 measure failed");             // handle failure
        } else{
            lastValidECO2 = sgp.eCO2;
        }                              
    }

    // temperature
        if (now - LastSampleTemperatur >= updateTimeTemperature) {
        LastSampleTemperatur = now;

        // read temperature and humidity
        float temp = dht.readTemperature();
        float hum = dht.readHumidity();

        if (!isnan(temp)) lastValidTemperature = temp;
        if (!isnan(hum)) lastValidHumidity = hum;
    }
        
    
    // iii) TODO: warm-up handling (STATE_INIT for 30 s)
    if (now - systemStartTime < warmUpDuration) {
        currentState = STATE_INIT;
        u8g2.clearBuffer();
        u8g2.setFont(u8g2_font_6x10_tf);
        // show message
        u8g2.setCursor(0, 11);
        u8g2.print("Warming Up...");
        u8g2.sendBuffer();

        AlertSystem(currentState, now); // Make sure buzzer and led are off in warm up phase

    }
    else{
        // iii) TODO: compute health score (0–100)
        if (now - LastHealthScoring >= updateHealthScoring) {
            LastHealthScoring = now;

            healthScore = 0;
            
            // Every options adds 25 points to the total score
            if (lastValidTemperature >= 18 && lastValidTemperature <= 30){
                healthScore += 25;
            }
            if (lastValidHumidity >= 30 && lastValidHumidity <= 75){
                healthScore += 25;
            } 
            if (lastValidLight >= 25 && lastValidLight <= 90){
                healthScore += 25;
            } 
            if (lastValidECO2 < 1200){
                healthScore += 25;
            } 

            // iii) TODO: implement state transitions (including critical override)
            if(lastValidECO2 > 2200){
                currentState=STATE_STRESSED;
            }
            else if(healthScore >=75){
                currentState=STATE_HEALTHY;
            }
            else if(healthScore >=50){
                currentState=STATE_ATTENTION;
            }
            else{
                currentState=STATE_STRESSED;
            }
        }


        // iv) TODO: update OLED display (~2 Hz)
        if(now - LastSampleDisplay >= updateTimeDisplay){
            // set time
            LastSampleDisplay = now;
            display_values(currentState, 
                            lastValidTemperature, 
                            lastValidHumidity, 
                            lastValidLight, 
                            lastValidECO2
                            );
        }
        // v) TODO: implement LED and buzzer behavior (non-blocking)
        AlertSystem(currentState, now);
            
        // vi) TODO: send BLE telemetry (formatted string, 1 Hz)
        if(now - LastSampleBLE >= updateTimeBLE){
            LastSampleBLE = now;

            if(BLE.central()){ // check if device is connected
                char sensorData[50]; // buffer for the sensor data string

                // bild string with sensor data
                snprintf(sensorData, sizeof(sensorData), 
                "T=%.1f H=%.1f L=%d C=%d S=%s", 
                lastValidTemperature, lastValidHumidity, 
                lastValidLight, lastValidECO2, 
                stateToString(currentState).c_str());
                
                // push data with notify
                plantCharacteristic.writeValue(sensorData);

            }
        }
    }
}