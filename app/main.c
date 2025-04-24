#include <stdbool.h>
#include <SSD1306.h>

/*
#define SCR1_TIMER_GET_TIME() \
    (((uint64_t)(SCR1_TIMER->MTIMEH) << 32) | (SCR1_TIMER->MTIME))

#define SYSTEM_FREQ_HZ 32000000UL

#define SET_TWO_BIT(REG, NUM, TWO_BITS) \
    do { (REG) = (((REG) & ~(PAD_CONFIG_PIN_M(NUM))) | (PAD_CONFIG_PIN(NUM, TWO_BITS))); } while (false)
*/

static void SystemClock_Config(void);
static void SPI_Init(void);
static void GPIO_Init(void);

static SPI_HandleTypeDef hspi;
static struct GPIO_Pin sck_pin = (struct GPIO_Pin){GPIO_0, GPIO_PIN_2}; // Пин D6 (SCK, D0)
static struct GPIO_Pin sda_pin = (struct GPIO_Pin){GPIO_0, GPIO_PIN_1}; // Пин D5 (MOSI, D1)
static struct GPIO_Pin res_pin = (struct GPIO_Pin){GPIO_0, GPIO_PIN_5}; // Пин D0
static struct GPIO_Pin dc_pin  = (struct GPIO_Pin){GPIO_0, GPIO_PIN_6}; // Пин D1
static struct GPIO_Pin cs_pin  = (struct GPIO_Pin){GPIO_0, GPIO_PIN_4}; // Пин A2

#define BUFFER_SIZE   1024
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64

static uint8_t    buffer[BUFFER_SIZE];
static const char plane[11][15] =
{
    "000000010000000",
    "000000010000000",
    "000000111000000",
    "000001111100000",
    "000111111111000",
    "011111111111110",
    "111111010111111",
    "111000010000111",
    "110000010000011",
    "100000000000001",
    "100000000000001"
};

int main()
{
    SystemClock_Config();
    SPI_Init();
    GPIO_Init();

    SSD1306_HandleTypeDef display;
    SSD1306_Init(&display, &hspi, (struct GPIO_Pin[5]){sck_pin, sda_pin, res_pin, dc_pin, cs_pin}, SCREEN_WIDTH, SCREEN_HEIGHT);

    while (true)
    {
        for (size_t y = 0; y < 11; ++y)
        {
            for (size_t x = 0; x < 15; ++x)
            {
                if (plane[y][x] == '1')
                {
                    buffer[x + (y / 8) * SCREEN_WIDTH] |= (1 << (y % 8));
                }
                else
                {
                    buffer[x + (y / 8) * SCREEN_WIDTH] &= ~(1 << (y % 8));
                }
            }
        }

        SSD1306_DrawFrame(&display, buffer, BUFFER_SIZE);
    }
}

static void SystemClock_Config(void)
{
    WU->CLOCKS_SYS &=
        ~(0b11 << WU_CLOCKS_SYS_OSC32M_EN_S); // Включить OSC32M и HSI32M
    WU->CLOCKS_BU &=
        ~(0b11 << WU_CLOCKS_BU_OSC32K_EN_S); // Включить OSC32K и LSI32K
  
    // Поправочный коэффициент HSI32M
    WU->CLOCKS_SYS = (WU->CLOCKS_SYS & (~WU_CLOCKS_SYS_ADJ_HSI32M_M)) |
                      WU_CLOCKS_SYS_ADJ_HSI32M(128);

    // Поправочный коэффициент LSI32K
    WU->CLOCKS_BU = (WU->CLOCKS_BU & (~WU_CLOCKS_BU_ADJ_LSI32K_M)) |
                     WU_CLOCKS_BU_ADJ_LSI32K(8);

    // Автоматический выбор источника опорного тактирования
    WU->CLOCKS_SYS &= ~WU_CLOCKS_SYS_FORCE_32K_CLK_M;

    // ожидание готовности
    while (!(PM->FREQ_STATUS & PM_FREQ_STATUS_OSC32M_M));

    // переключение на тактирование от OSC32M
    PM->AHB_CLK_MUX = PM_AHB_CLK_MUX_OSC32M_M | PM_AHB_FORCE_MUX_UNFIXED;
    PM->DIV_AHB = 0;   // Задать делитель шины AHB.
    PM->DIV_APB_M = 0; // Задать делитель шины APB_M.
    PM->DIV_APB_P = 0; // Задать делитель шины APB_P.
}

static void SPI_Init(void)
{
    SPI_InitTypeDef spi_init =
    {
        .SPI_Mode = HAL_SPI_MODE_MASTER,
        .BaudRateDiv = SPI_BAUDRATE_DIV64,
        .ManualCS = SPI_MANUALCS_OFF,
        .CLKPhase = SPI_PHASE_OFF,
        .CLKPolarity = SPI_POLARITY_LOW,
        .Decoder = SPI_DECODER_NONE,
        .ChipSelect = SPI_CS_0
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
    /**< Включить  тактирование GPIO_0 */
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_0_M;

    /**< Включить  тактирование GPIO_1 */
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_1_M;

    /**< Включить  тактирование GPIO_2 */
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_2_M;

    /**< Включить  тактирование схемы формирования прерываний GPIO */
    PM->CLK_APB_P_SET = PM_CLOCK_APB_P_GPIO_IRQ_M;

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