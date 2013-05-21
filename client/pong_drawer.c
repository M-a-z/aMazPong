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


#include <general.h>
#ifdef __LINUX__
#include <arpa/inet.h>
#else
#include <winsock2.h>
#endif
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "general.h"
#include "pong.h"
#include "pong_dataStorage.h"
#include <pthread.h>
#include "pong_drawer.h"
#include "drawable_balls.h"
#include "drawable_paddles.h"
#include "drawable_walls.h"

extern const char* G_clientver;
void drawFuncNotSet();
static pthread_mutex_t Gitemlist_lock;
void init_drawable_items_list(drawable_items *item);

drawable_ball *ballarray[BALLS_MAX]={NULL};
drawable_paddle *baddlearray[4]={NULL};
drawable_wall *wallarray[4]={NULL};


SGameArea area;

int G_window_real_width;
int G_window_real_height;


drawable_items *Gitem;



void drawitems_callback()
{
    drawable_items *dri=Gitem;
    pthread_mutex_lock(&Gitemlist_lock);
    dri=dri->next;

    glClear(GL_COLOR_BUFFER_BIT);
    while(dri)
    {
        pthread_mutex_lock(&(dri->itemlock));
        printf("Drawer calling drawfunc at %p (mover=%p,item=%p)\n",dri->drawfunc,&(dri->mover),dri->item);
        dri->drawfunc(&(dri->mover),dri->item);
        pthread_mutex_unlock(&(dri->itemlock));
        dri=dri->next;
    }
    pthread_mutex_unlock(&Gitemlist_lock);
	glutSwapBuffers();
}

int add_paddle(drawable_items *item,SPlateInfo *plate,int paddleIndex)
{
    FdrawFunc fptr;
    
    if(!plate || !item)
    {
        printf("NULL ptr in add_paddle()!\n");
        return -1;
    }
    if(!(item=drawable_items_list_add_item(item,0)))
    {
        printf("Failed to add paddle!\n");
        return -1;
    }
    if(!( baddlearray[paddleIndex]=drawable_paddle_create(plate->width,PADDLE_START_HEIGHT,htonl(paddle_start_color[paddleIndex]), plate->wall,&fptr)))
    {
        printf("Failed to create Padddle\n");
        return -1;
    }
    item->item=baddlearray[paddleIndex];
    item->mover.set_position(&(item->mover), plate->x, plate->y,0);
    item->drawfunc=fptr;
    return 0;
}

int add_walls(drawable_items *item,SWallInfo *walls)
{
    FdrawFunc fptr;
    int i;
    for(i=0;i<4;i++)
    {
        if(!(item=drawable_items_list_add_item(item,0)))
        {
            printf("Failed to add Wall %d\n",i);
            return -1;
        }
        if(!( wallarray[i]=drawable_wall_create(&(walls[i]),&fptr)))
        {
            printf("Failed to create Wall %d\n",i);
            return -1;
        }
        item->item=wallarray[i];
        item->mover.set_position(&(item->mover), area.width, area.height,0);
        printf("Added WALL %u\n",i);
        item->drawfunc=fptr;
    }
    printf("All Walls processed\n");
    return 0;
}

int add_balls(drawable_items *item,SBallData *balls)
{
    FdrawFunc fptr;
    int i;
    for(i=0;i<BALLS_MAX;i++)
    {
        if(balls[i].ball_active)
        {
            if(!(item=drawable_items_list_add_item(item,0)))
            {
                printf("Failed to add Ball %d\n",i);
                return -1;
            }
            if(!( ballarray[i]=drawable_ball_create(/* ball size */5,&fptr)))
            {
                printf("Failed to create Ball %d\n",i);
                return -1;
            }
            item->item=ballarray[i];
            item->mover.set_position(&(item->mover), balls[i].xpos,balls[i].ypos,0);
            if(drawable_ball_updatedata(ballarray[i],&(balls[i])))
            {
                printf("Failed to set balldata for ball %d\n",i);
                drawable_ball_destroy(&(ballarray[i]));
                return -1;
            }
            /* After this is done, drawerprocess can try drawing ball item */
            printf("Added BALL %u\n",i);
            item->drawfunc=fptr;
        }
    }
    printf("All BALLs processed\n");
    return 0;
}
int create_items(drawable_items *item,SClientGameData *gameData)
{
    int i;
    /* item here should be the already initialized list head. */
    if(!item || !gameData)
    {
        printf("NULL ptrs in create_items!\n");
        return -1;
    }
    
    if(add_balls(item,gameData->BalldataArray))
    {
        printf("Failed to add balls to drawable items!\n");
        return -1;
    }
    printf("added balls to drawable items\n");
    for(i=0;i<gameData->player_amnt;i++)
        if(add_paddle(item,&(gameData->players[i].plate),i))
        {
            printf("Failed to add player %d paddle\n",i);
            return -1;
        }

    if(add_walls(item,gameData->walls))
    {
        printf("Failed to add walls to drawable items!\n");
        return -1;
    }


     printf("added players' paddles to drawable items\n");
    printf("create_items() Not yet done! %s:%d\n",__FILE__,__LINE__);
    return 0;
    
}

