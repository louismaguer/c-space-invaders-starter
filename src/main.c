#include <SDL.h>
#include <stdbool.h>
#include "entity.h"
#include "game.h"

int main(void){
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (!init(&window, &renderer)){
        return 1;
    }

    srand(time(NULL));

    bool running = true;
    Uint32 last_ticks = SDL_GetTicks();

    Entity player = {
        .x = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2,
        .y = SCREEN_HEIGHT - 60,
        .w = PLAYER_WIDTH,
        .h = PLAYER_HEIGHT,
        .vx = 0,
        .vy = 0,
        .hp = MAX_HP};

    Entity bullet = {0};
    bool bullet_active = false;

    Entity bullet_enemies = {0};
    bool bullet_enemies_active = false;
    float time_since_last_shot = 0;
    float time_since_last_shot_player = 0;

    Entity heart = {0};
    bool heart_active = false;
    float time_since_last_heart_attempt = 0;

    size_t enemies_count = ENEMIES_NUMBER;
    Entity enemies[ENEMIES_NUMBER*100];

    for (size_t i=0; i<ENEMIES_NUMBER_PER_COLUMN; i++){
        for (size_t j=0; j<ENEMIES_NUMBER_PER_LINE; j++){
            enemies[i*ENEMIES_NUMBER_PER_LINE + j] = (Entity){
                .enemy_type = BASIC_ENEMY,
                .alive = true,
                .x = SCREEN_WIDTH/ENEMIES_NUMBER_PER_LINE * (j+0.5) - ENEMY_WIDTH/2,
                .y = SCREEN_HEIGHT/(2*ENEMIES_NUMBER_PER_COLUMN) * i,
                .w = ENEMY_WIDTH,
                .h = ENEMY_HEIGHT,
                .vx = 0,
                .vy = ENEMY_SPEED,
                .hp = 1};
        }
    }

// Selection of fast enemies
size_t f = FAST_ENEMIES_NUMBER;
while (f > 0){
    int c = rand() % ENEMIES_NUMBER;
    if (enemies[c].enemy_type == BASIC_ENEMY){
        enemies[c].enemy_type = FAST_ENEMY;
        enemies[c].vy *= FAST_ENEMY_SPEED_MULTPLIER;
        f--;
    }
}

// Selection of tough enemies
size_t t = TOUGH_ENEMIES_NUMBER;
while (t > 0){
    int c = rand() % ENEMIES_NUMBER;
    if (enemies[c].enemy_type == BASIC_ENEMY){
        enemies[c].enemy_type = TOUGH_ENEMY;
        enemies[c].hp = TOUGH_ENEMY_HP;
        t--;
    }
}

// Selection of shooting enemies
size_t s = SHOOTING_ENEMIES_NUMBER;
while (s > 0){
    int c = rand() % ENEMIES_NUMBER;
    if (enemies[c].enemy_type == BASIC_ENEMY){
        enemies[c].enemy_type = SHOOTING_ENEMY;
        s--;
    }
}

size_t shooting_enemies_count = SHOOTING_ENEMIES_NUMBER;
bool next_is_shooting_enemy = true;

float time_since_last_acceleration = 0;

Game_States game_state = MENU;
size_t level = 1;
size_t enemies_number_tot = ENEMIES_NUMBER;
bool reset_game = false;

while (game_state == MENU){
    int choice = menu(window, &level);
    if (choice == 0){
        level = 1;
        game_state = RUNNING;
    }
    else if (choice == 1){
        switch(load_game(&level)){
            case 2:
                game_state = RUNNING;
                reset_game = true;
                break;
            case 1:
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fichier de sauvegarde invalide", "", window);
                break;
            case 0:
                SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fichier de sauvegarde absent", "", window);
                break;
        }
    }
    else{
        running = false;
        cleanup(window, renderer);
        return 0;
    }
}

    while (running){
        Uint32 ticks = SDL_GetTicks();
        float dt = (ticks - last_ticks) / 1000.0f;
        if (dt > 0.05f)
            dt = 0.05f;
        last_ticks = ticks;

        time_since_last_shot += dt;
        time_since_last_acceleration += dt;
        time_since_last_heart_attempt += dt;
        time_since_last_shot_player += dt;

        SDL_PumpEvents();
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        handle_input(&running, keys, &player, &bullet, &bullet_active, &game_state);
        update(&player, enemies, &enemies_count, &bullet, &bullet_active, &bullet_enemies, &bullet_enemies_active, &heart, &heart_active, &shooting_enemies_count, &next_is_shooting_enemy, &time_since_last_shot, &time_since_last_acceleration, &time_since_last_heart_attempt, dt, &running, &game_state, &level, &enemies_number_tot);
        render(renderer, &player, enemies, &bullet, bullet_active, &bullet_enemies, bullet_enemies_active, &heart, heart_active, &game_state, &running, &level, &reset_game, &enemies_number_tot);
        if (reset_game){
            reset(&player, enemies, &enemies_count, &bullet, &bullet_active, &bullet_enemies, &bullet_enemies_active, &heart, &heart_active, &shooting_enemies_count, &next_is_shooting_enemy, &time_since_last_shot, &time_since_last_acceleration, &time_since_last_heart_attempt, dt, &running, &game_state, &level, &enemies_number_tot);
            reset_game = false;
            game_state = RUNNING;
        }
    }

    cleanup(window, renderer);
    return 0;
}
