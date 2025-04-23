#include <stdbool.h>
#include <SSD1306.h>

static void SPI_Init(void);
static void GPIO_Init(void);

static SPI_HandleTypeDef hspi;
static struct GPIO_Pin sck_pin = (struct GPIO_Pin){GPIO_1, GPIO_PIN_2};
static struct GPIO_Pin sda_pin = (struct GPIO_Pin){GPIO_1, GPIO_PIN_1};
static struct GPIO_Pin res_pin = (struct GPIO_Pin){GPIO_1, GPIO_PIN_8};
static struct GPIO_Pin dc_pin  = (struct GPIO_Pin){GPIO_0, GPIO_PIN_3};
static struct GPIO_Pin cs_pin  = (struct GPIO_Pin){GPIO_1, GPIO_PIN_5};

int main()
{
    SPI_Init();
    GPIO_Init();

    SSD1306_HandleTypeDef display;
    SSD1306_Init(&display, sck_pin, sda_pin, dc_pin, cs_pin, 128, 64);

    HAL_GPIO_WritePin(res_pin.gpio, res_pin.pin, GPIO_PIN_HIGH);
    HAL_DelayMs(10);
    HAL_GPIO_WritePin(res_pin.gpio, res_pin.pin, GPIO_PIN_LOW);
    HAL_DelayMs(10);

    HAL_GPIO_WritePin(cs_pin.gpio, cs_pin.pin, GPIO_PIN_LOW);

    SSD1306_SendCommand(&display, &hspi, 0xAE);
    SSD1306_SendCommand(&display, &hspi, 0xD5);
    SSD1306_SendCommand(&display, &hspi, 0x80);
    SSD1306_SendCommand(&display, &hspi, 0xA8);
    SSD1306_SendCommand(&display, &hspi, 0x3F);
    SSD1306_SendCommand(&display, &hspi, 0xD3);
    SSD1306_SendCommand(&display, &hspi, 0x00);
    SSD1306_SendCommand(&display, &hspi, 0x40);
    SSD1306_SendCommand(&display, &hspi, 0x8D);
    SSD1306_SendCommand(&display, &hspi, 0x14);
    SSD1306_SendCommand(&display, &hspi, 0x20);
    SSD1306_SendCommand(&display, &hspi, 0x00);
    SSD1306_SendCommand(&display, &hspi, 0xA1);
    SSD1306_SendCommand(&display, &hspi, 0xC8);
    SSD1306_SendCommand(&display, &hspi, 0xDA);
    SSD1306_SendCommand(&display, &hspi, 0x12);
    SSD1306_SendCommand(&display, &hspi, 0x81);
    SSD1306_SendCommand(&display, &hspi, 0xCF);
    SSD1306_SendCommand(&display, &hspi, 0xD9);
    SSD1306_SendCommand(&display, &hspi, 0xF1);
    SSD1306_SendCommand(&display, &hspi, 0xDB);
    SSD1306_SendCommand(&display, &hspi, 0x40);
    SSD1306_SendCommand(&display, &hspi, 0xA4);
    SSD1306_SendCommand(&display, &hspi, 0xA6);
    SSD1306_SendCommand(&display, &hspi, 0xAF);

    HAL_GPIO_WritePin(cs_pin.gpio, cs_pin.pin, GPIO_PIN_HIGH);
 
    while (true)
    {
        HAL_GPIO_WritePin(cs_pin.gpio, cs_pin.pin, GPIO_PIN_LOW);

        SSD1306_SendCommand(&display, &hspi, 0x22);
        SSD1306_SendCommand(&display, &hspi, 0x00);
        SSD1306_SendCommand(&display, &hspi, 0x02);
        SSD1306_SendCommand(&display, &hspi, 0x40);
        SSD1306_SendCommand(&display, &hspi, 0xFF);
        SSD1306_SendCommand(&display, &hspi, 0xFF);
        SSD1306_SendCommand(&display, &hspi, 0xFF);
        SSD1306_SendCommand(&display, &hspi, 0xFF);
        SSD1306_SendCommand(&display, &hspi, 0xFF);
        SSD1306_SendCommand(&display, &hspi, 0xFF);
        SSD1306_SendCommand(&display, &hspi, 0xFF);
        SSD1306_SendCommand(&display, &hspi, 0xFF);

        HAL_GPIO_WritePin(cs_pin.gpio, cs_pin.pin, GPIO_PIN_HIGH);
    }
}

static void SPI_Init(void)
{
    SPI_InitTypeDef spi_init =
    {
        .SPI_Mode    = HAL_SPI_MODE_MASTER,
        .BaudRateDiv = SPI_BAUDRATE_DIV64,
        .ManualCS    = SPI_MANUALCS_OFF,
        .CLKPhase    = SPI_PHASE_OFF,
        .CLKPolarity = SPI_POLARITY_LOW,
        .Decoder     = SPI_DECODER_NONE,
        .ChipSelect  = SPI_CS_1
    };

    hspi.Init     = spi_init;
    hspi.Instance = SPI_1;

    HAL_SPI_Init(&hspi);
    HAL_SPI_Enable(&hspi);
    HAL_SPI_CS_Enable(&hspi, 1);
}

static void GPIO_Init(void)
{
    GPIO_InitTypeDef gpio_sck =
    {
        .Pin  = sck_pin.pin,
        .Mode = HAL_GPIO_MODE_SERIAL,
        .Pull = HAL_GPIO_PULL_NONE,
        .DS   = HAL_GPIO_DS_2MA
    };

    HAL_GPIO_Init(sck_pin.gpio, &gpio_sck);

    GPIO_InitTypeDef gpio_sda =
    {
        .Pin  = sda_pin.pin,
        .Mode = HAL_GPIO_MODE_SERIAL,
        .Pull = HAL_GPIO_PULL_NONE,
        .DS   = HAL_GPIO_DS_2MA
    };

    HAL_GPIO_Init(sda_pin.gpio, &gpio_sda);

    GPIO_InitTypeDef gpio_res =
    {
        .Pin  = res_pin.pin,
        .Mode = HAL_GPIO_MODE_GPIO_OUTPUT,
        .Pull = HAL_GPIO_PULL_NONE,
        .DS   = HAL_GPIO_DS_2MA
    };

    HAL_GPIO_Init(res_pin.gpio, &gpio_res);

    GPIO_InitTypeDef gpio_dc =
    {
        .Pin  = dc_pin.pin,
        .Mode = HAL_GPIO_MODE_GPIO_OUTPUT,
        .Pull = HAL_GPIO_PULL_NONE,
        .DS   = HAL_GPIO_DS_2MA
    };

    HAL_GPIO_Init(dc_pin.gpio, &gpio_dc);

    GPIO_InitTypeDef gpio_cs =
    {
        .Pin  = cs_pin.pin,
        .Mode = HAL_GPIO_MODE_SERIAL,
        .Pull = HAL_GPIO_PULL_NONE,
        .DS   = HAL_GPIO_DS_2MA
    };

    HAL_GPIO_Init(cs_pin.gpio, &gpio_cs);
}