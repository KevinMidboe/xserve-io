#include "Wire.h"
#include <PCA9554.h>  // Load the PCA9554 Library

#define TCAADDR 0x70

// byte saa1064 = 0x3B;
PCA9554 ioCon1(0x24);  // Create an object at this address


// LEDs have two addresses, one for RED another for GREEN
bool fanGreenLED;
bool fanRedLED;
bool tempGreenLED;
bool tempRedLED;
bool powerGreenLED;
bool powerRedLED;
bool modeSelect;

bool nic1State = 0;
bool nic2State = 0;
bool lockButtonState = 1;
bool modeButtonState = 1;

int powerState = 0;
// first bit is current value, second bit is alarm state
int fanState[2] = {0, 0};
int tempState[2] = {0, 0};

byte centerColumnsMemoryMap[2][4] = {
  {0b00000000, 0b00000000, 0b00000000, 0b00000000}, // left column
  {0b00000000, 0b00000000, 0b00000000, 0b00000000}  // right column
};

byte ioHash = 0;

// --- ADDRESS MAPs ---
int IOMapLength = 8;
bool* mapIO[8] = {&powerGreenLED, &powerRedLED, &fanGreenLED, &fanRedLED, &tempGreenLED, &tempRedLED, &lockButtonState, &modeSelect};
// bool mapNics[3] = {nic1State, nic2State};

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
const int IO_UPDATE_INTERVAL = 200;

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

  ioCon1.portMode(ALLOUTPUT);
  Serial.println("iocon1 setup");

  delay(500);
  
  selectRightHalf();
  lightUpIO();
  selectLeftHalf();
  lightUpIO();
}

void lightUpIO() {
  Serial.println("ligthing up top io");
  for (int i = 0; i < IOMapLength; ++i) {
    ioCon1.digitalWrite(i, mapIO[i]);
  }
  Serial.println("finished ligthing up top io");
}

unsigned long currentTime = millis();

void updateTime() {
  currentTime = millis();
}

int fanBlinkCounter = 0;

unsigned long fanToggledAt;
void updateFanState(int state) {
  // fan changed and did so within 1 second
  bool stateChanged = state != fanState[0];
  fanState[0] = state;

  // detect that blinking is happening
  if (stateChanged && (currentTime - fanToggledAt > 250)) {
    fanState[1] = 1;
    fanToggledAt = currentTime;
  }
  
  // detect that blinking stopped
  // if alarm && it changes && it has been more then 2 times the frex since last toggle
  if (fanState[1] == 1 && (currentTime > fanToggledAt + 1000)) {
    // nothing to see here
    fanState[1] = 0;
  }
}

unsigned long tempToggledAt;
void updateTempState(int state) {
  bool stateChanged = state != tempState[0];
  tempState[0] = state;

  // was 2 seconds ago last time it toggled
  if (stateChanged == 1 && (currentTime - tempToggledAt > 250)) {
    tempState[1] = 1;
    tempToggledAt = currentTime;
  }

  // detect that blinking stopped
  // if alarm && it changes && it has been more then 2 times the frex since last toggle
  if (tempState[1] == 1 && (currentTime > tempToggledAt + 1000)) {
    // nothing to see here
    tempState[1] = 0;
  }
}

// TODO check that this goes RED when off
void greenRed(bool* greenLED, bool* redLED, bool value) {
  // if no power set red led
  if (value == 0) {
    *greenLED = 0;
    *redLED = 1;
  // else if power set green led
  } else {
    *greenLED = 1;
    *redLED = 0;
  }
}

void greenRedOnAlarm(bool* greedLED, bool* redLED, bool alarm) {
  // if not alarm set green led
  if (alarm == 0) {
    *greedLED = 1;
    *redLED = 0;
  // if alarm set red led
  } else {
    *greedLED = 0;
    *redLED = 1;
  }
}

