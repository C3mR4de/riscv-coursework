#ifndef PLANE_H
#define PLANE_H

#include <stdint.h>
#include <stddef.h>

struct Plane
{
    size_t x;
    size_t y;
    const size_t width;
    const size_t height;
    const uint8_t** texture;
};

#endif