/* ************************************************************************** */
/*
 *    This file will contain functions to serve and update data on server 
 *    storage based on client's requests.
 *
 *    Revision history:
 *
 *    -0.0.4  19.02.2009/Maz  changed enum boolean to Eboolean due to some minGW naming clashes.
 *    -0.0.3  06.01.2009/Maz  Started changing this file to be the server's data storage.
 *    						  Actually, this will be quite clean. Server will access game
 *    						  data straight, and it is protected by mutex in data struct
 *    						  It will make it harder to update underlying data containers,
 *    						  but what the hell, this project has already lasted year and 
 *    						  half, so I guess it is better to try doing this quickly than
 *    						  wasting a few months implementing some wrapper funcs around
 *    						  the data access...
 *    -0.0.2  18.09.2008/Maz  Ironed out some compile errors.    
 *    -0.0.1  04.06.2008/Maz  First Draft.
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */


#ifndef __LINUX__
#include <windows.h>
#endif
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <general.h>
//#include "pong.h"
#include "pong_dataStorage.h"
#include "id_fifo.h"
//#include "place_counter_thread.h"

SIdFifo *G_gameIdFifo = NULL;
//TODO:
       //This should be written as independent process, serving and getting data via sockets.


//Lets just first create a simple version with global data struct, so I'll see easily that the data struct is reasonable.

static SGameData G_gameDataArr[MAX_GAMES_ON_SERVER];
void get_random_pos_at_area(float *xpos, float *ypos, SGameArea *area)
{
    *xpos=(float)(rand()%(area->width-20))+10;
    *ypos=(float)(rand()%(area->height-20))+10;
}

