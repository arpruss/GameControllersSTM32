#ifndef _DEBOUNCER_H
#define _DEBOUNCER_H

#define STM32_SPECIFIC

class Debouncer {
  protected:
    uint32_t lastToggleTime;
    uint8_t activeValue;
    bool curState;
    bool highState;
    bool lowState;
    uint32_t debounceTime;
    bool releaseCanceled = false;
#ifdef STM32_SPECIFIC
    volatile uint32_t* port;
    uint32_t mask;
#endif
    int pin;

    Debouncer() {}

  public:
    virtual inline bool getRawState(void) {
#ifdef STM32_SPECIFIC      
      if (*port & mask) 
        return highState;
      else
        return lowState;
#else            
      return activeValue==digitalRead(pin);      
#endif
    }

  private:
    bool update(void) {
      bool v = getRawState();
      uint32_t t = millis();
      if (v != curState && t - lastToggleTime >= debounceTime) {
        lastToggleTime = t;
        curState = v;
        return true;
      }
      return false;
    }

  public:
    Debouncer(int p, uint8_t active=HIGH, uint32_t time=20) {
      activeValue = active;
      debounceTime = time;
      highState = active==HIGH;
      lowState = active==LOW;
      pin = p;
#ifdef STM32_SPECIFIC
      port = &(PIN_MAP[p].gpio_device->regs->IDR);
      mask = 1u << PIN_MAP[p].gpio_bit;
#endif
    }

    void begin(void) {
      curState = getRawState();
      lastToggleTime = millis();
    }

    // Code using Debounce should choose exactly one of three polling methods:
    //   wasToggled(), getState() and wasPressed()
    // and should not switch between them, with the exception that a single initial call to
    // getState() is acceptable. 

    // for toggle monitoring
    inline bool wasToggled(void) {
      return update();
    }
    
    // for state monitoring
    bool getState(void) {
      update();
      return curState;
    }

    // for press monitoring
    bool wasPressed(void) {
      return wasToggled() && curState; // curState value is a side-effect of wasToggled()
    }

    void cancelRelease(void) {
      releaseCanceled = true;
    }

    // for release monitoring
    bool wasReleased(void) {
      if (wasToggled() && ! curState) { // curState value is a side-effect of wasToggled()        
        if (releaseCanceled) {
          releaseCanceled = false;
          return false;
        }
        else {
          return true;
        }
      }
      return false;
    }
};

class DebouncerAnalog : public Debouncer {
  private:
    uint16_t threshold;
    
  public:
    bool getRawState(void) {
      uint16_t value;
      // TODO: go analog
      if (analogRead(pin) >= threshold) 
        return highState;
      else
        return lowState; 
    }

    DebouncerAnalog(int p, uint8_t active=HIGH, uint16_t threshold=512, uint32_t time=20) : Debouncer() {
      activeValue = active;
      debounceTime = time;  
      highState = active==HIGH;
      lowState = active==LOW;
      pin = p;
    }
};
#endif

