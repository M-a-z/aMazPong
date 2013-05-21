/* ************************************************************************** */
/*                                                                            *
 *      The purpose of this file is to provide network (socket) interface     *
 *      for client to initiate game (or to join an already initiated) game    *
 *      and to request data from server, or to send keypress information      *
 *      to server.                                                            *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Revision history:                                                     *
 *                                                                            *
 *      -0.0.2  18.09.2008/Maz  Ironed out some compile errors.    
 *      -0.0.l  19.06.2008/Maz  First draft                                   *
 *                                                                            *
 *      PasteLeft 2008 Maz                                                    *
 *                                                                            */
/******************************************************************************/

//#include "common/general.h"
#include "pong_netw.h" 
#include "network_messages.h"
#include "pong_client_netwIF.h"
#include <string.h>
#include <time.h>
#ifdef __LINUX__
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include "pong_client_queue.h"

#endif

 
static int own_id;
static int serverId;
static SOCKET sock;
void handle_gamedata_update(void *data,size_t msgsize,SClientGameData *clidat)
{
    printf("I should now update gamedata\n");
}

int handle_ingame_servermsg(void *data,size_t msgsize,SClientGameData *clidat)
{
    int rval=0;
    printf("%s(): msg handler called\n",__FUNCTION__);
    if(!data)
        return 0;
    switch(msg_getId(data))
    {
        case NETW_GAMEDATA_UPDATE_MSG:
            printf("Msg was NETW_GAMEDATA_UPDATE_MSG\n");
            if(msgsize<sizeof(SClientGameData))
            {
                printf("Received ILL sized NETW_GAMEDATA_UPDATE_MSG => ignoring (expected %u, recv %u bytes)!\n",sizeof(SClientGameData),msgsize);
            }
            else
                handle_gamedata_update(data,msgsize,clidat);
            break;
        case NETW_GAME_KILL_MSG:
            printf
            (
                "Game KILL request from server: %u:%s\n",
                ((Snetw_game_kill_msg*)data)->reason,
                ((Snetw_game_kill_msg*)data)->message
            );
            rval=-1;
            break;
        default:
            printf("Server received unknown msg (0x%x)!\n",msg_getId(data));
            break;
    }
    return rval;
}
void *ingameloop(void *arg)
{
    void **arr = (void **)arg;
//    SOCKET sock=*(SOCKET *)arr[0];
    SClientSendQueue *queue=arr[1];
    void *replymsg;

    printf("Falling in ingameloop listening incoming server updates (socket %d)\n",sock);

    while(1)
    {
        replymsg=msg_receive( sock, INGAME_SERVERMSG_TMO, NULL );
        if(!replymsg)
        {
            printf("Looks like we've lost the server!\n");
            exit(-1);
        }
        printf("message from server received, adding data to queue and signaling\n");
        rx_client_queue_add_data(queue,replymsg,msg_getSize(replymsg));
        signal_sender(queue);
    }
    return NULL;
}
int peek_server_for_gameupdate(SClientSendQueue *rxqueue,SClientGameData *clidat)
{
    int rval=0;
    size_t msgsize;
    void *data;
    if(rx_poll_clientaction_startsend(rxqueue))
    {
        printf("Failed to start sending!\n");
        return -1;
    }
    while(NULL!=(data=rx_poll_clientaction_getdata(rxqueue,&msgsize)))
    {
        printf("Obtained data from server and handling msg...\n");
        if((rval=handle_ingame_servermsg(data,msgsize,clidat)))
        {
            break;
        }
        msg_free(&data);
    }
    printf("All servermessages handled.\n");
    rx_poll_clientaction_endsend(rxqueue);
    return rval;
}
int send_clientaction(SClientSendQueue *sendqueue)
{
    int rval=0;
    size_t msgsize;
    void *data;
    if(poll_clientaction_startsend(sendqueue))
    {
        printf("Failed to start sending!\n");
        return -1;
    }
    while(NULL!=(data=poll_clientaction_getdata(sendqueue,&msgsize)))
        if(EPongRet_SUCCESS!=msg_send(sock,data,msgsize))
        {
            printf("Msg send FAILED!\n");
            rval=-1;
            break;
        }
    poll_clientaction_endsend(sendqueue);
    printf("%s() returning %d\n",__FUNCTION__,rval);
    return rval;
}
SOCKET connect_to_server(char *ip, int port)
{
    struct sockaddr_in address;
    int conn_retval=0; 
#ifdef DEBUGPRINTS 
	printf("ip=%s",ip);
#endif
	memset(&address,0,sizeof(struct sockaddr_in));
    if( (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ) < 0 )
    {
		printf("Socket creation failed! retval=%d\n",sock);
        return sock;
    }
	printf("TCP/IP Socket %d created",sock);
    memset(&address, 0, sizeof(address));  
    address.sin_family      = AF_INET;           
    address.sin_addr.s_addr = inet_addr(ip); 
    address.sin_port        = htons((short)port);

    if( (conn_retval=connect(sock, (struct sockaddr *)&address, sizeof(address))) != 0 )
    {
		printf("Socket connection failed! retval %d\n",conn_retval);
		pong_sock_close(sock);
        return 0;
    }
    printf("Connected to socket %d\n\n",sock);
    return sock;
}

