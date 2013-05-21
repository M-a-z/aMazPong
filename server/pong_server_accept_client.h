/* ************************************************************************** */
/*
 * The purpose of this file is to provide socket IF for pong clients to
 * initialize connections to server. First player connecting should send
 * game initialization data. (at least number of players).
 * 
 * Each connection handling is started in it's own thread. Basically
 * these connections will end up in infinite recv loop, listening keypress
 * messages, and ball/paddle position queries and responding to these.
 *
 * All position updates/queries will use same datacontainer, wrapped in a
 * singleton class (Yeah, I believe I'll use c++ for that part.)
 *
 *    Revision history:
 *
 *    -0.0.5  26.09.2008/Maz  Separated server startup && changed files name.
 *    -0.0.4  24.09.2008/Maz  Added parseAndReplyHelloMsg
 *    -0.0.3  19.06.2008/Maz  Moved generic network stuff to pong_netw.h
 *    -0.0.2  12.06.2008/Maz  Lot's of pondering just to make this compile with 
 *                            winsock API.                                     
 *    -0.0.1  04.06.2008/Maz
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */

#ifndef SERVER_ACCEPT_CLIENT_H
#define SERVER_ACCEPT_CLIENT_H
#include <pthread.h>
#include "serverStartup.h"
#include "general.h"
#include "pong_netw.h"
#include "network_messages.h"
#include "pong_dataStorage.h"

#define BACKLOG 4
#define CONN_MAX_EXEED "CONN_MAX_EXEED"
#define CONN_MAX_EXCEED_MSG_SIZE 15
struct SConnectionInfo;

typedef EPongRet (*FHelloParse)(void *opaque);
typedef EPongRet (*FInitParse)(void *opaque);
typedef EPongRet (*FJoinreqParse)(void *opaque);
typedef void (*FInitInfo)(struct SConnectionInfo *info);
typedef void (*FUninitInfo)(struct SConnectionInfo *info);
typedef EPongRet (*FwaitAllJoinAndSend) (void *opaque);

typedef struct SConnectionInfo
{
    char *game_name;
//  unsigned int game_id;
    unsigned int own_player_id;
    unsigned int own_playerdata_index;
    int expected_msg_id;
    struct sockaddr_in clientaddr;
    void *msg;
    SGameData *gameData;
    SOCKET sock;
	int errors;
    FHelloParse parsehelloF;
    FInitParse  parseinitF;
    FJoinreqParse parsejoinreqF;
    FwaitAllJoinAndSend waitAllJoinAndSendF;
    FInitInfo infoinitF;
    FUninitInfo infouninitF;
}SConnectionInfo;


typedef struct SserverInternalClientData
{
    char ip[16];
    EWallPosition pos;
    short id;
    short num_of_players;
    short other_player_ids[3];
    short game_id;
}SserverInternalClientData;

//moved tp ServerStartup.h
//EPongRet start_server();
//int parseAndReplyHelloMsg( Snetw_hello_from_client * hello, SOCKET sock);
void * datalistener(void *arg);
void new_sock_conn(SOCKET sock, struct sockaddr_in their_addr);
void initBasicGame(SConnectionInfo *_this);
void pong_Basic_infouninitF(SConnectionInfo *_this);
void connInfoInit(SConnectionInfo *_this,SStartParams startparams);
void * datalistener(void *arg);

#endif   //SERVER_ACCEPT_CLIENT
