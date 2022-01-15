#include <Keypad.h>
#include <Joystick.h>
#include "EncoderStepCounter.h"
#include <EasyButton.h>

//defines
#define ENABLE_PULLUPS
#define NUMBUTTONS 64
//Encoder 1
#define ENCODER1_PIN1 16
#define ENCODER1_INT1 digitalPinToInterrupt(ENCODER1_PIN1)
#define ENCODER1_PIN2 17
#define ENCODER1_INT2 digitalPinToInterrupt(ENCODER1_PIN2)
//Encoder 2
#define ENCODER2_PIN1 19
#define ENCODER2_INT1 digitalPinToInterrupt(ENCODER2_PIN1)
#define ENCODER2_PIN2 20
#define ENCODER2_INT2 digitalPinToInterrupt(ENCODER2_PIN2)
//Encoder 3
#define ENCODER3_PIN1 14
#define ENCODER3_INT1 digitalPinToInterrupt(ENCODER3_PIN1)
#define ENCODER3_PIN2 15
#define ENCODER3_INT2 digitalPinToInterrupt(ENCODER3_PIN2)
//Encoder 4
#define ENCODER4_PIN1 51
#define ENCODER4_INT1 digitalPinToInterrupt(ENCODER4_PIN1)
#define ENCODER4_PIN2 46
#define ENCODER4_INT2 digitalPinToInterrupt(ENCODER4_PIN2)
//Encoder 5
#define ENCODER5_PIN1 52
#define ENCODER5_INT1 digitalPinToInterrupt(ENCODER5_PIN1)
#define ENCODER5_PIN2 48
#define ENCODER5_INT2 digitalPinToInterrupt(ENCODER5_PIN2)
//Encoder buttons
#define BUTTON_ONE_PIN 21
#define BUTTON_TWO_PIN 18
#define BUTTON_THREE_PIN 47
#define BUTTON_FOUR_PIN 49
#define BUTTON_FIVE_PIN 50
//button press delay for rotaries
#define ROTARY_DELAY_MS 10

//Keypad SETUP
const byte ROWS = 8;
const byte COLS = 6;

byte buttons[ROWS][COLS] = {
  {0,1,2,3,4,5},
  {6,7,8,9,10,11},
  {12,13,14,15,16,17},
  {18,19,20,21,22,23},
  {24,25,26,27,28,29},
  {30,31,32,34,35,36},
  {37,38,39,40,41,42},
  {43,44,45,46,47,48}
};

byte rowPins[ROWS] = {4,5,6,7,11,8,9,10};
byte colPins[COLS] = {23,22,34,35,36,37};

Keypad keypad = Keypad( makeKeymap(buttons), rowPins, colPins, ROWS, COLS );

//Joystick SETUP
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID, 
  JOYSTICK_TYPE_JOYSTICK, NUMBUTTONS, 0,
  false, false, false, false, false, false,
  false, false, false, false, false);

//Rotary encoders Setup
EncoderStepCounter encoder1(ENCODER1_PIN1, ENCODER1_PIN2, HALF_STEP);
EncoderStepCounter encoder2(ENCODER2_PIN1, ENCODER2_PIN2, HALF_STEP);
EncoderStepCounter encoder3(ENCODER3_PIN1, ENCODER3_PIN2, HALF_STEP);
EncoderStepCounter encoder4(ENCODER4_PIN1, ENCODER4_PIN2, HALF_STEP);
EncoderStepCounter encoder5(ENCODER5_PIN1, ENCODER5_PIN2, HALF_STEP);

//pressed
bool rotary1Pressed = false;
bool rotary2Pressed = false;
bool rotary3Pressed = false;
bool rotary4Pressed = false;
bool rotary5Pressed = false;

//delays
unsigned long rotary1Delay = millis();
unsigned long rotary2Delay = millis();
unsigned long rotary3Delay = millis();
unsigned long rotary4Delay = millis();
unsigned long rotary5Delay = millis();

//Rotary encoder buttons
EasyButton button1(BUTTON_ONE_PIN);
EasyButton button2(BUTTON_TWO_PIN);
EasyButton button3(BUTTON_THREE_PIN);
EasyButton button4(BUTTON_FOUR_PIN);
EasyButton button5(BUTTON_FIVE_PIN);

//pressed
bool rotaryButton1Pressed = false;
bool rotaryButton2Pressed = false;
bool rotaryButton3Pressed = false;
bool rotaryButton4Pressed = false;
bool rotaryButton5Pressed = false;


