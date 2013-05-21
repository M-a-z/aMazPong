#ifndef MAZPONG_DRAWABLE_TEXT
#define MAZPONG_DRAWABLE_TEXT
#include "pong_drawer.h"

typedef struct drawable_text
{
//    get_drawfuncF get_drawfunc;
    unsigned int color; //check later how color is specified
    unsigned int textset;
    size_t len;
    char *text;
    void *font;  //check later how font is specified
}drawable_text;


drawable_text *drawable_text_create(char *text,void *font,FdrawFunc *fptrPtr);
//int drawable_text_set_position(drawable_text *_this,unsigned int x,unsigned int y,unsigned int z);
int drawable_text_set_text(drawable_text *_this,char *text);
//int drawable_text_set_velocity(drawable_text *_this,unsigned int x,unsigned int y,unsigned int z);


#endif
