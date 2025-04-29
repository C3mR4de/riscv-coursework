#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <mik32_hal_adc.h>
#include <stdint.h>
#include "GPIO_Pin.h"

struct __Joystick;
typedef struct __Joystick* Joystick;

void          Joystick_Init(Joystick* joystick, ADC_HandleTypeDef* hadc, uint8_t channel_x, uint8_t channel_y, struct GPIO_Pin sw_pin);
int16_t       Joystick_ReadX(Joystick* joystick);
int16_t       Joystick_ReadY(Joystick* joystick);
GPIO_PinState Joystick_ReadSw(Joystick* joystick);

#endif