signed long position1 = 0;
signed long position2 = 0;
signed long position3 = 0;
signed long position4 = 0;
signed long position5 = 0;

void setup() {
  //Encoders
  encoder1.begin();
  encoder2.begin();
  encoder3.begin();
  encoder4.begin();
  encoder5.begin();
  // Initialize interrupts
  attachInterrupt(ENCODER1_INT1, interrupt1, CHANGE);
  attachInterrupt(ENCODER1_INT2, interrupt1, CHANGE);
  attachInterrupt(ENCODER2_INT1, interrupt2, CHANGE);
  attachInterrupt(ENCODER2_INT2, interrupt2, CHANGE);
  attachInterrupt(ENCODER3_INT1, interrupt3, CHANGE);
  attachInterrupt(ENCODER3_INT2, interrupt3, CHANGE);
  attachInterrupt(ENCODER4_INT1, interrupt4, CHANGE);
  attachInterrupt(ENCODER4_INT2, interrupt4, CHANGE);
  attachInterrupt(ENCODER5_INT1, interrupt5, CHANGE);
  attachInterrupt(ENCODER5_INT2, interrupt5, CHANGE);
  //Encoder buttons
  button1.begin();
  button2.begin();
  button3.begin();
  button4.begin();
  button5.begin();
  //Joystick
  Joystick.begin();
}

void loop(){
  CheckAllButtons();
  checkRotaries();
  checkRotaryButtons();
}

void CheckAllButtons(void) {
  if (keypad.getKeys()) {
    for (int i=0; i<LIST_MAX; i++) {
      if (keypad.key[i].stateChanged) {
        switch (keypad.key[i].kstate) {  
          case PRESSED:
            Joystick.setButton(keypad.key[i].kchar, 1);
            break;
          case HOLD:
              break;
          case RELEASED:
          case IDLE:
            Joystick.setButton(keypad.key[i].kchar, 0);
            break;
        }
      }   
    }
  }
}

void interrupt1() {
  encoder1.tick();
}

void interrupt2() {
  encoder2.tick();
}

void interrupt3() {
  encoder3.tick();
}

void interrupt4() {
  encoder4.tick();
}

void interrupt5() {
  encoder5.tick();
}

