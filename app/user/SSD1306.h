#ifndef SSD1306_H
#define SSD1306_h

#include <stdint.h>
#include <stddef.h>
#include <mik32_hal_spi.h>
#include "GPIO_Pin.h"

struct __SSD1306_HandleTypeDef;
typedef struct __SSD1306_HandleTypeDef* SSD1306_HandleTypeDef;

void SSD1306_Init(SSD1306_HandleTypeDef* hssd1306, SPI_HandleTypeDef* hspi, struct GPIO_Pin pins[5], size_t width, size_t height);
void SSD1306_SetSckPin(SSD1306_HandleTypeDef* hssd1306, struct GPIO_Pin pin);
void SSD1306_SetSdaPin(SSD1306_HandleTypeDef* hssd1306, struct GPIO_Pin pin);
void SSD1306_SetDcPin(SSD1306_HandleTypeDef* hssd1306, struct GPIO_Pin pin);
void SSD1306_SetCsPin(SSD1306_HandleTypeDef* hssd1306, struct GPIO_Pin pin);
void SSD1306_SetWidth(SSD1306_HandleTypeDef* hssd1306, size_t width);
void SSD1306_SetHeight(SSD1306_HandleTypeDef* hssd1306, size_t height);
void SSD1306_SendCommand(SSD1306_HandleTypeDef* hssd1306, SPI_HandleTypeDef* hspi, uint8_t command);
void SSD1306_SendData(SSD1306_HandleTypeDef* hssd1306, SPI_HandleTypeDef* hspi, uint8_t data);

#endif