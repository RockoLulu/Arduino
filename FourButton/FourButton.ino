#include <Eventually.h>
/*
    This is a program to controll a four footswitch midi controller with two red and two green leds
*/
// Pins
#define LED1_PIN 2 // First red led
#define LED2_PIN 3 // First green led
#define BUTTON1_PIN 8 // First button
#define BUTTON2_PIN 9 // Second button

#define LED3_PIN 5 // Second red led
#define LED4_PIN 6 // Second green led
#define BUTTON3_PIN 11 // Third button
#define BUTTON4_PIN 12 // Fourth button
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
#define NOTE5 0x32 // D2
#define NOTE6 0x3E // D3
#define NOTE7 0x4A // D4
#define NOTE8 0x56 // D5
// Blink and debounce
#define BLINK_SLOW 250 // slow blink
#define BLINK_FAST 100 // flash
#define DEBOUNCE_SHORT 40
#define DEBOUNCE_LONG 1000
#define DEBOUNCE 500
// States
bool pin1State = LOW;
bool rec1 = LOW;
bool play1 = LOW;
bool ovdb1 = LOW;
bool pin3State = LOW;
bool rec2 = LOW;
bool play2 = LOW;
bool ovdb2 = LOW;

EvtManager mgr;

void setup()
{
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(BUTTON3_PIN, INPUT);
  pinMode(BUTTON4_PIN, INPUT);
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_LONG, (EvtAction)erase1Listener));
  mgr.addListener(new EvtPinListener(BUTTON3_PIN, DEBOUNCE_SHORT, (EvtAction)rec2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_LONG, (EvtAction)erase2Listener));
  //  Set MIDI baud rate:
  Serial.begin(31250); // Midi connector
  //Serial.begin(9600); // USB Midi connector
  return true;
}
// Loop1
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
    if (rec1 == LOW && play1 == LOW && ovdb1 == LOW)
    { // recording
      rec1 = HIGH;
      //play1 = LOW;
      //ovdb1 = LOW;
    }
    else if (rec1 == LOW && play1 == HIGH && ovdb1 == LOW)
    { // overdubbing
      //rec1 = LOW;
      play1 = LOW;
      ovdb1 = HIGH;
    }
    else if (rec1 == LOW && play1 == LOW && ovdb1 == HIGH)
    { // playing after overdubbing
      // rec1 = LOW;
      play1 = HIGH;
      ovdb1 = LOW;
    }
    else
    { // playing after recording
      rec1 = LOW;
      play1 = HIGH;
      ovdb1 = LOW;
    }
    blinkLED1();
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
    rec1 = LOW;
    play1 = HIGH;
    ovdb1 = LOW;
  }
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)stop1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_LONG, false, (EvtAction)erase1Listener));
  mgr.addListener(new EvtPinListener(BUTTON3_PIN, DEBOUNCE_SHORT, (EvtAction)rec2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_SHORT, (EvtAction)stop2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_LONG, false, (EvtAction)erase2Listener));
  return false;
}
// Stop
bool stop1Listener()
{
  mgr.resetContext();
  sendMidi(NOTE_ON, NOTE2, VELOCITY);  // on
  sendMidi(NOTE_OFF, NOTE2, VELOCITY); // off
  // Reset states
  play1 = LOW;
  ovdb1 = LOW;
  rec1 = HIGH;
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)stop1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_LONG, false, (EvtAction)erase1Listener));
  mgr.addListener(new EvtPinListener(BUTTON3_PIN, DEBOUNCE_SHORT, (EvtAction)rec2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_SHORT, (EvtAction)stop2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_LONG, false, (EvtAction)erase2Listener));
  return true;
}
// Erase
bool erase1Listener()
{
  mgr.resetContext();
  sendMidi(NOTE_ON, NOTE4, VELOCITY);  // on
  sendMidi(NOTE_OFF, NOTE4, VELOCITY); // off
  // Set states
  rec1 = LOW;
  play1 = LOW;
  ovdb1 = LOW;
  digitalWrite(LED1_PIN, LOW);
  digitalWrite(LED2_PIN, LOW);
  delay(BLINK_FAST);
  digitalWrite(LED2_PIN, HIGH);
  delay(BLINK_FAST);
  digitalWrite(LED2_PIN, LOW);
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)stop1Listener));
  mgr.addListener(new EvtPinListener(BUTTON3_PIN, DEBOUNCE_SHORT, (EvtAction)rec2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_SHORT, (EvtAction)stop2Listener));
  return false;
}

bool blinkLED1()
{
  // red led
  if (rec1 == HIGH || ovdb1 == HIGH)
  {
    digitalWrite(LED1_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED1_PIN, LOW);
  }
  // green led
  if (play1 == HIGH || ovdb1 == HIGH)
  {
    digitalWrite(LED2_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED2_PIN, LOW);
  }
  return false;
}

