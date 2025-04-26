#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>
#include <stddef.h>
#include <mik32_hal_spi.h>
#include "GPIO_Pin.h"

#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 64
#define DISPLAY_BUFFER_SIZE ((DISPLAY_WIDTH * DISPLAY_HEIGHT) / 8)

struct __Display;
typedef struct __Display* Display;

void Display_Init(Display* display, SPI_HandleTypeDef* hspi, struct GPIO_Pin pins[5]);
void Display_DrawFrame(Display* display, uint8_t* buffer, size_t size);

#endif