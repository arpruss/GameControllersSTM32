#include <USBXBox360.h>
#include <GameControllers.h>

// NB: connect A10 to TX, A9 to RX

GenesisController gen(PA0, PA1, PA2, PA3, PA4, PA5, PA6);
USBXBox360 XBox360;
GameControllerData_t data;

#define XBOX_A 13
#define XBOX_B 14
#define XBOX_X 15
#define XBOX_Y 16
#define XBOX_DUP 1
#define XBOX_DDOWN 2
#define XBOX_DLEFT 3
#define XBOX_DRIGHT 4
#define XBOX_START 5
#define XBOX_LSHOULDER 9
#define XBOX_RSHOULDER 10
#define XBOX_GUIDE  11

const uint16_t remap_retroarch[16] = {
  XBOX_B, // A
  XBOX_A, // B
  XBOX_X, // C
  XBOX_LSHOULDER, // X

  XBOX_Y, // Y
  XBOX_RSHOULDER, // Z
  XBOX_START, // START
  XBOX_GUIDE, // MODE

  XBOX_DLEFT, // LEFT
  XBOX_DRIGHT, // RIGHT
  XBOX_DDOWN, // DOWN
  XBOX_DUP, // UP

  0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

const uint16_t* remap = remap_retroarch;

inline int16_t range10u16s(uint16_t x) {
  return (((int32_t)(uint32_t)x - 512) * 32767 + 255) / 512;
}

void setup() {
  gen.begin();
  USBComposite.setProductString("GenesisToXBox360");
  XBox360.begin();
  XBox360.setManualReportMode(true);
}

void loop() {
  if (gen.read(&data)) {
    XBox360.X(range10u16s(data.joystickX));
    XBox360.Y(-range10u16s(data.joystickY));
    uint16_t mask = 1;
    for (int i = 0; i < 16; i++, mask <<= 1) {
      uint16_t xb = remap[i];
      if (xb != 0xFFFF)
        XBox360.button(xb, 0 != (data.buttons & mask));
    }
    XBox360.send();
  }
}

