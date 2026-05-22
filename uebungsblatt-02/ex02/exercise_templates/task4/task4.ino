
// ------------------------------------------------------------
//  Task 4:
//      Make necessary changes to your code from Task 3. Provided tests
//      do not cover all cases.
// ------------------------------------------------------------

#include <Arduino.h>


void setup() {
  NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;           // define mode
  NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_32Bit;  // define bitmode
  NRF_TIMER1->PRESCALER = 4;                          // define prescaler for 1000hz
  NRF_TIMER1->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;   
  NRF_TIMER1->EVENTS_COMPARE[0] = 0;
  NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Msk;
  
  NVIC_EnableIRQ(TIMER1_IRQn);                        // enable interrupt

  NRF_P0->DIRSET = (1UL << 29);                       // Set Buzzer for Task 2/3 
}


void loop() {
// tests
  setBuzzerFreq(50);  //A4
  delay(300);
  setBuzzerFreq(440);  //A4
  delay(300);
  setBuzzerFreq(554);  //C#5
  delay(300);
  setBuzzerFreq(659);  //E5
  delay(300);
  setBuzzerFreq(880);  //A5
  delay(500);
  setBuzzerFreq(5000); //out of range
  delay(3000);
  for (int f = 100; f <= 3000; f += 50) {
    setBuzzerFreq(f);
    delay(20);}
}


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