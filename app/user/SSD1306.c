#include "SSD1306.h"
#include <string.h>
#include <stdbool.h>

struct __SSD1306_HandleTypeDef
{
    SPI_HandleTypeDef* hspi;
    struct GPIO_Pin sck_pin;
    struct GPIO_Pin sda_pin;
    struct GPIO_Pin res_pin;
    struct GPIO_Pin dc_pin;
    struct GPIO_Pin cs_pin;
    size_t width;
    size_t height;
};

#define MAX_DISPLAYS 1
#define HAL_MAX_DELAY 0xFFFFFFFF

static struct __SSD1306_HandleTypeDef devices[MAX_DISPLAYS];
static size_t count;

void SSD1306_Init(SSD1306_HandleTypeDef* hssd1306, SPI_HandleTypeDef* hspi, struct GPIO_Pin pins[5], size_t width, size_t height)
{
    *hssd1306 = &devices[count++];

    const struct GPIO_Pin sck_pin = pins[0];
    const struct GPIO_Pin sda_pin = pins[1];
    const struct GPIO_Pin res_pin = pins[2];
    const struct GPIO_Pin dc_pin  = pins[3];
    const struct GPIO_Pin cs_pin  = pins[4];

    **hssd1306 = (struct __SSD1306_HandleTypeDef)
    {
        hspi,
        sck_pin,
        sda_pin,
        res_pin,
        dc_pin,
        cs_pin,
        width,
        height
    };

    // Инициализируем дисплей
    static uint8_t init_commands[] =
    {
        0xAE,             // Display OFF
        0xD5, 0x80,       // Set display clock divide (osc freq)
        0xA8, 0x3F,       // Set multiplex (64 rows)
        0xD3, 0x00,       // Set display offset
        0x40,             // Set start line
        0x8D, 0x14,       // Charge pump ON
        0x20, 0x00,       // Horizontal addressing mode
        0xA1,             // Segment remap (column 127 mapped to SEG0)
        0xC8,             // COM output scan direction (remapped mode)
        0xDA, 0x12,       // COM pins hardware configuration
        0x81, 0xFF,       // Contrast control
        0xD9, 0xF1,       // Pre-charge period
        0xDB, 0x40,       // VCOMH deselect level
        0xA4,             // Entire display ON (ignore RAM)
        0xA6,             // Normal display (not inverted)
        0xAF              // Display ON
    };

    // Сбрасываем LCD
    HAL_GPIO_WritePin(res_pin.gpio, res_pin.pin, GPIO_PIN_LOW);
    HAL_DelayMs(100);

    HAL_GPIO_WritePin(res_pin.gpio, res_pin.pin, GPIO_PIN_HIGH);
    HAL_DelayMs(100);

    // Отправка команд инициализации
    HAL_GPIO_WritePin(cs_pin.gpio, cs_pin.pin, GPIO_PIN_LOW); // CS = LOW

    for (size_t i = 0; i < sizeof(init_commands); ++i)
    {
        HAL_GPIO_WritePin(dc_pin.gpio, dc_pin.pin, GPIO_PIN_LOW); // DC = 0 (команда)

        uint8_t tx_buf[1] = {init_commands[i]};
        uint8_t rx_buf[1];

        HAL_SPI_Exchange(hspi, tx_buf, rx_buf, 1, HAL_MAX_DELAY); // Используем Exchange вместо Transmit
    }

    HAL_GPIO_WritePin(cs_pin.gpio, cs_pin.pin, GPIO_PIN_HIGH); // CS = HIGH

    uint8_t display_data[1024] = {0};
    uint8_t rx[1024];
    memset(display_data, 0xFF, sizeof(display_data)); // Все пиксели включены

    bool on = true;

    while (true)
    {
        // Заливка экрана
        HAL_GPIO_WritePin(cs_pin.gpio, cs_pin.pin, GPIO_PIN_LOW);
        HAL_GPIO_WritePin(dc_pin.gpio, dc_pin.pin, GPIO_PIN_HIGH);
        HAL_SPI_Exchange(hspi, on ? display_data : rx, rx, sizeof(display_data), HAL_MAX_DELAY);
        HAL_GPIO_WritePin(cs_pin.gpio, cs_pin.pin, GPIO_PIN_HIGH);
        on = !on;
        HAL_DelayMs(1000);
    }

    // uint8_t white[128] = { [0 ... 127] = 0xFF }; // строка из единиц (все пиксели включены)
    // uint8_t black[128] = { [0 ... 127] = 0x00 }; // строка из единиц (все пиксели включены)

    // for (int page = 0; page < 8; page++)
    // {
    //     uint8_t set_page[] = {0xB0 + page, 0x00, 0x10};
    //     HAL_GPIO_WritePin((*hssd1306)->dc_pin.gpio, (*hssd1306)->dc_pin.pin, GPIO_PIN_LOW);
    //     HAL_SPI_Exchange(hspi, set_page, &rx, sizeof(set_page), HAL_MAX_DELAY);

    //     HAL_GPIO_WritePin((*hssd1306)->dc_pin.gpio, (*hssd1306)->dc_pin.pin, GPIO_PIN_HIGH);
    //     HAL_SPI_Exchange(hspi, black, &rx, 128, HAL_MAX_DELAY);
    // }

    // Сбрасываем LCD
    // HAL_GPIO_WritePin((*hssd1306)->res_pin.gpio, (*hssd1306)->res_pin.pin, GPIO_PIN_HIGH);
    // uint8_t rx[1];
    // HAL_SPI_Exchange(hspi, init_commands, rx, sizeof(init_commands), HAL_MAX_DELAY);
    // uint8_t white[128] = { [0 ... 127] = 0xFF }; // строка из единиц (все пиксели включены)
    // uint8_t black[128] = { [0 ... 127] = 0x00 }; // строка из единиц (все пиксели включены)

    // while (1)
    // {
    //     for (int page = 0; page < 8; page++)
    //     {
    //         uint8_t set_page[] = {0xB0 + page, 0x00, 0x10};
    //         HAL_GPIO_WritePin((*hssd1306)->dc_pin.gpio, (*hssd1306)->dc_pin.pin, GPIO_PIN_LOW);
    //         //GPIO_1->CLEAR |= (1 << 13); // команда
    //         HAL_SPI_Exchange(hspi, set_page, rx, 3, 1000);

    //         HAL_GPIO_WritePin((*hssd1306)->dc_pin.gpio, (*hssd1306)->dc_pin.pin, GPIO_PIN_HIGH);
    //         //GPIO_1->OUTPUT |= (1 << 13); // данные
    //         HAL_SPI_Exchange(hspi, page % 2 == 0 ? white : black, rx, 128, 1000);
    //     }
    // }
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
    HAL_SPI_Exchange(hspi, &command, receive, 1, HAL_MAX_DELAY);
    HAL_DelayMs(10);
    //HAL_GPIO_WritePin((*hssd1306)->cs_pin.gpio, (*hssd1306)->cs_pin.pin, GPIO_PIN_HIGH);
}

void SSD1306_SendData(SSD1306_HandleTypeDef* hssd1306, SPI_HandleTypeDef* hspi, uint8_t data)
{
    HAL_GPIO_WritePin((*hssd1306)->dc_pin.gpio, (*hssd1306)->dc_pin.pin, GPIO_PIN_HIGH);
    HAL_DelayMs(10);
    //HAL_GPIO_WritePin((*hssd1306)->cs_pin.gpio, (*hssd1306)->cs_pin.pin, GPIO_PIN_LOW);
    uint8_t receive[1];
    HAL_SPI_Exchange(hspi, &data, receive, 1, HAL_MAX_DELAY);
    HAL_DelayMs(10);
    //HAL_GPIO_WritePin((*hssd1306)->cs_pin.gpio, (*hssd1306)->cs_pin.pin, GPIO_PIN_HIGH);
}