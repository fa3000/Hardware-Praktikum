
// ------------------------------------------------------------
//  Task 2 and 3:
//      Write the body of setTimer1Freq() as specified in the exercise sheet.
//      This should include timer settings.
//      Implement the ISR in TIMER1_IRQHandler().
//      Make necessary changes for setBuzzerFreq()
// ------------------------------------------------------------

#include <Arduino.h>


void setup() {

  NRF_P0->DIRSET = (1UL << 29);     // Set Buzzer for Task 2
  
  NRF_P0->DIRCLR = (1UL << 3);    // set button for Task 3
  NRF_P0->PIN_CNF[3] =            //  activate button
    (GPIO_PIN_CNF_DIR_Input << GPIO_PIN_CNF_DIR_Pos) |
    (GPIO_PIN_CNF_INPUT_Connect << GPIO_PIN_CNF_INPUT_Pos) |
    (GPIO_PIN_CNF_PULL_Pullup << GPIO_PIN_CNF_PULL_Pos);

  //setTimer1Freq();            //Task2  
  setBuzzerFreq();              //Task3 
}

bool isPlay = false;
bool wasPressed = false;

void loop() {
  bool pressed = !(NRF_P0->IN & (1UL << 3)); // check if button is pressed(active low)
  
  if(pressed && !wasPressed){                 // check for switch
    isPlay = !isPlay;                     // toggle state
  }

  wasPressed = pressed;              // remember state

  if(isPlay){
    NRF_TIMER1->TASKS_START = 1;    // Start timer
  }
  else {
    NRF_TIMER1->TASKS_STOP = 1;     // stop timer 
    NRF_P0->OUTCLR = (1UL << 29);
  }
}


void setTimer1Freq() {
  NRF_TIMER1->MODE = TIMER_MODE_MODE_Timer;           // define mode
  NRF_TIMER1->BITMODE = TIMER_BITMODE_BITMODE_32Bit;  // define bitmode
  NRF_TIMER1->PRESCALER = 4;                          // define prescaler for 1000hz

  uint32_t ticks = 1000000/ (2*1046);                     //calculate ticks 
  
  NRF_TIMER1->CC[0] = ticks ;                             //capture/compare 
  NRF_TIMER1->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Msk;   
  NRF_TIMER1->EVENTS_COMPARE[0] = 0;
  NRF_TIMER1->INTENSET = TIMER_INTENSET_COMPARE0_Msk;

  NVIC_EnableIRQ(TIMER1_IRQn);    // enable interrupt

  NRF_TIMER1->TASKS_CLEAR = 1;    
  NRF_TIMER1->TASKS_START = 1;
}


void setBuzzerFreq() {
  setTimer1Freq();
  NRF_TIMER1->TASKS_STOP = 1;
  NRF_TIMER1->TASKS_CLEAR = 1;
 }


extern "C" void TIMER1_IRQHandler() {
  if(NRF_TIMER1->EVENTS_COMPARE[0]){
    NRF_TIMER1->EVENTS_COMPARE[0] = 0;
  
    if(NRF_P0->OUT & (1UL << 29)){  //check if Pin is high
    NRF_P0->OUTCLR = (1UL << 29); // Set Buzzer low
    }
    else{
    NRF_P0->OUTSET = (1UL << 29); // Set Buzzer high
    }
  }
}



