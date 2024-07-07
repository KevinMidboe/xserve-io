

/*
 Example 39.1 - NXP SAA1064 I2C LED Driver IC Demo I
 Demonstrating display of digits
 http://tronixstuff.com
 */
#include "Wire.h" // enable I2C bus
#include <PCA9554.h>  // Load the PCA9554 Library

byte saa1064 = 0x3B; // define the I2C bus address for our SAA1064 (pin 1 to GND) ****
PCA9554 ioCon1(0x24);  // Create an object at this address

int digits[16]={63, 6, 91, 79, 102, 109, 125,7, 127, 111, 119, 124, 57, 94, 121, 113};
// these are the byte representations of pins required to display each digit 0~9 then A~F
void setup()
{
  Serial.begin(9600);
  Wire.begin(); // start up I2C bus

  Serial.println("setting up ports");
  ioCon1.portMode(ALLOUTPUT);

  delay(500);
  initDisplay();
  lightUpIO();
}
void initDisplay()
// turns on dynamic mode and adjusts segment current to 12mA
{
Serial.println("starting display");
 Wire.beginTransmission(saa1064);
 Wire.write(B00000000); // this is the instruction byte. Zero means the next byte is the control byte
 Wire.write(B01000111); // control byte (dynamic mode on, digits 1+3 on, digits 2+4 on, 12mA segment current
 Wire.endTransmission();
}

void lightUpIO() {
  Serial.println("light up IO");
  ioCon1.digitalWrite(0, LOW); // power green
  ioCon1.digitalWrite(1, HIGH); // power red
  ioCon1.digitalWrite(2, LOW); // fan green
  ioCon1.digitalWrite(3, HIGH); // fan red
  ioCon1.digitalWrite(4, LOW); // temp green
  ioCon1.digitalWrite(5, HIGH); // temp red
  ioCon1.digitalWrite(6, LOW); // lock
}

int cursor = 0;
#define WORD_SIZE 25

byte registers[4] = {0, 0, 0, 0};

byte words[WORD_SIZE][4] = {
  { 0x00, 0x00, 0x00, 0x00 },
  { 0x01, 0x00, 0x00, 0x00 },
  { 0x03, 0x00, 0x00, 0x00 },
  { 0x03, 0x03, 0x00, 0x00 },
  { 0x07, 0x03, 0x00, 0x00 },
  { 0x07, 0x07, 0x00, 0x00 },
  { 0x0F, 0x07, 0x00, 0x00 },
  { 0x0F, 0x0F, 0x00, 0x00 },
  { 0x1F, 0x0F, 0x00, 0x00 },
  { 0x1F, 0x1F, 0x00, 0x00 },
  { 0x3F, 0x1F, 0x00, 0x00 },
  { 0x3F, 0x3F, 0x00, 0x00 },
  { 0x7F, 0x3F, 0x00, 0x00 },
  { 0x7F, 0x7F, 0x00, 0x00 },
  { 0x7F, 0x7F, 0x01, 0x00 },
  { 0x7F, 0x7F, 0x01, 0x01 },
  { 0x7F, 0x7F, 0x03, 0x01 },
  { 0x7F, 0x7F, 0x03, 0x03 },
  { 0x7F, 0x7F, 0x07, 0x03 },
  { 0x7F, 0x7F, 0x07, 0x07 },
  { 0x7F, 0x7F, 0x0F, 0x07 },
  { 0x7F, 0x7F, 0x0F, 0x0F },
  { 0x7F, 0x7F, 0x1F, 0x0F },
  { 0x7F, 0x7F, 0x3F, 0x1F },
  { 0x7F, 0x7F, 0x7F, 0x1F }
};

byte words_reversed[WORD_SIZE][4] = {
  // { 0x7F, 0x7F, 0x7F, 0x1F },
  { 0x7F, 0x7F, 0x3F, 0x1F },
  { 0x7F, 0x7F, 0x1F, 0x0F },
  { 0x7F, 0x7F, 0x0F, 0x0F },
  { 0x7F, 0x7F, 0x0F, 0x07 },
  { 0x7F, 0x7F, 0x07, 0x07 },
  { 0x7F, 0x7F, 0x07, 0x03 },
  { 0x7F, 0x7F, 0x03, 0x03 },
  { 0x7F, 0x7F, 0x03, 0x01 },
  { 0x7F, 0x7F, 0x01, 0x01 },
  { 0x7F, 0x7F, 0x01, 0x00 },
  { 0x7F, 0x7F, 0x00, 0x00 },
  { 0x7F, 0x3F, 0x00, 0x00 },
  { 0x3F, 0x3F, 0x00, 0x00 },
  { 0x3F, 0x1F, 0x00, 0x00 },
  { 0x1F, 0x1F, 0x00, 0x00 },
  { 0x1F, 0x0F, 0x00, 0x00 },
  { 0x0F, 0x0F, 0x00, 0x00 },
  { 0x0F, 0x07, 0x00, 0x00 },
  { 0x07, 0x07, 0x00, 0x00 },
  { 0x07, 0x03, 0x00, 0x00 },
  { 0x03, 0x03, 0x00, 0x00 },
  { 0x03, 0x00, 0x00, 0x00 },
  { 0x01, 0x00, 0x00, 0x00 }
  // { 0x00, 0x00, 0x00, 0x00 }
};

void displayDigits()
// show all digits 0~9, A~F on all digits of display
{
  Serial.println("displaying digits");


  // first array
  // 000000
  // 000001
  // 000011
  // 000111

  Wire.beginTransmission(saa1064);
  Wire.write(1);

  // write whatever we have placed in these registers
  Wire.write(registers[0]);
  Wire.write(registers[1]);
  Wire.write(registers[2]);
  Wire.write(registers[3]);

  Wire.endTransmission();
}

void movePage() {
  cursor = cursor + 1;

  if (cursor > (WORD_SIZE * 2)) {
    cursor = 0;
  }

  if (cursor <= 24) {
    registers[0] = words[cursor][0];
    registers[1] = words[cursor][1];
    registers[2] = words[cursor][2];
    registers[3] = words[cursor][3];
  } else {
    registers[0] = words_reversed[cursor - WORD_SIZE][0];
    registers[1] = words_reversed[cursor - WORD_SIZE][1];
    registers[2] = words_reversed[cursor - WORD_SIZE][2];
    registers[3] = words_reversed[cursor - WORD_SIZE][3];
  }
}

void clearDisplay()
// clears all digits
{
 Wire.beginTransmission(saa1064);

 Wire.write(1); // instruction byte - first digit to control is 1 (right hand side)
 Wire.write(0); // digit 1 (RHS)
 Wire.write(0); // digit 2
 Wire.write(0); // digit 3
 Wire.write(0); // digit 4 (LHS)
 Wire.endTransmission();
}

#define TCAADDR 0x70

void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void selectLeftHalf() {
  tcaselect(2);
}

void selectRightHalf() {
  tcaselect(1);
}

void loop()
{
  selectRightHalf();
 displayDigits();
 delay(1);

   selectLeftHalf();
 displayDigits();

 movePage();
 delay(20 + (cursor * 2));
 // clearDisplay();
}
/* **** We bitshift the address as the SAA1064 doesn't have the address 0x70 (ADR pin
to GND) but 0x38 and Arduino uses 7-bit addresses- so 8-bit addresses have to
be shifted to the right one bit. Thanks to Malcolm Cox */