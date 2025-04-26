#ifndef SSD1306_H
#define SSD1306_h

#include <stdint.h>
#include <stddef.h>
#include <mik32_hal_spi.h>
#include "GPIO_Pin.h"

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define SSD1306_BUFFER_SIZE ((SCREEN_WIDTH * SCREEN_HEIGHT) / 8)

struct __SSD1306;
typedef struct __SSD1306* SSD1306;

void SSD1306_Init(SSD1306* hssd1306, SPI_HandleTypeDef* hspi, struct GPIO_Pin pins[5]);
void SSD1306_DrawFrame(SSD1306* hssd1306, uint8_t* buffer, size_t size);

#endif