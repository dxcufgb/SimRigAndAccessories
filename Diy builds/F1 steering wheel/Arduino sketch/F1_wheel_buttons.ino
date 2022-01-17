#include <Key.h>
#include <Keypad.h>
#include "EncoderStepCounter.h"
#include <EasyButton.h>

#define ENABLE_PULLUPS
#define NUMBUTTONS 20
#define ROTARY_DELAY_MS 10
//Encoder 1
#define ENCODER1_PIN1 12
#define ENCODER1_INT1 digitalPinToInterrupt(ENCODER1_PIN1)
#define ENCODER1_PIN2 11
#define ENCODER1_INT2 digitalPinToInterrupt(ENCODER1_PIN2)
EncoderStepCounter encoder1(ENCODER1_PIN1, ENCODER1_PIN2, HALF_STEP);

//Encoder 2
#define ENCODER2_PIN1 10
#define ENCODER2_INT1 digitalPinToInterrupt(ENCODER2_PIN1)
#define ENCODER2_PIN2 9
#define ENCODER2_INT2 digitalPinToInterrupt(ENCODER2_PIN2)
EncoderStepCounter encoder2(ENCODER2_PIN1, ENCODER2_PIN2, HALF_STEP);

//Encoder 3
#define ENCODER3_PIN1 8
#define ENCODER3_INT1 digitalPinToInterrupt(ENCODER3_PIN1)
#define ENCODER3_PIN2 25
#define ENCODER3_INT2 digitalPinToInterrupt(ENCODER3_PIN2)
EncoderStepCounter encoder3(ENCODER3_PIN1, ENCODER3_PIN2, HALF_STEP);

//Keypad SETUP
const byte ROWS = 4;
const byte COLS = 3;

byte buttons[ROWS][COLS] = {
  {0,1,2},
  {3,4,5},
  {6,7,8},
  {9,10,11}
};

byte rowPins[ROWS] = {13,4,0,27};
byte colPins[COLS] = {7,5,1};

Keypad keypad = Keypad( makeKeymap(buttons), rowPins, colPins, ROWS, COLS );

bool rotary1Pressed = false;
bool rotary2Pressed = false;
bool rotary3Pressed = false;

unsigned long rotary1Delay = millis();
unsigned long rotary2Delay = millis();
unsigned long rotary3Delay = millis();

signed long position1 = 0;
signed long position2 = 0;
signed long position3 = 0;

int clutchValue = 0;

#define ClutchPin1 A0
#define ClutchPin2 A1
#define BitePointAdjustment A2

#define CalibrateClutchPaddlesBtn1Index 0
#define CalibrateClutchPaddlesBtn2Index 1

#define MIN_CLUTCH_PIN_1_EEPROM_ADDRESS 120
#define MIN_CLUTCH_PIN_2_EEPROM_ADDRESS 121
#define MAX_CLUTCH_PIN_1_EEPROM_ADDRESS 122
#define MAX_CLUTCH_PIN_2_EEPROM_ADDRESS 123

int CLUTCH_PIN_1_MAX_VAL = 460;
//int CLUTCH_PIN_1_MAX_VAL EEPROM.read(MAX_CLUTCH_PIN_1_EEPROM_ADDRESS);
int CLUTCH_PIN_2_MAX_VAL = 420;
//int CLUTCH_PIN_2_MAX_VAL EEPROM.read(MAX_CLUTCH_PIN_2_EEPROM_ADDRESS);
#define BITE_POINT_MAX_VAL 1003

int CLUTCH_PIN_1_MIN_VAL = 360;
//int CLUTCH_PIN_1_MIN_VAL EEPROM.read(MIN_CLUTCH_PIN_1_EEPROM_ADDRESS);
int CLUTCH_PIN_2_MIN_VAL  =330;
//int CLUTCH_PIN_2_MIN_VAL EEPROM.read(MIN_CLUTCH_PIN_2_EEPROM_ADDRESS);
#define BITE_POINT_MIN_VAL 20

float c1 = 0;
float c2 = 0;
float bp = 0;

unsigned long startup = millis();

bool calibrated = false;

#define SPEAKER_PIN 24
#define SPEAKER_TONE 4000

void setup() {
  Serial.begin(9600);
  // Initialize encoder
  encoder1.begin();
  encoder2.begin();
  encoder3.begin();
  Joystick.Z(clutchValue);
  // Initialize interrupts
  attachInterrupt(ENCODER1_INT1, interrupt1, CHANGE);
  attachInterrupt(ENCODER1_INT2, interrupt1, CHANGE);
  attachInterrupt(ENCODER2_INT1, interrupt2, CHANGE);
  attachInterrupt(ENCODER2_INT2, interrupt2, CHANGE);
  attachInterrupt(ENCODER3_INT1, interrupt3, CHANGE);
  attachInterrupt(ENCODER3_INT2, interrupt3, CHANGE);
  pinMode(ClutchPin1, INPUT_PULLUP);
  pinMode(ClutchPin2, INPUT_PULLUP);
  pinMode(BitePointAdjustment, INPUT_PULLUP);
}

