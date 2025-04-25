#ifndef GAME_FIELD_H
#define GAME_FIELD_H

#include <stddef.h>
#include "Plane.h"

struct __GameField;
typedef struct __GameField* GameField;

void GameField_Init(GameField* game_field, uint8_t* map, size_t size, size_t width, size_t height, struct Plane plane);
void GameField_MovePlane(GameField* game_field, size_t dx, size_t dy);

#endif