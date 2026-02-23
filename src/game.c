#include <SDL.h>
#include "game.h"
#include <stdio.h>

bool init(SDL_Window **window, SDL_Renderer **renderer){
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        SDL_Log("Erreur SDL_Init: %s", SDL_GetError());
        return false;
    }

    *window = SDL_CreateWindow("Space Invaders (SDL)", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
    if (!*window){
        SDL_Log("Erreur SDL_CreateWindow: %s", SDL_GetError());
        SDL_Quit();
        return false;
    }

    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!*renderer){
        SDL_Log("Erreur SDL_CreateRenderer: %s", SDL_GetError());
        SDL_DestroyWindow(*window);
        SDL_Quit();
        return false;
    }

    return true;
}

void handle_input(bool *running, const Uint8 *keys, Entity *player, Entity *bullet, bool *bullet_active){
    SDL_Event event;
    while (SDL_PollEvent(&event)){
        if (event.type == SDL_QUIT)
            *running = false;
    }

    player->vx = 0.0f;
    if (keys[SDL_SCANCODE_LEFT])
        player->vx = -PLAYER_SPEED;
    if (keys[SDL_SCANCODE_RIGHT])
        player->vx = PLAYER_SPEED;

    if (keys[SDL_SCANCODE_SPACE] && !*bullet_active){
        *bullet_active = true;
        bullet->x = player->x + player->w / 2 - BULLET_WIDTH / 2;
        bullet->y = player->y;
        bullet->w = BULLET_WIDTH;
        bullet->h = BULLET_HEIGHT;
        bullet->vy = -BULLET_SPEED;
    }
}

void update(Entity *player, Entity *enemies, size_t *enemies_count, Entity *bullet, bool *bullet_active, Entity *bullet_enemies, bool *bullet_enemies_active, Entity *heart, bool *heart_active, size_t *shooting_enemies_count, bool *next_is_shooting_enemy, float *time_since_last_shot, float *time_since_last_acceleration, float *time_since_last_heart_attempt, float dt, bool *running){
    player->x += player->vx * dt;

    if (player->x < 0)
        player->x = 0;
    if (player->x + player->w > SCREEN_WIDTH)
        player->x = SCREEN_WIDTH - player->w;

    if (*bullet_active){
        bullet->y += bullet->vy * dt;
        if (bullet->y + bullet->h < 0)
            *bullet_active = false;
    }

    if (*bullet_enemies_active){
        bullet_enemies->y += bullet_enemies->vy * dt;
        if (bullet_enemies->y + bullet_enemies->h > SCREEN_HEIGHT)
            *bullet_enemies_active = false;
    }

    if (*time_since_last_acceleration >= TIME_BETWEEN_ACCELERATIONS){
        *time_since_last_acceleration = 0;
        for (size_t i=0; i<ENEMIES_NUMBER; i++){
            enemies[i].vy += SPEED_INCREMENT;
        }
    }

    if (*time_since_last_heart_attempt >= TIME_BETWEEN_HEART_ATTEMPTS && !*heart_active){
        *time_since_last_heart_attempt = 0;
        if ((float)rand()/RAND_MAX <= HEART_CHANCE){
            *heart_active = true;
            heart->x = rand() % (SCREEN_WIDTH - HEART_WIDTH);
            heart->y = rand() % (SCREEN_HEIGHT - 60 - HEARTH_HEIGHT);
            heart->w = HEART_WIDTH;
            heart->h = HEARTH_HEIGHT;
            heart->vy = HEART_SPEED;
        }
    }

    if (*heart_active){
        heart->y += HEART_SPEED*dt;
        SDL_Rect * heart_rect = &(heart->rect);
        SDL_Rect * player_rect = &(player->rect);
        if (SDL_HasIntersection(heart_rect, player_rect)){
            if (player->hp < MAX_HP){
                player->hp += 1;
            }
            *heart_active = false;
        }
        if (heart->y > SCREEN_HEIGHT){
            *heart_active = false;
        }
    }
                
    for (size_t i=0; i<ENEMIES_NUMBER; i++){
        if (enemies[i].alive){
            enemies[i].y += enemies[i].vy*dt;
            if (enemies[i].y > SCREEN_HEIGHT - 60){
                *running = false;
                printf("DÉFAITE...");
                break;
            }
        }
        if (*bullet_active){
            SDL_Rect * bullet_rect = &(bullet->rect);
            SDL_Rect * enemy_rect = &(enemies[i].rect);
            if (enemies[i].alive && SDL_HasIntersection(bullet_rect, enemy_rect)){
                enemies[i].hp -= 1;
                if (enemies[i].hp == 0){
                    enemies[i].alive = false;
                    *enemies_count -= 1;
                }
                *bullet_active = false;
            }
        }
    }

    if (*enemies_count == 0){
        *running = false;
        printf("VICTOIRE !");
    }

    if (*bullet_enemies_active){
        SDL_Rect * bullet_enemies_rect = &(bullet_enemies->rect);
        SDL_Rect * player_rect = &(player->rect);
        if (SDL_HasIntersection(bullet_enemies_rect, player_rect)){
            *bullet_enemies_active = false;
            player->hp -= 1;
        }
    }
    
    if (player->hp == 0){
        *running = false;
        printf("DÉFAITE...");
    }

    if (*time_since_last_shot >= TIME_BETWEEN_SHOTS/2.0f && !*bullet_enemies_active){
        if (*next_is_shooting_enemy && *shooting_enemies_count > 0){
            size_t shooting_enemies_index[*enemies_count];
            size_t shooting_enemies_count_aux = 0;
            for (size_t i=0; i<ENEMIES_NUMBER; i++){
                if (enemies[i].alive && enemies[i].enemy_type == SHOOTING_ENEMY){
                    shooting_enemies_index[shooting_enemies_count_aux] = i;
                    shooting_enemies_count_aux++;
                }
            }
            if(shooting_enemies_count_aux > 0){
                size_t index_aux_shooting_enemy = rand() % shooting_enemies_count_aux;
                size_t index_shooting_enemy = shooting_enemies_index[index_aux_shooting_enemy];
                *bullet_enemies_active = true;
                bullet_enemies->x = enemies[index_shooting_enemy].x + enemies[index_shooting_enemy].w / 2 - BULLET_WIDTH / 2;
                bullet_enemies->y = enemies[index_shooting_enemy].y;
                bullet_enemies->w = BULLET_WIDTH;
                bullet_enemies->h = BULLET_HEIGHT;
                bullet_enemies->vy = BULLET_SPEED;
            }
            *shooting_enemies_count = shooting_enemies_count_aux;
            *next_is_shooting_enemy = false;
        }
        if (*time_since_last_shot >= TIME_BETWEEN_SHOTS){
            size_t index_alive[*enemies_count];
            size_t alive_count = 0;
            for (size_t i=0; i<ENEMIES_NUMBER; i++){
                if (enemies[i].alive){
                    index_alive[alive_count] = i;
                    alive_count++;
                }
            }
            if(alive_count > 0){
                size_t index_aux_enemy = rand() % alive_count;
                size_t index_enemy = index_alive[index_aux_enemy];
                *time_since_last_shot = 0;
                *next_is_shooting_enemy = true;
                *bullet_enemies_active = true;
                bullet_enemies->x = enemies[index_enemy].x + enemies[index_enemy].w / 2 - BULLET_WIDTH / 2;
                bullet_enemies->y = enemies[index_enemy].y;
                bullet_enemies->w = BULLET_WIDTH;
                bullet_enemies->h = BULLET_HEIGHT;
                bullet_enemies->vy = BULLET_SPEED;
            }
        }
    }
}

