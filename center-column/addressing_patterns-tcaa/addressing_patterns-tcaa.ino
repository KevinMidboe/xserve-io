#include "Wire.h" // enable I2C bus

#define TCAADDR 0x70
byte saa1064 = 0x3B; // define the I2C bus address for our SAA1064

byte bank1;
byte bank2;
byte bank3;
byte bank4;

int TOTAL_LEDS = 23;
byte activityLED = 0b00000001;
byte leds[23][4] = {
  {0b00000010, 0b00000000, 0b00000000, 0b00000000}, //  1
  {0b00000000, 0b00000010, 0b00000000, 0b00000000}, //  2
  {0b00000100, 0b00000000, 0b00000000, 0b00000000}, //  3
  {0b00000000, 0b00000100, 0b00000000, 0b00000000}, //  4
  {0b00001000, 0b00000000, 0b00000000, 0b00000000}, //  5
  {0b00000000, 0b00001000, 0b00000000, 0b00000000}, //  6
  {0b00010000, 0b00000000, 0b00000000, 0b00000000}, //  7
  {0b00000000, 0b00010000, 0b00000000, 0b00000000}, //  8
  {0b00100000, 0b00000000, 0b00000000, 0b00000000}, //  9
  {0b00000000, 0b00100000, 0b00000000, 0b00000000}, // 10
  {0b01000000, 0b00000000, 0b00000000, 0b00000000}, // 11
  {0b00000000, 0b01000000, 0b00000000, 0b00000000}, // 12
  {0b00000000, 0b00000000, 0b00000001, 0b00000000}, // 13
  {0b00000000, 0b00000000, 0b00000000, 0b00000001}, // 14
  {0b00000000, 0b00000000, 0b00000010, 0b00000000}, // 15
  {0b00000000, 0b00000000, 0b00000000, 0b00000010}, // 16
  {0b00000000, 0b00000000, 0b00000100, 0b00000000}, // 17
  {0b00000000, 0b00000000, 0b00000000, 0b00000100}, // 18
  {0b00000000, 0b00000000, 0b00001000, 0b00000000}, // 19
  {0b00000000, 0b00000000, 0b00000000, 0b00001000}, // 20
  {0b00000000, 0b00000000, 0b00010000, 0b00000000}, // 21
  {0b00000000, 0b00000000, 0b00100000, 0b00000000}, // 22
  {0b00000000, 0b00000000, 0b01000000, 0b00000000}  // 23
};

void initDisplay()
{
  Serial.println("starting display");
  Wire.beginTransmission(saa1064);
  Wire.write(B00000000); // this is the instruction byte. Zero means the next byte is the control byte
  Wire.endTransmission();
}

void setup()
{
  Serial.begin(9600);
  Wire.begin(); // start up I2C bus

  Serial.println("setting up ports");

  resetBanks();
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

void _write() {
  selectLeft();
  write();
  delay(1);
  selectRight();
  write();
}

void write() {
  Wire.beginTransmission(saa1064);
  Wire.write(1);

  Wire.write(bank1);
  Wire.write(bank2);
  Wire.write(bank3);
  Wire.write(bank4);

  Wire.endTransmission();
}

void resetBanks() {
  bank1 = 0;
  bank2 = 0;
  bank3 = 0;
  bank4 = 0;
}

void displayNumber(int number) { 
  bank1 = leds[number - 1][0];
  bank2 = leds[number - 1][1];
  bank3 = leds[number - 1][2];
  bank4 = leds[number - 1][3];
}

void displayUpToNumber(int number) { 
  for (int i = 0; i < number; i++) {
    bank1 |= leds[i][0];
    bank2 |= leds[i][1];
    bank3 |= leds[i][2];
    bank4 |= leds[i][3];
  }
}

void displayNumbers(int* numbersList, size_t length) {
  for (int i = 0; i < length; i++) {
    int num = numbersList[i] - 1;
    bank1 |= leds[num][0];
    bank2 |= leds[num][1];
    bank3 |= leds[num][2];
    bank4 |= leds[num][3];
  }
}

void displayPercentage(double percentDecimal) {
  int numberOfTotal = round(percentDecimal * TOTAL_LEDS);
  displayUpToNumber(numberOfTotal);
}

void computeEthernetActivity(bool val) {
  bank1 |= val; // 0 or 1
}

void loop() {
  resetBanks();
  delay(10);

  int length = 8;
  int numbers[8] = {2,3,5,6,7,13,17,22};
  displayNumbers(numbers, length);
  // displayUpToNumber(15);
  // displayNumber(16);
  // displayPercentage(0.91);
  computeEthernetActivity(false);

  _write();
  delay(1000);
}