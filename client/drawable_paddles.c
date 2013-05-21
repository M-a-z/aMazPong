#include "drawable_paddles.h"
#include "pong_drawer.h"
#include "pong_dataStorage.h"

const unsigned paddle_start_color[4]=
{ 
    0x48003800, 
    0x00885800, 
    0x28780000,
    0x7f000000 
};



static void paddleDrawer(movable_item *mover,void *ptr)
{
    char *wallposstrings[4]={"Left","Right","Top","Bottom"};
    char *wallposstring;
    drawable_paddle *_this=(drawable_paddle *)ptr;
//    int x1,x2,x3,x4y1,y2,y3,y4;
    float x_inc;
    float y_inc;

    switch(_this->wall)
    {
        case EWallPosition_Left:
            wallposstring=wallposstrings[0];
            y_inc=(float)_this->width;
            x_inc=(float)_this->height;
        break;
        case EWallPosition_Right:
            wallposstring=wallposstrings[1];
            y_inc=(float)_this->width;
            x_inc=(float)_this->height*-1.0;
        break;
        case EWallPosition_Up:
            wallposstring=wallposstrings[2];
            x_inc=(float)_this->width;
            y_inc=(float)_this->height;
        break;
        case EWallPosition_Bottom:
            wallposstring=wallposstrings[3];
            x_inc=(float)_this->width;
            y_inc=(float)_this->height*-1.0;
        break;
        default:
        printf("Invalid position for paddle\n");
        return;
    }

    if(!ptr)
    {
        printf("NULL ptr in paddleDrawer!\n");
        return;
    }

    printf("Paddle drawing callback called!\n");

    printf
    (
        "Setting color: 0x%02x, 0x%02x, 0x%02x\n",
        (unsigned)(((unsigned char*)&(_this->color))[0]),
        (unsigned)(((unsigned char*)&(_this->color))[1]),
        (unsigned) (((unsigned char*)&(_this->color))[2])
    );

    printf
    (
        "Drawing %s  paddle corners at (%f,%f), (%f,%f), (%f,%f), (%f,%f)\n",
        wallposstring,
        mover->getX(mover),mover->getY(mover),
        mover->getX(mover),mover->getY(mover)+y_inc,
        mover->getX(mover)+(x_inc),mover->getY(mover)+y_inc,
        mover->getX(mover)+(x_inc),mover->getY(mover)
    );

    glBegin(GL_QUADS);
    glColor3bv((const GLbyte *)&(_this->color));
    glVertex2f(mover->getX(mover),mover->getY(mover));
    glVertex2f(mover->getX(mover),mover->getY(mover)+y_inc);
    glVertex2f(mover->getX(mover)+(x_inc),mover->getY(mover)+y_inc);
    glVertex2f(mover->getX(mover)+(x_inc),mover->getY(mover));
    glEnd();

}

drawable_paddle *drawable_paddle_create(unsigned int width,unsigned int height,unsigned int color, EWallPosition wall, FdrawFunc *fptrptr)
{
    drawable_paddle *_this;
    _this=calloc(1,sizeof(drawable_paddle));
    if(!_this)
    {
        printf("Calloc FAILED!\n");
        return NULL;
    }
    _this->width=width;
    _this->height=height;
    _this->color=color;
    _this->origColor=color;
    _this->wall=wall;
    memset(&_this->specials,0,sizeof(SPlateSpecials));
    *fptrptr=paddleDrawer;
    return _this;
}
void drawable_paddle_destroy(drawable_paddle **_this_)
{
    drawable_paddle *_this=NULL;
    if(!_this_)
        return;
    _this=*_this_;
    *_this_=NULL;
    if(_this)
        free(_this);
}
int drawable_paddle_updatedata(drawable_paddle *_this,SPlateInfo *paddle)
{
    SPlateSpecials tmp;
    memset(&tmp,0,sizeof(SPlateSpecials));
    if(!_this || !paddle)
    {
        printf("drawable_paddle_updatedata NULL ptr!\n");
        return -1;
    }
    /*
    */
    _this->color=_this->origColor;
    _this->width=paddle->width;
    if( memcmp(&tmp,&(paddle->specials),sizeof(SPlateSpecials) ))
    {
        /* Special field is propably only thing to inspect here. If we have some special mode for paddle, it may 
         * for example change color when speed is increased or something. TBD */
        /*
         Nowadays width is handled at server side!
        if(paddle->specials.wide!=_this->specials.wide)
        {
            int tmp=_this->specials.wide;
            _this->specials.wide=paddle->specials.wide;
            tmp=(tmp+_this->specials.wide)*PADDLE_WIDTH_QUANTUM;
            if(tmp<PADDLE_WIDTH_MIN)
                tmp=PADDLE_WIDTH_MIN;
            else if(tmp>PADDLE_WIDTH_MAX)
                tmp=PADDLE_WIDTH_MAX;
            _this->width=tmp;
        } 
      */  
        if(paddle->specials.stuck!=_this->specials.stuck)
            _this->specials.stuck=paddle->specials.stuck;
        if(paddle->specials.fast!=_this->specials.fast)
            _this->specials.fast=paddle->specials.fast;
        if(_this->specials.fast)
            _this->color=PLATE_FAST_COLOR;
        if(_this->specials.stuck)
            _this->color=PLATE_STUCK_COLOR;
        if(_this->specials.wide>0)
        {
            _this->color=PLATE_WIDER_COLOR;
        }
        else if(_this->specials.wide)
            _this->color=PLATE_SHORTER_COLOR;

        
//        printf("Heck, unknown specials in paddle data - I don't know how to draw it!\n");
//        return -1;
    }
    return 0;
}
