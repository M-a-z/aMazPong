#include "drawable_walls.h"
#include "pong_drawer.h"
#include "pong_dataStorage.h"

extern SGameArea G_virtual_area;
//extern int G_window_real_width;
//extern int G_window_real_height;

static void wallDrawer(movable_item *mover,void *ptr)
{
    drawable_wall *_this=(drawable_wall *)ptr;
    

    if(!ptr)
    {
        printf("NULL ptr in wallDrawer!\n");
        return;
    }
    if(_this->type == EWallType_None)
        return;
    glBegin(GL_POLYGON);
    glColor3f(1.0,0.0,1.0);
    switch(_this->home)
    {
        case EWallPosition_Left:
            glVertex2i(0,0);
            glVertex2i(5,0);
            glVertex2i(5,mover->getY(mover));
            glVertex2i(0,mover->getY(mover));
            break;
        case EWallPosition_Right:
            glVertex2i(mover->getX(mover),0);
            glVertex2i(mover->getX(mover)-5,0);
            glVertex2i(mover->getX(mover)-5,mover->getY(mover));
            glVertex2i(mover->getX(mover),mover->getY(mover));
            break;
        case EWallPosition_Up:
            glVertex2i(0,0);
            glVertex2i(mover->getX(mover),0);
            glVertex2i(mover->getX(mover),5);
            glVertex2i(0,5);
            break;
        case EWallPosition_Bottom:
            glVertex2i(0,mover->getY(mover)-5);
            glVertex2i(mover->getX(mover),mover->getY(mover)-5);
            glVertex2i(mover->getX(mover),mover->getY(mover));
            glVertex2i(0,mover->getY(mover));
            break;
        default:
            printf("Unknown wall position!\n");
    }
    glEnd();

    printf("Wall drawing callback called!\n");
}

drawable_wall *drawable_wall_create(SWallInfo *winfo, FdrawFunc *fptrptr)
{
    drawable_wall *_this;
    _this=calloc(1,sizeof(drawable_wall));
    if(!_this)
    {
        printf("Calloc FAILED!\n");
        return NULL;
    }
    _this->type = winfo->type;
    _this->home = winfo->wpos;
    *fptrptr=wallDrawer;
    return _this;
}
void drawable_wall_destroy(drawable_wall **_this_)
{
    drawable_wall *_this=NULL;
    if(!_this_)
        return;
    _this=*_this_;
    *_this_=NULL;
    if(_this)
        free(_this);
}
int drawable_wall_updatedata(drawable_wall *_this,SWallInfo *wll)
{
    if(!_this || !wll)
    {
        printf("drawable_wall_updatedata NULL ptr!\n");
        return -1;
    }
    _this->type = wll->type;
    _this->home = wll->wpos;
    return 0;
}
