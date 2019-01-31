#ifndef _NINTENDO_CONTROLLER_H
#define _NINTENDO_CONTROLLER_H

#include <Arduino.h>
#include "debouncer.h"
#define NUNCHUCK_SOFT_I2C // currently, HardWire doesn't work well for hotplugging
                 // Also, it probably won't well with SPI remap

#ifdef NUNCHUCK_SOFT_I2C
#include <SoftWire.h>
#else
#include <Wire.h>
#endif                 
                 
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
};

class NunchuckController : public GameController {
    private:
        static uint16_t rescale(uint8_t value); // 8 to 10 bit
        uint8_t sendBytes(uint8_t location, uint8_t value);
        const uint8_t i2cAddress = 0x52;
        uint8_t buffer[6];
        unsigned scl;
        unsigned sda;
#ifdef NUNCHUCK_SOFT_I2C
        SoftWire* wire;
#else
        TwoWire* wire;
#endif    
        
    
    public:
        bool begin(void);
        bool read(GameControllerData_t* data);
        NunchuckController(unsigned scl=PB6, unsigned sda=PB7);
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
        void sendBits(uint32_t data, uint8_t bits);
        bool receiveBits(void* data0, uint32_t bits);
    public:
        bool begin(void);
        bool read(GameControllerData_t* data);
        bool readWithRumble(GameControllerData_t* data, bool rumble);
        GameCubeController(unsigned pin);
};

#endif // _NINTENDO_CONTROLLER_H