void render(SDL_Renderer *renderer, Entity *player, Entity *enemies, Entity *bullet, bool bullet_active, Entity *bullet_enemies, bool bullet_enemies_active, Entity *heart, bool heart_active){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect player_rect = {
        (int)player->x, (int)player->y,
        player->w, player->h};
        player->rect = player_rect;
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &player_rect);

    for (size_t i=0; i<ENEMIES_NUMBER; i++){
        if(enemies[i].alive){
            SDL_Rect enemy_rect = {
                (int)enemies[i].x, (int)enemies[i].y,
                enemies[i].w, enemies[i].h};
            enemies[i].rect = enemy_rect;
            switch (enemies[i].enemy_type){
                case BASIC_ENEMY:
                    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                    break;
                case FAST_ENEMY:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                    break;
                case TOUGH_ENEMY:
                    SDL_SetRenderDrawColor(renderer, 0, 100, 255, 255);
                    break;
                case SHOOTING_ENEMY:
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                    break;
            }
            SDL_RenderFillRect(renderer, &enemy_rect);
        }
    }

    if (heart_active){
        SDL_Rect heart_rect = {
            (int)heart->x, (int)heart->y,
            heart->w, heart->h};
        heart->rect = heart_rect;
        SDL_SetRenderDrawColor(renderer, 133, 6, 6, 255);
        SDL_RenderFillRect(renderer, &heart_rect);
    }

    if (bullet_active){
        SDL_Rect bullet_rect = {
            (int)bullet->x, (int)bullet->y,
            bullet->w, bullet->h};
        bullet->rect = bullet_rect;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &bullet_rect);
    }

    if (bullet_enemies_active){
        SDL_Rect bullet_enemies_rect = {
            (int)bullet_enemies->x, (int)bullet_enemies->y,
            bullet_enemies->w, bullet_enemies->h};
        bullet_enemies->rect = bullet_enemies_rect;
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &bullet_enemies_rect);
    }

int progression = (int)(((float)player->hp/MAX_HP)*BAR_WIDTH);

SDL_Rect hp_rect = {(SCREEN_WIDTH - BAR_WIDTH)/2, BAR_DISTANCE_TOP, progression, BAR_HEIGHT};
SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); 
SDL_RenderFillRect(renderer, &hp_rect);

SDL_Rect outline_rect = {(SCREEN_WIDTH - BAR_WIDTH)/2, BAR_DISTANCE_TOP, BAR_WIDTH, BAR_HEIGHT};
SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
SDL_RenderDrawRect(renderer, &outline_rect);

SDL_RenderPresent(renderer);
}

void cleanup(SDL_Window *window, SDL_Renderer *renderer){
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}
