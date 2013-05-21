#include "movable_item.h"
#include <stdio.h>
#include <string.h>

static float getX(movable_item *_this)
{
    /* If needed, these funcs can do some translations between coords etc. */
    return _this->xpos;
}
static float getY(movable_item *_this)
{
    return _this->ypos;
}


static int set_velocity(movable_item *_this,float x,float y,float z)
{
    if(!_this)
    {
        printf("NULL ptr in set_velocity()\n");
        return -1;
    }
    _this->speedx=x;
    _this->speedy=y;
    _this->speedz=z;
    return 0;
}


static int set_position(movable_item *_this,float x,float y,float z)
{
    if(!_this)
    {
        printf("NULL ptr in set_position()\n");
        return -1;
    }
    _this->xpos=x;
    _this->ypos=y;
    _this->zpos=z;
    return 0;
}


int init_movable(movable_item *_this)
{
    if(!_this)
    {
        printf("NULL ptr in init_movable()!\n");
        return -1;
    }
    memset(_this,0,sizeof(*_this));
    _this->set_position=&set_position;
    _this->set_velocity=&set_velocity;
    _this->getX=&getX;
    _this->getY=&getY;
    return 0;
}
