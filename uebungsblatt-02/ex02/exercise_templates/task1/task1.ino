
// ------------------------------------------------------------
//  Task 1:
//      Write the body of setP026() as specified in the exercise sheet.
//      To toggle at the right frequency, you can use
//      this function in a loop.
// ------------------------------------------------------------
void setup() {
  NRF_P0->DIRSET = (1UL << 26); // Set P0.26 as output
}

// toggle Pin for 1Hz
void loop() {
  setP026(true);  // set high for 500ms
  delay(500);
  setP026(false); // set low for 500ms 
  delay(500);
}

// switches high / low
void setP026(boolean high) {
  if (high) {
    NRF_P0->OUTSET = (1UL << 26);  // Set P0.26 HIGH
  } else {
    NRF_P0->OUTCLR = (1UL << 26);  // Set P0.26 LOW
  }
}