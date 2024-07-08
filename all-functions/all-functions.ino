#include "Wire.h"

#define TCAADDR 0x70

// bits; 1 - current value, 2 - alarm state, 3 - last updated seconds
int fanState[3] = {0, 0, -1};
int tempState[3] = {0, 0, -1};
// bits; 1 - nic 1 value, 2 - nic 2 value
bool nicState[2] = {0, 0};
// boolean 
bool powerState = 0;
int modeSelect = 0;
// int; 1 - space available, 2 - CPU
double usbData[2] = {0.91, 2};
// int; 1 - value, 2 - binary latch updated
int usbDiskSpace[2] = {21, 0};
int usbCPUUsage[2] = {0, 0};

// current physical button/switch state
bool lockButtonState = 1;
bool modeButtonState = 1;

byte sideIOHash = 0b00000000;
byte centerIOHash = 0b00000000;

// TCAA ports for left & right sides of IO & center column
int sideDevicePorts[2] = {
  2,  // left side
  1   // right side
};

// - - - PHYSICAL SHIFT REGISTER MEMORY ADDRESSES SPACES START - - -
// 1-to-1 map of physical shift register devices address map
byte leftRightMemoryMap[2][1] = { 
  0b00000000, // left I/O
  0b00000000  // right I/O
};
byte centerColumnsMemoryMap[2][4] = {
  {0b00000000, 0b00000000, 0b00000000, 0b00000000}, // left center column
  {0b00000000, 0b00000000, 0b00000000, 0b00000000}  // right center column
};


// --- PIN DEFINITIONS ---
int POWER_FAIL_PIN = 8;
int FAN_PIN = 9;
int POWER_PIN = 12;
int NIC_1_PIN = 10;
int NIC_2_PIN = 11;
int BUTTON_LOCK_PIN = A3;
int BUTTON_MODE_PIN = A2;

unsigned long lastDebounceTimeLock = 0;
unsigned long lastDebounceTimeMode = 0;
const int DEBOUNCE_DELAY = 30;

unsigned long lastUpdateIO = 0;
const int USB_POLL_INTERVAL = 600;

void setup() {
  Serial.begin(9600);
  // Serial.println(mapIO[0]);
  pinMode(POWER_FAIL_PIN, INPUT);
  pinMode(FAN_PIN, INPUT);
  pinMode(NIC_1_PIN, INPUT);
  pinMode(NIC_2_PIN, INPUT);
  pinMode(POWER_PIN, INPUT);

  pinMode(BUTTON_LOCK_PIN, INPUT_PULLUP);
  pinMode(BUTTON_MODE_PIN, INPUT_PULLUP);

  Serial.println("setup all pinouts");
  Wire.begin(); // start up I2C bus
  Serial.println("i2c setup");

  // ioCon1.portMode(ALLOUTPUT);
  // Serial.println("iocon1 setup");
  _init();
}

void _init() {
  displayUpToNumber(centerColumnsMemoryMap[0], 21);
  displayUpToNumber(centerColumnsMemoryMap[1], 4);

  updateCenterIO();
}

unsigned long currentTime = millis();

void updateTime() {
  currentTime = millis();
}

int fanBlinkCounter = 0;

// TODO use the helper functions to bitwise shift using OR.
// TODO need a better way to compare state changes, and mapping
// bit values with states.
bool shouldCenterUpdate() {
  byte stateHash = 0;
  // stateHash |= nicState[0] << 0; // nic left bit 0
  // stateHash |= nicState[1] << 1; // nic right bit 1
  stateHash |= modeSelect << 0; // nic right bit 2
  // stateHash |= usbDiskSpace[0, 1] << 2;
  // stateHash |= usbData[1] << 6;

  if (stateHash == centerIOHash) {
    return false;
  }

  Serial.print("updating center w/ hash: ");
  Serial.println(stateHash);

  centerIOHash = stateHash;
  return true;
}

bool shouldSidesUpdate() {
  byte stateHash = 0;
  stateHash |= fanState[0] << 0; // fan value bit 0
  stateHash |= fanState[1] << 1; // fan alarm bit 1
  stateHash |= tempState[0] << 2; // temp value bit 2
  stateHash |= tempState[1] << 3; // temp alarm bit 3
  stateHash |= powerState << 4; // power value bit 4
  stateHash |= lockButtonState << 5; // lock button state bit 5

  if (stateHash == sideIOHash) {
    return false;
  }

  Serial.print("updating side w/ hash: ");
  Serial.println(stateHash);

  sideIOHash = stateHash;
  return true;
}

void checkInputForAlarm(int value, int* state) {
  bool _value = state[0];
  bool alarm = state[1];
  int lastAlarmAt = state[2];
  bool stateChanged = value != _value;

  if (stateChanged) {
    state[0] = value; // TODO does this work (?)
  }

  // was 2 seconds ago last time it toggled
  if (stateChanged == 1 && (currentTime - lastAlarmAt > 250)) {
    state[1] = 1;
    state[2] = currentTime;
    Serial.println("alarm!");
  }

  // detect that blinking stopped
  // if alarm && it changes && it has been more then 2 times the frex since last toggle
  if (alarm == 1 && (currentTime > lastAlarmAt + 1000)) {
    // nothing to see here
    state[1] = 0;
    // state[2] = currentTime;
    Serial.println("no alarm!");
  }
}

