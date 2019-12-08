#include "GameControllers.h"
#include "dwt.h"

static const uint32_t genesisReadDelayMS = 5+1;

static const uint8_t maxFails = 4;
static const uint32_t cyclesPerUS = (SystemCoreClock / 1000000ul);
static const uint32_t quarterBitSendingCycles = cyclesPerUS * 5 / 4;
static const uint32_t bitReceiveCycles = cyclesPerUS * 4;
static const uint32_t halfBitReceiveCycles = cyclesPerUS * 2;

GenesisController::GenesisController(unsigned pin1, unsigned pin2, unsigned pin3, unsigned pin4, unsigned pin6, unsigned pin7, unsigned pin9) {
    sega = new SegaController(pin7, pin1, pin2, pin3, pin4, pin6, pin9);
}
    
bool GenesisController::begin() {
    return true;
}

bool GenesisController::read(GameControllerData_t* data) {
    word state = sega->getState();
    memset(data, 0, sizeof(*data));
    data->joystickX = 512;
    data->joystickY = 512;
    data->cX = 512;
    data->cY = 512;
    if (state & SC_CTL_ON) {
        data->buttons = ( (state & SC_BTN_A) ? segaMaskA : 0 ) |
                        ( (state & SC_BTN_B) ? segaMaskB : 0 ) |
                        ( (state & SC_BTN_C) ? segaMaskC : 0 ) |
                        ( (state & SC_BTN_X) ? segaMaskX : 0 ) |
                        ( (state & SC_BTN_Y) ? segaMaskY : 0 ) |
                        ( (state & SC_BTN_Z) ? segaMaskZ : 0 ) |
                        ( (state & SC_BTN_MODE) ? segaMaskMode : 0 ) |
                        ( (state & SC_BTN_START) ? segaMaskStart : 0);
        if (dpadToJoystick) {
            if (state & SC_BTN_LEFT) 
                data->joystickX = 0;
            else if (state & SC_BTN_RIGHT)
                data->joystickX = 1023;
        
            if (state & SC_BTN_UP) 
                data->joystickY = 0;
            else if (state & SC_BTN_DOWN) 
                data->joystickY = 1023;
        }
        else {
            data->buttons |= ( (state & SC_BTN_UP) ? segaMaskUp : 0 ) |
                             ( (state & SC_BTN_DOWN) ? segaMaskDown : 0 ) |
                             ( (state & SC_BTN_LEFT) ? segaMaskLeft : 0 ) |
                             ( (state & SC_BTN_RIGHT) ? segaMaskRight : 0 );
        }
        return true;
    }
    return false;
}

