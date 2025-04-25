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
};

#define MAX_DISPLAYS  1
#define HAL_MAX_DELAY 0xFFFFFFFF

static struct __SSD1306_HandleTypeDef devices[MAX_DISPLAYS];
static size_t count;

void SSD1306_Init(SSD1306_HandleTypeDef* hssd1306, SPI_HandleTypeDef* hspi, struct GPIO_Pin pins[5])
{
    *hssd1306 = &devices[count++];

    const struct GPIO_Pin sck_pin = pins[0];
    const struct GPIO_Pin sda_pin = pins[1];
    const struct GPIO_Pin res_pin = pins[2];
    const struct GPIO_Pin dc_pin  = pins[3];
    const struct GPIO_Pin cs_pin  = pins[4];

    *hssd1306[count - 1] = (struct __SSD1306_HandleTypeDef)
    {
        hspi,
        sck_pin,
        sda_pin,
        res_pin,
        dc_pin,
        cs_pin,
    };

    // Инициализируем дисплей
    static uint8_t init_commands[] =
    {
        0xAE,       // Display OFF
        0xD5, 0x80, // Set display clock divide (osc freq)
        0xA8, 0x3F, // Set multiplex (64 rows)
        0xD3, 0x00, // Set display offset
        0x40,       // Set start line
        0x8D, 0x14, // Charge pump ON
        0x20, 0x00, // Horizontal addressing mode
        0xA1,       // Segment remap (column 127 mapped to SEG0)
        0xC8,       // COM output scan direction (remapped mode)
        0xDA, 0x12, // COM pins hardware configuration
        0x81, 0xFF, // Contrast control
        0xD9, 0xF1, // Pre-charge period
        0xDB, 0x40, // VCOMH deselect level
        0xA4,       // Entire display ON (ignore RAM)
        0xA6,       // Normal display (not inverted)
        0xAF        // Display ON
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

    uint8_t display_data[SSD1306_BUFFER_SIZE] = {0};
    SSD1306_DrawFrame(hssd1306, display_data, SSD1306_BUFFER_SIZE);
}

void SSD1306_DrawFrame(SSD1306_HandleTypeDef* const hssd1306, uint8_t* const buffer, const size_t size)
{
    uint8_t rx[size];

    SPI_HandleTypeDef* const hspi = (*hssd1306)->hspi;
    const struct GPIO_Pin cs_pin = (*hssd1306)->cs_pin;
    const struct GPIO_Pin dc_pin = (*hssd1306)->dc_pin;

    HAL_GPIO_WritePin(cs_pin.gpio, cs_pin.pin, GPIO_PIN_LOW);
    HAL_GPIO_WritePin(dc_pin.gpio, dc_pin.pin, GPIO_PIN_HIGH);
    HAL_SPI_Exchange(hspi, buffer, rx, size, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(cs_pin.gpio, cs_pin.pin, GPIO_PIN_HIGH);
}