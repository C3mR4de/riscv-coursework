#include "GameField.h"
#include <stdbool.h>
#include <stdlib.h>
#include "Asteroid.h"
#include "Bullet.h"

#define MAX_ASTEROIDS 10
#define MAX_BULLETS   30

struct __GameField
{
    uint8_t*        map;
    size_t          size;
    size_t          width;
    size_t          height;
    struct Plane    plane;
    struct Asteroid asteroids[MAX_ASTEROIDS];
    struct Bullet   bullets[MAX_BULLETS];
};

#define MAX_FIELDS 1

static struct __GameField fields[MAX_FIELDS];
static size_t count;

static void   __GameField_SetPixel(GameField* game_field, size_t x, size_t y, bool state);
static void   __GameField_ClearPlane(GameField* game_field);
static void   __GameField_FillPlane(GameField* game_field);
static void   __GameField_ClearAsteroid(GameField* game_field, size_t index);
static void   __GameField_FillAsteroid(GameField* game_field, size_t index);
static void   __GameField_SetBullet(GameField* game_field, size_t index, bool enable);
static void   __GameField_ClearBullet(GameField* const game_field, const size_t index);
static void   __GameField_FillBullet(GameField* const game_field, const size_t index);
static void   __GameField_RespawnAsteroid(GameField* game_field, size_t index);
static size_t __GameField_Clamp(ptrdiff_t value, ptrdiff_t low, ptrdiff_t high);
static bool   __GameField_Collides(struct Plane plane, struct Asteroid asteroid);
static bool   __GameField_IsShot(struct Asteroid asteroid, struct Bullet bullet);

void GameField_Init(GameField* const game_field, uint8_t* const map, const size_t size, const size_t width, const size_t height, const struct Plane plane)
{
    *game_field = &fields[count++];

    fields[count - 1] = (struct __GameField)
    {
        .map    = map,
        .size   = size,
        .width  = width,
        .height = height,
        .plane  = plane
    };

    srand(0);

    for (size_t i = 0; i < MAX_ASTEROIDS; ++i)
    {
        __GameField_RespawnAsteroid(game_field, i);
    }

    __GameField_FillPlane(game_field);
}

void GameField_MovePlane(GameField* const game_field, const ptrdiff_t dx, const ptrdiff_t dy)
{
    __GameField_ClearPlane(game_field);

    (*game_field)->plane.x = __GameField_Clamp((*game_field)->plane.x + dx, 0, (*game_field)->width - (*game_field)->plane.width);
    (*game_field)->plane.y = __GameField_Clamp((*game_field)->plane.y + dy, 0, (*game_field)->height - (*game_field)->plane.height);

    __GameField_FillPlane(game_field);
}

void GameField_MoveAsteroids(GameField* const game_field)
{
    for (size_t i = 0; i < MAX_ASTEROIDS; ++i)
    {
        __GameField_ClearAsteroid(game_field, i);

        ++(*game_field)->asteroids[i].y;

        if ((*game_field)->asteroids[i].y > (ptrdiff_t)(*game_field)->height)
        {
            __GameField_RespawnAsteroid(game_field, i);
        }

        __GameField_FillAsteroid(game_field, i);
    }
}

void GameField_MoveBullets(GameField* const game_field, const bool shot)
{
    for (size_t i = 0; i < MAX_BULLETS; ++i)
    {
        if ((*game_field)->bullets[i].is_active)
        {
            __GameField_ClearBullet(game_field, i);

            (*game_field)->bullets[i].y -= 4;

            if ((*game_field)->bullets[i].y + BULLET_HEIGHT < 0)
            {
                (*game_field)->bullets[i].is_active = false;
            }

            __GameField_FillBullet(game_field, i);
        }
    }

    if (shot)
    {
        for (size_t i = 0; i < MAX_BULLETS; ++i)
        {
            if (!(*game_field)->bullets[i].is_active)
            {
                (*game_field)->bullets[i].x = (*game_field)->plane.x + (*game_field)->plane.width / 2 - BULLET_WIDTH / 2;
                (*game_field)->bullets[i].y = (*game_field)->plane.y;
                (*game_field)->bullets[i].is_active = true;

                break;
            }
        }
    }
}

void GameField_CheckCollisions(GameField* game_field)
{
    for (size_t i = 0; i < MAX_ASTEROIDS; ++i)
    {
        if (__GameField_Collides((*game_field)->plane, (*game_field)->asteroids[i]))
        {
            __GameField_ClearAsteroid(game_field, i);
            __GameField_ClearPlane(game_field);
            __GameField_FillPlane(game_field);
            __GameField_RespawnAsteroid(game_field, i);
        }

        for (size_t j = 0; j < MAX_BULLETS; ++j)
        {
            if ((*game_field)->bullets[j].is_active && __GameField_IsShot((*game_field)->asteroids[i], (*game_field)->bullets[j]))
            {
                __GameField_ClearAsteroid(game_field, i);
                __GameField_ClearBullet(game_field, j);

                __GameField_RespawnAsteroid(game_field, i);
                (*game_field)->bullets[j].is_active = false;
            }
        }
    }
}

