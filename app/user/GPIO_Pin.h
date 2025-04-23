#ifndef GPIO_PIN_H
#define GPIO_PIN_H

#include <mik32_hal_gpio.h>

struct GPIO_Pin
{
    GPIO_TypeDef*   gpio;
    HAL_PinsTypeDef pin;
};

#endif