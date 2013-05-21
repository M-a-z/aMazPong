/*    -0.0.1  18.09.2008/Maz  First Draft.    */

#ifndef PoNG_NETW_CLIENT_IF_H
#define PoNG_NETW_CLIENT_IF_H

#include "general.h"
#include "pong_netw.h"
#include "network_messages.h"
#include "pong_client_queue.h"

typedef struct SClientStartArgs
{
    char serverip[16];
    int host_game;
    char game_name[16];
    int amnt_of_players;        
}SClientStartArgs;

int client_queue_init(SClientSendQueue *_this);

SOCKET connect_to_server(char *ip, int port);
EPongRet pong_wait_game_start(Snetw_game_start_msg *gameStartInfo);
EPongRet pong_join_inited_game(int game_id, char *player_name, char *passwd);
EPongRet pong_initiate_game(int game_id, int players, char passwd[15]);
EPongRet netw_client_hello_send( char *game_name, int *num_of_players, int *game_id, int initiator,int *ownId);
int send_clientaction(SClientSendQueue *sendqueue);
int peek_server_for_gameupdate(SClientSendQueue *rxqueue,SClientGameData *clidat);
void *ingameloop(void *arg);

#endif //PoNG_NETW_CLIENT_IF_H

