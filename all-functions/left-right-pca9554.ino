#include <PCA9554.h>  // Load the PCA9554 Library

PCA9554 ioCon1(0x24);  // Create an object at this address

byte LED_STATES_LEFT_RIGHT[7] = {
  0b00000001, // power green LED
  0b00000010, // power red LED
  0b00000100, // fan green LED
  0b00001000, // fan red LED
  0b00010000, // temp green LED
  0b00100000, // temp red LED
  0b01000000  // lock button LED
};

void setupPCADevice() {
  ioCon1.portMode(ALLOUTPUT);
}

void resetSideBanks(byte* bank) {
  *bank = 0;
}

void updateIOLED(byte* bank) {
  for (int i = 0; i < 8; ++i) {
    ioCon1.digitalWrite(i, bitRead(*bank, i) == 1 ? 0 : 1);
  }
}

void writeIOLEDPoweredOff(byte* bank) {
  byte mask = 0b01011110;
  bitWrite(mask, bank[0], 0);
  bank = mask;
}

void setPowerGreen(byte* bank) {
  *bank |= LED_STATES_LEFT_RIGHT[0];
}

void setPowerRed(byte* bank) {
  *bank |= LED_STATES_LEFT_RIGHT[1];
}

void setPowerOff(byte* bank) {
  bitWrite(*bank, 0, 0); // green LED off
  bitWrite(*bank, 1, 0); // red LED off
}

void setFanGreen(byte* bank) {
  *bank |= LED_STATES_LEFT_RIGHT[2];
}

void setFanRed(byte* bank) {
  *bank |= LED_STATES_LEFT_RIGHT[3];
}

void setFanOff(byte* bank) {
  bitWrite(*bank, 2, 0); // green LED off
  bitWrite(*bank, 3, 0); // red LED off
}

void setTempGreen(byte* bank) {
  *bank |= LED_STATES_LEFT_RIGHT[4];
}

void setTempRed(byte* bank) {
  *bank |= LED_STATES_LEFT_RIGHT[5];
}

void setTempOff(byte* bank) {
  bitWrite(*bank, 4, 0); // green LED off
  bitWrite(*bank, 5, 0); // red LED off
}

void setLockButton(byte* bank) {
  *bank |= LED_STATES_LEFT_RIGHT[6];
}

void setLockButtonOff(byte* bank) {
  bitWrite(*bank, 6, 0); // lock button LED off
}
