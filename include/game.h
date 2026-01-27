#ifndef GAME_H
#define GAME_H

#include <SDL.h>
#include <stdbool.h>
#include "entity.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 20
#define PLAYER_SPEED 400.0f

#define BULLET_WIDTH 10
#define BULLET_HEIGHT 20
#define BULLET_SPEED 600.0f

#define ENEMIES_NUMBER_PER_LINE 5
#define ENEMIES_NUMBER_PER_COLUMN 2
#define ENEMIES_NUMBER (ENEMIES_NUMBER_PER_LINE * ENEMIES_NUMBER_PER_COLUMN)

#define ENEMY_WIDTH 20
#define ENEMY_HEIGHT 20
#define ENEMY_SPEED 10.0f

bool init(SDL_Window **window, SDL_Renderer **renderer);
void handle_input(bool *running, const Uint8 *keys, Entity *player, Entity *bullet, bool *bullet_active);
void update(Entity *player, Entity *enemies, size_t *enemies_count, Entity *bullet, bool *bullet_active, float dt, bool *running);
void render(SDL_Renderer *renderer, Entity *player, Entity *enemies, Entity *bullet, bool bullet_active);
void cleanup(SDL_Window *window, SDL_Renderer *renderer);

#endif
