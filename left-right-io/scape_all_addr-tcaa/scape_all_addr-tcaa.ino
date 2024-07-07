#include "Wire.h" // enable I2C bus
#include <PCA9554.h>  // Load the PCA9554 Library

byte saa1064 = 0x3B; // define the I2C bus address for our SAA1064 (pin 1 to GND) ****
#define TCAADDR 0x70
PCA9554 ioCon1(0x24);  // Create an object at this address

uint8_t mapIO = 0b10000000;

void shiftL() {
  mapIO = (mapIO << 1) | ((mapIO & 0x80) >> 7);
}

void _write() {
  for (int i = 0; i < 8; ++i) {
    ioCon1.digitalWrite(i, (mapIO & (1 << i)) ? 0 : 1);
  }
}

void write() {
  Serial.println("writing to PCA9554 device");

  selectLeft();
  _write();
  delay(1);

  selectRight();
  _write();
}

void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void selectLeft() { tcaselect(2); }
void selectRight() { tcaselect(1); }

void setup()
{
  Serial.begin(9600);
  Serial.println("Setup");

  Wire.begin();
  ioCon1.portMode(ALLOUTPUT);
  delay(500);
}


void loop() 
{
  write();
  shiftL();
  delay(300);
}