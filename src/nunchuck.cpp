#include "GameControllers.h"
#include <string.h>

#define SOFT_I2C // currently, HardWire doesn't work well for hotplugging
                 // Also, it probably won't well with SPI remap

NunchuckController::NunchuckController(unsigned scl, unsigned sda) {
#ifdef NUNCHUCK_SOFT_I2C
    wire = new TwoWire(scl, sda, SOFT_STANDARD);      
#else
    wire = new HardWire(1, 0); // I2C_FAST_MODE); 
#endif    
}

bool NunchuckController::begin() {
#ifdef MANUAL_DECRYPT
    if (!sendBytes(0x40,0x00)) {
        return false; 
    }
    delayMicroseconds(250);
#else
    if (! sendBytes(0xF0, 0x55)) {
        return false;
    }
    delayMicroseconds(250);
    if (! sendBytes(0xFB, 0x00)) 
        return false;
#endif
    delayMicroseconds(250);
    return true; 
}

uint8_t NunchuckController::sendBytes(uint8_t location, uint8_t value) {
    wire->beginTransmission(i2cAddress);
    wire->write(location);
    wire->write(value);
    return 0 == wire->endTransmission();
}

uint16_t NunchuckController::rescale(uint8_t x) {
  int32_t x1 = 512+((int32_t)x-128)*48/10;
  if (x1 < 0)
    return 0;
  else if (x1 > 1023)
    return 1023;
  else
    return x1;
}

bool NunchuckController::read(GameControllerData_t* data) {
    wire->beginTransmission(i2cAddress);
    wire->write(0x00);
    if (0!=wire->endTransmission()) 
      return 0;

    delayMicroseconds(500);
#ifdef SERIAL_DEBUG
//    Serial.println("Requested");
#endif

    wire->requestFrom(i2cAddress, 6);
    int count = 0;
    while (wire->available() && count<6) {
#ifdef MANUAL_DECRYPT
      buffer[count++] = ((uint8_t)0x17^(uint8_t)wire->read()) + (uint8_t)0x17;
#else
      buffer[count++] = wire->read();
#endif      
    }
    if (count < 6)
      return 0;
    memset(data, 0, sizeof(GameControllerData_t));
    data->joystickX = rescale(buffer[0]);
    data->joystickY = rescale(buffer[1]);
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
    // ignore acceleration for now

    return true;
}


