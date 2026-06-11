// Task 3: Air Quality Monitoring with SGP30


#include <Adafruit_SGP30.h>

// --- Objects ---
Adafruit_SGP30 sgp;

// --- Timing ---
// TODO: define sampling interval (1 second)
const unsigned long interval = 1000;
// --- Warm-up ---
// TODO: define warm-up duration (15 seconds)
const unsigned long warmUp = 15000;

// TODO: maintain a warm-up state indicator
bool stateWarmUp = true;

unsigned long lastMeasure = 0; // last carried measure 

unsigned long systemStartTime = 0; // stores the system start time

void setup() {
    Serial.begin(115200);
    while (!Serial && millis() < 3000);

    // TODO: initialize the sensor and handle initialization failure
    bool initialized = sgp.begin();                                 // try to initialize
    if (!initialized){
        Serial.println("initialization failed");
        while(true);                                                // if unsuccessful stay in loop
    }
    Serial.println("initialization successful");
    // TODO: start air quality measurements
    sgp.IAQmeasure();                                               // air measure     
    // TODO: record system start time
    systemStartTime = millis();                                     // stores the required time to start systems
}

void loop() {
    unsigned long now = millis();                                   // time since start
    
    bool measure = false;                                           // air measure successful or not

    // TODO: implement periodic sampling at 1 Hz using millis()
    if (now - lastMeasure >= interval){                             // check for next measure
        lastMeasure = now;                                          // set back timer
    // TODO: perform a measurement and handle failure cases
        measure = sgp.IAQmeasure();
        if(!measure){
            Serial.println("eCO2/TVOC measure failed");             // handle failure
            return;                                                 // stop loop
        }
    // TODO: update warm-up state based on elapsed time
        if(stateWarmUp && now - systemStartTime >= warmUp){
            stateWarmUp = false;                                    // swotch to ready-state
        }
    // TODO: output measurement results
    //       - indicate warm-up vs ready state
    //       - ensure correct formatting for integer values
        uint16_t eCO2 = sgp.eCO2;                                   
        uint16_t TVOC = sgp.TVOC;

        if(stateWarmUp){                                           // print warmUp output
            Serial.print("eCO2 (UNSTABLE/WARMUP): ");
            Serial.print(eCO2);
            Serial.println(" ppm");
            Serial.print("TVOC (UNSTABLE/WARMUP): ");
            Serial.print(TVOC);
            Serial.println(" ppb");
        }
        else {                                                     // print ready output
            Serial.print("eCO2: ");
            Serial.print(eCO2);
            Serial.println(" ppm");
            Serial.print("TVOC: ");
            Serial.print(TVOC);
            Serial.println(" ppb");
        }
    }
}