void get_random_y_dir(int *d)
{
    *d=(rand()%1)?D_DIRECTION_DOWN:D_DIRECTION_UP;
}
void get_random_x_dir(int *d)
{
    *d=(rand()%1)?D_DIRECTION_RIGHT:D_DIRECTION_LEFT;
}
float get_random_velocity_1d(float maxVelocity,float minVelocity)
{
    /* Note, real velocity value is  ((pixels/microsec)) */
    return ( ((float) maxVelocity- (float)minVelocity)*((float)rand()/RAND_MAX)+(float)minVelocity);
//    return (rand()%(minVelocity-maxVelocity))+maxVelocity;
}
void get_random_velocity(float *velocityX,float *velocityY,float maxVelocity,float minVelocity)
{
    /* Note, real velocity ((pixels/microsec))*/
    *velocityX=( ((float) maxVelocity- (float)minVelocity)*((float)rand()/RAND_MAX)+(float)minVelocity);
    *velocityY=( ((float) maxVelocity- (float)minVelocity)*((float)rand()/RAND_MAX)+(float)minVelocity);
  // *velocityX=(rand()%(minVelocity-maxVelocity))+maxVelocity;
  // *velocityY=(rand()%(minVelocity-maxVelocity))+maxVelocity;
}
void uninit_ballslot(SBallData *bdata)
{
    pthread_mutex_lock(&(bdata->ballDataLock));
    memset( ((char *)bdata)+sizeof(pthread_mutex_t),0,sizeof(SBallData)-sizeof(pthread_mutex_t));
    pthread_mutex_unlock(&(bdata->ballDataLock));
}
int init_ballslot(SGameData *gdata,int ballindex,float maxVelocity,float minVelocity)
{
    SBallData *bdata=&(gdata->balldataArray[ballindex]);
//    pthread_mutex_init(&(bdata->ballDataLock),NULL);
    bdata->ball_active=1;
    get_random_pos_at_area(&(bdata->xpos),&(bdata->ypos),&(gdata->area));
    get_random_x_dir(&(bdata->direction_x));
    get_random_y_dir(&(bdata->direction_y));
    get_random_velocity(&(bdata->speed_x),&(bdata->speed_y),maxVelocity,minVelocity);
    return 0;
}
int default_init_walldata(SGameData *gdata)
{
    int i;
    for(i=0;i<4;i++)
    {
        gdata->walls[i].wpos=i;
        gdata->walls[i].type=EWallType_Normal;
    }
    return 0;
}
int SERVER_copyGamedataForClientById(SClientGameData *CligameData,int gameId)
{
    static int print=0;
    if(!print)
    printf("copyGamedataForClientById - not yet done!\n");
    print++;
    return 0;
}
int SERVER_setup_game_generics(SGameData *gameData)
{
    gameData->area.width=DEFAULT_AREA_WIDTH;
    gameData->area.height=DEFAULT_AREA_HEIGHT;
    gameData->ball_amnt=1;
    gameData->currentNormalPlateSpeed= get_random_velocity_1d(DEFAULT_PLATES_START_MAX_VELOCITY,DEFAULT_PLATE_START_MIN_VELOCITY);
    if(init_ballslot(gameData,0,DEFAULT_BALL_START_MAX_VELOCITY,DEFAULT_BALL_MIN_VELOCITY))
    {
        printf("Failed to init Ball info\n");
        return -1;
    }
    if(default_init_walldata(gameData))
    {
        printf("Failed to init Wall data!\n");
        return -1;
    }
    return 0;
}
SBallData *getClosestBall(SGameData *gameData, SPlateInfo *pla)
{
    int i;
    SBallData *closest=NULL;
    int compvalue=-1;
    printf("detecting closest ball\n");
    for(i=0;i<BALLS_MAX;i++)
    {
        printf("Inspecting ball %d\n",i);
        if(gameData->balldataArray[i].ball_active)
        {
            printf("ball %d active\n",i);
            switch(pla->wall)
            {
                case EWallPosition_Up:
                {
                    int myval=(gameData->balldataArray[i].direction_y==D_DIRECTION_DOWN)?
                        (2*gameData->area.height-(int)gameData->balldataArray[i].ypos):
                        (int)gameData->balldataArray[i].ypos;

                    printf("Baddle at top (y==0)\n");
                    printf("direction is %s\n",(gameData->balldataArray[i].direction_y==D_DIRECTION_DOWN)?"down":"up");
                    printf("ball is at y=%f\n",gameData->balldataArray[i].ypos);
                    printf("calculated myval=%d\n",myval);
                    if(compvalue==-1 || myval<compvalue)
                    {
                        compvalue=myval;
                        closest=&(gameData->balldataArray[i]);
                    }
                    
                    break;
                }
                case EWallPosition_Bottom:
                {
                    int myval=(gameData->balldataArray[i].direction_y==D_DIRECTION_DOWN)?
                        (gameData->area.height-(int)gameData->balldataArray[i].ypos):
                        gameData->area.height+(int)gameData->balldataArray[i].ypos;

                    printf("Baddle at bottom (y==400)\n");
                    printf("direction is %s\n",(gameData->balldataArray[i].direction_y==D_DIRECTION_DOWN)?"down":"up");
                    printf("ball is at y=%f\n",gameData->balldataArray[i].ypos);
                    printf("calculated myval=%d\n",myval);
                    if(compvalue==-1 || myval<compvalue)
                    {
                        compvalue=myval;
                        closest=&(gameData->balldataArray[i]);
                    }
                    
                    break;
                }
                case EWallPosition_Left:
                {
                    int myval=(gameData->balldataArray[i].direction_x==D_DIRECTION_LEFT)?
                        (gameData->area.width-(int)gameData->balldataArray[i].xpos):
                        gameData->area.height+(int)gameData->balldataArray[i].xpos;

                    printf("Baddle at left (x==0)\n");
                    printf("direction is %s\n",(gameData->balldataArray[i].direction_y==D_DIRECTION_LEFT)?"left":"right");
                    printf("ball is at x=%f\n",gameData->balldataArray[i].xpos);
                    printf("calculated myval=%d\n",myval);
                    if(compvalue==-1 || myval<compvalue)
                    {
                        compvalue=myval;
                        closest=&(gameData->balldataArray[i]);
                    }
                    
                    break;
                }
                case EWallPosition_Right:
                {
                    int myval=(gameData->balldataArray[i].direction_x==D_DIRECTION_RIGHT)?
                        (gameData->area.width-(int)gameData->balldataArray[i].xpos):
                        gameData->area.width+(int)gameData->balldataArray[i].xpos;

                    printf("Baddle at right (x==600)\n");
                    printf("direction is %s\n",(gameData->balldataArray[i].direction_y==D_DIRECTION_LEFT)?"left":"right");
                    printf("ball is at x=%f\n",gameData->balldataArray[i].xpos);
                    printf("calculated myval=%d\n",myval);
                    if(compvalue==-1 || myval<compvalue)
                    {
                        compvalue=myval;
                        closest=&(gameData->balldataArray[i]);
                    }
                    
                    break;
                }
                default:
                    printf("Invalid plate position encounetered!\n");
                    break;
            } //end of switch
        }
        else
            printf("ball %d not active\n",i);
    }
    return closest;
}
int init_plate(SGameData *gameData, SPlateInfo *pla)
{
    /* 0=>left, 1=> right, 2=top,3=bottom */
    SBallData *closestBall;
    pla->wall=gameData->players_joined;
    pla->width=PADDLE_START_WIDTH;
    closestBall=getClosestBall(gameData,pla);
    if(!closestBall)
    {
        printf("Failed to find closest ball!\n");
        return -1;
    }
    printf("Set paddle positions correctly, other coord to edge, other where closest ball is\n");
    if(pla->wall==EWallPosition_Right)
    {
        pla->y = closestBall->ypos;
        pla->x = (float)gameData->area.width;
    }
    else if(pla->wall==EWallPosition_Left)
    {
        pla->y = closestBall->ypos;
        pla->x = 0.0;
    }
    else if(pla->wall==EWallPosition_Up)
    {
        pla->x = closestBall->xpos;
        pla->y = (float)gameData->area.height;
    }
    else if(pla->wall==EWallPosition_Bottom)
    {
        pla->x = closestBall->xpos;
        pla->y = 0.0;
    }
    else
    {
        printf("Invalid player wall position %d\n",pla->wall);
        return -1;
    }
    printf("New plate found at (x=%f,y=%f)\n",pla->x,pla->y);
    pla->direction=0;
    pla->speed=gameData->currentNormalPlateSpeed;
    pla->specials.wide=Efalse;
    pla->specials.stuck=Efalse;
    pla->specials.fast=Efalse;
    return 0;
}
int player_init(SGameData *gameData,SPlayerData *pl,char *player_name)
{ 
    pl->player_alive=1;
    pthread_mutex_init(&(pl->playerDataLock),NULL);
    memcpy(pl->playerName,player_name,PLAYER_NAME_MAX);
    pl->playerName[PLAYER_NAME_MAX-1]='\0';
    pl->lives=D_PLAYER_LIVES;
    pl->player_has_joined=1;
    if(init_plate(gameData,&(pl->plate)))
    {
        printf("player_init: Plate Init FAILED!\n");
        return -1;
    }
    return 0;
}
int SERVER_add_player_to_gamedata(SGameData *gameData,char *player_name)
{
    SPlayerData *pl;
    int i;
    if(gameData->players_joined>=gameData->player_amnt)
    {
        printf("Too many players trying to JOIN!\n");
        return -1;
    }
    pl=&(gameData->players[gameData->players_joined]);
    if(player_init(gameData,pl,player_name))
    {
        printf("Failed to init player data!\n");
        return -1;
    }
    for(i=0;i<4;i++)
        if( gameData->walls[i].wpos == pl->plate.wall)
            gameData->walls[i].type = EWallType_None;
    gameData->clients_alive++;
    gameData->players_joined++;
    return 0;
}

