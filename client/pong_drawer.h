/* ************************************************************************ */
/*         The purpose of this file is to create a glut window, and a list
 *         holding items to be drawn. Simple list has some drawbacks, mainly
 *         difficulty in knowing if new item to be drawn should replace some
 *         item from the list... So the idea needs to be tuned later...
 *
 *         Revision history:
 *
 *         0.0.1 19.02.2009/Maz First draft
 *
 *         PasteLeft Maz (2009)
 */
/* ************************************************************************ */
#ifndef PONG_DRAWER_H
#define PONG_DRAWER_H

#include <general.h>


#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "general.h"
#include "pong.h"
#include "pong_dataStorage.h"
#include <pthread.h>
#include "pong_drawer.h"
#include "movable_item.h"

typedef void (*FdrawFunc)(movable_item *mover, void *);

//typedef  void (* ( (*get_drawfuncF) ( void * )))(void *) ;

//typedef void (*  ((*get_drawfuncF) ( (*)() ) )) ( void *);

//typedef  void  (*  (*get_drawfuncF) ( void (*)( void (*)(void *) ) ) ) ();

/* Drawable items */
/*
 * Current drawable items: 
 * drawable_text
*/


typedef struct drawable_items
{
    pthread_mutex_t itemlock;
	FdrawFunc drawfunc;
    int item_id;
    /* mover.set_position(mover*,x,y,z) and mover.set_velocity(mover*,sx,sy,sz) */
    movable_item mover;
    void *item;
    struct drawable_items *head;
	struct drawable_items *next;
	struct drawable_items *prev;
    struct drawable_items *free;
}drawable_items;


int init_and_launch_maindrawer(SClientGameData *gameData, pthread_t *mainDrawLoopId);
void *client_main_draw_loop(void *);
void init_drawable_items_list(drawable_items *item);
drawable_items * drawable_items_list_add_item(drawable_items *list,size_t itsemstruct_size);
void * drawable_items_list_remove_item(drawable_items *item);
drawable_items *drawable_items_list_get_first(drawable_items *item);
drawable_items *drawable_items_list_get_next(drawable_items *item);
drawable_items *drawable_items_list_get_prev(drawable_items *item);
void *drawable_items_list_reserve_item_for_data(drawable_items *item);
void drawable_items_list_release_item(drawable_items *item,FdrawFunc drawFunc);


//TODO: modify this to be many lists (for different type of things to draw, so we can handle the situation where the list needs to be cleared)
drawable_items *Gitemlist_head;

#endif