// Call tick on every change interrupt
void interrupt1() {
  encoder1.tick();
}

void interrupt2() {
  encoder2.tick();
}

void interrupt3() {
  encoder3.tick();
}

void loop() {
  if (millis() < startup + 10000) {
    if (keypad.key[CalibrateClutchPaddlesBtn1Index].kstate == HOLD && keypad.key[CalibrateClutchPaddlesBtn2Index].kstate == HOLD && !calibrated) {
      calibrateClutch();
    }
  } else if (!calibrated) {
    okBeeps();
    calibrated = true;
  }
  checkAllButtons();
  checkRotaries();
}

void checkAllButtons(void) {
  if (keypad.getKeys()) {
    for (int i=0; i<LIST_MAX; i++) {
      if (keypad.key[i].stateChanged) {
        switch (keypad.key[i].kstate) {  
          case PRESSED:
            Joystick.button(keypad.key[i].kchar, 1);
            break;
          case HOLD:
              break;
          case RELEASED:
          case IDLE:
            Joystick.button(keypad.key[i].kchar, 0);
            break;
        }
      }   
    }
  }
}

void checkRotaries() {
  position1 += encoder1.getPosition();
  position2 += encoder2.getPosition();
  position3 += encoder3.getPosition();
  encoder1.reset();
  encoder2.reset();
  encoder3.reset();
  if (position1 < 0) {
    if (rotary1Pressed && (millis() - rotary1Delay >= ROTARY_DELAY_MS)) {
      Joystick.button(12, 0);
      rotary1Pressed = false;
      position1++;
      rotary1Delay = millis();
    } else if (!rotary1Pressed && (millis() - rotary1Delay >= ROTARY_DELAY_MS)) {
     Joystick.button(12, 1);
     rotary1Pressed = true;
     rotary1Delay = millis();
    }
  } else if (position1 > 0) {
    if (rotary1Pressed && (millis() - rotary1Delay >= ROTARY_DELAY_MS)) {
      Joystick.button(13, 0);
      rotary1Pressed = false;
      position1--;
      rotary1Delay = millis();
    } else if (!rotary1Pressed && (millis() - rotary1Delay >= ROTARY_DELAY_MS)) {
     Joystick.button(13, 1);
     rotary1Pressed = true;
     rotary1Delay = millis();
    }
  }
  
  if (position2 < 0) {
    if (rotary2Pressed && (millis() - rotary2Delay >= ROTARY_DELAY_MS)) {
      Joystick.button(14, 0);
      rotary2Pressed = false;
      position2++;
      rotary2Delay = millis();
    } else if (!rotary2Pressed && (millis() - rotary2Delay >= ROTARY_DELAY_MS)) {
     Joystick.button(14, 1);
     rotary2Pressed = true;
     rotary2Delay = millis();
    }
  } else if (position2 > 0) {
    if (rotary2Pressed && (millis() - rotary2Delay >= ROTARY_DELAY_MS)) {
      Joystick.button(15, 0);
      rotary2Pressed = false;
      position2--;
      rotary2Delay = millis();
    } else if (!rotary2Pressed && (millis() - rotary2Delay >= ROTARY_DELAY_MS)) {
     Joystick.button(15, 1);
     rotary2Pressed = true;
     rotary2Delay = millis();
    }
  }

  if (position3 < 0) {
    if (rotary3Pressed && (millis() - rotary3Delay >= ROTARY_DELAY_MS)) {
      Joystick.button(16, 0);
      rotary3Pressed = false;
      position3++;
      rotary3Delay = millis();
    } else if (!rotary3Pressed && (millis() - rotary3Delay >= ROTARY_DELAY_MS)) {
     Joystick.button(16, 1);
     rotary3Pressed = true;
     rotary3Delay = millis();
    }
  } else if (position3 > 0) {
    if (rotary3Pressed && (millis() - rotary3Delay >= ROTARY_DELAY_MS)) {
      Joystick.button(17, 0);
      rotary3Pressed = false;
      position3--;
      rotary3Delay = millis();
    } else if (!rotary3Pressed && (millis() - rotary3Delay >= ROTARY_DELAY_MS)) {
     Joystick.button(17, 1);
     rotary3Pressed = true;
     rotary3Delay = millis();
    }
  }
}

int filterAxisValue(int value)
{
  if (value > 1023) {
    return 1023;
  } else if (value < 0) {
    return 0;
  } else {
    return value;
  }
}

int calculateClutchAxis(int c1, int c2, int bp)
{
  int cTot = (c1 * 0.25) + (((c2 * 0.25) + (bp * 2)) / 2);
  return filterAxisValue(cTot);
}

