#include <Eventually.h>
/*
    This is a program to controll a four footswitch midi controller with two red and two green leds
*/
#define LED1_PIN       2 // First red led
#define LED2_PIN       3 // First green led

#define BUTTON1_PIN    8 // First button
#define BUTTON2_PIN    9 // Second button

#define DEBOUNCE_SHORT 40
#define DEBOUNCE_LONG  1000

#define NOTE_OFF       0x80
#define NOTE_ON        0x90
#define CHANNEL        11

#define NOTE1          0x30 // C2
#define NOTE2          0x3C // C3
#define NOTE3          0x48 // C4
#define NOTE4          0x54 // C5

#define VELOCITY       0x7F

EvtManager mgr;

bool led1State  =      LOW; // led is on or off
bool blink1Type =      LOW; // slow blink or flash

bool led2State  =      LOW;
bool blink2Type =      LOW;

// states
bool rec        =      LOW;
bool play       =      LOW;
bool ovdb       =      LOW;
bool undo       =      LOW;
bool stop       =      LOW;
bool erase      =      LOW;

void setup() {
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)button1Listener1));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)button2Listener1));
  //  Set MIDI baud rate:
  Serial.begin(31250);
  return true;
}
// Rec / Play / Ovdb
bool button1Listener1() {
  mgr.resetContext();
  // Send midi note here
  sendMidi(NOTE_ON, NOTE1, VELOCITY); // on
  sendMidi(NOTE_OFF, NOTE1, VELOCITY); // off
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
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_LONG, (EvtAction)button1Listener2));
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)button1Listener1));
  
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)button2Listener1));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_LONG, (EvtAction)button2Listener2));
  return true;
}
// Stop
bool button2Listener1() {
  mgr.resetContext();
  // Send midi note here
  sendMidi(NOTE_ON, NOTE2, VELOCITY); // on
  sendMidi(NOTE_OFF, NOTE2, VELOCITY); // off
  // Reset states
  rec = LOW;
  play = LOW;
  ovdb = LOW;
  blinkLED();
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)button1Listener1));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)button2Listener1));
  return true;
}
// Undo
bool button1Listener2() {
  mgr.resetContext();
  // Send midi note here
  sendMidi(NOTE_ON, NOTE3, VELOCITY); // on
  sendMidi(NOTE_OFF, NOTE3, VELOCITY); // off
  // Set states
  rec = LOW;
  play = HIGH;
  ovdb = LOW;
  blinkLED();
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)button1Listener1));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)button2Listener1));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_LONG, (EvtAction)button2Listener2));
  return true;
}
// Erase
bool button2Listener2() {
  mgr.resetContext();
  // Send midi note here
  sendMidi(NOTE_ON, NOTE4, VELOCITY); // on
  sendMidi(NOTE_OFF, NOTE4, VELOCITY); // off
  // Set states
  rec = LOW;
  play = LOW;
  ovdb = LOW;
  blinkLED();
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)button1Listener1));
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_LONG, (EvtAction)button1Listener2));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)button2Listener1));
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
void sendMidi(int cmd, int pitch, int velocity) {
  Serial.write(cmd | CHANNEL - 1);
  Serial.write(pitch);
  Serial.write(velocity);
}

USE_EVENTUALLY_LOOP(mgr)