// Loop2
// Rec / Play / Ovdb
bool rec2Listener()
{
  mgr.resetContext();
  digitalWrite(LED3_PIN, LOW);
  sendMidi(NOTE_ON, NOTE5, VELOCITY); // on
  delay(DEBOUNCE); // Wait for the button to change state
  pin3State = digitalRead(BUTTON3_PIN);
  if (pin3State == LOW) // Recording
  {
    sendMidi(NOTE_OFF, NOTE5, VELOCITY); // off
    // statemachine (recording -> playing <-> overdubbing)
    if (rec2 == LOW && play2 == LOW && ovdb2 == LOW)
    { // recording
      rec2 = HIGH;
      //play2 = LOW;
      //ovdb2 = LOW;
    }
    else if (rec2 == LOW && play2 == HIGH && ovdb2 == LOW)
    { // overdubbing
      //rec2 = LOW;
      play2 = LOW;
      ovdb2 = HIGH;
    }
    else if (rec2 == LOW && play2 == LOW && ovdb2 == HIGH)
    { // playing after overdubbing
      // rec2 = LOW;
      play2 = HIGH;
      ovdb2 = LOW;
    }
    else
    { // playing after recording
      rec2 = LOW;
      play2 = HIGH;
      ovdb2 = LOW;
    }
    blinkLED2();
  }
  else
  { // Wait with sending the off note to trigger the ableton 2 sec undo/redo
    delay(1600);
    sendMidi(NOTE_OFF, NOTE5, VELOCITY); // off
    /* sendMidi(NOTE_ON, NOTE3, VELOCITY);  // on
    sendMidi(NOTE_OFF, NOTE3, VELOCITY); // off */
    // LED 1 flash
    digitalWrite(LED3_PIN, LOW);
    delay(BLINK_FAST);
    digitalWrite(LED3_PIN, HIGH);
    delay(BLINK_FAST);
    digitalWrite(LED3_PIN, LOW);
    // LED 2 on
    digitalWrite(LED4_PIN, HIGH);
    rec2 = LOW;
    play2 = HIGH;
    ovdb2 = LOW;
  }
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)stop1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_LONG, false, (EvtAction)erase1Listener));
  mgr.addListener(new EvtPinListener(BUTTON3_PIN, DEBOUNCE_SHORT, (EvtAction)rec2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_SHORT, (EvtAction)stop2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_LONG, false, (EvtAction)erase2Listener));
  return false;
}
// Stop
bool stop2Listener()
{
  mgr.resetContext();
  sendMidi(NOTE_ON, NOTE6, VELOCITY);  // on
  sendMidi(NOTE_OFF, NOTE6, VELOCITY); // off
  // Reset states
  play2 = LOW;
  ovdb2 = LOW;
  rec2 = HIGH;
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)stop1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_LONG, false, (EvtAction)erase1Listener));
  mgr.addListener(new EvtPinListener(BUTTON3_PIN, DEBOUNCE_SHORT, (EvtAction)rec2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_SHORT, (EvtAction)stop2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_LONG, false, (EvtAction)erase2Listener));
  return true;
}
// Erase
bool erase2Listener()
{
  mgr.resetContext();
  sendMidi(NOTE_ON, NOTE8, VELOCITY);  // on
  sendMidi(NOTE_OFF, NOTE8, VELOCITY); // off
  // Set states
  rec2 = LOW;
  play2 = LOW;
  ovdb2 = LOW;
  digitalWrite(LED3_PIN, LOW);
  digitalWrite(LED4_PIN, LOW);
  delay(BLINK_FAST);
  digitalWrite(LED4_PIN, HIGH);
  delay(BLINK_FAST);
  digitalWrite(LED4_PIN, LOW);
  mgr.addListener(new EvtPinListener(BUTTON1_PIN, DEBOUNCE_SHORT, (EvtAction)rec1Listener));
  mgr.addListener(new EvtPinListener(BUTTON2_PIN, DEBOUNCE_SHORT, (EvtAction)stop1Listener));
  mgr.addListener(new EvtPinListener(BUTTON3_PIN, DEBOUNCE_SHORT, (EvtAction)rec2Listener));
  mgr.addListener(new EvtPinListener(BUTTON4_PIN, DEBOUNCE_SHORT, (EvtAction)stop2Listener));
  return false;
}

bool blinkLED2()
{
  // red led
  if (rec2 == HIGH || ovdb2 == HIGH)
  {
    digitalWrite(LED3_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED3_PIN, LOW);
  }
  // green led
  if (play2 == HIGH || ovdb2 == HIGH)
  {
    digitalWrite(LED4_PIN, HIGH);
  }
  else
  {
    digitalWrite(LED4_PIN, LOW);
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