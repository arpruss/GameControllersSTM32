#include "GameControllers.h"
#include "dwt.h"

static const uint8_t maxFails = 4;
static const uint32_t cyclesPerUS = (SystemCoreClock / 1000000ul);
static const uint32_t quarterBitSendingCycles = cyclesPerUS * 5 / 4;
static const uint32_t bitReceiveCycles = cyclesPerUS * 4;
static const uint32_t halfBitReceiveCycles = cyclesPerUS * 2;
static const uint32_t bitTimeoutCycles = cyclesPerUS * 7;

// Timeout for controller response:
//  http://www.int03.co.uk/crema/hardware/gamecube/gc-control.html measured no delay or 15 us
//  8us works for a knockoff controller I have
//  16us works for Konami DDR pad; 15us works half the time
static const uint32_t responseTimeoutCycles = 32*cyclesPerUS; // cyclesPerUS * 100; 

GameCubeController::GameCubeController(unsigned p) {
    setPortData(&port, p);
}
    
bool GameCubeController::begin() {
  gpio_set_mode(port.device, port.pinNumber, GPIO_OUTPUT_OD); // set open drain output
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= 1;
  fails = maxFails; // force update
  return true;
}

// at most 32 bits can be sent
void GameCubeController::sendBits(uint32_t data, uint8_t bits) {
  data <<= 32 - bits;
  DWT->CYCCNT = 0;
  uint32_t timerEnd = DWT->CYCCNT;
  do {
    gpio_write_bit(port.device, port.pinNumber, 0);
    if (0x80000000ul & data)
      timerEnd += quarterBitSendingCycles;
    else
      timerEnd += 3 * quarterBitSendingCycles;
    while (DWT->CYCCNT < timerEnd) ;
    gpio_write_bit(port.device, port.pinNumber, 1);
    if (0x80000000ul & data)
      timerEnd += 3 * quarterBitSendingCycles;
    else
      timerEnd += quarterBitSendingCycles;
    data <<= 1;
    bits--;
    while (DWT->CYCCNT < timerEnd) ;
  } while (bits);
}

// bits must be greater than 0
bool GameCubeController::receiveBits(void* data0, uint32_t bits) {
  uint8_t* data = (uint8_t*)data0;
  uint8_t bitmap = 0x80;
  
  uint32_t timeOut = responseTimeoutCycles;

  *data = 0;
  do {
      // wait for start of bit (low)
      DWT->CYCCNT = 0;
      while (gpio_read_bit(port.device, port.pinNumber)) {
          if (DWT->CYCCNT >= timeOut)
              return false;
      }
      
      DWT->CYCCNT = 0;
      while (DWT->CYCCNT < halfBitReceiveCycles - 2) ;
      
      if (gpio_read_bit(port.device, port.pinNumber)) {
        *data |= bitmap;
      }
      bitmap >>= 1;
      bits--;
      if (bitmap == 0) {
        data++;
        bitmap = 0x80;
        if (bits)
          *data = 0;
      }

      // wait for high part of bit if necessary
      DWT->CYCCNT = 0;
      while (!gpio_read_bit(port.device, port.pinNumber)) {
          if (DWT->CYCCNT >= bitTimeoutCycles)
              return false;
      }
      timeOut = bitTimeoutCycles; 
  } while (bits);

  return true;
}

bool GameCubeController::readWithRumble(GameCubeData_t* data, bool rumble) {
  if (fails >= maxFails) {
    nvic_globalirq_disable();
    sendBits(0b000000001l, 9);
    nvic_globalirq_enable();
    delayMicroseconds(400);
    fails = 0;
  }
  nvic_globalirq_disable();
  sendBits(rumble ? 0b0100000000000011000000011l : 0b0100000000000011000000001l, 25);
  bool success = receiveBits(data, 64);
  nvic_globalirq_enable();
  return success;
}

bool GameCubeController::readWithRumble(GameControllerData_t* data, bool rumble) {
  bool success = readWithRumble(&gcData, rumble);
  if (success && 0 == (gcData.buttons & 0x80) && (gcData.buttons & 0x8000) ) {
    data->device = CONTROLLER_GAMECUBE;
    data->buttons = gcData.buttons;
    data->joystickX = rescale(gcData.joystickX);
    data->joystickY = rescaleReversed(gcData.joystickY);
    data->cX = rescale(gcData.cX);
    data->cY = rescaleReversed(gcData.cY);
    data->shoulderLeft = rescale(gcData.shoulderLeft);
    data->shoulderRight = rescale(gcData.shoulderRight);
    /* support Konami pads */
    if (0 == (gcData.buttons & 0x20) && gcData.joystickX == 1 && gcData.joystickY == 1 && gcData.cX == 1 && gcData.cY == 1 && 
            gcData.shoulderLeft == 1 && gcData.shoulderRight == 1 /*&& gcData.buttons*/ ) {
        data->joystickX = 512;
        data->joystickY = 512;

        if (dpadToJoystick) {            
            if (gcData.buttons & gcmaskDLeft) {
                data->joystickX = 0;
            }
            else if (gcData.buttons & gcmaskDRight) {
                data->joystickX = 1023;
            }
            
            if (gcData.buttons & gcmaskDUp) {
                data->joystickY = 0;
            }
            else if (gcData.buttons & gcmaskDDown) {
                data->joystickY = 1023;
            } 
        }
    }
    return true;
  }
  else {
    fails++;
    return false;
  }
}

bool GameCubeController::read(GameControllerData_t* data) {
    return readWithRumble(data, false);
}
