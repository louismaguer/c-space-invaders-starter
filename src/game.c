#include <SDL.h>
#include "game.h"
#include <stdio.h>

// Vérification de l'initialisation la console
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

// Traitement des touches pressées
void handle_input(bool *running, const Uint8 *keys, Entity *player, Entity *bullet, bool *bullet_active, Game_States *game_state){
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

    if(keys[SDL_SCANCODE_ESCAPE] || keys[SDL_SCANCODE_P]){
        *game_state = PAUSE;
    }
}

// Mise à jour de la partie
void update(Entity *player, Entity *enemies, size_t *enemies_count, Entity *bullet, bool *bullet_active, Entity *bullet_enemies, bool *bullet_enemies_active, Entity *heart, bool *heart_active, size_t *shooting_enemies_count, bool *next_is_shooting_enemy, float *time_since_last_shot, float *time_since_last_acceleration, float *time_since_last_heart_attempt, float dt, Game_States *game_state, size_t *enemies_number_tot){
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

    if (!*bullet_active){
        *bullet = (Entity){0};
    }

    if (*bullet_enemies_active){
        bullet_enemies->y += bullet_enemies->vy * dt;
        if (bullet_enemies->y + bullet_enemies->h > SCREEN_HEIGHT)
            *bullet_enemies_active = false;
    }

    // Accélération progressive des ennemis
    if (*time_since_last_acceleration >= TIME_BETWEEN_ACCELERATIONS){
        *time_since_last_acceleration = 0;
        for (size_t i=0; i<*enemies_number_tot; i++){
            if (enemies[i].enemy_type == FAST_ENEMY){
                enemies[i].vy += FAST_ENEMY_SPEED_MULTPLIER * SPEED_INCREMENT;
            }
            else {
                enemies[i].vy += SPEED_INCREMENT;
            }
        }
    }

    // Apparation pseudo-aléatoire et mécanique des cœurs
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

    // Attribution des différents "Rect"
    SDL_Rect player_rect = {
        (int)player->x, (int)player->y,
        player->w, player->h};
    player->rect = player_rect;

    for (size_t i=0; i<*enemies_number_tot; i++){
        SDL_Rect enemy_rect = {
            (int)enemies[i].x, (int)enemies[i].y,
            enemies[i].w, enemies[i].h};
        enemies[i].rect = enemy_rect;
    }

    SDL_Rect heart_rect = {
        (int)heart->x, (int)heart->y,
        heart->w, heart->h};
        heart->rect = heart_rect;
    
    SDL_Rect bullet_rect = {
        (int)bullet->x, (int)bullet->y,
        bullet->w, bullet->h};
        bullet->rect = bullet_rect;

    SDL_Rect bullet_enemies_rect = {
        (int)bullet_enemies->x, (int)bullet_enemies->y,
        bullet_enemies->w, bullet_enemies->h};
        bullet_enemies->rect = bullet_enemies_rect;

    // Impact de la balle du joueur sur les ennemis
    for (size_t i=0; i<*enemies_number_tot; i++){
        if (enemies[i].alive){
            enemies[i].y += enemies[i].vy*dt;
            if (enemies[i].y > SCREEN_HEIGHT - 60){
                *game_state = DEFEAT;
                break;
            }
        }
        if (*bullet_active){
            SDL_Rect * enemy_rect = &(enemies[i].rect);
            if (enemies[i].alive && SDL_HasIntersection(&bullet_rect, enemy_rect)){
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
        *game_state = VICTORY;
    }

    // Impact de la balle des ennemis sur le joueur
    if (*bullet_enemies_active){
        if (SDL_HasIntersection(&bullet_enemies_rect, &player_rect)){
            *bullet_enemies_active = false;
            player->hp -= 1;
        }
    }
    
    if (player->hp == 0){
        *game_state = DEFEAT;
    }

    // Tir des ennemis
    if (*time_since_last_shot >= TIME_BETWEEN_SHOTS/2.0f && !*bullet_enemies_active){
        // Les ennemis tirant plus fréquemment tirent une balle supplémentaire dans l'intervalle entre deux tirs "réguliers"
        if (*next_is_shooting_enemy && *shooting_enemies_count > 0){
            size_t shooting_enemies_index[ENEMIES_NUMBER*100];
            size_t shooting_enemies_count_aux = 0;
            for (size_t i=0; i<*enemies_number_tot; i++){
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
            size_t index_alive[ENEMIES_NUMBER*100];
            size_t alive_count = 0;
            for (size_t i=0; i<*enemies_number_tot; i++){
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

// Affichage des entités
void render(SDL_Renderer *renderer, Entity *player, Entity *enemies, Entity *bullet, bool bullet_active, Entity *bullet_enemies, bool bullet_enemies_active, Entity *heart, bool heart_active, Game_States *game_state, bool *running, size_t *level, bool *reset_game, size_t *enemies_number_tot){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect player_rect = player->rect;
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    SDL_RenderFillRect(renderer, &player_rect);

    for (size_t i=0; i<*enemies_number_tot; i++){
        if(enemies[i].alive){
            SDL_Rect enemy_rect = enemies[i].rect;
            switch (enemies[i].enemy_type){
                case BASIC_ENEMY:
                    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
                    break;
                case FAST_ENEMY:
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
                    break;
                case TOUGH_ENEMY: {
                    float alpha = ((float)enemies[i].hp/TOUGH_ENEMY_HP) * 255;
                    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
                    SDL_SetRenderDrawColor(renderer, 0, 100, 255, alpha);
                    break;
                    }
                case SHOOTING_ENEMY:
                    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                    break;
            }
            SDL_RenderFillRect(renderer, &enemy_rect);
        }
    }

    if (heart_active){
        SDL_Rect heart_rect = heart->rect;
        SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255);
        SDL_RenderFillRect(renderer, &heart_rect);
    }

    if (bullet_active){
        SDL_Rect bullet_rect = bullet->rect;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderFillRect(renderer, &bullet_rect);
    }

    if (bullet_enemies_active){
        SDL_Rect bullet_enemies_rect = bullet_enemies->rect;
        SDL_SetRenderDrawColor(renderer, 255, 100, 0, 255);
        SDL_RenderFillRect(renderer, &bullet_enemies_rect);
    }

    int progression = (int)(((float)player->hp/MAX_HP)*BAR_WIDTH);
    SDL_Rect hp_rect = {(SCREEN_WIDTH - BAR_WIDTH)/2, BAR_DISTANCE_TOP, progression, BAR_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 255, 0, 255, 255); 
    SDL_RenderFillRect(renderer, &hp_rect);
    SDL_Rect outline_rect = {(SCREEN_WIDTH - BAR_WIDTH)/2, BAR_DISTANCE_TOP, BAR_WIDTH, BAR_HEIGHT};
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &outline_rect);

    // Affichage du menu contextuel en cas de victoire
    if (*game_state == VICTORY){
        const SDL_MessageBoxButtonData buttons[] = {
            {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Niveau suivant"},
            {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Sauvegarder et quitter"},
        };
        char victory[50];
        snprintf(victory, sizeof(victory), "Niveau %zu", *level);
        const SDL_MessageBoxData data = {
            SDL_MESSAGEBOX_INFORMATION,
            SDL_RenderGetWindow(renderer),
            "VICTOIRE !",
            victory,
            SDL_arraysize(buttons),
            buttons,
            NULL
        };
        int buttonid;
        SDL_ShowMessageBox(&data, &buttonid);
        if (buttonid == 0){
            (*level)++;
            save_game(*level);
            *reset_game = true;
        } 
        else{
            (*level)++;
            save_game(*level);
            *running = false;
        }}

    // Affichage du menu contextuel en cas de défaite
    if (*game_state == DEFEAT){
        const SDL_MessageBoxButtonData buttons[] = {
            {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Recommencer le niveau"},
            {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 1, "Quitter"},
        };
        char defeat[50];
        snprintf(defeat, sizeof(defeat), "Niveau %zu", *level);
        const SDL_MessageBoxData data = {
            SDL_MESSAGEBOX_INFORMATION,
            SDL_RenderGetWindow(renderer),
            "DÉFAITE...",
            defeat,
            SDL_arraysize(buttons),
            buttons,
            NULL
        };
        int buttonid;
        SDL_ShowMessageBox(&data, &buttonid);
        if (buttonid == 0){
            *reset_game = true;
        } 
        else{
            *running = false;
        }
    }

    // Affichage du menu contextuel en cas de pause
    if (*game_state == PAUSE){
        const SDL_MessageBoxButtonData buttons[] = {
            {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Reprendre"},
            {0, 1, "Quitter"},
        };
        char pause[50];
        snprintf(pause, sizeof(pause), "Niveau %zu", *level);
        const SDL_MessageBoxData data = {
            SDL_MESSAGEBOX_INFORMATION,
            SDL_RenderGetWindow(renderer),
            "PAUSE",
            pause,
            SDL_arraysize(buttons),
            buttons,
            NULL
        };
        int buttonid;
        SDL_ShowMessageBox(&data, &buttonid);
        if (buttonid == 0){
            *game_state = RUNNING;
        }
        else{
            *running = false;
        }
    }

    SDL_RenderPresent(renderer);
}

// Paramétrage du menu de démarrage
int menu(SDL_Window *window, size_t *level){
    const SDL_MessageBoxButtonData buttons[] = {
        {SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT, 0, "Nouvelle partie"},
        {0, 1, "Continuer"},
        {SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT, 2, "Quitter"},
    };
    char menu[50];
    if(load_game(level) == 2){
        snprintf(menu, sizeof(menu), "Partie sauvegardée au niveau %zu", *level);
    }
    else{
        snprintf(menu, sizeof(menu), "");
    }
    const SDL_MessageBoxData messageboxdata = {
        SDL_MESSAGEBOX_INFORMATION,
        window,
        "MENU",
        menu,
        SDL_arraysize(buttons),
        buttons,
        NULL
    };
    int buttonid;
    SDL_ShowMessageBox(&messageboxdata, &buttonid);
    return buttonid;
}

// Initialisation d'une partie de niveau quelconque
// On reprend essentiellement le code de l'initialisation au niveau 1, en modifiant le nombre d'ennemis et leur vitesse
void reset(Entity *player, Entity *enemies, size_t *enemies_count, Entity *bullet, bool *bullet_active, Entity *bullet_enemies, bool *bullet_enemies_active, Entity *heart, bool *heart_active, size_t *shooting_enemies_count, bool *next_is_shooting_enemy, float *time_since_last_shot, float *time_since_last_acceleration, float *time_since_last_heart_attempt, size_t *level, size_t *enemies_number_tot){
    *player = (Entity){
        .x = SCREEN_WIDTH / 2 - PLAYER_WIDTH / 2,
        .y = SCREEN_HEIGHT - 60,
        .w = PLAYER_WIDTH,
        .h = PLAYER_HEIGHT,
        .vx = 0,
        .vy = 0,
        .hp = MAX_HP};

    *bullet = (Entity){0};
    *bullet_active = false;

    *bullet_enemies = (Entity){0};
    *bullet_enemies_active = false;
    *time_since_last_shot = 0;

    *heart = (Entity){0};
    *heart_active = false;
    *time_since_last_heart_attempt = 0;

    // Mise à jour du nombre d'ennemis
    size_t enemies_number_col = ENEMIES_NUMBER_PER_COLUMN + ((*level-1)/2);
    size_t enemies_number_lin = ENEMIES_NUMBER_PER_LINE + (*level/2);
    *enemies_number_tot = enemies_number_col*enemies_number_lin;

    *enemies_count = *enemies_number_tot;

    // Mise à jour de la vitesse des ennemis
    float enemy_speed_updated = ENEMY_SPEED*(1+(float)(*level-1)/10.0f);

    *time_since_last_acceleration = 0;

    for (size_t i=0; i<enemies_number_col; i++){
        for (size_t j=0; j<enemies_number_lin; j++){
            enemies[i*enemies_number_lin + j] = (Entity){
                .enemy_type = BASIC_ENEMY,
                .alive = true,
                .x = SCREEN_WIDTH/enemies_number_lin * (j+0.5) - ENEMY_WIDTH/2,
                .y = SCREEN_HEIGHT/(2*enemies_number_col) * i,
                .w = ENEMY_WIDTH,
                .h = ENEMY_HEIGHT,
                .vx = 0,
                .vy = enemy_speed_updated,
                .hp = 1};
        }
    }

    size_t fast_enemies_nb = (int)(*enemies_number_tot*FAST_ENEMIES_RATIO);
    size_t tough_enemies_nb = (int)(*enemies_number_tot*TOUGH_ENEMIES_RATIO);
    size_t shooting_enemies_nb = (int)(*enemies_number_tot*SHOOTING_ENEMIES_RATIO);

    size_t f = fast_enemies_nb;
    while (f > 0){
        int c = rand() % *enemies_number_tot;
        if (enemies[c].enemy_type == BASIC_ENEMY){
            enemies[c].enemy_type = FAST_ENEMY;
            enemies[c].vy *= FAST_ENEMY_SPEED_MULTPLIER;
            f--;
        }
    }

    size_t t = tough_enemies_nb;
    while (t > 0){
        int c = rand() % *enemies_number_tot;
        if (enemies[c].enemy_type == BASIC_ENEMY){
            enemies[c].enemy_type = TOUGH_ENEMY;
            enemies[c].hp = TOUGH_ENEMY_HP;
            t--;
        }
    }

    size_t s = shooting_enemies_nb;
    while (s > 0){
        int c = rand() % *enemies_number_tot;
        if (enemies[c].enemy_type == BASIC_ENEMY){
            enemies[c].enemy_type = SHOOTING_ENEMY;
            s--;
        }
    }

        *shooting_enemies_count = shooting_enemies_nb;
        *next_is_shooting_enemy = true;
}

// Sauvegarde du jeu
void save_game(size_t level){
    FILE *file = fopen("save.txt", "w");
    fprintf(file, "%zu", level);
    fclose(file);
}

// Chargement d'une sauvegarde
// La sauvegarde est enregistrée sous la forme d'un fichier "save.txt", qui est donc accessible dans le dossier de travail
int load_game(size_t *level){
    FILE *file = fopen("save.txt", "r");
    if (file == NULL){
        return 0;
    }
    else if (fscanf(file, "%zu", level) != 1){
        fclose(file);
        return 1;
    }
    else {
        fscanf(file, "%zu", level);
        fclose(file);
        return 2;
    }
}

// Fermeture de la console
void cleanup(SDL_Window *window, SDL_Renderer *renderer){
    if (renderer)
        SDL_DestroyRenderer(renderer);
    if (window)
        SDL_DestroyWindow(window);
    SDL_Quit();
}
