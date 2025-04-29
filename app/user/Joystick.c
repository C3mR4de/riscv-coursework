#include "Joystick.h"
#include <stddef.h>

struct __Joystick
{
    ADC_HandleTypeDef* hadc;
    struct GPIO_Pin    sw_pin;
    uint8_t            channel_x;
    uint8_t            channel_y;
    uint16_t           zero_x;
    uint16_t           zero_y;
};

#define MAX_JOYSTICKS 1
#define ADC_DIVIDER   128

static struct __Joystick joysticks[MAX_JOYSTICKS];
static size_t count;

static int16_t __Joystick_ReadRaw(Joystick* joystick, uint8_t channel);

void Joystick_Init(Joystick* const joystick, ADC_HandleTypeDef* const hadc, const uint8_t channel_x, const uint8_t channel_y, const struct GPIO_Pin sw_pin)
{
    *joystick = &joysticks[count++];

    joysticks[count - 1] = (struct __Joystick)
    {
        .hadc       = hadc,
        .sw_pin     = sw_pin,
        .channel_x  = channel_x,
        .channel_y  = channel_y,
    };

    HAL_ADC_ContinuousEnable(hadc);

    (*joystick)->zero_x = __Joystick_ReadRaw(joystick, channel_x); // 2983 (zero) 4095 (left)  213 (right)
    (*joystick)->zero_y = __Joystick_ReadRaw(joystick, channel_y); // 2907 (zero)  225 (up)   4095 (down)
}

int16_t Joystick_ReadX(Joystick* const joystick)
{
    return -((__Joystick_ReadRaw(joystick, (*joystick)->channel_x) - (*joystick)->zero_x) / ADC_DIVIDER);
}

int16_t Joystick_ReadY(Joystick* const joystick)
{
    return (__Joystick_ReadRaw(joystick, (*joystick)->channel_y) - (*joystick)->zero_y) / ADC_DIVIDER;
}

bool Joystick_ReadSw(Joystick* const joystick)
{
    return HAL_GPIO_ReadPin((*joystick)->sw_pin.gpio, (*joystick)->sw_pin.pin);
}

static int16_t __Joystick_ReadRaw(Joystick* const joystick, const uint8_t channel)
{
    (*joystick)->hadc->Init.Sel = channel;
    HAL_ADC_ChannelSet((*joystick)->hadc);
    return (int16_t)HAL_ADC_GetValue((*joystick)->hadc);
}