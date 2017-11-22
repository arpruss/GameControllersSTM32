#include "GameControllers.h"

const uint32_t R2 = 10000;
const uint32_t R1max = 100000;
const uint32_t maximumAnalogValue = 4095;
const uint32_t minimumAnalogValue = 4095 * 10 / 110;

GamePortController::GamePortController(unsigned axis1, unsigned axis2, unsigned axis3, unsigned axis4,
                unsigned button1, unsigned button2, unsigned button3, unsigned button4) {
    xPin = axis1;
    yPin = axis2;
    sliderPin = axis3;
    rzPin = axis4;
    haveAxes = axis1 != INPUT_NOT_IMPLEMENTED || axis2 != INPUT_NOT_IMPLEMENTED || axis3 != INPUT_NOT_IMPLEMENTED || axis4 != INPUT_NOT_IMPLEMENTED;

    buttonPins[0]=button1;
    buttonPins[1]=button2;
    buttonPins[2]=button3;
    buttonPins[3]=button4;
}

int16_t GamePortController::getValue(unsigned pin) {
    if (pin == INPUT_NOT_IMPLEMENTED)
        return -1;
    uint32_t v = analogRead(pin) & ~3;
    if (v < minimumAnalogValue/2)
      return -1;
    else { 
      uint32_t R = R2*4095/v-R2;
      if (R > R1max)
        return 1023;
      else
        return R*1023/R1max;
    }
}

bool GamePortController::begin() {
    for (unsigned i=0; i<4; i++) 
        if (INPUT_NOT_IMPLEMENTED != buttonPins[i]) {
            pinMode(buttonPins[i], INPUT_PULLUP);
            debouncers[i] = new Debouncer(buttonPins[i], LOW);
        }
        else {
            debouncers[i] = NULL;
        }
        
    if (xPin != INPUT_NOT_IMPLEMENTED)
        pinMode(xPin, INPUT_ANALOG);
    if (yPin != INPUT_NOT_IMPLEMENTED)
        pinMode(yPin, INPUT_ANALOG);
    if (sliderPin != INPUT_NOT_IMPLEMENTED)
        pinMode(sliderPin, INPUT_ANALOG);
    if (sliderPin != INPUT_NOT_IMPLEMENTED)
        pinMode(rzPin, INPUT_ANALOG);
   
    return true;
}

bool GamePortController::read(GameControllerData_t* data) {
    if (haveAxes) {
        int16_t x = getValue(xPin);
        int16_t y = getValue(yPin);
        int16_t s = getValue(sliderPin);
        int16_t rz = getValue(rzPin);
        if (x < 0 && y < 0 && s < 0 && rz < 0) 
            return false;
        memset(data, 0, sizeof(GameControllerData_t));
        data->device = CONTROLLER_GAMEPORT;
        data->joystickX = x >= 0 ? x : 512;
        data->joystickY = y >= 0 ? y : 512;
        data->cX = rz >= 0 ? rz : 512;
        data->shoulderRight = s >= 0 ? 1023-s : 0;
    }
    else {
        memset(data, 0, sizeof(GameControllerData_t));
    }
    uint8_t mask = 1;
    for (unsigned i = 0; i < 4 ; i++, mask <<= 1) 
        if (debouncers[i] != NULL && debouncers[i]->getState())
            data->buttons |= mask;            
    return true;
}