void init_dataStorage()
{
    size_t gameIdFifoSize=0;
    gameIdFifoSize=IdPool_calcAllocSize(MAX_GAMES_ON_SERVER);
    G_gameIdFifo=malloc(gameIdFifoSize);
    if(NULL==G_gameIdFifo)
    {
        printf("GameIdPool (size %u) Allocation FAILED at %s:%d",gameIdFifoSize,__FILE__,__LINE__);
        fflush(stdout);
        exit(1);
    }
    fifopool_init(G_gameIdFifo,1,MAX_GAMES_ON_SERVER);
    memset(G_gameDataArr,0,sizeof(G_gameDataArr));

}
void init_gamedata_mutexes(SGameData* gameData)
{
    int i;
    int pos;
    size_t mutexOffsetArray[MUTEX_AMOUNT_IN_GAME_DATA]=
    {
        offsetof(SGameData,gameDataLock), /* First mutex */
        offsetof(SGameData,balldataArray)
    };
    printf("GameData start is at 0x%p\n",gameData);
    pos=2;
    for(i=0;i<BALLS_MAX-1;i++)
        mutexOffsetArray[i+pos]=mutexOffsetArray[i+pos-1]+sizeof(SBallData);
    pos+=i;
    mutexOffsetArray[pos]=offsetof(SGameData,players);
    pos++;
    for(i=0;i<CLIENTS_MAX-1;i++)
        mutexOffsetArray[i+pos]=mutexOffsetArray[i+pos-1]+sizeof(SPlayerData);

    for(i=0;i<MUTEX_AMOUNT_IN_GAME_DATA;i++)
    {
        pthread_mutex_init((pthread_mutex_t *) (((char *)gameData)+mutexOffsetArray[i]),NULL);
        printf("Initializing mutex at 0x%p\n",(((char *)gameData)+mutexOffsetArray[i]));
        printf("Offset from GameData start is %u\n",mutexOffsetArray[i]);
        printf("Diff to prev offset %d\n",(0==i)?0:mutexOffsetArray[i]-mutexOffsetArray[i-1]);
    }
//    return;
}
SGameData* init_dataStorageForGame(const char *game_name)
{
    unsigned int gameId=idpool_idReserve(G_gameIdFifo);
    int namelen;
    if(gameId==IDFIFO_ID_INVALID)
    {
        printf("Could not allocate game Id, idPool FULL?");
        //TODO: return errorcode and let this game thread die instead of killing whole server... 
        return NULL;
    }
    memset(&(G_gameDataArr[gameId-1]),0,sizeof(SGameData));
    init_gamedata_mutexes(&(G_gameDataArr[gameId-1]));
    namelen=strlen(game_name);
    if(namelen<=0 || namelen>=GAME_NAME_MAX)
    {
        printf("Invalid game name %s",game_name);
        return NULL;
    }
    printf("Created game data for game '%s'\n",game_name);
    strcpy(G_gameDataArr[gameId-1].game_name,game_name);
    printf("GameDataStorage: init_dataStorageForGame - creating new game storage, game ID %d, name %s\n",gameId,game_name);
    G_gameDataArr[gameId-1].gameId=gameId;
//perhaps RWlock would be better than mutex?
    pthread_mutex_init(&(G_gameDataArr[gameId-1].gameDataLock),NULL);
//    G_gameDataArr[gameId-1].
    G_gameDataArr[gameId-1].gameDataInited = 1;
    return &(G_gameDataArr[gameId-1]);
}
// How to prevent situation where 
// 1. client connects
// 2. server performs this check
// 3 client 2 connects
// 4 server performs this check
// 5. client 1 updates game_name
// 6. client 1 is asked to send init data
// 7 client 2 is asked to send init data...
// Answer is to loxk a mutex and update game name here right after the check.
// No. It is a bad answer... I need to think of a decent synchronization mechanism for this whole scenario...
SGameData* gamedata_lookup(const char *game_name)
{
    int i;
    for(i=0;i<MAX_GAMES_ON_SERVER;i++)
    {
        if(1==G_gameDataArr[i].gameDataInited)
        {
            printf("gamedata_lookup: found inited game '%s', looking for '%s'\n",G_gameDataArr[i].game_name,game_name);
            GAME_DATA_LOCK(&G_gameDataArr[i]);
            if(!strncmp(G_gameDataArr[i].game_name,game_name,GAME_NAME_MAX))
            {
                GAME_DATA_UNLOCK(&G_gameDataArr[i]);
                printf("GameDataStorage: gamedata_lookup - existing game data found for game %s, returning that data\n",game_name);
                return &G_gameDataArr[i];
            }
            GAME_DATA_UNLOCK(&G_gameDataArr[i]);
        }
    }
    printf("GameDataStorage: gamedata_lookup - existing game data NOT found for game %s\n",game_name);
    return init_dataStorageForGame(game_name);
}