void calibrateClutch()
{  
  while (calibrated == false) {
    waitForButtonRelease();

    readClutchMinValues();
    
    waitForButtonRelease();

    readClutchMaxValues();
    
    waitForButtonRelease();
    
    if ((CLUTCH_PIN_1_MIN_VAL < CLUTCH_PIN_1_MAX_VAL) && (CLUTCH_PIN_2_MIN_VAL < CLUTCH_PIN_2_MAX_VAL)) {
      successfullCalibration();
      calibrated = true;
    } else {
      Serial.println("Beeeeeeeeeeeeeeep");
      tone(SPEAKER_PIN, SPEAKER_TONE);
      delay(2000);
      Serial.println("No beep!");
      noTone(SPEAKER_PIN);
    }
  }
}

void readClutchMinValues()
{
  int minC1 = 1023;
  int minC2 = 1023;
  
  while (waitForButtonPress()) {
    if (analogRead(ClutchPin1) < minC1) {
      minC1 = analogRead(ClutchPin1);
    }
    if (analogRead(ClutchPin2) < minC2) {
      minC2 = analogRead(ClutchPin2);
    }
  }
  
  CLUTCH_PIN_1_MIN_VAL = minC1 + 20;

  CLUTCH_PIN_2_MIN_VAL = minC2 + 20;
}

void readClutchMaxValues()
{
  int maxC1 = 0;
  int maxC2 = 0;
  
  while (waitForButtonPress()) {
    if (analogRead(ClutchPin1) > maxC1) {
      maxC1 = analogRead(ClutchPin1);
    }
    if (analogRead(ClutchPin2) > maxC2) {
      maxC2 = analogRead(ClutchPin2);
    }
  }
  
  CLUTCH_PIN_1_MAX_VAL = maxC1 - 20;

  CLUTCH_PIN_2_MAX_VAL = maxC2 - 20;
}

void successfullCalibration()
{
  //EEPROM.write(MIN_CLUTCH_PIN_1_EEPROM_ADDRESS, CLUTCH_PIN_1_MIN_VAL);
  //EEPROM.write(MIN_CLUTCH_PIN_2_EEPROM_ADDRESS, CLUTCH_PIN_2_MIN_VAL);
  //EEPROM.write(MAX_CLUTCH_PIN_1_EEPROM_ADDRESS, CLUTCH_PIN_1_MAX_VAL);
  //EEPROM.write(MAX_CLUTCH_PIN_2_EEPROM_ADDRESS, CLUTCH_PIN_2_MAX_VAL);
  
  Serial.println("Paddles calibrated!");

  Serial.print("Min value for clutch paddle 1: ");
  Serial.println(CLUTCH_PIN_1_MIN_VAL);
  Serial.print("Max value for clutch paddle 1: ");
  Serial.println(CLUTCH_PIN_1_MAX_VAL);
  
  Serial.print("Min value for clutch paddle 2: ");
  Serial.println(CLUTCH_PIN_2_MIN_VAL);
  Serial.print("Max value for clutch paddle 2: ");
  Serial.println(CLUTCH_PIN_2_MAX_VAL);
  
  okBeeps();
}

void waitForButtonRelease()
{
  tone(SPEAKER_PIN, SPEAKER_TONE);
  Serial.println("Beep!");
  bool buttonsHeld = true;
  bool button1Held = true;
  bool button2Held = true;
  while (buttonsHeld) {
    if (keypad.getKeys()) {
      switch (keypad.key[CalibrateClutchPaddlesBtn1Index].kstate) {  
        case RELEASED:
        case IDLE:
          button1Held = false;
          break;
        default:
          //continue
          break;
      }  

      switch (keypad.key[CalibrateClutchPaddlesBtn2Index].kstate) {  
        case RELEASED:
        case IDLE:
          button2Held = false;
          break;
        default:
          //continue
          break;
      }
    }
    
    if (!button1Held && !button2Held) {
      buttonsHeld = false;
    }
  }
  Serial.println("No beep!");
  noTone(SPEAKER_PIN);
  delay(100);
}

bool waitForButtonPress()
{
  bool buttonsHeld = false;
  bool button1Held = false;
  bool button2Held = false;
  if (keypad.getKeys()) {
    switch (keypad.key[CalibrateClutchPaddlesBtn1Index].kstate) {  
      case HOLD:
        button1Held = true;
        break;
      default:
        //continue
        break;
    }
    switch (keypad.key[CalibrateClutchPaddlesBtn2Index].kstate) {  
      case HOLD:
        button2Held = true;
        break;
      default:
        //continue
        break;
    }
  }
    
  if (button1Held && button2Held) {
    return false;
  } else {
    return true;
  }
}

void okBeeps() {
  for (int i = 0; i < 4; i++) {
    Serial.println("Beep!");
    tone(SPEAKER_PIN, SPEAKER_TONE);
    delay(100);
    Serial.println("No beep!");
    noTone(SPEAKER_PIN);
    delay(100);
  }
}
