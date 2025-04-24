#ifndef SSD1306_H
#define SSD1306_h

#include <stdint.h>
#include <stddef.h>
#include <mik32_hal_spi.h>
#include "GPIO_Pin.h"

#define SSD1306_BUFFER_SIZE 1024

struct __SSD1306_HandleTypeDef;
typedef struct __SSD1306_HandleTypeDef* SSD1306_HandleTypeDef;

void SSD1306_Init(SSD1306_HandleTypeDef* hssd1306, SPI_HandleTypeDef* hspi, struct GPIO_Pin pins[5]);
void SSD1306_DrawFrame(SSD1306_HandleTypeDef* hssd1306, uint8_t* buffer, size_t size);

#endif