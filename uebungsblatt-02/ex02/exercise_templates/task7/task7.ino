


// ------------------------------------------------------------
//  Task 7:
//      Use your code from Task 6, adding a parser function to
//      go over any given string and create the array needed to play 
//      it.
//      You are free to use or discard any helper functions and add
//      any helper functions you need for parsing.
// ------------------------------------------------------------

#include <Adafruit_TinyUSB.h>
#include <Arduino.h>

#define MAX_MELODY_LEN 100

const char noteNames[] = {'c', 'C', 'd', 'D', 'e', 'f', 'F', 'g', 'G', 'a', 'A', 'b'};

// renamed so we can use 'notes' as it is demanded in the exercise
const uint16_t noteTable[] = {262, 277, 294, 311, 330, 349, 370, 392, 415, 440, 466, 494};

// the arrays
uint16_t notes[MAX_MELODY_LEN];
uint16_t durations[MAX_MELODY_LEN];

char buffer[]="Test:d=4,o=5,b=200:8g,8a,8c6,8a,e6,8p,e6,8p,d6.,8p,8g,8a,8c6,8a,d6,8p,d6,8p,c6,8p,a.,8g,8a,8c6,8a,2c6,d6,b,a,g.,8p,g,2d6,2c6.,p,8g,8a,8c6,8a,e6,8p,e6,8p,d6.,8p,8g,8a,8c6,8a,g6,b,c6,8p,b,8a,p,8g,8a,8c6,8a,2c6,d6,b,a,g,p,g,d6,c6";

volatile uint8_t melodyIdx = 0;
volatile uint8_t melodyLen = 0; // new: saves the amount of notes of the current song
volatile uint32_t tCount = 0;
uint16_t currentIdx = 0; // global index for parsing

uint16_t standardDuration = 4;
uint16_t standardOctave = 6;
uint16_t standardBPM = 63;

// needed for parseRTTLNote
struct Note {
    uint16_t frequency;
    uint32_t duration;
};


// ______________________________________________________________________________________________
// Helper functions


bool isDigit(char c) { 
  return (c >= '0' && c <= '9');
}


uint16_t str2uint(char * buf, uint16_t * idx) {
  uint16_t val = 0;
  while (isDigit(buf[*idx])) {
      // ascii shenannigans
      val = val * 10 + (buf[*idx] - '0');
      (*idx)++; // increment index
  }
  return val;
}


uint16_t freqFromNote(char note, bool sharp) {
    if (note == 'p') return 0; // pause
    
    int8_t offset = -1;
    // get the right base note
    switch (note) {
        case 'c': offset = 0; break;
        case 'd': offset = 2; break;
        case 'e': offset = 4; break;
        case 'f': offset = 5; break;
        case 'g': offset = 7; break;
        case 'a': offset = 9; break;
        case 'b': offset = 11; break;
    }

    if (offset == -1) return 0; // error case
    if (sharp) offset += 1;     // sharp notes
    
    return noteTable[offset];
}

