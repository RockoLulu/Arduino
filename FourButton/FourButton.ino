#include <Eventually.h>
/*
    This is a program to controll a four footswitch midi controller with two red and two green leds
*/
// Pins
#define LED1_PIN 2 // First red led
#define LED2_PIN 3 // First green led
#define BUTTON1_PIN 8 // First button
#define BUTTON2_PIN 9 // Second button
// Midi
#define NOTE_OFF 0x80
#define NOTE_ON 0x90
#define CHANNEL 11
#define VELOCITY 0x7F // 127 full midi velocity
// Loop1
#define NOTE1 0x30 // C2
#define NOTE2 0x3C // C3
#define NOTE3 0x48 // C4
#define NOTE4 0x54 // C5
// Loop2
#define NOTE5 0x26 // D2
#define NOTE6 0x32 // D3
#define NOTE7 0x3E // D4
#define NOTE8 0x4A // D5
// Blink and debounce
#define BLINK_SLOW 250 // slow blink
#define BLINK_FAST 100 // flash
#define DEBOUNCE_SHORT 40
#define DEBOUNCE_LONG 1000
#define DEBOUNCE 500
// States
bool led1State = LOW; // led is on or off
bool led2State = LOW;
bool pin1State = LOW;
bool rec = LOW;
bool play = LOW;
bool ovdb = LOW;

EvtManager mgr;

void setup()
{
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_LONG, (EvtAction)erase1Listener));
  //  Set MIDI baud rate:
  Serial.begin(31250);
  return true;
}
// Rec / Play / Ovdb
bool rec1Listener()
{
  mgr.resetContext();
  digitalWrite(LED1_PIN, LOW);
  sendMidi(NOTE_ON, NOTE1, VELOCITY); // on
  delay(DEBOUNCE); // Wait for the button to change state
  pin1State = digitalRead(BUTTON1_PIN);
  if (pin1State == LOW) // Recording
  {
    sendMidi(NOTE_OFF, NOTE1, VELOCITY); // off
    // statemachine (recording -> playing <-> overdubbing)
    if (rec == LOW && play == LOW && ovdb == LOW)
    { // recording
      rec = HIGH;
      //play = LOW;
      //ovdb = LOW;
    }
    else if (rec == LOW && play == HIGH && ovdb == LOW)
    { // overdubbing
      //rec = LOW;
      play = LOW;
      ovdb = HIGH;
    }
    else if (rec == LOW && play == LOW && ovdb == HIGH)
    { // playing after overdubbing
      // rec = LOW;
      play = HIGH;
      ovdb = LOW;
    }
    else
    { // playing after recording
      rec = LOW;
      play = HIGH;
      ovdb = LOW;
    }
    mgr.addListener(new EvtTimeListener(BLINK_SLOW, true, (EvtAction)blinkLED));
  }
  else
  { // Wait with sending the off note to trigger the ableton 2 sec undo/redo
    delay(1600);
    sendMidi(NOTE_OFF, NOTE1, VELOCITY); // off
    /* sendMidi(NOTE_ON, NOTE3, VELOCITY);  // on
    sendMidi(NOTE_OFF, NOTE3, VELOCITY); // off */
    // LED 1 flash
    digitalWrite(LED1_PIN, LOW);
    delay(BLINK_FAST);
    digitalWrite(LED1_PIN, HIGH);
    delay(BLINK_FAST);
    digitalWrite(LED1_PIN, LOW);
    // LED 2 on
    digitalWrite(LED2_PIN, HIGH);
    rec = LOW;
    play = HIGH;
    ovdb = LOW;
  }
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)stop1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_LONG, false, (EvtAction)erase1Listener));
  return false;
}
// Stop
bool stop1Listener()
{
  mgr.resetContext();
  sendMidi(NOTE_ON, NOTE2, VELOCITY);  // on
  sendMidi(NOTE_OFF, NOTE2, VELOCITY); // off
  // Reset states
  play = LOW;
  ovdb = LOW;
  rec = HIGH;
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)stop1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_LONG, false, (EvtAction)erase1Listener));
  return true;
}
// Erase
bool erase1Listener()
{
  mgr.resetContext();
  sendMidi(NOTE_ON, NOTE4, VELOCITY);  // on
  sendMidi(NOTE_OFF, NOTE4, VELOCITY); // off
  // Set states
  rec = LOW;
  play = LOW;
  ovdb = LOW;
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)stop1Listener));
  return false;
}

bool blinkLED()
{
  // red led
  if (rec == HIGH || ovdb == HIGH)
  {
    led1State = !led1State;
    digitalWrite(LED1_PIN, led1State);
  }
  else
  {
    led1State = LOW;
    digitalWrite(LED1_PIN, led1State);
  }
  // green led
  if (play == HIGH || ovdb == HIGH)
  {
    led2State = HIGH;
    digitalWrite(LED2_PIN, led2State);
  }
  else
  {
    led2State = LOW;
    digitalWrite(LED2_PIN, led2State);
  }
  return false;
}

//  plays a MIDI note.  Doesn't check to see that
//  cmd is greater than 127, or that data values are less than 127:
void sendMidi(int cmd, int pitch, int velocity)
{
  Serial.write(cmd | CHANNEL - 1);
  Serial.write(pitch);
  Serial.write(velocity);
}

USE_EVENTUALLY_LOOP(mgr)