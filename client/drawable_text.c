#include "drawable_text.h"
#include <stdio.h>
#include <stdlib.h>

int drawable_text_set_fontopt(drawable_text *_this,void *font,int color)
{
    if(!_this)
    {
        printf("NULL ptr in drawable_text_set_fontopt()\n");
        return -1;
    }
    if(font)
        _this->font=font;
    if(color != 0xdeadbabe)
        _this->color=color;
    return 0;
}

static void draw_text(movable_item *mover,void *ptr)
{
    drawable_text *_this=ptr;
    /* Do actual drawing here! */
    (void)_this;
    printf("Text drawing not yet done!\n");
}

#if 0
static int get_drawfunc(FdrawFunc *fptrPtr, void *ptr)
{
    drawable_text *_this=(drawable_text *)ptr;
    if(!_this)
        return -1;
    *fptrPtr=&draw_text;
    return 0;
}
#endif
int drawable_text_set_text(drawable_text *_this,char *text)
{
    size_t newlen;
    if(!text || !_this)
    {
        printf("NULL ptr in drawable_text_set_text()!\n");
        return -1;
    }
    if(_this->len<=(newlen=strlen(text)))
    {
        if(_this->textset)
        {
            free(_this->text);
        }
        _this->text=malloc(newlen+1);
        if(!_this->text)
        {
            printf("Mallocing %u bytes FAILED!\n",newlen+1);
            return -1;
        }
        _this->len=newlen;
        _this->textset=1;
    }
    strncpy(_this->text,text,_this->len);
    _this->text[_this->len]='\0';
    return 0;
}
drawable_text *drawable_text_create(char *text,void *font,FdrawFunc *fptrPtr)
{
    drawable_text *_this;
    _this=calloc(1,sizeof(drawable_text));
    if(!_this)
    {
        printf("calloc FAILED!\n");
        return NULL;
    }
    if(text)
    {
        _this->len=strlen(text);
        /* Lets allocate at least 128 bytes to avoid constant frees/reallocs if item is reused */
        if(127>_this->len)
            _this->len=127;
        _this->text=malloc((_this->len+1));
        if(!_this->text)
        {
            printf("Malloc FAILED!\n");
            free(_this);
            return NULL;
        }
        strncpy(_this->text,text,_this->len);
        _this->text[_this->len]='\0';
        _this->textset=1;
    }
    *fptrPtr=&draw_text;
    return _this;
}


