#include "GameControllers.h"

GamePortController controller(PA0,PA1,PA2,PA3,PA4,PA5,PA6,PA7);

void setup() {
  Serial.begin();
  controller.begin();
}

void loop() {
  GameControllerData_t data;
  Serial.println(String(analogRead(PA3)));
  if (controller.read(&data)) {
    Serial.println(String(data.joystickX)+" "+String(data.joystickY)+
      +" "+String(data.shoulderRight)+" "+String(data.cX)+" "+
      String(data.buttons,HEX));
  }
  delay(200);
}
