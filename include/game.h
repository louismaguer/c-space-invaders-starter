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
#define MAX_HP 3

#define BULLET_WIDTH 10
#define BULLET_HEIGHT 20
#define BULLET_SPEED 600.0f

// Level 1 values

#define ENEMIES_NUMBER_PER_LINE 5
#define ENEMIES_NUMBER_PER_COLUMN 2
#define ENEMIES_NUMBER (ENEMIES_NUMBER_PER_LINE * ENEMIES_NUMBER_PER_COLUMN)
#define TIME_BETWEEN_SHOTS 2.5f

#define ENEMY_WIDTH 20
#define ENEMY_HEIGHT 20
#define ENEMY_SPEED 10.0f
#define SPEED_INCREMENT 2.5f
#define TIME_BETWEEN_ACCELERATIONS 2.5f

#define FAST_ENEMIES_RATIO 0.2f
#define TOUGH_ENEMIES_RATIO 0.2f
#define SHOOTING_ENEMIES_RATIO 0.1f

#define FAST_ENEMIES_NUMBER (int)(ENEMIES_NUMBER*FAST_ENEMIES_RATIO)
#define TOUGH_ENEMIES_NUMBER (int)(ENEMIES_NUMBER*TOUGH_ENEMIES_RATIO)
#define SHOOTING_ENEMIES_NUMBER (int)(ENEMIES_NUMBER*SHOOTING_ENEMIES_RATIO)

#define FAST_ENEMY_SPEED_MULTPLIER 2.0f
#define TOUGH_ENEMY_HP 3

#define BAR_WIDTH 250
#define BAR_HEIGHT 25
#define BAR_DISTANCE_TOP 25

#define HEART_WIDTH 20
#define HEARTH_HEIGHT 20
#define TIME_BETWEEN_HEART_ATTEMPTS 2.5f
#define HEART_CHANCE 0.5
#define HEART_SPEED 100.0f

typedef enum{
        RUNNING,
        VICTORY,
        DEFEAT,
        MENU,
        PAUSE
    } Game_States;

bool init(SDL_Window **window, SDL_Renderer **renderer);
void handle_input(bool *running, const Uint8 *keys, Entity *player, Entity *bullet, bool *bullet_active);
void update(Entity *player, Entity *enemies, size_t *enemies_count, Entity *bullet, bool *bullet_active, Entity *bullet_enemies, bool *bullet_enemies_active, Entity *heart, bool *heart_active, size_t *shooting_enemies_count, bool *next_is_shooting_enemy, float *time_since_last_shot, float *time_since_last_acceleration, float *time_since_last_heart_attempt, float dt, bool *running, Game_States *game_state, size_t *level, size_t *enemies_number_tot);
void render(SDL_Renderer *renderer, Entity *player, Entity *enemies, Entity *bullet, bool bullet_active, Entity *bullet_enemies, bool bullet_enemies_active, Entity *heart, bool heart_active, Game_States *game_state, bool *running, size_t *level, bool *reset_game, size_t *enemies_number_tot);
void cleanup(SDL_Window *window, SDL_Renderer *renderer);
void save_game(size_t level);
int load_game(size_t *level);
int menu(SDL_Window *window);
void reset(Entity *player, Entity *enemies, size_t *enemies_count, Entity *bullet, bool *bullet_active, Entity *bullet_enemies, bool *bullet_enemies_active, Entity *heart, bool *heart_active, size_t *shooting_enemies_count, bool *next_is_shooting_enemy, float *time_since_last_shot, float *time_since_last_acceleration, float *time_since_last_heart_attempt, float dt, bool *running, Game_States *game_state, size_t *level, size_t *enemies_number_tot);

#endif