void greenRedBlinkOnAlarm(bool* greenLED, bool* redLED, bool value, bool alarm) {
// void greenRedBlinkOnAlarm(bool* greenLED, bool* redLED, bool* currentState, bool* alarmState) {
  // if no alarm --> set green led
  if (alarm == 0) {
    *greenLED = 1;
    *redLED = 0;
    return;
  }

  // everything below assumes alarm state
  // alarm and signal --> red led (blink on)
  if (value == 0) {
    *greenLED = 0;
    *redLED = 1;
  } 
  // alarm and no signal --> no led (blink off)
  else {
    *greenLED = 0;
    *redLED = 0;
  }
}

void writeIOLEDPoweredOff() {
    ioCon1.digitalWrite(0, HIGH);
    ioCon1.digitalWrite(1, LOW);
    ioCon1.digitalWrite(2, HIGH);
    ioCon1.digitalWrite(3, HIGH);
    ioCon1.digitalWrite(4, HIGH);
    ioCon1.digitalWrite(5, HIGH);
    ioCon1.digitalWrite(6, *(mapIO[6]) == 1 ? 0 : 1);
}

void updateIOLED() {
  if (powerState == 0) {
    writeIOLEDPoweredOff();
    return;
  }

  for (int i = 0; i < IOMapLength; ++i) {
    ioCon1.digitalWrite(i, *(mapIO[i]) == 1 ? 0 : 1);
  }
}

int lastIOHash = -1;
bool shouldUpdateIOLed() {
  greenRedBlinkOnAlarm(&fanGreenLED, &fanRedLED, fanState[0], fanState[1]);
  greenRedOnAlarm(&tempGreenLED, &tempRedLED, tempState[1]);
  greenRed(&powerGreenLED, &powerRedLED, powerState);

  byte ioHash = 0;
  for (byte i = 0; i < IOMapLength; i++)
  {
    bitWrite(ioHash, i, *(mapIO[i]));
  }

  if (ioHash == lastIOHash) {
    // Don't update i2c ctrl if same value
    // Serial.println("Not updating!!");
    return false;
  }

  // Serial.println("Updating IO!!");
  lastIOHash = ioHash;
  return true;
}

void updateState() {
  bool fanS = digitalRead(FAN_PIN);
  updateFanState(fanS);
  updateTempState(fanS);
  powerState = digitalRead(POWER_PIN);

  nic1State = !digitalRead(NIC_1_PIN);
  nic2State = !digitalRead(NIC_2_PIN);
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
    modeSelect = !modeSelect;
  }
}

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

void writeEthernetActivity() {
  selectLeftHalf();
  resetEthernetBit(centerColumnsMemoryMap[0]);
  computeEthernetActivity(centerColumnsMemoryMap[0], nic1State);
  writeCenterColumnFirstBank(centerColumnsMemoryMap[0]);
  delay(1);

  selectRightHalf();
  resetEthernetBit(centerColumnsMemoryMap[1]);
  computeEthernetActivity(centerColumnsMemoryMap[1], nic2State);
  writeCenterColumnFirstBank(centerColumnsMemoryMap[1]);
}

void loop() {
  // Serial.println("loop start");
  
  updateTime();
  updateState();
  handleButtonPress(BUTTON_LOCK_PIN, &lockButtonState, &lastDebounceTimeLock);
  if (handleButtonPress(BUTTON_MODE_PIN, &modeButtonState, &lastDebounceTimeMode)) {
    changeModes();
  }
  
  writeEthernetActivity();
  
  if (shouldUpdateIOLed() == true) {
    resetBanks(centerColumnsMemoryMap[0]);

    selectLeftHalf();
    updateIOLED();
    // displayUpToNumber(21);
    displayPercentage(centerColumnsMemoryMap[0], 0.91);

    delay(1);
    resetBanks(centerColumnsMemoryMap[1]);
    selectRightHalf();
    updateIOLED();
    displayUpToNumber(centerColumnsMemoryMap[1], random(24));
  }
  // if (currentTime - lastUpdateIO > IO_UPDATE_INTERVAL) {
  //   lastUpdateIO = currentTime;
  //   updateIOLED();
  // }
//   selectRightHalf();
// selectLeftHalf();
// shouldUpdateIOLed

//   updateRightIOLED();
  
  delay(20);
  // Serial.println("loop end");
}