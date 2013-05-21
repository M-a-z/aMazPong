#include "drawable_balls.h"
#include "pong_drawer.h"
#include "pong_dataStorage.h"

extern SGameArea G_virtual_area;
//extern int G_window_real_width;
//extern int G_window_real_height;

static void ballDrawer(movable_item *mover,void *ptr)
{
    drawable_ball *_this=(drawable_ball *)ptr;
//    int screen_x;
//    int screen_y;

//    screen_x=
//    screen_y=
    if(!ptr)
    {
        printf("NULL ptr in ballDrawer!\n");
        return;
    }

    printf("Window width is %u\n", glutGet(GLUT_WINDOW_WIDTH));
    printf("Window height is %u\n",glutGet(GLUT_WINDOW_HEIGHT));

    printf("Ball drawing callback called!\n");

//	glMatrixMode(GL_MODELVIEW);
//    glLoadIdentity();

    //drawable_ball *_this=(drawable_ball *)ptr;
#if 0
    glBegin(GL_POLYGON);
    glColor3f(1.0,0.0,1.0);
    glVertex2f(0.1,0.1);
    glVertex2f(0.1,0.2);
    glVertex2f(0.2,0.2);
    glVertex2f(0.2,0.1);
#endif
//#if 0
    printf("Drawing central ballpoint at (%f,%f)\n",mover->getX(mover),mover->getY(mover));
    printf("Drawing upper left ballpoint at (%f,%f)\n",mover->getX(mover)-4.0,mover->getY(mover)+4.0);
    printf("Drawing leftmost ballpoint at (%f,%f)\n",mover->getX(mover)-6.0,mover->getY(mover));
    printf("1.st (leftmost upper) triangle done\n");
    printf("Drawing leftmost down triangle corner at (%f,%f)\n",mover->getX(mover)-4.0,mover->getY(mover)-4.0);
    printf("2.nd (leftmost lower) triangle done\n");
    printf("Drawing bottom triangle corner at (%f,%f)\n",mover->getX(mover),mover->getY(mover)-6.0);
    printf
    (
        "Setting color: 0x%02x, 0x%02x, 0x%02x\n",
        (unsigned)(((unsigned char*)&(_this->color))[0]),
        (unsigned)(((unsigned char*)&(_this->color))[1]),
        (unsigned) (((unsigned char*)&(_this->color))[2])
    );
    glBegin(GL_TRIANGLE_FAN);
    glColor3bv((const GLbyte *)&(_this->color));
//    glColor3b(1,1,1);
    /* Central point */
    glVertex2f(mover->getX(mover),mover->getY(mover));
    glVertex2f(mover->getX(mover)-4.0,mover->getY(mover)+4.0);
    glVertex2f(mover->getX(mover)-6.0,mover->getY(mover));
    /* 1.st (leftmost upper) triangle done */
    glVertex2f(mover->getX(mover)-4.0,mover->getY(mover)-4.0);
    glVertex2f(mover->getX(mover),mover->getY(mover)-6.0);
    glVertex2f(mover->getX(mover)+4.0,mover->getY(mover)-4.0);
    glVertex2f(mover->getX(mover)+6.0,mover->getY(mover));
    glVertex2f(mover->getX(mover)+4.0,mover->getY(mover)+4.0);
    glVertex2f(mover->getX(mover),mover->getY(mover)+6.0);
    glVertex2f(mover->getX(mover)-4.0,mover->getY(mover)+4.0);
//#endif
    glEnd();

}

drawable_ball *drawable_ball_create(unsigned int radius, FdrawFunc *fptrptr)
{
    drawable_ball *_this;
    _this=calloc(1,sizeof(drawable_ball));
    if(!_this)
    {
        printf("Calloc FAILED!\n");
        return NULL;
    }
    _this->size=radius;
    _this->color=BALL_COLOUR_DEFAULT;
    *fptrptr=ballDrawer;
    return _this;
}
void drawable_ball_destroy(drawable_ball **_this_)
{
    drawable_ball *_this=NULL;
    if(!_this_)
        return;
    _this=*_this_;
    *_this_=NULL;
    if(_this)
        free(_this);
}
int drawable_ball_updatedata(drawable_ball *_this,SBallData *bll)
{
    if(!_this || !ball)
    {
        printf("drawable_ball_updatedata NULL ptr!\n");
        return -1;
    }
    if(bll->special)
    {
        /* Special field is propably only thing to inspect here. If we have some special mode for ball, it may 
         * for example change color when speed is increased or something. TBD */
        printf("Heck, unknown special in ball data - I don't know how to draw it!\n");
        return -1;
    }
    return 0;
}
