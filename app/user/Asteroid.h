#ifndef ASTEROID_H
#define ASTEROID_H

#include <stddef.h>
#include "Vector2.h"

#define ASTEROID_WIDTH  6
#define ASTEROID_HEIGHT 6

static const bool asteroid_texture[ASTEROID_WIDTH * ASTEROID_HEIGHT] =
{
    false,  true,  true,  true, false, false,
    false,  true, false, false,  true, false,
     true, false,  true, false, false,  true,
     true, true, false,  true,  true,  true,
     true,  true,  true,  true,  true,  true,
    false,  true,  true,  true,  true, false,
};


struct Asteroid
{
    struct Vector2z position;
};

#endif