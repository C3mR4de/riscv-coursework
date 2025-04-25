#include "GameField.h"
#include <stdbool.h>

static void GameField_SetPixel(struct GameField* game_field, size_t x, size_t y, bool state);

void GameField_MovePlane(struct GameField* game_field, size_t dx, size_t dy)
{
    for (size_t i = 0; i < game_field->plane.height; ++i)
    {
        for (size_t j = 0; j < game_field->plane.width; ++j)
        {
            GameField_SetPixel(game_field, game_field->plane.x + j, game_field->plane.y + i, false);
        }
    }

    game_field->plane.x += dx;
    game_field->plane.y += dy;

    for (size_t i = 0; i < game_field->plane.height; ++i)
    {
        for (size_t j = 0; j < game_field->plane.width; ++j)
        {
            GameField_SetPixel(game_field, game_field->plane.x + j, game_field->plane.y + i, game_field->plane.texture[i][j]);
        }
    }
}

static void GameField_SetPixel(struct GameField* game_field, size_t x, size_t y, bool state)
{
    if (x < game_field->width && y < game_field->height && x >= 0 && y >= 0)
    {
        if (state)
        {
            game_field->buffer[x + (y / 8) * game_field->width] |= (1 << (y % 8));
        }
        else
        {
            game_field->buffer[x + (y / 8) * game_field->width] &= ~(1 << (y % 8));
        }
    }
}