#include "GameField.h"
#include <stdbool.h>
#include <stdlib.h>
#include "Asteroid.h"
#include "Bullet.h"

#define MAX_ASTEROIDS 10
#define MAX_BULLETS   30

struct __GameField
{
    uint8_t*         map;
    size_t           size;
    struct Vector2uz rect;
    struct Plane     plane;
    struct Asteroid  asteroids[MAX_ASTEROIDS];
    struct Bullet    bullets[MAX_BULLETS];
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
static bool   __GameField_Intersects(struct Vector2z lhs_position, struct Vector2uz lhs_rect, struct Vector2z rhs_position, struct Vector2uz rhs_rect);
static bool   __GameField_Collides(const struct Plane plane, const struct Asteroid asteroid);
static bool   __GameField_IsShot(const struct Asteroid asteroid, const struct Bullet bullet);

void GameField_Init(GameField* const game_field, uint8_t* const map, const size_t size, const struct Vector2uz rect, const struct Plane plane)
{
    *game_field = &fields[count++];

    fields[count - 1] = (struct __GameField)
    {
        .map   = map,
        .size  = size,
        .rect  = rect,
        .plane = plane
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

    (*game_field)->plane.position.x = __GameField_Clamp((*game_field)->plane.position.x + dx, 0, (*game_field)->rect.x - (*game_field)->plane.rect.x);
    (*game_field)->plane.position.y = __GameField_Clamp((*game_field)->plane.position.y + dy, 0, (*game_field)->rect.y - (*game_field)->plane.rect.y);

    __GameField_FillPlane(game_field);
}

void GameField_MoveAsteroids(GameField* const game_field)
{
    for (size_t i = 0; i < MAX_ASTEROIDS; ++i)
    {
        __GameField_ClearAsteroid(game_field, i);

        ++(*game_field)->asteroids[i].position.y;

        if ((*game_field)->asteroids[i].position.y > (ptrdiff_t)(*game_field)->rect.y)
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

            (*game_field)->bullets[i].position.y -= 4;

            if ((*game_field)->bullets[i].position.y + BULLET_HEIGHT < 0)
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
                (*game_field)->bullets[i].position.x = (*game_field)->plane.position.x + (*game_field)->plane.rect.x / 2 - BULLET_WIDTH / 2;
                (*game_field)->bullets[i].position.y = (*game_field)->plane.position.y;
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
    if (x < (*game_field)->rect.x && y < (*game_field)->rect.y)
    {
        if (state)
        {
            (*game_field)->map[x + (y / 8) * (*game_field)->rect.x] |= (1 << (y % 8));
        }
        else
        {
            (*game_field)->map[x + (y / 8) * (*game_field)->rect.x] &= ~(1 << (y % 8));
        }
    }
}

static void __GameField_ClearPlane(GameField* const game_field)
{
    for (size_t i = 0; i < (*game_field)->plane.rect.y; ++i)
    {
        for (size_t j = 0; j < (*game_field)->plane.rect.x; ++j)
        {
            const size_t x = (*game_field)->plane.position.x + j;
            const size_t y = (*game_field)->plane.position.y + i;

            __GameField_SetPixel(game_field, x, y, false);
        }
    }
}

static void __GameField_FillPlane(GameField* const game_field)
{
    for (size_t i = 0; i < (*game_field)->plane.rect.y; ++i)
    {
        for (size_t j = 0; j < (*game_field)->plane.rect.x; ++j)
        {
            const size_t x     = (*game_field)->plane.position.x + j;
            const size_t y     = (*game_field)->plane.position.y + i;
            const bool   state = (*game_field)->plane.texture[i * (*game_field)->plane.rect.x + j];

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
            const size_t x = (*game_field)->asteroids[index].position.x + j;
            const size_t y = (*game_field)->asteroids[index].position.y + i;

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
            const size_t x     = (*game_field)->asteroids[index].position.x + j;
            const size_t y     = (*game_field)->asteroids[index].position.y + i;
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
            const size_t x = (*game_field)->bullets[index].position.x + j;
            const size_t y = (*game_field)->bullets[index].position.y + i;

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
    (*game_field)->asteroids[index].position.x = rand() % ((*game_field)->rect.x - ASTEROID_WIDTH);
    (*game_field)->asteroids[index].position.y = rand() % ((*game_field)->rect.y - ASTEROID_HEIGHT) - (*game_field)->rect.y;
}

static size_t __GameField_Clamp(const ptrdiff_t value, const ptrdiff_t low, const ptrdiff_t high)
{
    return value < low ? low : value > high ? high : value;
}

static bool __GameField_Intersects(const struct Vector2z lhs_position, const struct Vector2uz lhs_rect, const struct Vector2z rhs_position, const struct Vector2uz rhs_rect)
{
    return lhs_position.x + lhs_rect.x > rhs_position.x && rhs_position.x + rhs_rect.x > lhs_position.x
        && lhs_position.y + lhs_rect.y > rhs_position.y && rhs_position.y + rhs_rect.y > lhs_position.y;
}

static bool __GameField_Collides(const struct Plane plane, const struct Asteroid asteroid)
{
    return __GameField_Intersects(plane.position, plane.rect, asteroid.position, (struct Vector2uz){ASTEROID_WIDTH, ASTEROID_HEIGHT});
}

static bool __GameField_IsShot(const struct Asteroid asteroid, const struct Bullet bullet)
{
    return __GameField_Intersects(asteroid.position, (struct Vector2uz){ASTEROID_WIDTH, ASTEROID_HEIGHT}, bullet.position, (struct Vector2uz){BULLET_WIDTH, BULLET_HEIGHT});
}