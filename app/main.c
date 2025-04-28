#include <mik32_hal_adc.h>
#include <mik32_hal_usart.h>
#include <mik32_hal_irq.h>
#include <xprintf.h>
#include <inttypes.h>
#include <Display.h>
#include <GameField.h>
#include <Joystick.h>

static void SystemClock_Config(void);
static void SPI_Init(void);
static void USART_Init(void);
static void GPIO_Init(void);
static void ADC_Init(void);
// static void Shoot_TrapHandler_Raw(void) __attribute__((interrupt("machine"), section(".text.trap_handler")));
// void Shoot_TrapHandler(void) __attribute__((weak));

static SPI_HandleTypeDef   hspi;
static USART_HandleTypeDef husart0;
static ADC_HandleTypeDef   hadc;

// Пины OLED-дисплея
static const struct GPIO_Pin sck_pin = (struct GPIO_Pin){GPIO_0, GPIO_PIN_2}; // Пин D6 (SCK, D0)
static const struct GPIO_Pin sda_pin = (struct GPIO_Pin){GPIO_0, GPIO_PIN_1}; // Пин D5 (MOSI, D1)
static const struct GPIO_Pin res_pin = (struct GPIO_Pin){GPIO_0, GPIO_PIN_5}; // Пин D0
static const struct GPIO_Pin dc_pin  = (struct GPIO_Pin){GPIO_0, GPIO_PIN_6}; // Пин D1
static const struct GPIO_Pin cs_pin  = (struct GPIO_Pin){GPIO_0, GPIO_PIN_4}; // Пин A2

// Пины джойстика
static const struct GPIO_Pin adc_x_pin = (struct GPIO_Pin){GPIO_1, GPIO_PIN_7};
static const struct GPIO_Pin adc_y_pin = (struct GPIO_Pin){GPIO_1, GPIO_PIN_5};
static const struct GPIO_Pin sw_pin    = (struct GPIO_Pin){GPIO_1, GPIO_PIN_2};

// Каналы джойстика
#define JOYSTICK_CHANNEL_X ADC_CHANNEL0
#define JOYSTICK_CHANNEL_Y ADC_CHANNEL1

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

static GameField game_field;

int main()
{
    SystemClock_Config();
    SPI_Init();
    USART_Init();
    GPIO_Init();
    ADC_Init();

    Display display;
    Display_Init(&display, &hspi, (struct GPIO_Pin[5]){sck_pin, sda_pin, res_pin, dc_pin, cs_pin});

    static const size_t start_x = (DISPLAY_WIDTH - PLANE_WIDTH) / 2;
    static const size_t start_y = (DISPLAY_HEIGHT - PLANE_HEIGHT) * 7 / 8;

    GameField_Init(&game_field, map, DISPLAY_BUFFER_SIZE, DISPLAY_WIDTH, DISPLAY_HEIGHT, (struct Plane)
    {
        .x       = start_x,
        .y       = start_y,
        .width   = PLANE_WIDTH,
        .height  = PLANE_HEIGHT,
        .texture = plane_texture
    });

    Joystick joystick;
    Joystick_Init(&joystick, &hadc, JOYSTICK_CHANNEL_X, JOYSTICK_CHANNEL_Y);

    while (true)
    {
        const int16_t dx = Joystick_ReadX(&joystick);
        const int16_t dy = Joystick_ReadY(&joystick);

        xprintf("dx = %d; dy = %d;\r\n", dx, dy);

        GameField_MovePlane(&game_field, dx, dy);
        GameField_MoveAsteroids(&game_field);
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

static void USART_Init(void)
{
    husart0.Instance = UART_0;
    husart0.transmitting = Enable;
    husart0.receiving = Enable;
    husart0.frame = Frame_8bit;
    husart0.parity_bit = Disable;
    husart0.parity_bit_inversion = Disable;
    husart0.bit_direction = LSB_First;
    husart0.data_inversion = Disable;
    husart0.tx_inversion = Disable;
    husart0.rx_inversion = Disable;
    husart0.swap = Disable;
    husart0.lbm = Disable;
    husart0.stop_bit = StopBit_1;
    husart0.mode = Asynchronous_Mode;
    husart0.xck_mode = XCK_Mode3;
    husart0.last_byte_clock = Disable;
    husart0.overwrite = Disable;
    husart0.rts_mode = AlwaysEnable_mode;
    husart0.dma_tx_request = Disable;
    husart0.dma_rx_request = Disable;
    husart0.channel_mode = Duplex_Mode;
    husart0.tx_break_mode = Disable;
    husart0.Interrupt.ctsie = Disable;
    husart0.Interrupt.eie = Disable;
    husart0.Interrupt.idleie = Disable;
    husart0.Interrupt.lbdie = Disable;
    husart0.Interrupt.peie = Disable;
    husart0.Interrupt.rxneie = Disable;
    husart0.Interrupt.tcie = Disable;
    husart0.Interrupt.txeie = Disable;
    husart0.Modem.rts = Disable; //out
    husart0.Modem.cts = Disable; //in
    husart0.Modem.dtr = Disable; //out
    husart0.Modem.dcd = Disable; //in
    husart0.Modem.dsr = Disable; //in
    husart0.Modem.ri = Disable;  //in
    husart0.Modem.ddis = Disable;//out
    husart0.baudrate = 115200;

    HAL_USART_Init(&husart0);
}

static void GPIO_Init(void)
{
    __HAL_PCC_GPIO_0_CLK_ENABLE();
    __HAL_PCC_GPIO_1_CLK_ENABLE();
    __HAL_PCC_GPIO_2_CLK_ENABLE();
    __HAL_PCC_GPIO_IRQ_CLK_ENABLE();
    __HAL_PCC_ANALOG_REGS_CLK_ENABLE();
    // __HAL_PCC_EPIC_CLK_ENABLE();

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
        .Pull = HAL_GPIO_PULL_NONE,
        .DS   = HAL_GPIO_DS_2MA
    };

    HAL_GPIO_Init(sw_pin.gpio, &gpio_sw);

    HAL_EPIC_MaskEdgeSet(HAL_EPIC_GPIO_IRQ_MASK);
    HAL_GPIO_InitInterruptLine(GPIO_MUX_LINE_2_PORT1_2, GPIO_INT_MODE_RISING);

    HAL_IRQ_EnableInterrupts();
}

static void ADC_Init(void)
{
    hadc.Instance    = ANALOG_REG;
    hadc.Init.EXTRef = ADC_EXTREF_OFF;
    hadc.Init.EXTClb = ADC_EXTCLB_CLBREF;

    HAL_ADC_Init(&hadc);
}

// static void Shoot_TrapHandler_Raw(void)
// {
//     Shoot_TrapHandler();
// }

// void Shoot_TrapHandler(void)
// {
//     if (EPIC_CHECK_GPIO_IRQ())
//     {
//         GameField_Shoot(&game_field);
//     }
// }