// _______________________________________________________________________________________________
// Timer and buzzer stuff from previous exercises

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
    // task 7: changed 10 to melodyLen
    if(melodyIdx < melodyLen){
      setBuzzerFreq(notes[melodyIdx]);
    }
    else{
      setTimer2(false); // stop Timer2
      setBuzzerFreq(0); // silence buzzer
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

// ____________________________________________________________________________________________
// Task 7 parsing stuff

bool parseRTTLNote(Note * note) {
    if (buffer[currentIdx] == '\0') return false; 

    // 1. Get Duration
    uint16_t duration = str2uint(buffer, &currentIdx);
    if (duration == 0) duration = standardDuration;

    // 2. Get Note Char
    char noteChar = buffer[currentIdx++];
    if (noteChar >= 'A' && noteChar <= 'Z') noteChar += 32; // to lowercase

    // 3. Check for Sharp
    bool sharp = false;
    if (buffer[currentIdx] == '#') {
        sharp = true;
        currentIdx++;
    }

    // 4. Check for Octave
    uint16_t octave = str2uint(buffer, &currentIdx);
    if (octave == 0) octave = standardOctave;

    // 5. Check for Dotted
    bool dotted = false;
    if (buffer[currentIdx] == '.') {
        dotted = true;
        currentIdx++;
    }

    // Calculate Frequency
    uint32_t freq = freqFromNote(noteChar, sharp);
    if (freq > 0 && noteChar != 'p') {
        freq = freq << (octave - 4);
    }
    note->frequency = (uint16_t)freq;

    // Calculate Duration in ms
    uint32_t wholeNoteMs = 240000UL / standardBPM;
    uint32_t ms = wholeNoteMs / duration;
    if (dotted) ms = (ms * 3) / 2;
    note->duration = ms;

    // Skip comma or space
    while (buffer[currentIdx] == ',' || buffer[currentIdx] == ' ') {
        currentIdx++;
    }
    return true;
}

void playRTTTL() {
    uint16_t idx = 0;

    // skip song name
    while (buffer[idx] != ':' && buffer[idx] != '\0') idx++;
    if (buffer[idx] == ':') idx++;

    // parse control section (d=, o=, b=)
    while (buffer[idx] != ':') {
        if (buffer[idx] == 'd') {
            idx += 2;
            standardDuration = str2uint(buffer, &idx);
        } else if (buffer[idx] == 'o') {
            idx += 2;
            standardOctave = str2uint(buffer, &idx);
        } else if (buffer[idx] == 'b') {
            idx += 2;
            standardBPM = str2uint(buffer, &idx);
        } else {
            idx++;
        }
        if (buffer[idx] == '\0') return;
    }
    idx++; // skip second colon

    currentIdx = idx;
    uint8_t count = 0;

    // parse all notes into arrays
    while (count < MAX_MELODY_LEN) {
        Note n;
        if (!parseRTTLNote(&n)) break;

        notes[count] = n.frequency;
        durations[count] = n.duration;
        count++;
    }

    melodyLen = count;
    playMelody(); 
}

void setup() {
  String song0 = "";
  String smokeOnTheWater = "smokeOnTheWater:d=4,o=4,b=112:c,d#,f.,c,d#,8f#,f,p,c,d#,f.,d#,c";
  String bondTheme = "bondTheme:o=5,d=4,b=320,b=320:c,8d,8d,d,2d,c,c,c,c,8d#,8d#,2d#,d,d,d,c,8d,8d,d,2d,c,c,c,c,8d#,8d#,d#,2d#,d,c#,c,c6,1b.,g,f,1g.";
  String poisonAliceCooper = "poisonAliceCooper:o=5,d=8,b=112,b=112:d,d,a,d,e6,d,d6,d,f#,g,c6,f#,g,c6,e,d,d,d,a,d,e6,d,d6,d,f#,g,c6,f#,g,c6,e,d,c,d,a,d,e6,d,d6,d,f#,g,c6,f#,g,c6,e,d,c,d,a,d,e6,d,d6,d,a,d,e6,d,d6";
  String axelFoley = "axelFoley:o=5,d=8,b=125,b=125:16g,16g,a#.,16g,16p,16g,c6,g,f,4g,d6.,16g,16p,16g,d#6,d6,a#,g,d6,g6,16g,16f,16p,16f,d,a#,2g,4p,16f6,d6,c6,a#,4g,a#.,16g,16p,16g,c6,g,f,4g,d6.,16g,16p,16g,d#6,d6,a#,g,d6,g6,16g,16f,16p,16f,d,a#,2g";
  String ententanz = "ententanz:o=5,d=16,b=100,b=100:g,g,a,a,e,e,8g,g,g,a,a,e,e,8g,g,g,a,a,c6,c6,8b,8b,8a,8g,8f,f,f,g,g,d,d,8f,f,f,g,g,d,d,8f,f,f,g,g,a,b,8c6,8a,8g,8e,4c";
  String colonelBogeyMarch = "colonelBogeyMarch:o=5,d=8,b=140,b=140:g,e,4p,p,e,f,g,e6,p,e6,p,2c6,g,e,4p,p,e,f,e,g,p,g,p,2f,f,d,4p,p,d,e,f,g,e,4p,p,e,f#,e,d,g,p,e,f#,d,p,a,g.,16f#,g,a,g,f,e,d";
  String stairwayToHeaven = "stairwayToHeaven:o=5,d=8,b=63,b=63:a4,c,e,a,b,e,c,b,c6,e,c,c6,f#,d,a4,f#,e.,16c,a4,4e,c,a4,e,g4,a4,4a4";
  String flintstonesTheme = "flintstonesTheme:o=5,d=8,b=200,b=200:g#,4c#,p,4c#6,a#,4g#,4c#,p,4g#,f#,f,f,f#,g#,4c#,4d#,2f,2p,4g#,4c#,p,4c#6,a#,4g#,4c#,p,4g#,f#,f,f,f#,g#,4c#,4d#,2c#";
  String funkytown = "funkytown:o=4,d=8,b=125,b=125:c6,c6,a#5,c6,p,g5,p,g5,c6,f6,e6,c6,2p,c6,c6,a#5,c6,p,g5,p,g5,c6,f6,e6,c6";
  String YMCA = "YMCA:o=5,d=8,b=160,b=160:c#6,a#,2p,a#,g#,f#,g#,a#,4c#6,a#,4c#6,d#6,a#,2p,a#,g#,f#,g#,a#,4c#6,a#,4c#6,d#6,b,2p,b,a#,g#,a#,b,4d#6,f#6,4d#6,4f6.,4d#6.,4c#6.,4b.,4a#,4g#";
  String finalCountdown = "finalCountdown:o=5,d=16,b=125,b=125:b,a,4b,4e,4p,8p,c6,b,8c6,8b,4a,4p,8p,c6,b,4c6,4e,4p,8p,a,g,8a,8g,8f#,8a,4g.,f#,g,4a.,g,a,8b,8a,8g,8f#,4e,4c6,2b.,b,c6,b,a,1b";
  strcpy(buffer, song11.c_str());
  playRTTTL();
}

void loop() {}
