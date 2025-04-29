#ifndef PLANE_H
#define PLANE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "Vector2.h"

struct Plane
{
    struct Vector2z  position;
    struct Vector2uz rect;
    const bool*      texture;
};

#endif