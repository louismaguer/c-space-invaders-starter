#ifndef ENTITY_H
#define ENTITY_H

typedef struct{
    Enemy_Type enemy_type;
    bool alive;
    float x, y;
    float vx, vy;
    int w, h;
    SDL_Rect rect;
    int hp;
} Entity;

typedef enum{
    BASIC_ENEMY,
    FAST_ENEMY,
    TOUGH_ENEMY,
    SHOOTING_ENEMY
} Enemy_Type;

#endif
