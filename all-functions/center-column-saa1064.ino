// #include "Wire.h" // enable I2C bus

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

void writeCenterColumn(byte* banks) {
  Wire.beginTransmission(saa1064);
  Wire.write(1);

  Wire.write(banks[0]);
  Wire.write(banks[1]);
  Wire.write(banks[2]);
  Wire.write(banks[3]);

  Wire.endTransmission();
}

void writeCenterColumnFirstBank(byte* banks) {
  Wire.beginTransmission(saa1064);
  Wire.write(1);

  Wire.write(banks[0]);

  Wire.endTransmission();
}

// TODO prevent this from overwriting ethernet bit
void resetCenterBanks(byte* banks) {
  banks[0] = 0;
  banks[1] = 0;
  banks[2] = 0;
  banks[3] = 0;
}

void resetEthernetBit(byte* banks) {
  bitWrite(banks[0], 0, 0);
}

void displayNumber(byte* banks, int number) { 
  resetCenterBanks(banks);

  banks[0] = leds[number - 1][0];
  banks[1] = leds[number - 1][1];
  banks[2] = leds[number - 1][2];
  banks[3] = leds[number - 1][3];
}

void displayUpToNumber(byte* banks, int number) { 
  resetCenterBanks(banks);

  for (int i = 0; i < number; i++) {
    banks[0] |= leds[i][0];
    banks[1] |= leds[i][1];
    banks[2] |= leds[i][2];
    banks[3] |= leds[i][3];
  }
}

void displayNumbers(byte* banks, int* numbersList, size_t length) {
  resetCenterBanks(banks);

  for (int i = 0; i < length; i++) {
    int num = numbersList[i] - 1;
    banks[0] |= leds[num][0];
    banks[1] |= leds[num][1];
    banks[2] |= leds[num][2];
    banks[3] |= leds[num][3];
  }
}

void displayPercentage(byte* banks, double percentDecimal) {
  int numberOfTotal = round(percentDecimal * TOTAL_LEDS);
  displayUpToNumber(banks, numberOfTotal);
}

void computeEthernetActivity(byte* banks, bool val) {
  // bitWrite(bank1, 0, val);
  banks[0] |= val; // 0 or 1
}
