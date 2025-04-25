#include "GameField.h"
#include <stdbool.h>

struct __GameField
{
    uint8_t*     map;
    size_t       size;
    size_t       width;
    size_t       height;
    struct Plane plane;
};

#define MAX_FIELDS 1

static struct __GameField fields[MAX_FIELDS];
static size_t count;

static void GameField_SetPixel(GameField* game_field, size_t x, size_t y, bool state);

void GameField_Init(GameField* game_field, uint8_t* map, size_t size, size_t width, size_t height, struct Plane plane)
{
    *game_field = &fields[count++];

    *game_field[count - 1] = (struct __GameField)
    {
        map,
        size,
        width,
        height,
        plane
    };

    for (size_t i = 0; i < (*game_field)->plane.height; ++i)
    {
        for (size_t j = 0; j < (*game_field)->plane.width; ++j)
        {
            const size_t x     = (*game_field)->plane.x + j;
            const size_t y     = (*game_field)->plane.y + i;
            const bool   state = (*game_field)->plane.texture[i * (*game_field)->plane.width + j] == 0x31;

            GameField_SetPixel(game_field, x, y, state);
        }
    }
}

void GameField_MovePlane(GameField* game_field, size_t dx, size_t dy)
{
    for (size_t i = 0; i < (*game_field)->plane.height; ++i)
    {
        for (size_t j = 0; j < (*game_field)->plane.width; ++j)
        {
            GameField_SetPixel(game_field, (*game_field)->plane.x + j, (*game_field)->plane.y + i, false);
        }
    }

    (*game_field)->plane.x += dx;
    (*game_field)->plane.y += dy;

    for (size_t i = 0; i < (*game_field)->plane.height; ++i)
    {
        for (size_t j = 0; j < (*game_field)->plane.width; ++j)
        {
            GameField_SetPixel(game_field, (*game_field)->plane.x + j, (*game_field)->plane.y + i, (*game_field)->plane.texture[i * (*game_field)->plane.height + j] == 0x31);
        }
    }
}

static void GameField_SetPixel(GameField* game_field, size_t x, size_t y, bool state)
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