// Task 4: Smart Plant Monitoring Station

#include <ArduinoBLE.h>
#include <Wire.h>
#include <Adafruit_SGP30.h>
#include <U8g2lib.h>
#include <DHT.h>

// --- Hardware Configuration ---
// TODO: define pins (DHT, light sensor, LED, buzzer)

// DHT
const uint8_t DHT_PIN = 7; 
const uint8_t DHTTYPE = DHT11;
// light sensor
const uint8_t SENSOR_PIN_light = A0;
// LED
const uint8_t LED_PIN = 26;
// buzzer
const uint8_t BUZZER_PIN = 29;


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


// --- Objects ---
// TODO: initialize display, sensors, BLE service and characteristic

//display
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
// Air Quality Sensor
Adafruit_SGP30 sgp;  
 // temperature and humidity Sensor     
DHT dht(DHT_PIN, DHTTYPE); 

// --- FSM ---
enum SystemState {
    STATE_INIT,
    STATE_HEALTHY,
    STATE_ATTENTION,
    STATE_STRESSED
};

SystemState currentState = STATE_INIT;

// TODO: define timing variables for asynchronous operation
unsigned long LastSampleDisplay;
unsigned long LastSampleBLE;
unsigned long LastSampleWarmUp;

unsigned long LastSampleLight;
unsigned long LastSampleTemperatur;
unsigned long LastSampleAir;

//LED
unsigned long LastSampleLED;


// TODO: define variables for sensor data storage

float lastValidTemperature = 0.0;
float lastValidHumidity = 0.0;
uint16_t lastValidECO2 = 0;
uint16_t lastValidLight = 0;

//system start time
unsigned long systemStartTime = 0;

//LED flag
bool LED_ON = false;

// TODO: helper function(s), e.g.:
// - state to string conversion

String stateToString(SystemState state){
    switch (state){
        case STATE_INIT: 
            return "Initialisiert";
        case STATE_HEALTHY:
            return "Normalbetrieb";
        case STATE_ATTENTION:
            return "Kritisch";
        case STATE_STRESSED:
            return "Überlastet";
        default:
            return "Unbekannt";
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
  u8g2.print("Zustand: ");
  u8g2.print(stateToString(state));
  // show temperatur 
  u8g2.setCursor(0,22);
  u8g2.print("Temperatur: ");
  u8g2.print(temperature);
  u8g2.print(" grad");
  // show humidity
  u8g2.setCursor(0, 33);
  u8g2.print("Luftfeuchtigkeit: ");
  u8g2.print(humidity);
  u8g2.print(" %");
  // show light level
  u8g2.setCursor(0, 44);
  u8g2.print("Lichtstärke: ");
  u8g2.print(lightLevel);
  u8g2.print(" %");
  // show CO2 Levels
  u8g2.setCursor(0, 55);
  u8g2.print("eCO2: ");
  u8g2.print(eCo2);
  u8g2.print(" ppm");
  u8g2.sendBuffer();
}

//LED alert
// switches LED Pin high / low
void SetLED(boolean high) {
  if (high) {
    digitalWrite(LED_PIN, HIGH);
  } else {
    digitalWrite(LED_PIN, LOW);
  }
}

bool LEDAlertSystem(SystemState state, unsigned long now){
    switch (state){
        case STATE_INIT:
            SetLED(false);
            LED_ON = false; 
            return false;

        case STATE_HEALTHY:
            SetLED(true);
            LED_ON = true;
            return false;

        case STATE_ATTENTION:
            if(now - LastSampleLED >= updateTimeLEDAttention){
                LED_ON = !LED_ON;
                SetLED(LED_ON);
                return true;    //update last blinking
            }
            return false;

        case STATE_STRESSED:
            if(now - LastSampleLED >= updateTimeLEDStressed){
                LED_ON = !LED_ON;
                SetLED(LED_ON);
                return true;    //update last blinking
            }
            return false;
        }
        return false;       //nothing to update
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

    if (!sgp.begin()) {
        Serial.println("SGP30 init failed");
        while (true);
    }
    
    // TODO: initialize BLE and start advertising

    // TODO: store system start time (for warm-up)
    systemStartTime = millis(); 
}


void loop() {
    unsigned long now = millis();

    // TODO: maintain BLE stack (if required)

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

    // iii) TODO: compute health score (0–100)

    // iii) TODO: implement state transitions (including critical override)

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
        if(LEDAlertSystem(currentState, now)){
            LastSampleLED = now;
        }
        
    // vi) TODO: send BLE telemetry (formatted string, 1 Hz)
}