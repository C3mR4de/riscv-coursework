#ifndef GAME_FIELD_H
#define GAME_FIELD_H

#include <stddef.h>
#include "Plane.h"

struct __GameField;
typedef struct __GameField* GameField;

void GameField_Init(GameField* game_field, uint8_t* map, size_t size, size_t width, size_t height, struct Plane plane);
void GameField_MovePlane(GameField* game_field, ptrdiff_t dx, ptrdiff_t dy);
void GameField_MoveAsteroids(GameField* game_field);
void GameField_Shoot(GameField* game_field);
void GameField_MoveBullets(GameField* game_field);

#endif