EPongRet pong_wait_game_start(Snetw_game_start_msg *gameStartInfo)
{
    time_t starttime=time(NULL);
    Snetw_game_start_msg *startupmsg;
    int msgId;
/*
 *
//Msgs inited by server and client's replies.
#define NETW_GAME_START_MSG 0xF00BAF00
typedef struct Snetw_game_start_msg
{
//SGameData?? Or just some collection? Think.

}Snetw_game_start_msg;

//Ping:
#define NETW_PING_REQ 0xFEEDBEEF
typedef struct Snetw_ping_req
{
    size_t msgSize;
    int serverIP;
    unsigned int trid;
    unsigned long long server_gametime;
}Snetw_ping_req;
*/


    void *replymsg;
    for(;;)
    {
        if(time(NULL)-starttime > GAME_READY_TMO)
        {
            printf("No GAME_READY received from server in %u seconds!\n",(unsigned int)time(NULL)-(unsigned int)starttime);
            return EPongRet_ERROR;
        }
        replymsg=msg_receive( sock, GAME_READY_TMO,NULL );
        if(replymsg==NULL)
        {
            printf("No GAME_READY received from server!\n");
            return EPongRet_ERROR;
        }
        if((msgId=msg_getId(replymsg))==NETW_GAME_START_MSG )
        {
            break;
        }
        else
            printf("Server alive ping received\n");
    }
    startupmsg=(Snetw_game_start_msg *)replymsg;
    memcpy(gameStartInfo,replymsg,sizeof(Snetw_game_start_msg));
    printf("Received game client data for game '%s'\n",startupmsg->client_game_data.game_name);
    
    return EPongRet_SUCCESS;
}



EPongRet pong_join_inited_game(int game_id, char *player_name, char *passwd )
{
    Snetw_game_join_req *msg;
    msg=msg_create(game_id,serverId,own_id, NETW_GAME_JOIN_REQ, sizeof(Snetw_game_join_req));
    pong_assert(NULL==msg,"msgCreate (JOIN msg) returned NULL!\n");
//    msg->game_id=game_id;
    if(!player_name || PLAYER_NAME_MAX <= strlen(player_name))
    {
        printf("pong_join_inited_game: Invalid player name '%s'!\n",player_name);
        return EPongRet_ERROR;
    }
//    strcpy(msg->player_name,"player_");
    strcpy(msg->player_name,player_name);
    //msg->player_name[7]=('0'+(char)player_id);
    memset(msg->passwd,0,PASSWD_MAX);
    strncpy(msg->passwd,passwd,PASSWD_MAX-1);
//    msg->passwdSize=strlen(msg->passwd);   
    printf("sending NETW_GAME_JOIN_REQ with playername '%s', passwd '%s'\n",msg->player_name,msg->passwd);
    return msg_send(sock,msg,sizeof(Snetw_game_join_req));
}
         
