#include <mik32_hal_adc.h>
#include <inttypes.h>
#include <Display.h>
#include <GameField.h>
#include <Joystick.h>
#include <string.h>

static void SystemClock_Config(void);
static void SPI_Init(void);
static void GPIO_Init(void);
static void ADC_Init(void);

static SPI_HandleTypeDef hspi;
static ADC_HandleTypeDef hadc;

// Пины OLED-дисплея
static const struct GPIO_Pin sck_pin = (struct GPIO_Pin){GPIO_0, GPIO_PIN_2}; // Пин D6 (SCK, D0)
static const struct GPIO_Pin sda_pin = (struct GPIO_Pin){GPIO_0, GPIO_PIN_1}; // Пин D5 (MOSI, D1)
static const struct GPIO_Pin res_pin = (struct GPIO_Pin){GPIO_0, GPIO_PIN_5}; // Пин D0
static const struct GPIO_Pin dc_pin  = (struct GPIO_Pin){GPIO_0, GPIO_PIN_6}; // Пин D1
static const struct GPIO_Pin cs_pin  = (struct GPIO_Pin){GPIO_0, GPIO_PIN_4}; // Пин A2

// Пины джойстика
static const struct GPIO_Pin adc_x_pin  = (struct GPIO_Pin){GPIO_1, GPIO_PIN_7};
static const struct GPIO_Pin adc_y_pin  = (struct GPIO_Pin){GPIO_1, GPIO_PIN_5};
static const struct GPIO_Pin sw_pin     = (struct GPIO_Pin){GPIO_0, GPIO_PIN_7};

// Каналы джойстика
#define JOYSTICK_CHANNEL_X  ADC_CHANNEL0
#define JOYSTICK_CHANNEL_Y  ADC_CHANNEL1
#define JOYSTICK_CHANNEL_SW ADC_CHANNEL3

#define PLANE_WIDTH  16
#define PLANE_HEIGHT 10

static uint8_t    map[DISPLAY_BUFFER_SIZE];
static const bool plane_texture[PLANE_WIDTH * PLANE_HEIGHT] =
{
    false, false, false, false, false, false, false,  true,  true, false, false, false, false, false, false, false,
    false, false, false, false, false, false,  true,  true,  true,  true, false, false, false, false, false, false,
    false, false, false, false, false,  true,  true,  true,  true,  true,  true, false, false, false, false, false,
    false, false, false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false, false, false,
    false,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true,  true, false,
     true,  true,  true,  true,  true,  true, false,  true,  true, false,  true,  true,  true,  true,  true,  true,
     true,  true,  true, false, false, false, false,  true,  true, false, false, false, false,  true,  true,  true,
     true,  true, false, false, false, false, false,  true,  true, false, false, false, false, false,  true,  true,
     true, false, false, false, false, false, false, false, false, false, false, false, false, false, false,  true,
};

int main()
{
    SystemClock_Config();
    SPI_Init();
    GPIO_Init();
    ADC_Init();

    static Display display;
    Display_Init(&display, &hspi, (struct GPIO_Pin[5]){sck_pin, sda_pin, res_pin, dc_pin, cs_pin});

    static const size_t start_x = (DISPLAY_WIDTH - PLANE_WIDTH) / 2;
    static const size_t start_y = (DISPLAY_HEIGHT - PLANE_HEIGHT) * 7 / 8;

    static GameField game_field;
    GameField_Init(&game_field, map, DISPLAY_BUFFER_SIZE, DISPLAY_WIDTH, DISPLAY_HEIGHT, (struct Plane)
    {
        .x       = start_x,
        .y       = start_y,
        .width   = PLANE_WIDTH,
        .height  = PLANE_HEIGHT,
        .texture = plane_texture
    });

    static Joystick joystick;
    Joystick_Init(&joystick, &hadc, JOYSTICK_CHANNEL_X, JOYSTICK_CHANNEL_Y, sw_pin);

    while (true)
    {
        const int16_t dx   = Joystick_ReadX(&joystick);
        const int16_t dy   = Joystick_ReadY(&joystick);
        const bool    shot = Joystick_ReadSw(&joystick);

        GameField_MoveAsteroids(&game_field);
        GameField_MovePlane(&game_field, dx, dy);
        GameField_MoveBullets(&game_field, shot);
        GameField_CheckCollisions(&game_field);
        Display_DrawFrame(&display, map, DISPLAY_BUFFER_SIZE);
    }
}

static void SystemClock_Config(void)
{
    PCC_InitTypeDef PCC_OscInit =
    {
        .OscillatorEnable         = PCC_OSCILLATORTYPE_ALL,
        .FreqMon.OscillatorSystem = PCC_OSCILLATORTYPE_OSC32M,
        .FreqMon.ForceOscSys      = PCC_FORCE_OSC_SYS_UNFIXED,
        .FreqMon.Force32KClk      = PCC_FREQ_MONITOR_SOURCE_OSC32K,
        .AHBDivider               = 0,
        .APBMDivider              = 0,
        .APBPDivider              = 0,
        .HSI32MCalibrationValue   = 128,
        .LSI32KCalibrationValue   = 8,
        .RTCClockSelection        = PCC_RTC_CLOCK_SOURCE_AUTO,
        .RTCClockCPUSelection     = PCC_CPU_RTC_CLOCK_SOURCE_OSC32K
    };

    HAL_PCC_Config(&PCC_OscInit);
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
        .ChipSelect  = SPI_CS_0
    };

    hspi.Instance = SPI_0;
    hspi.Init = spi_init;

    if (HAL_SPI_Init(&hspi) == HAL_OK)
    {
        HAL_SPI_Enable(&hspi);
    }
}

static void GPIO_Init(void)
{
    __HAL_PCC_GPIO_0_CLK_ENABLE();
    __HAL_PCC_GPIO_1_CLK_ENABLE();
    __HAL_PCC_GPIO_2_CLK_ENABLE();
    __HAL_PCC_ANALOG_REGS_CLK_ENABLE();

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

    GPIO_InitTypeDef gpio_adc_x =
    {
        .Pin  = adc_x_pin.pin,
        .Mode = HAL_GPIO_MODE_ANALOG,
        .Pull = HAL_GPIO_PULL_NONE,
        .DS   = HAL_GPIO_DS_2MA
    };

    HAL_GPIO_Init(adc_x_pin.gpio, &gpio_adc_x);

    GPIO_InitTypeDef gpio_adc_y =
    {
        .Pin  = adc_y_pin.pin,
        .Mode = HAL_GPIO_MODE_ANALOG,
        .Pull = HAL_GPIO_PULL_NONE,
        .DS   = HAL_GPIO_DS_2MA
    };

    HAL_GPIO_Init(adc_y_pin.gpio, &gpio_adc_y);

    GPIO_InitTypeDef gpio_sw =
    {
        .Pin  = sw_pin.pin,
        .Mode = HAL_GPIO_MODE_GPIO_INPUT,
        .Pull = HAL_GPIO_PULL_DOWN,
        .DS   = HAL_GPIO_DS_2MA
    };

    HAL_GPIO_Init(sw_pin.gpio, &gpio_sw);
}

static void ADC_Init(void)
{
    hadc.Instance    = ANALOG_REG;
    hadc.Init.EXTRef = ADC_EXTREF_OFF;
    hadc.Init.EXTClb = ADC_EXTCLB_CLBREF;

    HAL_ADC_Init(&hadc);
}