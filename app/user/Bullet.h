#ifndef BULLET_H
#define BULLET_H

#include <stdbool.h>

#define BULLET_WIDTH 2
#define BULLET_HEIGHT 6

struct Bullet
{
    ptrdiff_t x;
    ptrdiff_t y;
    bool is_active;
};

#endif