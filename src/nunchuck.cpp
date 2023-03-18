#include "GameControllers.h"
#include <string.h>
#define MULT 2

//#define MANUAL_DECRYPT

bool NunchuckControllerBase::initNunchuck() {
    delayMicroseconds(MULT*250);
    
#ifdef MANUAL_DECRYPT
    if (!sendBytes(0x40,0x00)) {
        return false; 
    }
    delayMicroseconds(MULT*250);
#else
    if (! sendBytes(0xF0, 0x55)) {
        return false;
    }
    delayMicroseconds(MULT*250);
    if (! sendBytes(0xFB, 0x00)) 
        return false;
#endif
    delayMicroseconds(MULT*250);
    return true; 
}

uint8_t NunchuckControllerBase::sendBytes(uint8_t location, uint8_t value) {
    wirebase->beginTransmission(i2cAddress);
    wirebase->write(location);
    wirebase->write(value);
    return 0 == wirebase->endTransmission();
}

uint16_t NunchuckControllerBase::rescale(uint8_t x) {
  int32_t x1 = 512+((int32_t)x-128)*48/10;
  if (x1 < 0)
    return 0;
  else if (x1 > 1023)
    return 1023;
  else
    return x1;
}

bool NunchuckControllerBase::read(GameControllerData_t* data) {
    unsigned retries = 2;
    while (1) {
        wirebase->beginTransmission(i2cAddress);
        wirebase->write(0x00);
        if (0 == wirebase->endTransmission()) 
            break;
        retries--;
        if (!retries)
            return false;
        delayMicroseconds(MULT*500);
    }

    delayMicroseconds(MULT*500);
#ifdef SERIAL_DEBUG
//    Serial.println("Requested");
#endif

    wirebase->requestFrom(i2cAddress, 6);
    int count = 0;
    while (wirebase->available() && count<6) {
#ifdef MANUAL_DECRYPT
      buffer[count++] = ((uint8_t)0x17^(uint8_t)wirebase->read()) + (uint8_t)0x17;
#else
      buffer[count++] = wirebase->read();
#endif      
    }
    if (count < 6)
      return 0;
    memset(data, 0, sizeof(GameControllerData_t));
    data->joystickX = rescale(buffer[0]);
    data->joystickY = 1023-rescale(buffer[1]);
    data->buttons = 0;
    if (! (buffer[5] & 1) ) // Z
      data->buttons |= 1;
    if (! (buffer[5] & 2) ) // C
      data->buttons |= 2;
    data->cX = 512;
    data->cY = 512;
    data->shoulderLeft = 0;
    data->shoulderRight = 0;
    data->device = CONTROLLER_NUNCHUCK;
    return true;
}
