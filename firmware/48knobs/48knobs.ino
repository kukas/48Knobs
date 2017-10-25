#define OUT_MIDI

#ifdef OUT_MIDI
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();
#endif

// pin definitions

// select pins
const int S0 = 2;
const int S1 = 3;
const int S2 = 4;

// column number mapping
const int knobCols[8] = {5,7,3,1,2,4,0,6};
// analog output pins
const int knobPins[6] = {A4, A5, A0, A1, A2, A3};

const int avg = 4;
int knobs[6][8][avg];
float lastSent[6][8];
int pAvg = 0;

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

void setup() {
  #ifdef OUT_MIDI
  MIDI.begin(1);
  #else
  Serial.begin(9600);
  #endif

  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);

  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
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

      if(abs(avgKnob-lastSent[row][col]) > 1){
        lastSent[row][col] = avgKnob;
        int intAvgKnob = ((int)round(avgKnob))>>3;
        #ifdef OUT_MIDI
        MIDI.sendControlChange(75+row*8+col, intAvgKnob, 1);
        #else
        Serial.println(avgKnob);
        #endif
        digitalWrite(LED_BUILTIN, HIGH);
      }

      knobs[row][col][pAvg] = knob;
    }
  }
  pAvg = (pAvg+1)%avg;
  delay(10);
}
