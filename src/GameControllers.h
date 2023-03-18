#ifndef _NINTENDO_CONTROLLER_H
#define _NINTENDO_CONTROLLER_H

#include <Arduino.h>
#include "debouncer.h"
#include "SegaController.h"
#include <SoftWire.h>
#include <Wire.h>
                 
#define CONTROLLER_NONE     0
#define CONTROLLER_GAMECUBE 1
#define CONTROLLER_NUNCHUCK 2
#define CONTROLLER_GAMEPORT 3

#define INPUT_NOT_IMPLEMENTED ((unsigned)-1)

typedef struct {
    gpio_dev* device;
    uint32_t mask;
    uint32_t pinNumber;
} PortData;

const uint16_t gcmaskA = 0x01;
const uint16_t gcmaskB = 0x02;
const uint16_t gcmaskX = 0x04;
const uint16_t gcmaskY = 0x08;
const uint16_t gcmaskDLeft = 0x100;
const uint16_t gcmaskDRight = 0x200;
const uint16_t gcmaskDDown = 0x400;
const uint16_t gcmaskDUp = 0x800;
const uint16_t gcmaskZ = 0x1000;
const uint16_t gcmaskR = 0x2000;
const uint16_t gcmaskL = 0x4000;

const uint16_t gcmaskDPad = 0xF00;
const uint16_t gcmaskStart = 0x10;

const uint16_t segaMaskA = 1;
const uint16_t segaMaskB = 2;
const uint16_t segaMaskC = 4;
const uint16_t segaMaskX = 8;
const uint16_t segaMaskY = 16;
const uint16_t segaMaskZ = 32;
const uint16_t segaMaskStart = 64;
const uint16_t segaMaskMode = 128;
const uint16_t segaMaskLeft = gcmaskDLeft;
const uint16_t segaMaskRight = gcmaskDRight;
const uint16_t segaMaskDown = gcmaskDDown;
const uint16_t segaMaskUp = gcmaskDUp;

typedef struct {
  uint16_t buttons;
  uint8_t joystickX;
  uint8_t joystickY;
  uint8_t cX;
  uint8_t cY;
  uint8_t shoulderLeft;
  uint8_t shoulderRight;
} GameCubeData_t;

typedef struct {
  uint8_t device;
  uint16_t buttons;
  uint16_t joystickX; // 10 bit range for analog values
  uint16_t joystickY;
  uint16_t cX;
  uint16_t cY;
  uint16_t shoulderLeft;
  uint16_t shoulderRight;
} GameControllerData_t;

class GameController {
    protected:
        bool dpadToJoystick = true;
        void setPortData(PortData *p, unsigned pin) {
            if (pin == INPUT_NOT_IMPLEMENTED) {
                p->device = NULL;
            }
            else {
                p->device = digitalPinToPort(pin);
                p->mask = digitalPinToBitMask(pin);
                p->pinNumber = PIN_MAP[pin].gpio_bit;
            }
        }
    public: 
        bool read(GameControllerData_t* data) {
            return false;
        }
        bool begin(void) {
            return true;
        }
        void setDPadToJoystick(bool value) {
            dpadToJoystick = value;
        }
        bool getDPadToJoystick() {
            return dpadToJoystick;
        }
};

class NunchuckControllerBase : public GameController {
    private:
        static uint16_t rescale(uint8_t value); // 8 to 10 bit
        uint8_t sendBytes(uint8_t location, uint8_t value);
        const uint8_t i2cAddress = 0x52;
        uint8_t buffer[6];
        WireBase* wirebase;
            
    public:
        bool initNunchuck(void); // wire must be initialized first
        bool read(GameControllerData_t* data);
        NunchuckControllerBase(WireBase* _wirebase) {
            wirebase = _wirebase;
        }
};

class NunchuckController : public NunchuckControllerBase {
    private:
        TwoWire wire;
    
    public:
        bool begin(void) {
            wire.begin();
            return initNunchuck();
        }

        NunchuckController(unsigned port=1, unsigned mode=0) : 
            wire(port, mode), NunchuckControllerBase(&wire) {};
};

class NunchuckController_SoftWire : public NunchuckControllerBase {
    private:
        SoftWire wire;
    
    public:
        bool begin(void) {
            wire.begin();
            return initNunchuck();
        }

        NunchuckController_SoftWire(unsigned scl=PB6, unsigned sda=PB7, unsigned speed=SOFT_STANDARD) : 
            wire(scl, sda, speed), NunchuckControllerBase(&wire) {};
};

class GamePortController : public GameController {
    private:
        Debouncer* debouncers[4];
        unsigned buttonPins[4];
        uint32_t axisResistor = 10000;
        int16_t getValue(unsigned pin);
        unsigned xPin;
        unsigned yPin;
        unsigned sliderPin;
        unsigned rzPin;
        bool haveAxes;
        unsigned samples=4;
    public:
        bool begin(void);
        bool read(GameControllerData_t* data);
        GamePortController(unsigned axis1, unsigned axis2, unsigned axis3, unsigned axis4,
                unsigned button1, unsigned button2, unsigned button3, unsigned button4);
        void setSamples(unsigned n) {
            samples = n;
        }
        void setAxisResistors(uint32_t ohms) {
            axisResistor = ohms;
        }
};

class GameCubeController : public GameController {
    private:
        PortData port;
        unsigned fails;
        GameCubeData_t gcData;
        static inline uint16_t rescale(uint8_t value) {
			int32_t v = 512+((int32)value-128)*(1023*9)/(8*255);
			if (v<0)
				return 0;
			else if (v>1023)
				return 1023;
			else
				return v;
        }
        static inline uint16_t rescaleReversed(uint8_t value) {
			int32_t v = 512-((int32)value-128)*(1023*9)/(8*255);
			if (v<0)
				return 0;
			else if (v>1023)
				return 1023;
			else
				return v;
        }
    public:
        void sendBits(uint32_t data, uint8_t bits);
        bool receiveBits(void* data0, uint32_t bits);
        bool begin(void);
        bool read(GameControllerData_t* data);
        bool readWithRumble(GameControllerData_t* data, bool rumble);
        bool readWithRumble(GameCubeData_t* data, bool rumble);
        GameCubeController(unsigned pin);
};

#define NUM_GENESIS_PINS 7
enum GenesisPins { GENESIS_PIN_1 = 0, GENESIS_PIN_2, GENESIS_PIN_3, GENESIS_PIN_4, GENESIS_PIN_5, GENESIS_PIN_6, GENESIS_PIN_7, GENESIS_PIN_9 };
#define GENESIS_PIN_SELECT = GENESIS_PIN_7;

class GenesisController : public GameController {
    private:
        SegaController* sega;
    public:
        bool begin(void);
        bool read(GameControllerData_t* data);
        GenesisController(unsigned pin1, unsigned pin2, unsigned pin3, unsigned pin4, unsigned pin6, unsigned pin7, unsigned pin9);
};

#endif // _NINTENDO_CONTROLLER_H
