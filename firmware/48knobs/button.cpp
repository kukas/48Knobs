#include "Arduino.h"

// handles button events
class Button {
  public:
    Button(int pin){
      pin_ = pin;
      pinMode(pin, INPUT);
    }
    void (*onDown)(void);
    void update(){
      state_ = digitalRead(pin_);
      if(lastState_ != state_){
        if(state_ == HIGH){
          onDown();
        }
      }

      lastState_ = state_;
    }
  private:
    int state_;
    int lastState_;
    int pin_;
};
