#ifndef MAZPONG_DRAWABLE_BALLS
#define MAZPONG_DRAWABLE_BALLS

#include "pong_dataStorage.h"
#include "pong_drawer.h"

typedef struct drawable_ball
{
    unsigned int size;
    unsigned int color;
}drawable_ball;


drawable_ball *drawable_ball_create(unsigned int radius, FdrawFunc *fptrptr);
void drawable_ball_destroy(drawable_ball **_this);
int drawable_ball_updatedata(drawable_ball *_this, struct SBallData *bll);

#endif
