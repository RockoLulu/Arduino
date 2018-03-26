#include <Eventually.h>
/*
    This is a program to controll a four footswitch midi controller with two red and two green leds
*/
#define LED1_PIN 2 // First red led
#define LED2_PIN 3 // First green led

#define BUTTON1_PIN 8 // First button
#define BUTTON2_PIN 9 // Second button

EvtManager mgr;

bool led1State = LOW; // led is on or off
bool blink1Type = LOW; // slow blink or flash

bool led2State = LOW;
bool blink2Type = LOW;

// states
bool rec = LOW;
bool play = LOW;
bool ovdb = LOW;
bool stop = LOW;

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, (EvtAction)buttonListener1));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, (EvtAction)buttonListener2));
  //  Set MIDI baud rate:
  Serial.begin(31250);
  return true;
}

bool buttonListener1() {
  mgr.resetContext();
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, (EvtAction)buttonListener1));
  // Send midi note here
  sendNote(0x90, 0x30, 0x45); // on
  sendNote(0x90, 0x30, 0x45); // off
  // statemachine (recording -> playing <-> overdubbing)
  if (rec == LOW && play == LOW && ovdb == LOW) {            // recording
    rec = HIGH;
    //play = LOW;
    //ovdb = LOW;
    stop = LOW;
  } else if (rec == LOW && play == HIGH && ovdb == LOW) {    // overdubbing
    //rec = LOW;
    play = LOW;
    ovdb = HIGH;
  } else if (rec == LOW && play == LOW && ovdb == HIGH) {    // playing after overdubbing
    // rec = LOW;
    play = HIGH;
    ovdb = LOW;
  } else {                                                   // playing after recording
    rec = LOW;
    play = HIGH;
    ovdb = LOW;
  }
  mgr.addListener(new EvtTimeListener(250, true, (EvtAction)blinkLED));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, (EvtAction)buttonListener2));
  return true;
}

bool buttonListener2() {
  mgr.resetContext();
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, (EvtAction)buttonListener2));
  // Send midi note here
  sendNote(0x90, 0x30, 0x45); // on
  sendNote(0x90, 0x30, 0x45); // off
  // Reset states
  rec = LOW;
  play = LOW;
  ovdb = LOW;
  blinkLED();
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, (EvtAction)buttonListener1));
  return true;
}

bool blinkLED() {
  // red led
  if (rec == HIGH || ovdb == HIGH) {
    led1State = !led1State;
    digitalWrite(LED1_PIN, led1State);
  } else {
    led1State = LOW;
    digitalWrite(LED1_PIN, led1State);
  }
  // green led
  if (play == HIGH || ovdb == HIGH) {
    led2State = !led2State;
    digitalWrite(LED2_PIN, led2State);
  } else {
    led2State = LOW;
    digitalWrite(LED2_PIN, led2State);
  }
  return false;
}

//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 127, or that data values are less than 127:
void sendNote(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

USE_EVENTUALLY_LOOP(mgr)
