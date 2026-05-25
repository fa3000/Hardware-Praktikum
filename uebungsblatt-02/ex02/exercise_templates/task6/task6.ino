
// ------------------------------------------------------------
//  Task 6:
//      Use your code from Task 4 and Task 5, and make necessary changes. 
//      Implement the function playMelody() according to the exercise sheet.
//      The array is already filled with the right frequency for each note.
// ------------------------------------------------------------

#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
uint16_t durations[10] = {300, 600, 1000, 400, 500, 800, 700, 200, 500, 1000};
uint16_t notes[10]     = {262, 294, 330, 349, 392, 440, 494, 523, 587, 659};
volatile uint32_t tCount = 0;
volatile uint8_t melodyIdx = 0;


void setup() {
  // outcommented beacuse otherwise the code waits for the serial monitor to be open before playing the sound
  // Serial.begin(115200);
  // while(!Serial);
  playMelody();
}


void loop() {

}

// From Task 5 with no changes
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

// From Task 4 but created analog setTimer function as for Timer2
void setTimer1(bool enable) {
  if(enable){
    NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;           // define mode
    NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_32Bit;  // define bitmode
    NRF_TIMER1->PRESCALER = 4;                          // define prescaler for 1000hz
    NRF_TIMER1->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;   
    NRF_TIMER1->EVENTS_COMPARE[0] = 0;
    NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
    
    NVIC_EnableIRQ(TIMER1_IRQn);                        // enable interrupt

    NRF_P0->DIRSET = (1UL << 29);                       // Set Buzzer for Task 2/3 
  }
  else{
    NRF_TIMER1->TASKS_STOP = 1; // stop timer
    NVIC_DisableIRQ(TIMER1_IRQn); // disbale interrupt
  }
}

// From Task 4 with no changes
void setBuzzerFreq(uint32_t freq) {
  if(freq > 3000 || freq < 100){              //check out of range
      NRF_P0->OUTCLR = (1UL << 29);           // Set Buzzer low
      NRF_TIMER1->TASKS_STOP = 1;
      NRF_TIMER1->TASKS_CLEAR = 1;
  }
  else {
  uint32_t ticks = 1000000/ (2*freq);         //calculate ticks uses param instead of 1046
    NRF_TIMER1->TASKS_STOP = 1;               //stop timer
    NRF_TIMER1->TASKS_CLEAR = 1;              //clear timer

    NRF_TIMER1->CC[0] = ticks ;               //capture/compare 

    NRF_TIMER1->TASKS_START = 1;              // start timer
  }
}

// ISR from Task 5
// Added new feature from Task 6 to switch melodies
extern "C" void TIMER2_IRQHandler() {
  // Check if TIMER2 event happend
  if(NRF_TIMER2->EVENTS_COMPARE){
    // Reset TIMER2 event
    NRF_TIMER2->EVENTS_COMPARE[0] = 0;
    tCount++;
  }
  // Frist increase tCount above then check note duration
  if(tCount >= durations[melodyIdx]){
    tCount = 0;
    // Jump to new note
    melodyIdx++;
    // check if still within melody range
    if(melodyIdx < 10){
      setBuzzerFreq(notes[melodyIdx]);
    }
    else{
      setTimer2(false); // stop Timer2
      setBuzzerFreq(0); // silence buzzer
    }
  }
}

// ISR from Task 4
// No changes
extern "C" void TIMER1_IRQHandler() {
  if(NRF_TIMER1->EVENTS_COMPARE[0]){          //check for event
    NRF_TIMER1->EVENTS_COMPARE[0] = 0;        //clear event
  
    if(NRF_P0->OUT & (1UL << 29)){            //check if Pin is high
    NRF_P0->OUTCLR = (1UL << 29);             // Set Buzzer low
    }
    else{
    NRF_P0->OUTSET = (1UL << 29);             // Set Buzzer high
    }
  }
}

void playMelody() {
  melodyIdx = 0;
  tCount = 0;
  setTimer2(true); // Config for Timer 2
  setTimer1(true); // Config for Timer 1
  setBuzzerFreq(notes[melodyIdx]); // starts playing the first note [0]
}

