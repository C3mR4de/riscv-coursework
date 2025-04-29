#ifndef ASTEROID_H
#define ASTEROID_H

#include <stddef.h>
#include "Vector2.h"

#define ASTEROID_WIDTH  6
#define ASTEROID_HEIGHT 6

static const bool asteroid_texture[ASTEROID_WIDTH * ASTEROID_HEIGHT] =
{
    0, 1, 1, 1, 0, 0,
    0, 1, 0, 0, 1, 0,
    1, 0, 1, 0, 0, 1,
    1, 1, 0, 1, 1, 1,
    1, 1, 1, 1, 1, 1,
    0, 1, 1, 1, 1, 0
};


struct Asteroid
{
    struct Vector2z position;
};

#endif