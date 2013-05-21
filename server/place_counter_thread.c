/* ************************************************************************** */
/*
 *    The purpose of this file is to create a thread which calculates the ball
 *    and paddle positions && updates the data structures in commP singleton
 *    class.
 *
 *    Revision history:
 *
 *    -0.0.2  12.06.2008/Maz  Added pong asserts and utilization of cpp wrapper.
 *    -0.0.1  04.06.2008/Maz  First Draft.
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */

#include <pthread.h>
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "pong_misc_func.h"
#include "general.h"
//#include "pong.h"
#include "pong_dataStorage.h"
#include "pong_server_accept_client.h"
#include <pthread.h>
#include <time.h>
#include "place_counter_thread.h"
#define ball_baddle_speed_ratio 2
//#include "pong_commP_CppToC_wrapper.h"

//TODO: Redo for supporting multiple games running on server...
//Probs give the ptr to game data as arg :)

static void * schedfunc(void *arg);

EPongRet launch_timerthread(SGameData *gameData)
{
    pthread_t tid;
    if(pthread_create(&tid,/* attr */NULL,schedfunc,gameData))
    {
        printf("Failed to launch timerthread\n");
        return EPongRet_ERROR;
    }
    return EPongRet_SUCCESS;
}

static void * schedfunc(void *arg)
{
     int i;
     struct timespec tp_bs;
     struct timespec tp_as;
     SGameData *gameData=(SGameData *)arg;
     float travelled_x=0;
     float travelled_y=0;
     //clock_t prevtime=0;
     //clock_t currtime=0;
     int amnt_of_players;
     SPlayerData **pldata;
     amnt_of_players=(int)gameData->player_amnt;
//     amnt_of_players=(int *)arg;
     pldata=malloc(amnt_of_players*sizeof(SPlayerData *));
     //prevtime=clock();
     while(1)
     {
        unsigned long long int reftime; 
        int hit=0;
        clock_gettime(CLOCK_REALTIME, &tp_bs);
        usleep(1000*GAME_TIME_QUANTA_MSEC);
        for(i=1;i<=amnt_of_players;i++)
        {
             pldata[i-1]=playerdata_get(1,i);
             /* make it so that playerdata_get() returns NULL if game has ended */ 
             if(!pldata[i-1])
                 return NULL;
        }
        pong_assert(0,"Ball Position Get not done!");
         //Get time
        clock_gettime(CLOCK_REALTIME, &tp_as);
        /* reftime as milliseconds */
        reftime=(unsigned long long int) tp_as.tv_sec*1000ULL;
        reftime+=(unsigned long long int)tp_as.tv_nsec/1000000ULL;
        reftime-=(unsigned long long int)tp_bs.tv_sec*1000ULL;
        reftime-=(unsigned long long int)tp_bs.tv_nsec/1000000ULL;
        /* Move plates */
         for(i=0;i<amnt_of_players;i++)
         {
             travelled_x=0;
             travelled_y=0;
             if(reftime<10)
                 reftime=10; /* 10 millisecs as minimum */
              
               switch(pldata[i]->plate.wall)
               {
                   case EWallPosition_Right:
                   case EWallPosition_Left:
                        if(pldata[i]->plate.direction && pldata[i]->plate.speed)
                            travelled_y=TRANSITION_BASEDON_VELOCITYVALUE_AND_MILLISECONDS((float)pldata[i]->plate.direction*pldata[i]->plate.speed,reftime);
                    break;
                   case EWallPosition_Up:
                   case EWallPosition_Bottom:
                        if(pldata[i]->plate.direction && pldata[i]->plate.speed)
                            travelled_x=TRANSITION_BASEDON_VELOCITYVALUE_AND_MILLISECONDS((float)pldata[i]->plate.direction*pldata[i]->plate.speed,reftime);
                    break;
                   default:
                       printf("Invalid plate wall for player %d\n",i);
                   break;
                }
                
                if(travelled_x+pldata[i]->plate.x<0)
                {
                    pldata[i]->plate.direction=-1*pldata[i]->plate.direction;
                    pldata[i]->plate.x=-1*(travelled_x+pldata[i]->plate.x);
                }
                else if(travelled_x+pldata[i]->plate.x>gameData->area.width)
                {
                    pldata[i]->plate.direction=-1*pldata[i]->plate.direction;
                    pldata[i]->plate.x=2*gameData->area.width-pldata[i]->plate.x;
                }
                else
                    pldata[i]->plate.x+=travelled_x;


                if(travelled_y+pldata[i]->plate.y<0)
                {
                    pldata[i]->plate.direction=-1*pldata[i]->plate.direction;
                    pldata[i]->plate.y=-1*(travelled_y+pldata[i]->plate.y);
                }
                else if(travelled_y+pldata[i]->plate.y>gameData->area.height)
                {
                    pldata[i]->plate.direction=-1*pldata[i]->plate.direction;
                    pldata[i]->plate.y=2*gameData->area.height-pldata[i]->plate.y;
                }
                else
                    pldata[i]->plate.y+=travelled_y;
         }
        /* Move ball(s) */
        for(i=0;i< gameData->ball_amnt && i<BALLS_MAX ;i++)
        {
            float tmp_x=0.0;
            float tmp_y=0.0;
            int check=0;
            unsigned char *checktop=(unsigned char *)&check;
            unsigned char *checkbottom=checktop+1;
            unsigned char *checkleft=checkbottom+1;
            unsigned char *checkright=checkleft+1;
            SBallData *pball=&(gameData->balldataArray[i]);

            printf
            (
                "ball%d OLD: pos (%f,%f), speed = (%f,%f), dir =(%d,%d)\n",
                i,
                pball->xpos,
                pball->ypos,
                pball->speed_x,
                pball->speed_y,
                pball->direction_x,
                pball->direction_y
                );

            tmp_x=TRANSITION_BASEDON_VELOCITYVALUE_AND_MILLISECONDS((float)pball->direction_x*pball->speed_x,reftime);
            tmp_y=TRANSITION_BASEDON_VELOCITYVALUE_AND_MILLISECONDS((float)pball->direction_y*pball->speed_y,reftime);
            printf("transition = (%f,%f)\n",tmp_x,tmp_y);
            tmp_x+=pball->xpos;
            tmp_y+=pball->ypos;
            if(tmp_x<0)
            {
                hit=1;
                pball->direction_x=1; /* Change position to be increasing */
                tmp_x*=-1.0;
            
                /* Check if the wall can pass thru the ball */
                if(gameData->walls[EWallPosition_Left].type == EWallType_None)
                {
                    *checkleft=1;
                }
            }
            else if(tmp_x>(float)gameData->area.width)
            {
                hit=1;
                pball->direction_x=-1;
                tmp_x=2.0*(float)gameData->area.width-tmp_x;
                /* Check if there is plate in place */
                if(gameData->walls[EWallPosition_Right].type == EWallType_None)
                {
                    *checkright=1;
                }
            }
            if(tmp_y<0)
            {
                hit=1;
                pball->direction_y=1;
                tmp_y*=-1.0;
                /* Check if there is plate in place */
                if(gameData->walls[EWallPosition_Up].type == EWallType_None)
                {
                    *checktop=1;
                }

            }
            else if(tmp_y>(float)gameData->area.height)
            {
                hit=1;
                pball->direction_y=-1;
                tmp_y=2.0*(float)gameData->area.height-tmp_y;
                /* Check if there is plate in place */
                if(gameData->walls[EWallPosition_Bottom].type == EWallType_None)
                {
                    *checkbottom=1;
                }

            }
            pball->xpos=tmp_x;
            pball->ypos=tmp_y;
            printf("Ball at (%f,%f) - time quanta %lld\n",tmp_x,tmp_y,reftime);
            if(check)
            {
                for(i=0;i<amnt_of_players;i++)
                {
                    /* Check if there is plate in place */
                    switch(pldata[i]->plate.wall)
                    {
                        case EWallPosition_Left:
                            if(*checkleft)
                            {
                                /* This is not accurate enough - this does not check position where ball hit the wall, but position where ball was at the end of time quanta */
                                if(pldata[i]->plate.y >= tmp_y && pldata[i]->plate.y + pldata[i]->plate.width<=tmp_y)
                                    printf("Ball hit the plate %d at (x=0,y=%f)\n",i,tmp_y);
                                else
                                    printf("Player %d miss ball at (%f,%f)\n",i,tmp_x,tmp_y);
                            }
                            break;
                        case EWallPosition_Right:
                            if(*checkright)
                            {
                                /* This is not accurate enough - this does not check position where ball hit the wall, but position where ball was at the end of time quanta */
                                if(pldata[i]->plate.y >= tmp_y && pldata[i]->plate.y + pldata[i]->plate.width<=tmp_y)
                                    printf("Ball hit the plate %d at (x=%f,y=%f)\n",i,tmp_x,tmp_y);
                                else
                                    printf("Player %d miss ball at (%f,%f)\n",i,tmp_x,tmp_y);
                            }
                            break;

                        case EWallPosition_Up:
                            if(*checktop)
                            {
                                /* This is not accurate enough - this does not check position where ball hit the wall, but position where ball was at the end of time quanta */
                                if(pldata[i]->plate.x >= tmp_x && pldata[i]->plate.x + pldata[i]->plate.width<=tmp_x)
                                    printf("Ball hit the plate %d at (x=%f,y=0)\n",i,tmp_x);
                                else
                                    printf("Player %d miss ball at (%f,%f)\n",i,tmp_x,tmp_y);
                            }
                            break;

                        case EWallPosition_Bottom:
                            if(*checktop)
                            {
                                /* This is not accurate enough - this does not check position where ball hit the wall, but position where ball was at the end of time quanta */
                                if(pldata[i]->plate.x >= tmp_x && pldata[i]->plate.x + pldata[i]->plate.width<=tmp_x)
                                    printf("Ball hit the plate %d at (x=%f,y=0)\n",i,tmp_x);
                                else
                                    printf("Player %d miss ball at (%f,%f)\n",i,tmp_x,tmp_y);
                            }
                        default:
                            printf("Invalid plate position %u\n",pldata[i]->plate.wall);
                        } /* switch */
                    } /* for amnt of players */
                } /* If we hit border and check is needed */
                if(hit)
                    send_to_clients(gameData);
                hit=0;
            }/* for ball amount */
        } /* While loop */

//         printf(1,"Ensure the lives check done somewhere else\n");
//         printf ("Add ball position calc at %s:%d\n",__FILE__,__LINE__);
//         usleep(1);
     printf("Sched Thread Exiting..");
     
     return NULL;
     
}