int init_and_launch_maindrawer(SClientGameData *gameData, pthread_t *mainDrawLoopId)
{
	char *item_text;
	pthread_attr_t attr;
//	pthread_t mainDrawLoopId;
	Gitem=malloc(sizeof(drawable_items));
	item_text=malloc(30*sizeof(char)+1);
	if(NULL==Gitem || NULL==item_text)
	{
		printf("Malloc failed, out of mem? %s:%d",__FILE__,__LINE__);
		exit(1);
	}

    area.width=gameData->area.width;
    area.height=gameData->area.height;
	init_drawable_items_list(Gitem);
    if(create_items(Gitem,gameData))
    {
        printf("Failed to create drawable items!\n");
        return -1;
    }
//	item->drawfunc=&drawFuncNotSet;
	snprintf(item_text,30,"Starting MazPong v %s",G_CLIENTVER);
//	item->item=item_text;
	pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
	pthread_create(mainDrawLoopId,&attr,&client_main_draw_loop,NULL);
	return 0;
}

extern int G_argc;
extern char **G_argv;

void reshape_callback(int width, int height)
{

    if(!height)
        height=1;

    glViewport(0,0,width,height); // map the actual pixels to window pixels.
    glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    glScalef(1,-1,1);
	gluOrtho2D(0,width,0,height);
    glTranslatef(0,-height,0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

void *client_main_draw_loop(void *args)
{
	(void)args;
    int height=area.height;
    int width=area.width;
    printf("Initializing glut...\n");
	glutInit(&G_argc,G_argv);
    //int winhandler;
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE); //Init displaymode: RGB colors, Double buffering)
    printf("glutInitWindowSize(width=%d,height=%d)\n",width,height);
    glutInitWindowSize(width,height); //Set window size, width,height (pixels)
    glutInitWindowPosition(0,0);
    /* winhandler  = */ glutCreateWindow("Maz Bot ... errm PONG!");
//    G_window_real_width = glutGet(GLUT_WINDOW_WIDTH);
//    G_window_real_heigh = glutGet(GLUT_WINDOW_HEIGHT);
    printf("viewport as whole window: %d,%d\n",width,height);
	glViewport(0,0,width,height); // map the actual pixels to window pixels.

    glMatrixMode(GL_PROJECTION);

//	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
    printf("Set world window? %d,%d\n",width,height);
	gluOrtho2D(0,width,0,height);
    glScalef(1,-1,1);
    glTranslatef(0,-height,0);
//	glScalef(((double)area.width)/((double)width),((double)-1.0)*((double)area.height)/((double)height),1.0);	
	/* this flips the coordinate system so
				 that y increases going down the
				 screen (see below) */
//    G_window_real_width = glutGet(GLUT_WINDOW_WIDTH);
//    G_window_real_heigh = glutGet(GLUT_WINDOW_HEIGHT);
//    glTranslatef(((double)area.width)/((double)width),((double)-1.0)*((double)area.height)/((double)height),1.0);
//	glTranslatef(0.0,-height,0.0);
//    glTranslatef(0.0,((double)-1.0)*((double)area.height)/((double)height),0);
//	glMatrixMode(GL_MODELVIEW);
//	glLoadIdentity();
//	glTranslatef(0.375,0.375,0.0);
    /* so we can draw using integer
				   coordinates (see above) */
    
    
    //glLoadIdentity();
//    glOrtho
/*    DEFINE THESE */
    glutDisplayFunc(&drawitems_callback);
//    glutReshapeFunc(&CBresizeWindow);
//    glutKeyboardFunc(&CBkeyPress);
//    glutIdleFunc(&CBdisplay);
    glClearColor(0,0,0,0); //Set 'clearing color' to be black
    glutMainLoop();
#ifdef __WIN32__
    while(1)
    {
        printf("Mainwindow proc - falling eternal sleep\n");
        sleep(1000);
    }
#endif
    return NULL;
}

void drawFuncNotSet()
{
	

}
void drawable_items_list_release_item(drawable_items *item,FdrawFunc drawFunc)
{
    if(!item)
        return;
    if(drawFunc)
        item->drawfunc=drawFunc;
     pthread_mutex_unlock(&(item->itemlock));
}

void *drawable_items_list_reserve_item_for_data(drawable_items *item)
{

    if(!item)
        return NULL;
    pthread_mutex_lock(&(item->itemlock));
    return item->item;
}

drawable_items *drawable_items_list_get_prev(drawable_items *item)
{
    if(!item) 
        return NULL;
    return item->prev;
}


drawable_items *drawable_items_list_get_next(drawable_items *item)
{
    if(!item) 
        return NULL;
    return item->next;
}

drawable_items *drawable_items_list_get_first(drawable_items *item)
{
    if(!item)
        return NULL;
    return item->head->next;
}
void * drawable_items_list_remove_item(drawable_items *item)
{
    void *listdata;
    if(!item)
    {
        printf("NULL ptr in drawable_items_list_remove_item!\n");
        return NULL;
    }
    if(item->head==item)
    {
        printf("drawable_items_list_remove_item(): Cannot remove list head!\n");
        return item->item;
    }
    pthread_mutex_lock(&(item->itemlock));
    listdata=item->item;
    item->item=NULL;
    pthread_mutex_unlock(&(item->itemlock));
    pthread_mutex_lock(&(Gitemlist_lock));
    item->drawfunc=&drawFuncNotSet;
    item->prev=item->head->free;
    item->head->free=item;
    pthread_mutex_unlock(&(Gitemlist_lock));
    return listdata;
}
drawable_items * drawable_items_list_add_item(drawable_items *list,size_t itsemstruct_size)
{
    drawable_items *new_item=NULL;
    
    if(!list)
    {
        printf("NULL ptr in drawable_items_list_add_item\n");
        goto err_out;
    }
    /* Lock whole list */
    pthread_mutex_lock(&(Gitemlist_lock));
    new_item=list->head->free;
    if(new_item)
    {
        printf("Using old item from itemlist, setting free to %p\n",list->head->free->prev);
        list->head->free=list->head->free->prev;
        pthread_mutex_unlock(&(Gitemlist_lock));
    }
    else
    {
        pthread_mutex_unlock(&(Gitemlist_lock));
        printf("No free items available, allocating new one\n");
        new_item=calloc(1,sizeof(drawable_items));
        pthread_mutex_init(&new_item->itemlock,NULL);
    }
    if(!new_item)
    {
        printf("drawable_items_list_add_item(): calloc FAILED!\n");
        goto err_out;
    }
    if(itsemstruct_size)
    {
        new_item->item=calloc(1,itsemstruct_size);
        if(!new_item->item)
        {
            printf("drawable_items_list_add_item(): calloc FAILED!\n");
            goto err_out;
        }
    }
    else
        new_item->item=NULL;
    init_movable(&new_item->mover);
    new_item->head=list->head;
    new_item->next=list->head->next;
    new_item->prev=list->head;
    pthread_mutex_lock(&(Gitemlist_lock));
    list->head->next=new_item;
    if(new_item->next)
        new_item->next->prev=new_item;
    new_item->drawfunc=&drawFuncNotSet;

    pthread_mutex_unlock(&(Gitemlist_lock));
    if(0)
    {
err_out:
        if(new_item)
        {
            if(new_item->item)
                free(new_item->item);
            free(new_item);
        }
    }
    return new_item;
}
void init_drawable_items_list(drawable_items *item)
{
	pong_assert(NULL!=Gitemlist_head,"Itemlist init requested but head already set!");
	pthread_mutex_init(&Gitemlist_lock,NULL);
	pthread_mutex_lock(&Gitemlist_lock);
	pthread_mutex_init(&(item->itemlock),NULL);
	Gitemlist_head=item;
	item->next=NULL;
	item->prev=NULL;
    item->free=NULL;
    item->head=item;
    item->drawfunc=&drawFuncNotSet;
    init_movable(&item->mover);
	pthread_mutex_unlock(&Gitemlist_lock);

}
