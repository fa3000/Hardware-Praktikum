
// ------------------------------------------------------------
//  Task 5:
//      Write the body of setTimer2() as specified in the exercise sheet.
//      This should include timer settings.
//      Implement the ISR in TIMER2_IRQHandler().
// ------------------------------------------------------------

#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
volatile uint32_t tCount = 0;
uint32_t lastCount = 0;

void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Initializing Timer 2 (1ms interval)...");

  setTimer2(true); // Config for Timer 2
}


void loop() {

  // Wait 1ms and print updated tCount
  if(tCount - lastCount >= 1000) {
    Serial.print("tCount in ms: ");
    Serial.println(tCount);
    lastCount = tCount;
  }

}


extern "C" void TIMER2_IRQHandler() {
  // Check if TIMER2 event happend
  if(NRF_TIMER2->EVENTS_COMPARE){
    // Reset TIMER2 event
    NRF_TIMER2->EVENTS_COMPARE[0] = 0;
    tCount++;
  }

}


void setTimer2(bool enable) {
  if(enable){
    NRF_TIMER2->MODE = TIMER_MODE_MODE_Timer;               // define mode
    NRF_TIMER2->BITMODE = TIMER_BITMODE_BITMODE_32Bit;      // define bitmode
    NRF_TIMER2->PRESCALER = 4;                              // define prescaler fpr 100hz
    NRF_TIMER2->CC[0] = 1000;                               // interrupt every 1ms
    NRF_TIMER2->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk; 
    NRF_TIMER2->INTENSET = TIMER_INTENSET_COMPARE0_Msk;   
    NVIC_EnableIRQ(TIMER2_IRQn);                            // enable interrupt

    NRF_TIMER2->TASKS_START = 1; // start timer
  }
  else{
    NRF_TIMER2->TASKS_STOP = 1; // stop timer
    NVIC_DisableIRQ(TIMER2_IRQn); // disbale interrupt
  }


}
