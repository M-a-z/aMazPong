/* ************************************************************************** */
/*
 *    This header will define structs in which the data defining ball an paddle
 *    properties is stored.
 *
 *    Revision history:
 *
 *
 *    -0.0.5  19.02.2009/Maz  changed enum boolean to Eboolean due to some minGW naming clashes.
 *    -0.0.4  06.01.2009/Maz  Further rewrite. This will be data structs for server.
 *    -0.0.3  05.01.2009/Maz  Started rewriting datastructs + added poolIdFifo
 *    -0.0.2  12.06.2008/Maz  Added struct for ball data.
 *    -0.0.1  04.06.2008/Maz  First Draft.
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */

#ifndef PONG_DATA_STORAGE_H
#define PONG_DATA_STORAGE_H

#include <stdlib.h>
#include <stdio.h>
#ifndef __LINUX__
#include <windows.h>
#endif
#include <math.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include "general.h"
//#include "pong.h"
#include "pong_queue.h"
#include "id_fifo.h"
/* This is ugly as hell */
#define SERVERQUEUELEN 255
#define SERVER_RX_QUEUELEN 20

typedef struct SServerSendQueue
{
    SSendQueue genqueue;
    SQueueItem data_array[SERVERQUEUELEN];
    SQueueItem rxdata_array[SERVER_RX_QUEUELEN];
}SServerSendQueue;


typedef enum EWallType
{
    EWallType_None = 0,
    EWallType_Normal = 1,
    EWallType_NmbrOf	
}EWallType;
typedef enum EWallPosition
{
    EWallPosition_Left = 0,
    EWallPosition_Right = 1,
    EWallPosition_Up = 2,
    EWallPosition_Bottom = 3,
    EWallPosition_NmbrOf
}EWallPosition;
/*
typedef enum EPlayerHome
{
    EPlayerHome_Left=0,
    EPlayerHome_Right=1,
    EPlayerHome_Up=2,
    EPlayerHome_Bottom=3,
    EPlayerHome_NmbrOf
}EPlayerHome;
*/
        

typedef struct SDataDelivery
{
    int mode;
    int player_no;
    void *data;
    size_t datasize;
}SDataDelivery;

//TODO: Use typedef int plateSpecials
//set to 0x00000000 at start-
//Use bitmasks like
//0x0000000F == wide  (#define PLATE_SPECIAL_WIDE 0x0000000F )
//0x000000F0 == stuck (#define PLATE_SPECIAL_STUCK 0x000000F0 )
//0x00000F00 == fast  (#define PLATE_SPECIAL_FAST 0x00000F00 )
//And combinations with PLATE_SPECIAL_STUCK | PLATE_SPECIAL_WIDE
/*
typedef enum EWallPosition
{
    EWallPosition_up = 0,
    EWallPosition_down=1,
    EWallPosition_left=2,
    EWallPosition_right=3,
    EWallPosition_Last
}EWallPosition;
*/
typedef struct SWallInfo
{
    EWallType type;
    EWallPosition wpos;
}SWallInfo;

typedef struct SPlateSpecials
{
    int wide;
    int stuck;
    int fast;
}SPlateSpecials;

typedef struct SPlateInfo
{
        float x;
        float y;
        float z;
        int width;
        int direction;
        float speed;
        SPlateSpecials specials;
        EWallPosition wall;
}SPlateInfo;
/*
typedef struct SPlayerSereverDetails
{
        
        
}SPlayerServerDetails;
*/

/*
    ToDo: Should reconsider placing mutex ptrs straight into these structs. 
    Maybe we do not want to send them with the data through the network since they're useless at the receiving side. 
*/

typedef struct SPlayerData
{
    pthread_mutex_t playerDataLock;
	SOCKET sock;
    char playerName[PLAYER_NAME_MAX];  //for sending this needs to be char playername[name_max]
	int player_alive;
    SPlateInfo plate;
    unsigned int lives;
//    SPlayerSereverDetails serverInfo;
    unsigned int userId;
    int player_has_joined;
}SPlayerData;

typedef struct SBallData
{
    pthread_mutex_t ballDataLock; 
    int ball_active;
    int direction_x;
    int direction_y;
    float speed_x; /* velocity value == 1 / (virtual pixels/micosecond) 
                             pixels / microsec is  much less than 1 =>
                             1/ (pixels / microsec) is a largish integer, larger the value slover the ball
                             See macros PIXELS_PER_MICROSEC_TO_VELOCITYVALUE() PIXELS_PER_MSEC_TO_VELOCITYVALUE()
                             in common/definitions.h */
    float speed_y; /* (virtual pixels/micosecond) */
    float xpos; /* position in virtual coord - sized as DEFAULT_AREA_WIDTH */
    float ypos; /* DEFAULT_AREA_HEIGHT */
    int special;
}SBallData;

// ToDo: Consider do we need to send whole gamedata sometimes?
// If so, ptrs should be replaced with arrays...
// perhaps it is easier to just collect relevant infos in individual structs, and send those.
// GameData struct can be used just to bind this all together.
typedef struct SHandshakeStatus
{
    unsigned char helloed;
    unsigned char init_data_recvd;
    unsigned char join_req_recvd;
    unsigned char game_running;
    unsigned int passw_errors;
    unsigned int game_id;
}SHandshakeStatus;
typedef struct SGameArea
{
    int width;
    int height;
}SGameArea;
typedef struct SGameData
{
    pthread_mutex_t gameDataLock;
    unsigned char gameDataInited;
	unsigned char players_joined;
    SHandshakeStatus handshakeStatus;
    float currentNormalPlateSpeed;
    char game_name[GAME_NAME_MAX];
    SGameArea area;
    unsigned char player_amnt;
    unsigned int gameId;
    unsigned int ball_amnt;
	unsigned int clients_alive;
    SWallInfo walls[4];
    SBallData balldataArray[BALLS_MAX];
    SPlayerData players[CLIENTS_MAX];
	char game_passwd[PASSWD_MAX];
    int abort_game;
    SIdFifo *playerIdFiFO; 
    SServerSendQueue serverqueue;
}SGameData;

typedef struct SClientGameData
{
    char game_name[GAME_NAME_MAX];
    SGameArea area;
    unsigned int player_amnt;
    unsigned int gameId;
    unsigned int ball_amnt;
    unsigned int clients_alive;
    SWallInfo walls[4];
    SBallData BalldataArray[BALLS_MAX];
    SPlayerData players[CLIENTS_MAX];
    unsigned int myplayer_index;
}SClientGameData;


#define GAME_DATA_LOCK(sgamedataPtr) pthread_mutex_lock(&((sgamedataPtr)->gameDataLock));
#define GAME_DATA_UNLOCK(sgamedataPtr) pthread_mutex_unlock(&((sgamedataPtr)->gameDataLock));
Eboolean player_data(SDataDelivery *keydata);


void init_dataStorage();
SGameData* init_dataStorageForGame(const char *game_name);
SGameData* gamedata_lookup(const char *game_name);
unsigned int create_player_id(SGameData *gameData);
SPlayerData *playerdata_get(unsigned int gameId, unsigned int player_id);
int SERVER_setup_game_generics(SGameData *gameData);
int SERVER_add_player_to_gamedata(SGameData *gameData,char *player_name);
int SERVER_copyGamedataForClientById(SClientGameData *,int gameId);


#endif //PONG_DATA_STORAGE_H
