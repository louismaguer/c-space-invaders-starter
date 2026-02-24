#ifndef ENTITY_H
#define ENTITY_H

// Définition des types d'ennemis
typedef enum{
    BASIC_ENEMY,
    FAST_ENEMY,
    TOUGH_ENEMY,
    SHOOTING_ENEMY
} Enemy_Type;

// Définition des entités
typedef struct{
    Enemy_Type enemy_type;
    bool alive;
    float x, y;
    float vx, vy;
    int w, h;
    SDL_Rect rect;
    int hp;
} Entity;

#endif
