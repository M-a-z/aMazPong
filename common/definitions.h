/* ************************************************************************** */
/*
 * The purpose of this file is to provide Generic definitions.
 *
 *    Revision history:
 *
 *    -0.0.5  19.02.2009/Maz  changed enum boolean to Eboolean due to some minGW naming clashes.
 *    						  Added version definition.
 *    -0.0.4  18.09.2008/Maz  Ironed out some compile errors.    
 *    -0.0.3  19.06.2008/Maz  Added EplatePosition
 *    -0.0.2  12.06.2008/Maz  Some winsock API related additions.
 *    -0.0.1  11.06.2008/Maz
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */



#ifndef DEFINITIONS_H
#define DEFINITIONS_H
#define G_CLIENTVER "0.000001"
#define DEBUG

#include "pong_assert.h"

#ifndef __LINUX__
        #define pong_sock_close(foo) closesocket(foo)           
        #include <winsock2.h>
        #define sleep Sleep
#else 
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
        #define pong_sock_close(foo) close(foo)
        #define INVALID_SOCKET -1
        typedef int SOCKET;
#endif

#define SECS_IN_MILLISEC(time) ((time)/1000)
#define MICRSECS_IN_MILLISECS(t1me) ((time)*1000) 
#define PONG_RET_CP_INVALID -666
#ifdef DEBUG
#define DEBUGPRINT printf
#else
#define DEBUGPRINT(...) void(0);
#endif
typedef enum Eboolean { Efalse=0x0000,Etrue=0x0001,Einvalid_boolean } Eboolean;
typedef long long int int_64;
typedef unsigned long long int uint_64;

typedef enum EPongRet
{
    EPongRet_ERROR = 0,
    EPongRet_SUCCESS = 1
}EPongRet;
/*
 * EWallPosition used instead
typedef enum EplatePosition
{
    EplatePosition_Left=0,
    EplatePosition_Right=1,
    EplatePosition_Top=2,
    EplatePosition_Bottom=3
}EplatePosition;
*/
#define MUTEX_AMOUNT_IN_GAME_DATA (1+BALLS_MAX+CLIENTS_MAX)
//maximum number of clients / server. Limited to 4 atm. - actually, this used to indicate clients/game...
#define CLIENTS_MAX 4
#define GAMES_MAX 1
#define BALLS_MAX 5
#define GAME_NAME_MAX 16
#define MAX_GAMES_ON_SERVER 10
#define INVALID_PLAYER -1
#define PLAYER_AMNT_UNDEF -1
#define PASSWD_MAX 24
#define ERROR_TOLERANCE 10
//ticks to msecs.
#define PIX_PER_SEC CLOCKS_PER_SEC/1000  

//msecs
/* after 10 secs of silence, just give up. We have probs lost server */
#define INGAME_SERVERMSG_TMO 10000

#define HELLO_MSG_TMO 1000
/* 60 secs for all clients to join */
#define GAME_READY_TMO 60000

#define SERVER_RECV_TMO 100000

#define DEFAULT_AREA_WIDTH 400
#define DEFAULT_AREA_HEIGHT 400

#define GAME_TIME_QUANTA_MSEC 10

/* Allright. I changed velocity value to float && straight to pixels/msec => below macros changed */

#define DEFAULT_BALL_MIN_VELOCITY 0.1
#define DEFAULT_BALL_START_MAX_VELOCITY 0.5
#define PIXELS_PER_MSEC_TO_VELOCITYVALUE(velval) ((float)(velval))
#define PIXELS_PER_MICROSEC_TO_VELOCITYVALUE(velval) ((float)(velval)/1000)
#define TRANSITION_BASEDON_VELOCITYVALUE_AND_MILLISECONDS(velval,msecs) ((float)(velval)*(float)(msecs))

//#define TRANSITION_BASEDON_VELOCITYVALUE_AND_MILLISECONDS(velval,msecs) ((float)(msecs) / (float)((velval)/1000))
/* Macro to convert understandable value (pixels/msec) to integer value describing the speed */
//#define PIXELS_PER_MICROSEC_TO_VELOCITYVALUE(velval) (unsigned int)((double)1.0/(double)(velval))
//#define PIXELS_PER_MSEC_TO_VELOCITYVALUE(velval) ((unsigned int)((double)1.0/(double)(velval))*(float)1000)
/* Max velocity is here 0.5 => 0.5 pixel/millisec => 5 pixel/10 msec */
//#define DEFAULT_BALL_START_MAX_VELOCITY PIXELS_PER_MSEC_TO_VELOCITYVALUE(0.5)
/* Max velocity is here 0.1 => 0.1 pixel/millisec => 1 pixel/10 msec */
//#define DEFAULT_BALL_MIN_VELOCITY PIXELS_PER_MSEC_TO_VELOCITYVALUE(0.1)


#define DEFAULT_PLATES_START_MAX_VELOCITY PIXELS_PER_MSEC_TO_VELOCITYVALUE(0.05)
#define DEFAULT_PLATE_START_MIN_VELOCITY PIXELS_PER_MSEC_TO_VELOCITYVALUE(0.01)

/* Time which client blocks polling client's messages to server, before peeking socket for server messages (msec) */
#define POLLTIMEQUANTA 50


/* first 8 bits => Red, next 8 => green, then 8 => Blue, last bits unused */
#define BALL_COLOUR_DEFAULT 0x7f007f00
#define PADDLE_WIDTH_QUANTUM 4
#define PADDLE_WIDTH_MIN    12
#define PADDLE_WIDTH_MAX    80
#define PADDLE_START_WIDTH 32
#define PADDLE_START_HEIGHT 6
extern const unsigned paddle_start_color[4];
#define PLATE_FAST_COLOR htonl(0x007f0000)
#define PLATE_WIDER_COLOR htonl(0x00007f00)
#define PLATE_SHORTER_COLOR htonl(0x7f005800)
#define PLATE_STUCK_COLOR htonl(0x7f000000)

#define ball 1
#define lbat 2
#define rbat 3
#define D_PLAYER_LIVES 5
//define directions
#define D_DIRECTION_UP -1
#define D_DIRECTION_DOWN 1
#define D_DIRECTION_STANDBY 0
#define D_DIRECTION_LEFT -1
#define D_DIRECTION_RIGHT 1


//Define keys:
#define DPLAYER1_KEY_UP 'a'
#define DPLAYER2_KEY_UP '5'
#define DPLAYER1_KEY_DOWN 'z'
#define DPLAYER2_KEY_DOWN '2'
#define DKEY_QUIT1 'q'
#define DKEY_QUIT2 'Q'
#define D_MAX_PLAYER 4
//Define modes for static data functions
#define DPLAYER_KEYPRESS 0xBEEF
#define DPLAYER_PLATE_QUERY 0xBAD1
#define DPLAYER_SPECIALS_QUERY 0xDAD1
#define DPLAYER_DATA_QUERY 0xABBA
#define DINIT_PLAYER_DATA_STORAGE 0xFEE1
//#define D_PLAYER_INVALID DPLAYER_MAX
#define DUPDATE_PLATE 0xDEAD
#define DUPDATE_SPECIALS 0xD00D
#define DUPDATE_PLAYER 0xFAB5
#define PLAYER_NAME_MAX 20
#endif
