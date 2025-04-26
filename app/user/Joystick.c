#include "Joystick.h"
#include <stddef.h>

struct __Joystick
{
    ADC_HandleTypeDef* hadc;
    uint8_t            channel_x;
    uint8_t            channel_y;
    uint16_t           zero_x;
    uint16_t           zero_y;
};

#define MAX_JOYSTICKS 1

static struct __Joystick joysticks[MAX_JOYSTICKS];
static size_t count;

static int16_t __Joystick_ReadRawX(Joystick* joystick);
static int16_t __Joystick_ReadRawY(Joystick* joystick);

void Joystick_Init(Joystick* joystick, ADC_HandleTypeDef* hadc, uint8_t channel_x, uint8_t channel_y)
{
    *joystick = &joysticks[count++];

    *joystick[count - 1] = (struct __Joystick)
    {
        .hadc      = hadc,
        .channel_x = channel_x,
        .channel_y = channel_y,
    };

    HAL_ADC_ContinuousEnable(hadc);

    (*joystick)->zero_x = __Joystick_ReadRawX(joystick); // 2983 (zero) 4095 (left)  213 (right)
    (*joystick)->zero_y = __Joystick_ReadRawY(joystick); // 2907 (zero)  225 (up)   4095 (down)
}

int16_t Joystick_ReadX(Joystick* joystick)
{
    return -((__Joystick_ReadRawX(joystick) - (*joystick)->zero_x) / 128);
}

int16_t Joystick_ReadY(Joystick* joystick)
{
    return (__Joystick_ReadRawY(joystick) - (*joystick)->zero_y) / 128;
}

static int16_t __Joystick_ReadRawX(Joystick* joystick)
{
    (*joystick)->hadc->Init.Sel = (*joystick)->channel_x;
    HAL_ADC_ChannelSet((*joystick)->hadc);
    return (int16_t)HAL_ADC_GetValue((*joystick)->hadc);
}

static int16_t __Joystick_ReadRawY(Joystick* joystick)
{
    (*joystick)->hadc->Init.Sel = (*joystick)->channel_y;
    HAL_ADC_ChannelSet((*joystick)->hadc);
    return (int16_t)HAL_ADC_GetValue((*joystick)->hadc);
}