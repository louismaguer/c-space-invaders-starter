#ifndef ENTITY_H
#define ENTITY_H

typedef struct
{
    bool alive;
    float x, y;
    float vx, vy;
    int w, h;
    SDL_Rect rect;
} Entity;

#endif