EPongRet pong_initiate_game(int game_id, int players, char passwd[15])
{
    Snetw_game_init_req *msg;
    /*#define NETW_GAME_INIT_REQ 0xB00BBABE
typedef struct Snetw_game_init_req
{
//    Snetw_internal_msg_header header;
    int number_of_players;
    size_t passwdSize;
    int game_id;
    char passwd[16];
}Snetw_game_init_req;
*/
    msg=msg_create(game_id,serverId,own_id,NETW_GAME_INIT_REQ,sizeof(Snetw_game_init_req));
    pong_assert(NULL==msg,"omg! msgCreate returned NULL!\n");
//    msg->game_id=game_id;
    msg->number_of_players=players;
    memset(msg->passwd,0,PASSWD_MAX);
    strncpy(msg->passwd,passwd,PASSWD_MAX-1);
//    msg->passwdSize=strlen(msg->passwd);   
    printf("sending NETW_GAME_INIT_REQ with gameId %d playerno %d, passwd '%s'\n",game_id,msg->number_of_players,msg->passwd);
    return msg_send(sock,msg,sizeof(Snetw_game_init_req));
}
         
//EPongRet_ERROR
//num_of players is set to 0, if game init msg needs to be sent by this client.
EPongRet netw_client_hello_send( char *game_name, int *num_of_players, int *game_id, int initiator, int *ownId)
{
    void *replymsg;
    Snetw_hello_full_resp *respStruct;
    Snetw_hello_short_resp *shortresp;
    Snetw_hello_from_client *hello_msg;
    int msgId;
    hello_msg=(Snetw_hello_from_client *)msg_create(0, 0, 0, NETW_GAME_HELLO_CLI, sizeof(Snetw_hello_from_client));
    
    strncpy( hello_msg->game_name,game_name,15);
    hello_msg->game_name[15]='\0';
    printf("Sending NETW_GAME_HELLO_CLI to server with game_name '%s'\n",hello_msg->game_name);
    pong_assert(
        EPongRet_SUCCESS!= msg_send(
            sock, 
            (void *)hello_msg, 
            sizeof(Snetw_hello_from_client)
        ),
        "Sending Hello to server failed!\n"
    );
    replymsg=msg_receive( sock, HELLO_MSG_TMO, NULL );
    if(replymsg==NULL)
    {
        printf("No reply from server received!\n");
        return EPongRet_ERROR;
    }
    else
    {
        *ownId=(own_id=msg_getReceiver(replymsg));
        serverId=msg_getSender(replymsg);
        *game_id=msg_getGameId(replymsg);
        printf("Received hello reply, own ID 0x%x, server 0x%x\n",own_id,serverId);
        if(INVALID_PLAYER==own_id)
        {
            printf("Server failed and returned invalid playerId... Aborting\n");
            return EPongRet_ERROR;
        }
        msgId=msg_getId(replymsg);
        printf("Received msgId was 0x%x, NETW_GAME_HELLO_SRV_FULL== 0x%x,NETW_GAME_HELLO_SRV_SHORT==0x%x\n",msgId,NETW_GAME_HELLO_SRV_FULL,NETW_GAME_HELLO_SRV_SHORT);
        switch(msgId)
        {
            case NETW_GAME_HELLO_SRV_FULL:
                if(initiator)
                {
                    printf("Server requested init data even though I'm not initiator\n");
                    return EPongRet_ERROR;
                }
                printf("NETW_GAME_HELLO_SRV_FULL received\n");
                respStruct=(Snetw_hello_full_resp*)replymsg;
                if(respStruct->req_init_data==0)
                {
                    *num_of_players=respStruct->num_of_players;
                }
                else
                {
                    *num_of_players=0;
                }
            break;
            case NETW_GAME_HELLO_SRV_SHORT:
                shortresp=(Snetw_hello_short_resp *)replymsg;
                printf("NETW_GAME_HELLO_SRV_SHORT received, init_req = %d\n",shortresp->req_init_data);
                if(!initiator)
                {
                    printf("Server did request init data even though I'm not initiator\n");
                    return EPongRet_ERROR;
                }
                
            break;
        }
    }
        return EPongRet_SUCCESS;
}
