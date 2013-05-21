#ifndef MAZPONG_DRAWABLE_WALLS
#define MAZPONG_DRAWABLE_WALLS

#include "pong_dataStorage.h"
#include "pong_drawer.h"

typedef struct drawable_wall
{
    EWallType type;
    EWallPosition home;

}drawable_wall;


drawable_wall *drawable_wall_create(SWallInfo *winfo, FdrawFunc *fptrptr);
void drawable_wall_destroy(drawable_wall **_this);
int drawable_wall_updatedata(drawable_wall *_this, struct SWallInfo *wll);

#endif
