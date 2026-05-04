#include <Adafruit_TinyUSB.h>
#define BLINK_INTERVAL_MS  1000UL  // 1 second on, 1 second off → 1 Hz

// ------------------------------------------------------------
//  Global variables for Task 1 ii) and beyond
//  Use the smallest type that fits:
//    unsigned long — required for millis() values (32-bit)
//    uint8_t       — sufficient for a cycle/colour counter (8-bit, max 255)
// ------------------------------------------------------------
unsigned long last_blink_ms = 0;  // timestamp of the last LED toggle
bool          led_on        = false; // current LED state

// TODO (Bonus): add a counter for completed blink cycles
//               and a variable to track the current colour

uint8_t cycles_counter = 0;
uint8_t current_color = 0;


void setup() {
  Serial.begin(115200);
  // while (!Serial);
  // Set LED pins as outputs.
  // pinMode(pin, OUTPUT) configures a pin for digital output
  // Start with all LEDs off (active-low → HIGH = off)
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  digitalWrite(LED_RED, HIGH);
  digitalWrite(LED_GREEN, HIGH);
  digitalWrite(LED_BLUE, HIGH);
}


void loop() {

  // ----------------------------------------------------------
  //  Task 1 i) — Blink using delay()
  //  Comment this section out once you move on to Task 1 ii).
  // ----------------------------------------------------------

  // TODO: turn LED_RED on,
  //       wait BLINK_INTERVAL_MS ms,
  //       turn LED_RED off,
  //       wait BLINK_INTERVAL_MS ms.

  /*
  digitalWrite(LED_RED, LOW);
  delay(BLINK_INTERVAL_MS);
  digitalWrite(LED_RED, HIGH);
  delay(BLINK_INTERVAL_MS);
  */

  // ----------------------------------------------------------
  //  Task 1 ii) — Blink using millis()  (comment out i) first)
  //
  // TODO: get the current time with millis().
  // TODO: check whether BLINK_INTERVAL_MS has elapsed since last_blink_ms.
  // TODO: if yes — update last_blink_ms, toggle led_on,
  //               and write the correct HIGH/LOW to LED_RED.
  
  /*
  unsigned long current_time = millis();

  if(current_time - last_blink_ms > BLINK_INTERVAL_MS){
    last_blink_ms = current_time;
    if(led_on==false){
      digitalWrite(LED_RED, LOW);
      led_on = true;
    }
    else{      
      digitalWrite(LED_RED, HIGH);
      led_on = false;
    }
  }
  */

  // ----------------------------------------------------------
  //  Task 1 iii) — Answer as a comment
  // ----------------------------------------------------------

  /*Mit delay() wird einfach für den festgelegten Zeitraum der Seeed blockiert und nichts wird ausgeführt. 
  Mit dem millis() Ansatz aus ii) wird der Seeed nicht blockiert und während dem warten können auch andere Aktionen ausgeführt werden.*/


  // ----------------------------------------------------------
  //  Bonus — Colour cycling after 10 blink cycles
  // ----------------------------------------------------------
  // TODO: count completed blink cycles (one cycle = on + off).
  //       Every 10 cycles, switch to the next colour:
  //         red → green → blue → red → ...
  //       Turn off all LEDs before switching, then turn on only
  //       the new active colour.

  unsigned long current_time = millis();

  if(cycles_counter > 9){
    cycles_counter = 0;
    current_color = (current_color + 1) % 3;
  }

  if(current_time - last_blink_ms > BLINK_INTERVAL_MS){
    last_blink_ms = current_time;
    if(current_color == 0){
      if(led_on==false){
        digitalWrite(LED_RED, LOW);
        led_on = true;
      }
      else{      
        digitalWrite(LED_RED, HIGH);
        led_on = false;
        cycles_counter += 1;
      }
    }
        if(current_color == 1){
      if(led_on==false){
        digitalWrite(LED_GREEN, LOW);
        led_on = true;
      }
      else{      
        digitalWrite(LED_GREEN, HIGH);
        led_on = false;
        cycles_counter += 1;
      }
    }
        if(current_color == 2){
      if(led_on==false){
        digitalWrite(LED_BLUE, LOW);
        led_on = true;
      }
      else{      
        digitalWrite(LED_BLUE, HIGH);
        led_on = false;
        cycles_counter += 1;
      }
    }
  }



}