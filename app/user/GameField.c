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

static void   __GameField_SetPixel(GameField* game_field, size_t x, size_t y, bool state);
static size_t __GameField_Clamp(size_t value, size_t low, size_t high);

void GameField_Init(GameField* game_field, uint8_t* map, size_t size, size_t width, size_t height, struct Plane plane)
{
    *game_field = &fields[count++];

    *game_field[count - 1] = (struct __GameField)
    {
        .map    = map,
        .size   = size,
        .width  = width,
        .height = height,
        .plane  = plane
    };

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

void GameField_MovePlane(GameField* game_field, size_t dx, size_t dy)
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

    (*game_field)->plane.x = __GameField_Clamp((ptrdiff_t)(*game_field)->plane.x + (ptrdiff_t)dx, 0, (*game_field)->width - (*game_field)->plane.width);
    (*game_field)->plane.y = __GameField_Clamp((ptrdiff_t)(*game_field)->plane.y + (ptrdiff_t)dy, 0, (*game_field)->height - (*game_field)->plane.height);

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

static void __GameField_SetPixel(GameField* game_field, size_t x, size_t y, bool state)
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

static size_t __GameField_Clamp(size_t value, size_t low, size_t high)
{
    return (ptrdiff_t)value < (ptrdiff_t)low ? low : value > high ? high : value;
}