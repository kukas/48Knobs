#define OUT_MIDI

#ifdef OUT_MIDI
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

#include "button.cpp"

// pin definitions
const int led1 = 8;
const int led2 = 9;

Button button1(12);
Button button2(11);
Button button3(10);

// select pins
const int S0 = 2;
const int S1 = 3;
const int S2 = 4;

// column number mapping
const int knobCols[8] = {5,7,3,1,2,4,0,6};
// analog output pins
const int knobPins[6] = {A6, A7, A0, A1, A2, A3};

const int avg = 4;
int knobs[6][8][avg];
float lastSent[6][8];
int pAvg = 0;

int ccStartDefault = 32;
int ccStart = ccStartDefault;
bool midiDumpFlag = false;

// Selects a column using column mapping
// currently the mapping is set so that
// 0 = leftmost column
// 7 = rightmost column
int setColumn (const byte which) {
  byte col = knobCols[which];
  digitalWrite (S0, (col & 1) ? HIGH : LOW);  // low-order bit
  digitalWrite (S1, (col & 2) ? HIGH : LOW);
  digitalWrite (S2, (col & 4) ? HIGH : LOW);  // high-order bit
}

// Reads a potentiometer
// 0 = top potentiometer
// 5 = bottom potentiometer
// returns a number 0-1023
int readPotentiometer(const byte which) {
  return 1023 - analogRead(knobPins[which]);
}

// button events
void ccStartLED(){
  if(ccStart < ccStartDefault){
    digitalWrite(led1, HIGH);
    digitalWrite(led2, LOW);
  }
  else if(ccStart > ccStartDefault){
    digitalWrite(led1, LOW);
    digitalWrite(led2, HIGH);
  }
  else {
    digitalWrite(led1, LOW);
    digitalWrite(led2, LOW);
  }
}
void incCCStart(){ ccStart += 48;ccStartLED(); }
void decCCStart(){ ccStart -= 48;ccStartLED(); }
void midiDump(){ midiDumpFlag = true; }

void setup() {
  #ifdef OUT_MIDI
  MIDI.begin(1);
  #else
  Serial.begin(9600);
  #endif

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);

  button1.onDown = &midiDump;
  button2.onDown = &decCCStart;
  button3.onDown = &incCCStart;
}

void loop() {
  button1.update();
  button2.update();
  button3.update();
  
  digitalWrite(LED_BUILTIN, LOW);
  
  for (byte col = 0; col < 8; col++) {
    setColumn(col);
    for (byte row = 0; row < 6; row++){
      int knob = readPotentiometer(row);

      int sum = 0;
      for(byte i = 0; i < avg; i++){
        sum += knobs[row][col][i];
      }

      float avgKnob = sum/(float)avg;

      if(abs(avgKnob-lastSent[row][col]) > 1 || midiDumpFlag){
        lastSent[row][col] = avgKnob;
        int intAvgKnob = ((int)round(avgKnob))>>3;
        #ifdef OUT_MIDI
        MIDI.sendControlChange(ccStart+row*8+col, intAvgKnob, 1);
        #else
        Serial.print(col);
        Serial.print("x");
        Serial.print(row);
        Serial.print("=");
        Serial.println(avgKnob);
        #endif
        digitalWrite(LED_BUILTIN, HIGH);
      }

      knobs[row][col][pAvg] = knob;
    }
  }
  pAvg = (pAvg+1)%avg;
  midiDumpFlag = false;
  delay(10);
}