static void __GameField_SetPixel(GameField* const game_field, const size_t x, const size_t y, const bool state)
{
    if (x < (*game_field)->width && y < (*game_field)->height)
    {
        if (state)
        {
            (*game_field)->map[x + (y / 8) * (*game_field)->width] |= (1 << (y % 8));
        }
        else
        {
            (*game_field)->map[x + (y / 8) * (*game_field)->width] &= ~(1 << (y % 8));
        }
    }
}

static void __GameField_ClearPlane(GameField* const game_field)
{
    for (size_t i = 0; i < (*game_field)->plane.height; ++i)
    {
        for (size_t j = 0; j < (*game_field)->plane.width; ++j)
        {
            const size_t x = (*game_field)->plane.x + j;
            const size_t y = (*game_field)->plane.y + i;

            __GameField_SetPixel(game_field, x, y, false);
        }
    }
}

static void __GameField_FillPlane(GameField* const game_field)
{
    for (size_t i = 0; i < (*game_field)->plane.height; ++i)
    {
        for (size_t j = 0; j < (*game_field)->plane.width; ++j)
        {
            const size_t x     = (*game_field)->plane.x + j;
            const size_t y     = (*game_field)->plane.y + i;
            const bool   state = (*game_field)->plane.texture[i * (*game_field)->plane.width + j];

            __GameField_SetPixel(game_field, x, y, state);
        }
    }
}

static void __GameField_ClearAsteroid(GameField* const game_field, const size_t index)
{
    for (size_t i = 0; i < ASTEROID_HEIGHT; ++i)
    {
        for (size_t j = 0; j < ASTEROID_WIDTH; ++j)
        {
            const size_t x = (*game_field)->asteroids[index].x + j;
            const size_t y = (*game_field)->asteroids[index].y + i;

            __GameField_SetPixel(game_field, x, y, false);
        }
    }
}

static void __GameField_FillAsteroid(GameField* const game_field, const size_t index)
{
    for (size_t i = 0; i < ASTEROID_HEIGHT; ++i)
    {
        for (size_t j = 0; j < ASTEROID_WIDTH; ++j)
        {
            const size_t x     = (*game_field)->asteroids[index].x + j;
            const size_t y     = (*game_field)->asteroids[index].y + i;
            const bool   state = asteroid_texture[i * ASTEROID_WIDTH + j];

            __GameField_SetPixel(game_field, x, y, state);
        }
    }
}

static void __GameField_SetBullet(GameField* const game_field, const size_t index, const bool enable)
{
    for (size_t i = 0; i < BULLET_HEIGHT; ++i)
    {
        for (size_t j = 0; j < BULLET_WIDTH; ++j)
        {
            const size_t x = (*game_field)->bullets[index].x + j;
            const size_t y = (*game_field)->bullets[index].y + i;

            __GameField_SetPixel(game_field, x, y, enable);
        }
    }
}

static void __GameField_ClearBullet(GameField* const game_field, const size_t index)
{
    __GameField_SetBullet(game_field, index, false);
}

static void __GameField_FillBullet(GameField* const game_field, const size_t index)
{
    __GameField_SetBullet(game_field, index, true);
}

static void __GameField_RespawnAsteroid(GameField* const game_field, const size_t index)
{
    (*game_field)->asteroids[index].x = rand() % ((*game_field)->width - ASTEROID_WIDTH);
    (*game_field)->asteroids[index].y = rand() % ((*game_field)->height - ASTEROID_HEIGHT) - (*game_field)->height;
}

static size_t __GameField_Clamp(const ptrdiff_t value, const ptrdiff_t low, const ptrdiff_t high)
{
    return value < low ? low : value > high ? high : value;
}

static bool __GameField_Collides(const struct Plane plane, const struct Asteroid asteroid)
{
    return plane.x + (ptrdiff_t)plane.width  > asteroid.x && asteroid.x + ASTEROID_WIDTH  > plane.x
        && plane.y + (ptrdiff_t)plane.height > asteroid.y && asteroid.y + ASTEROID_HEIGHT > plane.y;
}

static bool __GameField_IsShot(const struct Asteroid asteroid, const struct Bullet bullet)
{
    return asteroid.x + ASTEROID_WIDTH  > bullet.x && bullet.x + BULLET_WIDTH  > asteroid.x
        && asteroid.y + ASTEROID_HEIGHT > bullet.y && bullet.y + BULLET_HEIGHT > asteroid.y;
}