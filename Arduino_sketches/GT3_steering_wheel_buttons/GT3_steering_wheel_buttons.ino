#include <Key.h>
#include <Keypad.h>
#include "EncoderStepCounter.h"
#include <EasyButton.h>

#define ENABLE_PULLUPS
#define NUMBUTTONS 20
//Encoder 1
#define ENCODER1_PIN1 22
#define ENCODER1_INT1 digitalPinToInterrupt(ENCODER1_PIN1)
#define ENCODER1_PIN2 23
#define ENCODER1_INT2 digitalPinToInterrupt(ENCODER1_PIN2)
EncoderStepCounter encoder1(ENCODER1_PIN1, ENCODER1_PIN2, HALF_STEP);
#define BUTTON_ONE_PIN 6
EasyButton button1(BUTTON_ONE_PIN);

//Encoder 2
#define ENCODER2_PIN1 14
#define ENCODER2_INT1 digitalPinToInterrupt(ENCODER2_PIN1)
#define ENCODER2_PIN2 13
#define ENCODER2_INT2 digitalPinToInterrupt(ENCODER2_PIN2)
EncoderStepCounter encoder2(ENCODER2_PIN1, ENCODER2_PIN2, HALF_STEP);
#define BUTTON_TWO_PIN 5
EasyButton button2(BUTTON_TWO_PIN);
#define ROTARY_DELAY_MS 10

//Keypad SETUP
const byte ROWS = 3;
const byte COLS = 4;

byte buttons[ROWS][COLS] = {
  {0,1,2,3},
  {4,5,6,7},
  {8,9,10,11}
};

byte rowPins[ROWS] = {13,14,15};
byte colPins[COLS] = {0,1,2,3};

Keypad keypad = Keypad( makeKeymap(buttons), rowPins, colPins, ROWS, COLS );

bool rotary1Pressed = false;
bool rotary2Pressed = false;

unsigned long rotary1Delay = millis();
unsigned long rotary2Delay = millis();

bool rotaryButton1Pressed = false;
bool rotaryButton2Pressed = false;

signed long position1 = 0;
signed long position2 = 0;

void setup() {
  //Serial.begin(9600);
  // Initialize encoder
  encoder1.begin();
  encoder2.begin();
  // Initialize interrupts
  attachInterrupt(ENCODER1_INT1, interrupt1, CHANGE);
  attachInterrupt(ENCODER1_INT2, interrupt1, CHANGE);
  attachInterrupt(ENCODER2_INT1, interrupt1, CHANGE);
  attachInterrupt(ENCODER2_INT2, interrupt1, CHANGE);

  button1.begin();
  button2.begin();
}

// Call tick on every change interrupt
void interrupt1() {
  encoder1.tick();
}

void interrupt2() {
  encoder2.tick();
}

void loop() {
  checkAllButtons();
  checkRotaries();
  checkRotaryButtons();
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
  encoder1.reset();
  encoder2.reset();
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
}

void checkRotaryButtons()
{
  if (button1.read() && !rotaryButton1Pressed) {
    Joystick.button(16, 1);
    rotaryButton1Pressed = true;
  } else if (!button1.read() && rotaryButton1Pressed) {
    Joystick.button(16, 0);
    rotaryButton1Pressed = false;
  }
  
  if (button2.read() && !rotaryButton2Pressed) {
    Joystick.button(17, 1);
    rotaryButton2Pressed = true;
  } else if (!button2.read() && rotaryButton2Pressed) {
    Joystick.button(17, 0);
    rotaryButton2Pressed = false;
  }  
}
