#ifndef PLANE_H
#define PLANE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct Plane
{
    ptrdiff_t   x;
    ptrdiff_t   y;
    size_t      width;
    size_t      height;
    const bool* texture;
};

#endif