void updateState() {
  bool fanS = digitalRead(FAN_PIN);
  checkInputForAlarm(fanS, fanState);
  checkInputForAlarm(fanS, tempState);
  powerState = digitalRead(POWER_PIN);

  nicState[0] = !digitalRead(NIC_1_PIN);
  nicState[1] = !digitalRead(NIC_2_PIN);
}

unsigned long lastUSBPollTime = 0;
void updateUSBData() {
  if (currentTime > lastUSBPollTime + USB_POLL_INTERVAL) {
    Serial.println("polling usb");

    int USB_DATA = random(22);

    if (USB_DATA > 0) {
      usbDiskSpace[0] = USB_DATA;
      usbDiskSpace[1] = 1;

      updateCenterIO();
    }
    lastUSBPollTime = currentTime;
  }
}

bool handleButtonPress(int address, bool* value, long unsigned int* debounce) {
  bool btnVal = !digitalRead(address);

  if (btnVal != *value) {
    *debounce = currentTime;
    *value = btnVal;
  }

  if (*debounce > 0 && (currentTime - *debounce) > DEBOUNCE_DELAY) {
    *debounce = 0;
    return true;
  }

  return false;
}

void changeModes() {
  bool btnVal = !digitalRead(BUTTON_MODE_PIN);

  if (btnVal == 1) {
    if (modeSelect == 2) {
      modeSelect = 0;
      return;
    }

    modeSelect += 1;
  }
}

void tcaselect(uint8_t i) {
  if (i > 7) return;
 
  Wire.beginTransmission(TCAADDR);
  Wire.write(1 << i);
  Wire.endTransmission();  
}

void writeEthernetActivity() {
  for (int i = 0; i < 2; i++) {
    tcaselect(sideDevicePorts[i]);
    
    byte* reg = centerColumnsMemoryMap[i];
    resetEthernetBit(reg);
    computeEthernetActivity(reg, nicState[i]);
    writeCenterColumnFirstBank(reg);
    delay(1);
  }
}

void updateLeftRightIO() {
  for (int i = 0; i < 2; i++) {
    tcaselect(sideDevicePorts[i]);
    
    byte* reg = leftRightMemoryMap[i];
    resetSideBanks(reg);

    // power LED
    if (powerState == 0) {
      setPowerRed(reg);
    } else {
      setPowerGreen(reg);
    }
  
    // fan LED
    if (fanState[0] == 1 && fanState[1] == 0) {
      setFanGreen(reg);
    } else if (fanState[0] == 1 && fanState[1] == 1) {
      setFanRed(reg);
    } else if (fanState[0] == 0 && fanState[1] == 1) {
      setFanOff(reg);
    }

    // temp LED
    if (tempState[1] == 1) {
      setTempRed(reg);
    } else {
      setTempGreen(reg);
    }

    // lock LED
    if (lockButtonState == 1) {
      setLockButton(reg);
    }

    updateIOLED(reg);
    delay(1);
  }
}

void updateCenterIO() {
  int val;
  Serial.print("modeSelect: ");
  Serial.println(modeSelect);

  if (modeSelect == 0) { // CPU mode
    displayUpToNumber(centerColumnsMemoryMap[0], usbDiskSpace[0]);
    displayUpToNumber(centerColumnsMemoryMap[1], usbDiskSpace[0]);
  } else if (modeSelect == 1) { // disk mode
    displayPercentage(centerColumnsMemoryMap[0], 0.91);
    displayPercentage(centerColumnsMemoryMap[1], 0.91);
  } else if (modeSelect == 2) { // random
    displayNumber(centerColumnsMemoryMap[0], 1 + random(22));
    displayNumber(centerColumnsMemoryMap[1], 1 + random(22));
  }

  for (int i = 0; i < 2; i++) {
    tcaselect(sideDevicePorts[i]);
    writeCenterColumn(centerColumnsMemoryMap[i]);
    delay(1);
  }
}

void loop() {
  updateTime();
  updateState();
  updateUSBData();
  writeEthernetActivity();

  // lock switch
  handleButtonPress(BUTTON_LOCK_PIN, &lockButtonState, &lastDebounceTimeLock);

  // mode select button
  if (handleButtonPress(BUTTON_MODE_PIN, &modeButtonState, &lastDebounceTimeMode) && modeButtonState == 1) {
    changeModes();
    
    displayUpToNumber(centerColumnsMemoryMap[1], random(24));
  }

  if (shouldSidesUpdate()) {
    updateLeftRightIO();
  }

  if (shouldCenterUpdate()) {
    updateCenterIO();
  }
  
  delay(20);
}
