#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include <U8g2lib.h>


// Task 2 ii.) Constants — fill in the correct values from the datasheet
//             You can also write the answers to the questions here.
// ------------------------------------------------------------

#define SGP30_ADDR       0x58   // 7-bit I2C address of the SGP30 

/*
Which address belongs to the SGP30?
Antwort: 0x58

What is the two-byte command code for initialization?
Antwort: Init_air_quality -> 0x2003 (Aus Datenblatt Tabelle 10)

What is the two-byte command code for measurement?
Antwort: Measure_air_quality -> 0x2008 (Aus Datenblatt Tabelle 10)

How many bytes does a measurement response contain? What does each byte repre-
sent?
Antwort: Laut Datenblatt Tabelle 10 ist die response length 6 bytes.

"The sensor responds with 2 data bytes (MSB first) and 1 CRC byte for each of the
two preprocessed air quality signals in the order CO2eq (ppm) and TVOC (ppb)."

Byte 1: CO2 MSB
Byte 2: CO2 LSB
Byte 3: CRC for CO2

Byte 4: TVOC MSB
Byte 5: TVOC LSB
Byte 6: CRC for TVOC
*/

// Command codes (2 bytes each, MSB first — see datasheet )
#define CMD_INIT_MSB     0x20   //    first byte
#define CMD_INIT_LSB     0x03   //    second byte
#define CMD_MEAS_MSB     0x20
#define CMD_MEAS_LSB     0x08

// Display: air quality range for mapping CO2 to a percentage, you can change these to test more ranges
#define CO2_MIN          400    // ppm — clean outdoor air
#define CO2_MAX          2000   // ppm — poor indoor air quality

// ------------------------------------------------------------
//  Display constructor
// ------------------------------------------------------------
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);



//  Task 2 iii.) — Helper functions
  // ------------------------------------------------------------
  //  Raw byte storage for the last SGP30 measurement.
  //
  // We use global variables here to keep the function signatures simple. 
  // The cleaner alternatives (pointers or a result struct) use C concepts not yet introduced. 
  // At this scale globals are fine; in a larger project you would avoid them.
  //
  //  The SGP30 sends each 16-bit value as two separate bytes:
  //    MSB — most significant byte  (upper 8 bits of the value)
  //    LSB — least significant byte (lower 8 bits of the value)
  //  A third byte per value is a CRC checksum — we discard it.
  // ------------------------------------------------------------
uint8_t raw_co2_msb,  raw_co2_lsb;   // assign values with sgp30_read()
uint8_t raw_tvoc_msb, raw_tvoc_lsb;  // assign values with sgp30_read()

unsigned long last_measure = 0; // In order to measure 1s pauses



void sgp30_cmd(uint8_t msb, uint8_t lsb) {
  // ------------------------------------------------------------
  //  sgp30_cmd : send a 2-byte command to the SGP30
  //
  //  The SGP30 expects all commands as two bytes (MSB first).
  //  Example:  sgp30_cmd(CMD_INIT_MSB, CMD_INIT_LSB);
  // ------------------------------------------------------------
  // TODO: open a transmission to SGP30_ADDR,
  //       write msb, write lsb, close the transmission.
  // 

  Wire.beginTransmission(SGP30_ADDR);
  Wire.write(msb);
  Wire.write(lsb);
  Wire.endTransmission();

}


bool sgp30_read(uint8_t n) {
  // ------------------------------------------------------------
  //  sgp30_read : read one measurement(n bytes) from the SGP30.
  //
  //  Call sgp30_cmd() first, wait max measurement duration, 
  //  then call this function.
  //  The 4 raw bytes are stored in the global variables above.
  //  The two CRC bytes are read from the bus and discarded.
  //
  //  Returns true if all 6 bytes were received, false on error.
  // ------------------------------------------------------------
  // TODO: use Wire.requestFrom(SGP30_ADDR, n) to request n bytes.
  //       If the return value is not n, return false immediately. 
  //       The expected return value is in the Datasheet.
  //       Read the n bytes in order, remember wire.read() can only
  //       read one byte at a time. Look out for CRC bytes,  
  //       we don't need to store those.  
  //

  // Requested n data bytes get buffered
  if(Wire.requestFrom(SGP30_ADDR, n) != n){
    return false;
  };

  // Read buffered bytes one by one
  // We know for the measurement we will get 6 bytes
  raw_co2_msb = Wire.read();
  raw_co2_lsb = Wire.read();
  Wire.read(); // CRC not needed

  raw_tvoc_msb = Wire.read();
  raw_tvoc_lsb = Wire.read();
  Wire.read(); // CRC not needed

  return true;

}


