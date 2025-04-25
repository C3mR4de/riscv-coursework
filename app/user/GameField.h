#ifndef GAME_FIELD_H
#define GAME_FIELD_H

#include <stddef.h>
#include "Plane.h"

struct GameField
{
    uint8_t* buffer;
    const size_t size;
    const size_t width;
    const size_t height;
    struct Plane plane;
};

void GameField_MovePlane(struct GameField* game_field, size_t dx, size_t dy);

#endif