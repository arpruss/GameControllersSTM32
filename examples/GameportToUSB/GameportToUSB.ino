#include "GameControllers.h"
#include "Joystick.h"

#define LED_BUILTIN PB12 // change to match your board

//
// This requires an stm32f1 board compatible with the no-grounding-pin feature of ADCTouchSensor,
//  the libmaple-based stm32f1 core: https://github.com/rogerclarkmelbourne/Arduino_STM32
//  and these libraries: https://github.com/arpruss/GameControllersSTM32
//  and these libraries: https://github.com/arpruss/Joystick_stm32f1
//
// Pinout for reading gameport analog values:
// Gameport 3 (X1) A0 --10K-- ground (X)
// Gameport 6 (Y1) A1 --10K-- ground (Y)
// Gameport 13 (X2) A2 --10K-- ground (slider)
// Gameport 11 (Y2) A3 --10K-- ground (Z rotate)
// Gameport 1, 8, 9, 15 -- 3.3V
// Gameport 4, 5, 12 -- GND
// Gameport 2 (B1) -- A4 (button 1)
// Gameport 7 (B2) -- A5 (button 2)
// Gameport 10 (B3) -- A6 (button 3)
// Gameport 14 (B4) -- A7 (button 4)

GamePortController controller(PA0,PA1,PA2,PA3,PA4,PA5,PA6,PA7);

void setup() 
{
    pinMode(LED_BUILTIN, OUTPUT);
    
    digitalWrite(LED_BUILTIN, 1);     
    controller.begin();
    adc_set_sample_rate(ADC1, ADC_SMPR_13_5); // ADC_SMPR_13_5, ADC_SMPR_1_5
    Joystick.setManualReportMode(true);
} 

void loop() 
{
    GameControllerData_t data;
    if (controller.read(&data)) {
      Joystick.X(data.joystickX);
      Joystick.Y(data.joystickY);
      Joystick.Xrotate(data.cX);
      Joystick.sliderRight(data.shoulderRight);
      uint8_t mask = 1;
      for (int i=1; i<=8; i++, mask <<= 1)
        Joystick.button(i, (data.buttons & mask) != 0);
      Joystick.sendManualReport();
    }

    delay(10);
}

