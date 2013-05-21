#ifndef MAZPONG_DRAWABLE_PADDLESS
#define MAZPONG_DRAWABLE_PADDLESS

#include "pong_dataStorage.h"
#include "pong_drawer.h"

typedef struct drawable_paddle
{
    unsigned int width;
    unsigned int height;
    unsigned int color;
    unsigned int origColor;
    SPlateSpecials specials;
    EWallPosition wall;
}drawable_paddle;


drawable_paddle *drawable_paddle_create(unsigned int width,unsigned int height,unsigned int color, EWallPosition wall, FdrawFunc *fptrptr);
void drawable_paddle_destroy(drawable_paddle **_this);
int drawable_paddle_updatedata(drawable_paddle *_this, struct SPlateInfo *plate);

#endif
