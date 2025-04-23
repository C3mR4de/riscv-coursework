#include "SSD1306.h"

struct __SSD1306_HandleTypeDef
{
    struct GPIO_Pin sck_pin;
    struct GPIO_Pin sda_pin;
    struct GPIO_Pin dc_pin;
    struct GPIO_Pin cs_pin;
    size_t width;
    size_t height;
};

#define MAX_DISPLAYS 1

static struct __SSD1306_HandleTypeDef devices[MAX_DISPLAYS];
static size_t count;

void SSD1306_Init(SSD1306_HandleTypeDef* hssd1306, struct GPIO_Pin sck_pin, struct GPIO_Pin sda_pin, struct GPIO_Pin dc_pin, struct GPIO_Pin cs_pin, size_t width, size_t height)
{
    *hssd1306 = &devices[count++];

    **hssd1306 = (struct __SSD1306_HandleTypeDef)
    {
        sck_pin,
        sda_pin,
        dc_pin,
        cs_pin,
        width,
        height
    };


}

void SSD1306_SetSckPin(SSD1306_HandleTypeDef* hssd1306, struct GPIO_Pin pin)
{
    (*hssd1306)->sck_pin = pin;
}

void SSD1306_SetSdaPin(SSD1306_HandleTypeDef* hssd1306, struct GPIO_Pin pin)
{
    (*hssd1306)->sda_pin = pin;
}

void SSD1306_SetDcPin(SSD1306_HandleTypeDef* hssd1306, struct GPIO_Pin pin)
{
    (*hssd1306)->dc_pin = pin;
}

void SSD1306_SetCsPin(SSD1306_HandleTypeDef* hssd1306, struct GPIO_Pin pin)
{
    (*hssd1306)->cs_pin = pin;
}

void SSD1306_SetWidth(SSD1306_HandleTypeDef* hssd1306, size_t width)
{
    (*hssd1306)->width = width;
}

void SSD1306_SetHeight(SSD1306_HandleTypeDef* hssd1306, size_t height)
{
    (*hssd1306)->height = height;
}

void SSD1306_SendCommand(SSD1306_HandleTypeDef* hssd1306, SPI_HandleTypeDef* hspi, uint8_t command)
{
    HAL_GPIO_WritePin((*hssd1306)->dc_pin.gpio, (*hssd1306)->dc_pin.pin, GPIO_PIN_LOW);
    HAL_DelayMs(10);
    //HAL_GPIO_WritePin((*hssd1306)->cs_pin.gpio, (*hssd1306)->cs_pin.pin, GPIO_PIN_LOW);
    //HAL_DelayMs(10);
    uint8_t receive[1];
    HAL_SPI_Exchange(hspi, &command, receive, 1, 10);
    HAL_DelayMs(10);
    //HAL_GPIO_WritePin((*hssd1306)->cs_pin.gpio, (*hssd1306)->cs_pin.pin, GPIO_PIN_HIGH);
}

void SSD1306_SendData(SSD1306_HandleTypeDef* hssd1306, SPI_HandleTypeDef* hspi, uint8_t data)
{
    HAL_GPIO_WritePin((*hssd1306)->dc_pin.gpio, (*hssd1306)->dc_pin.pin, GPIO_PIN_HIGH);
    HAL_DelayMs(10);
    //HAL_GPIO_WritePin((*hssd1306)->cs_pin.gpio, (*hssd1306)->cs_pin.pin, GPIO_PIN_LOW);
    uint8_t receive[1];
    HAL_SPI_Exchange(hspi, &data, receive, 1, 10);
    HAL_DelayMs(10);
    //HAL_GPIO_WritePin((*hssd1306)->cs_pin.gpio, (*hssd1306)->cs_pin.pin, GPIO_PIN_HIGH);
}