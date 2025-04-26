#ifndef JOYSTICK_H
#define JOYSTICK_H

#include <mik32_hal_adc.h>
#include <stdint.h>

struct __Joystick;
typedef struct __Joystick* Joystick;

void     Joystick_Init(Joystick* joystick, ADC_HandleTypeDef* hadc, uint8_t channel_x, uint8_t channel_y);
int16_t Joystick_ReadX(Joystick* joystick);
int16_t Joystick_ReadY(Joystick* joystick);

#endif