void checkRotaries() {
  position1 += encoder1.getPosition();
  position2 += encoder2.getPosition();
  position3 += encoder3.getPosition();
  position4 += encoder4.getPosition();
  position5 += encoder5.getPosition();
  encoder1.reset();
  encoder2.reset();
  encoder3.reset();
  encoder4.reset();
  encoder5.reset();
  if (position1 < 0) {
    if (rotary1Pressed && (millis() - rotary1Delay >= ROTARY_DELAY_MS)) {
      Joystick.setButton(49, 0);
      rotary1Pressed = false;
      position1++;
      rotary1Delay = millis();
    } else if (!rotary1Pressed && (millis() - rotary1Delay >= ROTARY_DELAY_MS)) {
     Joystick.setButton(49, 1);
     rotary1Pressed = true;
     rotary1Delay = millis();
    }
  } else if (position1 > 0) {
    if (rotary1Pressed && (millis() - rotary1Delay >= ROTARY_DELAY_MS)) {
      Joystick.setButton(50, 0);
      rotary1Pressed = false;
      position1--;
      rotary1Delay = millis();
    } else if (!rotary1Pressed && (millis() - rotary1Delay >= ROTARY_DELAY_MS)) {
     Joystick.setButton(50, 1);
     rotary1Pressed = true;
     rotary1Delay = millis();
    }
  }
  
  if (position2 < 0) {
    if (rotary2Pressed && (millis() - rotary2Delay >= ROTARY_DELAY_MS)) {
      Joystick.setButton(51, 0);
      rotary2Pressed = false;
      position2++;
      rotary2Delay = millis();
    } else if (!rotary2Pressed && (millis() - rotary2Delay >= ROTARY_DELAY_MS)) {
     Joystick.setButton(51, 1);
     rotary2Pressed = true;
     rotary2Delay = millis();
    }
  } else if (position2 > 0) {
    if (rotary2Pressed && (millis() - rotary2Delay >= ROTARY_DELAY_MS)) {
      Joystick.setButton(52, 0);
      rotary2Pressed = false;
      position2--;
      rotary2Delay = millis();
    } else if (!rotary2Pressed && (millis() - rotary2Delay >= ROTARY_DELAY_MS)) {
     Joystick.setButton(52, 1);
     rotary2Pressed = true;
     rotary2Delay = millis();
    }
  }
  
  if (position3 < 0) {
    if (rotary3Pressed && (millis() - rotary3Delay >= ROTARY_DELAY_MS)) {
      Joystick.setButton(53, 0);
      rotary3Pressed = false;
      position3++;
      rotary3Delay = millis();
    } else if (!rotary3Pressed && (millis() - rotary3Delay >= ROTARY_DELAY_MS)) {
     Joystick.setButton(53, 1);
     rotary3Pressed = true;
     rotary3Delay = millis();
    }
  } else if (position3 > 0) {
    if (rotary3Pressed && (millis() - rotary3Delay >= ROTARY_DELAY_MS)) {
      Joystick.setButton(54, 0);
      rotary3Pressed = false;
      position3--;
      rotary3Delay = millis();
    } else if (!rotary3Pressed && (millis() - rotary3Delay >= ROTARY_DELAY_MS)) {
     Joystick.setButton(54, 1);
     rotary3Pressed = true;
     rotary3Delay = millis();
    }
  }
  
  if (position4 < 0) {
    if (rotary4Pressed && (millis() - rotary4Delay >= ROTARY_DELAY_MS)) {
      Joystick.setButton(55, 0);
      rotary4Pressed = false;
      position4++;
      rotary4Delay = millis();
    } else if (!rotary4Pressed && (millis() - rotary4Delay >= ROTARY_DELAY_MS)) {
     Joystick.setButton(55, 1);
     rotary4Pressed = true;
     rotary4Delay = millis();
    }
  } else if (position4 > 0) {
    if (rotary4Pressed && (millis() - rotary4Delay >= ROTARY_DELAY_MS)) {
      Joystick.setButton(56, 0);
      rotary4Pressed = false;
      position4--;
      rotary4Delay = millis();
    } else if (!rotary4Pressed && (millis() - rotary4Delay >= ROTARY_DELAY_MS)) {
     Joystick.setButton(56, 1);
     rotary4Pressed = true;
     rotary4Delay = millis();
    }
  }
  
  if (position5 < 0) {
    if (rotary5Pressed && (millis() - rotary5Delay >= ROTARY_DELAY_MS)) {
      Joystick.setButton(57, 0);
      rotary5Pressed = false;
      position5++;
      rotary5Delay = millis();
    } else if (!rotary5Pressed && (millis() - rotary5Delay >= ROTARY_DELAY_MS)) {
     Joystick.setButton(57, 1);
     rotary5Pressed = true;
     rotary5Delay = millis();
    }
  } else if (position5 > 0) {
    if (rotary5Pressed && (millis() - rotary5Delay >= ROTARY_DELAY_MS)) {
      Joystick.setButton(58, 0);
      rotary5Pressed = false;
      position5--;
      rotary5Delay = millis();
    } else if (!rotary5Pressed && (millis() - rotary5Delay >= ROTARY_DELAY_MS)) {
     Joystick.setButton(58, 1);
     rotary5Pressed = true;
     rotary5Delay = millis();
    }
  }
}

void checkRotaryButtons()
{
  if (button1.read() && !rotaryButton1Pressed) {
    Joystick.setButton(59, 1);
    rotaryButton1Pressed = true;
  } else if (!button1.read() && rotaryButton1Pressed) {
    Joystick.setButton(59, 0);
    rotaryButton1Pressed = false;
  }
  
  if (button2.read() && !rotaryButton2Pressed) {
    Joystick.setButton(60, 1);
    rotaryButton2Pressed = true;
  } else if (!button2.read() && rotaryButton2Pressed) {
    Joystick.setButton(60, 0);
    rotaryButton2Pressed = false;
  }
  
  if (button3.read() && !rotaryButton3Pressed) {
    Joystick.setButton(61, 1);
    rotaryButton3Pressed = true;
  } else if (!button3.read() && rotaryButton3Pressed) {
    Joystick.setButton(61, 0);
    rotaryButton3Pressed = false;
  }
  
  if (button4.read() && !rotaryButton4Pressed) {
    Joystick.setButton(62, 1);
    rotaryButton4Pressed = true;
  } else if (!button4.read() && rotaryButton4Pressed) {
    Joystick.setButton(62, 0);
    rotaryButton4Pressed = false;
  }
  
  if (button5.read() && !rotaryButton5Pressed) {
    Joystick.setButton(63, 1);
    rotaryButton5Pressed = true;
  } else if (!button5.read() && rotaryButton5Pressed) {
    Joystick.setButton(63, 0);
    rotaryButton5Pressed = false;
  }  
}
