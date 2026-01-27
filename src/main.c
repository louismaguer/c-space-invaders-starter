#include <SDL.h>
#include <stdbool.h>
#include "entity.h"
#include "game.h"

int main(void)
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!init(&window, &renderer))
    {
        return 1;
    }

    bool running = true;
    Uint32 last_ticks = SDL_GetTicks();

    Entity player = {
        .x = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2,
        .y = SCREEN_HEIGHT - 60,
        .w = PLAYER_WIDTH,
        .h = PLAYER_HEIGHT,
        .vx = 0,
        .vy = 0};

    Entity bullet = {0};
    bool bullet_active = false;

    size_t enemies_count = ENEMIES_NUMBER;
    Entity enemies[ENEMIES_NUMBER];

    for(size_t i=0; i<ENEMIES_NUMBER_PER_COLUMN; i++){
        for(size_t j=0; j<ENEMIES_NUMBER_PER_LINE; j++){
            enemies[i*ENEMIES_NUMBER_PER_LINE + j] = (Entity){
                .alive = true,
                .x = SCREEN_WIDTH/ENEMIES_NUMBER_PER_LINE * (j+0.5) - ENEMY_WIDTH/2,
                .y = SCREEN_HEIGHT/(2*ENEMIES_NUMBER_PER_COLUMN) * i,
                .w = ENEMY_WIDTH,
                .h = ENEMY_HEIGHT,
                .vx = 0,
                .vy = ENEMY_SPEED};
        }
    }

    while (running)
    {
        Uint32 ticks = SDL_GetTicks();
        float dt = (ticks - last_ticks) / 1000.0f;
        if (dt > 0.05f)
            dt = 0.05f;
        last_ticks = ticks;

        SDL_PumpEvents();
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        handle_input(&running, keys, &player, &bullet, &bullet_active);
        update(&player, enemies, &enemies_count, &bullet, &bullet_active, dt, &running);
        render(renderer, &player, enemies, &bullet, bullet_active);
    }

    cleanup(window, renderer);
    return 0;
}
