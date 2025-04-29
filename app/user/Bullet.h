#ifndef BULLET_H
#define BULLET_H

#include <stdbool.h>

#define BULLET_WIDTH 2
#define BULLET_HEIGHT 6

struct Bullet
{
    struct Vector2z position;
    bool is_active;
};

#endif