uint16_t to_uint16(uint8_t msb, uint8_t lsb) {
  // ------------------------------------------------------------
  //  to_uint16 : combine two bytes into one 16-bit value.
  //
  //  A sensor value like CO2 = 450 ppm cannot fit in a single
  //  byte (max 255). The sensor splits it across two bytes:
  //    MSB holds the upper half: 450 >> 8  = 1   (0x01)
  //    LSB holds the lower half: 450 & 0xFF = 194 (0xC2)
  //
  //  To reconstruct the original value:
  //    shift MSB left by 8 bits  →  0x01 becomes 0x0100 (= 256)
  //    OR with LSB               →  0x0100 | 0xC2 = 0x01C2 (= 450)
  //
  //  The cast to uint16_t before shifting is necessary because
  //  uint8_t would overflow when shifted — always cast first.
  //
  // ------------------------------------------------------------
  return ((uint16_t)msb << 8) | lsb;
}



//  Task 3 — Display helper  (optional, but keeps loop() clean)


void display_values(uint16_t co2, uint16_t tvoc) {
  // ------------------------------------------------------------
  //  display_values : show co2 and tvoc on the OLED
  //
  // ------------------------------------------------------------
  // TODO (Task 3): set cursor, print co2 and tvoc values.
  // TODO (Task 4): map co2 to pct (0-100), draw a filled bar with
  //                u8g2.drawBox(x, y, width, height).
  //                Bar width  = map(pct, 0, 100, 0, 128)
  //                Remember: constrain pct to [0, 100] before mapping.
}



void setup() {
  Serial.begin(115200);
  while (!Serial);           // Wait for USB Serial connection
  //     Task 2 i.): I2C scanner
  // TODO: Transmit to each available I2C address, print the adress when receiving an ACK.
  //       You can use decimal adresses when sending but convert them to hex when printing them out.
  //       Use Serial.print(address, HEX) to make it easier.

  Wire.begin(); // Initialise I2C hardware

  Serial.println("Starting address scan");

  uint8_t address = 8;
  
  for(address; address < 128; address++){
    Wire.beginTransmission(address);

    if(Wire.endTransmission() == 0 ) {
      Serial.print("0x");
      Serial.println(address, HEX);
    }
  }

  /*
  Adressen gefunden: 0x3C, 0x51, 0x58
  Laut Datenblatt hat der SGP30 Adresse 0x58
  */


  //     Task 2 iv.): Initialise SGP30 
  // TODO: send the init command, wait for initialization
  //       and print out a message.

  sgp30_cmd(CMD_INIT_MSB, CMD_INIT_LSB);

  // Laut Datenblatt max duration für INIT 10ms
  unsigned long start_init = millis();

  while(millis() - start_init < 10UL){
  }

  Serial.println("Initialization Phase complete"); 


  // --- Task 3 i.): Simple display use ---
  // TODO: initialize display, set a font, display "Hardware Praktikum 2026",
  //       and push it to the screen.
}


void loop() {
  // --- Task 2 iv.): Send measure command and read response ---
  // TODO: call sgp30_cmd() with the measure command bytes.

  if(millis() - last_measure > 1000UL){
    sgp30_cmd(CMD_MEAS_MSB, CMD_MEAS_LSB);
    last_measure = millis();
    // Measurement max duration 12ms
    delay(12);

  // TODO: call sgp30_read().
  //       If it returns false, print an error message and return early.
    if(!sgp30_read(6)){
      Serial.println("ERROR: Measurement unsuccessful");
      return;
    }
    // TODO: Reconstruct 16-bit values from the raw bytes ----
    //raw_msb muss um 8 bit nach links geschoben werden (2^8 = 256)
    uint16_t co2 = raw_co2_msb * 256 + raw_co2_lsb;
    uint16_t tvoc = raw_tvoc_msb * 256 + raw_tvoc_lsb;

  // TODO: print co2 and tvoc with appropriate labels and units.
    Serial.print("CO2eq: ");
    Serial.print(co2);
    Serial.print(" ppm, TVOC: ");
    Serial.print(tvoc);
    Serial.println(" ppb");
  }


  // --- Task 3 ii.): Print the sgp30 values on the display
  //                  in addition to the Serial monitor 

  // --- Task 4: Map CO2 to a percentage ---
  // TODO: use map() to scale co2 from raw values to 0-100%.
  //       Then use constrain() to make sure the percetange 
  //       doesnt go outside 0-100.

}