unsigned int create_player_id(SGameData *gameData)
{
	unsigned int player_id=0;
    if(gameData->player_amnt==0)
	{
		gameData->playerIdFiFO=malloc(IdPool_calcAllocSize(CLIENTS_MAX));
		//TODO: first player => init playerid Fifo
		fifopool_init(gameData->playerIdFiFO,1,CLIENTS_MAX);
	}
	if(IDFIFO_ID_INVALID==(player_id=idpool_idReserve(gameData->playerIdFiFO)))
	{
		printf("Reserving playerId failed at %s:%d",__FILE__,__LINE__);
		return 0;
	}
	gameData->players[player_id-1].userId=player_id;

    //Should we update socket etc for player data here, or at calling func??
	
	return player_id; 
}
//TODO: Fix this when schedthread is fixed.
SPlayerData *playerdata_get(unsigned int gameId, unsigned int player_id)
{
	pong_assert(player_id>=CLIENTS_MAX,"YaY! Invalid player ID requested!");
//	pong_assert(G_gameDataArr[gameId-1].gameId==0,"YAY! Uninitialized gamedata in Array where player data requested!");
    if(G_gameDataArr[gameId-1].gameId==0)
        return NULL;
	pong_assert(!G_gameDataArr[gameId-1].players[player_id-1].player_has_joined,"YAY! Player whose data was requested has not joined!");
	return &(G_gameDataArr[gameId-1].players[player_id-1]);
}
/* Some obsoleted code??
Eboolean player_data(SDataDelivery *data)
{
	int tmp_i;
	static SPlayerData player[D_MAX_PLAYER];

	switch(data->mode)
	{
		case DINIT_PLAYER_DATA_STORAGE:
			player[0].plate.y=glutGet(GLUT_WINDOW_HEIGHT)/2;
			player[0].plate.x=1;
			player[0].plate.z=0;
			player[0].plate.direction=D_DIRECTION_STANDBY;
			player[0].plate.specials.wide=Efalse;
			player[0].plate.specials.fast=Efalse;
			player[0].plate.specials.stuck=Efalse;
			player[0].lives=D_PLAYER_LIVES;

            player[0].plate.y=glutGet(GLUT_WINDOW_HEIGHT)/2;
            player[0].plate.x=glutGet(GLUT_WINDOW_WIDTH)-1;
            player[0].plate.z=0;
            player[0].plate.direction=D_DIRECTION_STANDBY;
            player[0].plate.specials.wide=Efalse;
            player[0].plate.specials.fast=Efalse;
            player[0].plate.specials.stuck=Efalse;
            player[0].lives=D_PLAYER_LIVES;

		break;
		case DPLAYER_KEYPRESS:
			switch(tmp_i=*((int*)(data->data)) )
			{
				case DPLAYER1_KEY_UP:
					player[0].plate.direction=D_DIRECTION_UP;
                    player[0].plate.y-=2;
                break;
                case DPLAYER1_KEY_DOWN:
                	player[0].plate.direction=D_DIRECTION_DOWN;
                    player[0].plate.y+=2;
                break;
                case DPLAYER2_KEY_UP:
					player[1].plate.direction=D_DIRECTION_UP;
                break;
                case DPLAYER2_KEY_DOWN:
                	player[1].plate.direction=D_DIRECTION_DOWN;
                break;
                default:
                	fprintf(stderr,"Unknown keypress message in data container!");
                	exit(0);
                break;
			}
                				 
		break;
		case DPLAYER_PLATE_QUERY:
			if(data->player_no<0||data->player_no>=D_MAX_PLAYER)
			{
				fprintf(stderr,"Bad player no given for storage function!");
				exit(0);
			}
			data->data=realloc((void *)data->data,sizeof(SPlateInfo));
			if(NULL==data->data)
			{
				fprintf(stderr,"Realloc failed, OutOfMem?");
				exit(0);
			}
			memcpy(((SDataDelivery*)data)->data,&player[data->player_no].plate,sizeof(SPlateInfo));
			((SDataDelivery*)data)->datasize=sizeof(SPlateInfo);
			 
		break;
		case DPLAYER_SPECIALS_QUERY:
			if(data->player_no<0||data->player_no>=D_MAX_PLAYER)
			{
				fprintf(stderr,"Bad player no given for storage function!");
				exit(0);
			}
			data->data=realloc((void *)data->data,sizeof(SPlateSpecials));
			if(NULL==data->data)
			{
				fprintf(stderr,"Realloc failed, OutOfMem?");
				exit(0);
			}
			*((SPlateSpecials*)data->data)=player[data->player_no].plate.specials;
		break;
		case DPLAYER_DATA_QUERY:
			if(data->player_no<0||data->player_no>=D_MAX_PLAYER)
			{
				fprintf(stderr,"Bad player no given for storage function!");
				exit(0);
			}
			data->data=realloc((void *)data->data,sizeof(SPlayerData));
			if(NULL==data->data)
			{
				fprintf(stderr,"Realloc failed, OutOfMem?");
				exit(0);
			}
			*((SPlayerData*)data->data)=player[data->player_no];
		break;
		case DUPDATE_PLATE:
			if(data->player_no<0||data->player_no>=D_MAX_PLAYER)
			{
				fprintf(stderr,"Bad player no given for storage function!");
				exit(0);
			}
			player[data->player_no].plate=*((SPlateInfo*)data->data);
			free(data->data);
			data->data=NULL;
		break;
		case DUPDATE_SPECIALS:
			if(data->player_no<0||data->player_no>=D_MAX_PLAYER)
			{
				fprintf(stderr,"Bad player no given for storage function!");
				exit(0);
			}
			player[data->player_no].plate.specials=*((SPlateSpecials*)data->data);
			free(data->data);
			data->data=NULL;
		break;
		case DUPDATE_PLAYER:
			if(data->player_no<0||data->player_no>=D_MAX_PLAYER)
			{
				fprintf(stderr,"Bad player no given for storage function!");
				exit(0);
			}
		    player[data->player_no].plate=*((SPlateInfo*)data->data);
			free(data->data);
			data->data=NULL;
		break;
		default:
		fprintf(stderr,"bad player data update mode 0x%x",data->mode);
		exit(1);
		break;
	}
	return Etrue;
}
*/
