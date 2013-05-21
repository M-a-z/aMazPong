/* ************************************************************************** */
/*
 * The purpose of this file is to start the server and provide listening socket
 * for pong clients to connect to server. First player connecting should send
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
 *    -0.0.7  06.01.2009/Maz  Restructuring started... Decided to do msg parsing in CPP like C :)
 *                            This may even be faster way. Nowadays CPUs have decent size FAST cache memories,
 *                            and when one is lucky, caching _this struct will really speed up things...
 *                            Written bunch of new code... now the handshake msging is again at same level as 
 *                            it was before restructuring, but at this time, some real data structs are updated
 *                            instead of dummy calls. Handling game_init_req and generating game_start_msg are still lacking
 *                            so is good cleanup procedure for all game datas, and interrupt routine for a game (all clients)
 *                            if something fails with one.
 *                            Next step is to be able to compile this again :)
 *    -0.0.6  26.09.2008/Maz  Separated server startup && changed files name.
 *    -0.0.5  24.09.2008/Maz  Wrote server / client handshake a lil further.
 *    -0.0.4  23.09.2008/Maz  Started adding server side of handshake & game data collection.
 *    -0.0.3  18.09.2008/Maz  Ironed out some compile errors.    
 *    -0.0.2  12.06.2008/Maz  Lot's of pondering just to make this compile with winsock API.
 *                            Also added temp main func for compilation/linking the server.
 *    -0.0.1  04.06.2008/Maz
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */


#include "pong_misc_func.h"
#include "pong_server_accept_client.h"
#include "place_counter_thread.h"
//#include "common/general.h"
#include "network_messages.h"
//#include <stdio.h>
#include <time.h>
#include "pong_server_queue.h"
//#include "pong_commP_CppToC_wrapper.h"

//how long do we wait for game init msg to arrive via socket? (millisec)
#define GAME_INIT_TMO 100000
#define INIT_REQ_TIM 100000


static int G_connections_alive=0;
//static int G_game_init_data_received=0;
int wait_game_init_data(SOCKET sock);

EPongRet pong_Basic_parsehelloF(void *opaque);
EPongRet pong_Basic_parseinitF(void *opaque);
EPongRet pong_Basic_parsejoinreqF(void *opaque);
EPongRet pong_Basic_waitAllJoinAndSendF(void *opaque);
EPongRet parseAndReplyHelloMsg( SConnectionInfo *_this);

//static SHandshakeStatus G_HandshakeStatus[MAX_GAMES_ON_SERVER];


void new_sock_conn(SOCKET sock,struct sockaddr_in their_addr)
{
//    char *dataptr;
    void *dataptr;
    pthread_attr_t attr;
    pthread_t datalistenerthread;
    dataptr=malloc(sizeof(SOCKET)+sizeof(their_addr));
/*
    *(SOCKET *)dataptr=sock;
    *(int *)(dataptr+sizeof(SOCKET))=playerno;
*/
    
    memcpy(dataptr,&sock,sizeof(SOCKET));
    memcpy( ((SOCKET *)dataptr)+1,&(their_addr),sizeof(their_addr));
    //pthread_t schedthread;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);
    pthread_attr_setschedpolicy(&attr, SCHED_RR );
    pthread_create(&datalistenerthread,&attr,&datalistener,(void *) dataptr);
//    free(dataptr);
}
void initBasicGame(SConnectionInfo *_this)
{
	_this->expected_msg_id=NETW_GAME_HELLO_CLI;
    _this->parsehelloF=&pong_Basic_parsehelloF;
    _this->waitAllJoinAndSendF=&pong_Basic_waitAllJoinAndSendF;
    _this->parseinitF=&pong_Basic_parseinitF;
    _this->parsejoinreqF=&pong_Basic_parsejoinreqF;
    _this->infouninitF=&pong_Basic_infouninitF;
}
void pong_Basic_infouninitF(SConnectionInfo *_this)
{
    //release playerId
    //if last player=>release gameId && clear game data
	if(NULL==_this || NULL==_this->gameData)
		return ;
	if(_this->gameData->clients_alive<=1)
	{
		//TODO: Last client left, clean gameData etc...
		memset(_this->gameData,0,sizeof(SGameData));
	}
    free(_this->game_name);
}
void connInfoInit(SConnectionInfo *_this,SStartParams startparams)
{
    //TODO: Add startup params check, and create appropriate init skenarios for other game types.
    memset(_this,0,sizeof(SConnectionInfo));
    _this->infoinitF=&initBasicGame;
}

void create_updatemsg_from_gamedata(Snetw_gamedata_update_msg *msg,SGameData *gdata)
{
    memcpy(&(msg->client_game_data),&(gdata->game_name),sizeof(SClientGameData));
}
void * ticker_thread(void *arg)
{
//    SClientGameData currentGamedata;
    SConnectionInfo *_this=(SConnectionInfo *)arg;
    int ok=1;
    while(ok)
    {
        Snetw_gamedata_update_msg *msg;
        //SClientGameData *clidata;
        SGameData *gdata;
        size_t datalen;
        int imlast;
        int rval;
        /*
        NETW_GAMEDATA_UPDATE_MSG 0xFA700A55
            typedef struct Snetw_gamedata_update_msg
            {
                    unsigned long long time_from_gamestart; - currently unused, may be needed in future 
                        SClientGameData client_game_data;
            }Snetw_gamedata_update_msg;
            */

        msg=msg_create(_this->gameData->gameId, _this->own_player_id, SERVER_ID, NETW_GAMEDATA_UPDATE_MSG, sizeof(Snetw_gamedata_update_msg));
        if(!msg)
        {
            printf("msg_create() FAILED!\n");
            return NULL;
        }
        while(!(rval=poll_serverdata_to_send(&(_this->gameData->serverqueue), 10000)));

        if(1!=rval)
        {
          printf("Something odd happened :/ %d\n",rval);
          exit(1);
        }
        if(server_lock_tx(&(_this->gameData->serverqueue)))
        {
            printf("%s:%d\n",__FILE__,__LINE__);
            exit(-1);
        }
        gdata=server_queue_get_tx_data(&(_this->gameData->serverqueue),&datalen,&imlast);
        if(!gdata || !datalen)
        {
            server_unlock_tx(&(_this->gameData->serverqueue));
            printf("%s:%d\n",__FILE__,__LINE__);
            return NULL;
        }
        create_updatemsg_from_gamedata(msg,gdata);
        msg->client_game_data.myplayer_index=_this->own_player_id;
        if(imlast)
            server_data_free(gdata);

        if(server_unlock_tx(&(_this->gameData->serverqueue) ))
        {
            printf("%s:%d\n",__FILE__,__LINE__);
            exit(-1);
        }
        msg_send(_this->sock,msg,sizeof(Snetw_gamedata_update_msg));
        //ok=SERVER_copyGamedataForClientById(&currentGamedata,_this->gameData->gameId);
        /* 10 msec sleep */
        //usleep(10000);
    }
    return NULL;
}
EPongRet startup_ticker_thread(SConnectionInfo *_this)
{
    pthread_t tid;
    /* Send position updates to client */
    if(pthread_create(&tid,NULL,ticker_thread,_this))
        return EPongRet_ERROR;
    return EPongRet_SUCCESS;
}


//TODO:
//Add *arg to contain possible startup configs given from commandline.
//Those may affect on the functions the SConnectionInfo is initialized with

void stop_game(SConnectionInfo *_this,int reason,char *reasonmsg)
{
    
    /* TODO: Allow user to drop off without killing the game - One can just draw a wall and let others to continue if user is dropped out */
    /* Notify other client handling threads */
    printf("Game stop requested...\n");
    if(!_this || !_this->gameData)
        return;
    
    _this->gameData->abort_game=reason;
    strncpy(_this->gameData->game_passwd,reasonmsg,PASSWD_MAX);
    _this->gameData->game_passwd[PASSWD_MAX-1]='\0';
    printf("Game state changed to stopped...\n");

}
void send_stop_to_client(SConnectionInfo *_this)
{
    void *msg;
    /* Notify own client */
    if(!_this)
        return;
    if(_this->sock>0)
    {
        msg = msg_create
        (
            (_this->gameData)?_this->gameData->gameId:-1,
            _this->own_player_id, 
            SERVER_ID, 
            NETW_GAME_KILL_MSG,
            sizeof(Snetw_game_kill_msg)
        );

        if(!msg)
        {
            printf("Msg alloc FAILED!\n");
            exit(1);
        }
        if(_this->gameData && _this->gameData->game_passwd)
            strncpy(((Snetw_game_kill_msg*)msg)->message,_this->gameData->game_passwd,49);
        else
            strcpy(((Snetw_game_kill_msg*)msg)->message,"Unknown");
            ((Snetw_game_kill_msg*)msg)->reason=(_this->gameData)?_this->gameData->abort_game:-1;
        msg_send(_this->sock,msg,sizeof(Snetw_game_kill_msg));
    }
}
/* Signal queue */
void handle_client_direction_change(SConnectionInfo *_this)
{
    Snetw_client_dir_change_pressed_msg *updatemsg;
    SGameData *gd;

    if(!_this || !_this->gameData || _this->msg )
        return;
    updatemsg=(Snetw_client_dir_change_pressed_msg *)_this->msg;
    _this->gameData->players[_this->own_playerdata_index].plate.direction=updatemsg->newdirection;
    /* LOCK */
    gd=malloc(sizeof(SGameData));
    if(!gd)
    {
        printf("Malloc F A I L E D!\n");
        return;
    }
    memcpy(gd,_this->gameData,sizeof(SGameData));
//    toclients=msg_create(_this->gameData->gameId, 0, unsigned int own_id, SERVER_ID, size_t msg_size)
    server_queue_add_tx_data(&(_this->gameData->serverqueue),gd,sizeof(SGameData),(unsigned char)gd->clients_alive);
    queue_signal(&(_this->gameData->serverqueue.genqueue));
/*    server_queue_add_rx is not needed because the _this->gameData which was updated is shared by everyone => no need to signal, change is instant */
  //  _this->gameData->
}


void * datalistener(void *arg)
{
    SOCKET sock;
    struct sockaddr_in their_addr;
    sock=*(SOCKET *)arg;
    SConnectionInfo *_this;
    SStartParams dummy={0};
    their_addr=*(struct sockaddr_in*) ( (SOCKET *)arg+1);
	_this=malloc(sizeof(SConnectionInfo));
	if(_this==NULL)
	{
		printf("YaY! malloc failed at %s:%d",__FILE__,__LINE__);
		exit(-1);
	}
    if(G_connections_alive>=CLIENTS_MAX)
    {           
        perror("Connection attempted but CONN_MAX already achieved\n");
        if(
            send(sock,
                CONN_MAX_EXEED,
                CONN_MAX_EXCEED_MSG_SIZE, 
                0   
            ) == -1
        )   
        {
            perror("send max conn\n");
        }
        pong_sock_close(sock);
        return NULL;
    }
    connInfoInit(_this,dummy);
    _this->clientaddr=their_addr;
    _this->sock=sock;
    G_connections_alive++;

	_this->expected_msg_id=NETW_GAME_HELLO_CLI;

    //So let's start handling messages.. loop loop forever loop :)
    while((_this->msg=msg_receive(_this->sock,SERVER_RECV_TMO,NULL)))
    {
        int breakout=0;
        printf("msg_receive() returned %p\n",_this->msg);
        if(_this->gameData)
            if(_this->gameData->abort_game)
            {
                printf("Game abort requested!\n");
                breakout=1;
                send_stop_to_client(_this);
                close(_this->sock);
                _this->sock=-1;
            }
		switch(msg_getId(_this->msg))
        {
/* 
 * Startup sequence msgs
 * TODO: Implement message informing all clients joining to a game, if one client fails
 * 		 Perform cleanup sequence in fail cases!
 */
            case NETW_GAME_HELLO_CLI:
                printf("NETW_GAME_HELLO_CLI recv'd by server\n");
				if(_this->expected_msg_id!=NETW_GAME_HELLO_CLI)
				{
					printf("Unexpected msg from %s (0x%x), expected 0x%x",inet_ntoa(_this->clientaddr.sin_addr),NETW_GAME_HELLO_CLI,_this->expected_msg_id);
                    _this->errors++;
					break;
				}
				_this->infoinitF(_this);
                if( EPongRet_SUCCESS != _this->parsehelloF((void *)_this))
                {
                    printf("Parsing hello msg FAILED! (sender %s)\n",inet_ntoa(_this->clientaddr.sin_addr));
                    //since info struct may be uninited, we need to just return instead of breaking!
                    return NULL;
                }
            break;
            case NETW_GAME_JOIN_REQ:
                printf("NETW_GAME_JOIN_REQ recv'd by server\n");
                if(_this->expected_msg_id!=NETW_GAME_JOIN_REQ)
                {
                    printf("Unexpected msg from %s (0x%x), expected 0x%x",inet_ntoa(_this->clientaddr.sin_addr),NETW_GAME_JOIN_REQ,_this->expected_msg_id);
                    _this->errors++;
                    break;
                }

			    if(EPongRet_SUCCESS!=_this->parsejoinreqF(_this))
                {
                    printf("Parsing gameJoinReq msg FAILED! (sender %s)\n",inet_ntoa(_this->clientaddr.sin_addr));
                    breakout=1;
                }
                if(EPongRet_SUCCESS!=_this->waitAllJoinAndSendF(_this))
                {
                    printf("Failed to create and send NETW_GAME_START_REQ\n");
                    breakout=1;
                    stop_game(_this,-3,"Failed in waitAllJoinAndSendF()");
                }
                else
                {
                    /* Think if it would be better to collect all filled replies in timer thread, and do sending from that thread after all messages to clients are filled - to avoid different starting times! */
                        /* Let the first client to spawn timer process at server */
                    if(!_this->own_playerdata_index)
                    {
                        if(EPongRet_SUCCESS!=launch_timerthread(_this->gameData))
                        {
                            printf("Failed to launch position updater thread\n");
                            stop_game(_this,-1,"Failed to launch position updater");
                            breakout=1;
                        }
                        else
                            printf("launched position updater thread\n");
                    /* start ticker here */
                        if(EPongRet_SUCCESS!=startup_ticker_thread(_this))
                        {
                            printf("Failed to launch ticker_thread\n");
                            stop_game(_this,-1,"Failed to launch ticker");
                            breakout=1;
                        }
                    }
                }
                break;
            break;
            case NETW_GAME_CLIENT_KEY_PRESSED_MSG:
                if(!_this->gameData)
                {
                    printf("Recved NETW_GAME_CLIENT_KEY_PRESSED_MSG: but gameData not yet inited!\n");
                    _this->errors++;
                    break;
                }
                if(_this->gameData->players[_this->own_playerdata_index].player_alive)
                    handle_client_direction_change(_this);
            break;
            case NETW_GAME_INIT_REQ:
                printf("NETW_GAME_INIT_REQ recv'd by server\n");
                if(_this->expected_msg_id!=NETW_GAME_INIT_REQ)
                {
                    printf("Unexpected msg (GAME_INIT_REQ) from %s (0x%x), expected 0x%x",inet_ntoa(_this->clientaddr.sin_addr),NETW_GAME_INIT_REQ,_this->expected_msg_id);
                    _this->errors++;
                    break;
                }
                if(EPongRet_SUCCESS!=_this->parseinitF(_this))
                {
                    printf("Parsing gameInitReq msg FAILED! (sender %s)\n",inet_ntoa(_this->clientaddr.sin_addr));
                    breakout=1;;
                }
/*TODO:  Data query msgs */
/*
 			break;
            case 
            break;
            case 
            break;
            case 
            break;
            case 
            break;
*/
                printf("NETW_GAME_INIT_REQ parsed!\n");
                break;
            default:
                _this->errors++;
                if(_this->errors>=ERROR_TOLERANCE)
                {
                    printf("unknown msg received, errorlimit exceed... GoodBye %s\n",inet_ntoa(_this->clientaddr.sin_addr));
                    breakout=1;
                }
                printf("Received unknown msg (0x%x), ignoring (sender %s)\n",msg_getId(_this->msg),inet_ntoa(_this->clientaddr.sin_addr));
            break;
        }
        msg_free(&(_this->msg));
        if(breakout || _this->errors>=ERROR_TOLERANCE)
        {
            break;
        }
    }
    
    G_connections_alive--;
    stop_game( _this,-2,"Unknown reason");
    printf("GoodBye %s\n",inet_ntoa(_this->clientaddr.sin_addr));
    if(_this->infouninitF)
        _this->infouninitF(_this);
    free(_this);
	return NULL;
}

void send_to_clients(SGameData *gameData)
{
    SGameData *gdcopy=malloc(sizeof(SGameData));
    /* LOCK */
    memcpy(gdcopy,gameData,sizeof(SGameData));
    if(server_queue_add_tx_data(&(gameData->serverqueue),gdcopy,sizeof(SGameData),gameData->clients_alive))
    {
        printf("Failed?!? %s:%d\n",__FILE__,__LINE__);
        return;
    }
    queue_signal(&(gameData->serverqueue.genqueue)); 
}


int fill_initial_gamedata(Snetw_game_start_msg *msg,SGameData *gameData)
{
    SClientGameData *clidata=&(msg->client_game_data);
    /*
     *
     * typedef struct Snetw_game_start_msg
     * {
          SClientGameData client_game_data;
          }Snetw_game_start_msg;
      */
    memcpy(clidata, (((char *)gameData)+offsetof(SGameData,game_name)), offsetof(SClientGameData,myplayer_index));
    printf("Going to send following:\n");
    printf("game_name '%s'\n",clidata->game_name);
    printf("player_amnt %u\n",clidata->player_amnt);
    printf("gameId 0x%x\n",clidata->gameId);
    printf("ball_amnt %u\n",clidata->ball_amnt);
    printf("clients_alive %u\n",clidata->clients_alive);

    printf("Wall 1 data: type %u, wpos %u\n",clidata->walls[0].type,clidata->walls[0].wpos);
    printf("Wall 2 data: type %u, wpos %u\n",clidata->walls[1].type,clidata->walls[1].wpos);
    printf("Wall 3 data: type %u, wpos %u\n",clidata->walls[2].type,clidata->walls[2].wpos);
    printf("Wall 4 data: type %u, wpos %u\n",clidata->walls[3].type,clidata->walls[3].wpos);

    printf
    (
        "Ball 1 data:\n ball_active %u\n direction_x %u\n direction_y %u\n speed_x %f\n speed_y %f\n xpos %f\n ypos %f\n special %u\n",
        clidata->BalldataArray[0].ball_active,
        clidata->BalldataArray[0].direction_x,
        clidata->BalldataArray[0].direction_y,
        clidata->BalldataArray[0].speed_x,
        clidata->BalldataArray[0].speed_y,
        clidata->BalldataArray[0].xpos,
        clidata->BalldataArray[0].ypos,
        clidata->BalldataArray[0].special
    );
    printf
    (
        "Ball 2 data:\n ball_active %u\n direction_x %u\n direction_y %u\n speed_x %f\n speed_y %f\n xpos %f\n ypos %f\n special %u\n",
        clidata->BalldataArray[1].ball_active,
        clidata->BalldataArray[1].direction_x,
        clidata->BalldataArray[1].direction_y,
        clidata->BalldataArray[1].speed_x,
        clidata->BalldataArray[1].speed_y,
        clidata->BalldataArray[1].xpos,
        clidata->BalldataArray[1].ypos,
        clidata->BalldataArray[1].special
    );
    printf
    (
        "Ball 3 data:\n ball_active %u\n direction_x %u\n direction_y %u\n speed_x %f\n speed_y %f\n xpos %f\n ypos %f\n special %u\n",
        clidata->BalldataArray[2].ball_active,
        clidata->BalldataArray[2].direction_x,
        clidata->BalldataArray[2].direction_y,
        clidata->BalldataArray[2].speed_x,
        clidata->BalldataArray[2].speed_y,
        clidata->BalldataArray[2].xpos,
        clidata->BalldataArray[2].ypos,
        clidata->BalldataArray[2].special
    );
    printf
    (
        "Ball 4 data:\n ball_active %u\n direction_x %u\n direction_y %u\n speed_x %f\n speed_y %f\n xpos %f\n ypos %f\n special %u\n",
        clidata->BalldataArray[3].ball_active,
        clidata->BalldataArray[3].direction_x,
        clidata->BalldataArray[3].direction_y,
        clidata->BalldataArray[3].speed_x,
        clidata->BalldataArray[3].speed_y,
        clidata->BalldataArray[3].xpos,
        clidata->BalldataArray[3].ypos,
        clidata->BalldataArray[3].special
    );

    printf
    (
        "Ball 5 data:\n ball_active %u\n direction_x %u\n direction_y %u\n speed_x %f\n speed_y %f\n xpos %f\n ypos %f\n special %u\n",
        clidata->BalldataArray[4].ball_active,
        clidata->BalldataArray[4].direction_x,
        clidata->BalldataArray[4].direction_y,
        clidata->BalldataArray[4].speed_x,
        clidata->BalldataArray[4].speed_y,
        clidata->BalldataArray[4].xpos,
        clidata->BalldataArray[4].ypos,
        clidata->BalldataArray[4].special
    );

    printf
    (
        "Player 1 Data:\n playerName '%s'\n player_alive %u\n lives %u, userId 0x%x, player_has_joined %u\n",
        clidata->players[0].playerName,
        clidata->players[0].player_alive,
        clidata->players[0].lives,
        clidata->players[0].userId,
        clidata->players[0].player_has_joined
    );
    printf
    (
        " Player 1 Plate Info:\n pos (x=%f,y=%f), direction %d, speed %f, wall %d specials (w=%d,s=%d,f=%d)\n",
        clidata->players[0].plate.x,
        clidata->players[0].plate.y,
        clidata->players[0].plate.direction,
        clidata->players[0].plate.speed,
        clidata->players[0].plate.wall,
        clidata->players[0].plate.specials.wide,
        clidata->players[0].plate.specials.stuck,
        clidata->players[0].plate.specials.fast
    );
    printf
    (
        "Player 2 Data:\n playerName '%s'\n player_alive %u\n lives %u, userId 0x%x, player_has_joined %u\n",
        clidata->players[1].playerName,
        clidata->players[1].player_alive,
        clidata->players[1].lives,
        clidata->players[1].userId,
        clidata->players[1].player_has_joined
    );
    printf
    (
        " Player 2 Plate Info:\n pos (x=%f,y=%f), direction %d, speed %f, wall %d specials (w=%d,s=%d,f=%d)\n",
        clidata->players[1].plate.x,
        clidata->players[1].plate.y,
        clidata->players[1].plate.direction,
        clidata->players[1].plate.speed,
        clidata->players[1].plate.wall,
        clidata->players[1].plate.specials.wide,
        clidata->players[1].plate.specials.stuck,
        clidata->players[1].plate.specials.fast
    );

    printf
    (
        "Player 3 Data:\n playerName '%s'\n player_alive %u\n lives %u, userId 0x%x, player_has_joined %u\n",
        clidata->players[2].playerName,
        clidata->players[2].player_alive,
        clidata->players[2].lives,
        clidata->players[2].userId,
        clidata->players[2].player_has_joined
    );

    printf
    (
        " Player 3 Plate Info:\n pos (x=%f,y=%f), direction %d, speed %f, wall %d specials (w=%d,s=%d,f=%d)\n",
        clidata->players[2].plate.x,
        clidata->players[2].plate.y,
        clidata->players[2].plate.direction,
        clidata->players[2].plate.speed,
        clidata->players[2].plate.wall,
        clidata->players[2].plate.specials.wide,
        clidata->players[2].plate.specials.stuck,
        clidata->players[2].plate.specials.fast
    );
    printf
    (
        "Player 4 Data:\n playerName '%s'\n player_alive %u\n lives %u, userId 0x%x, player_has_joined %u\n",
        clidata->players[3].playerName,
        clidata->players[3].player_alive,
        clidata->players[3].lives,
        clidata->players[3].userId,
        clidata->players[3].player_has_joined
    );
    printf
    (
        " Player 4 Plate Info:\n pos (x=%f,y=%f), direction %d, speed %f, wall %d specials (w=%d,s=%d,f=%d)\n",
        clidata->players[3].plate.x,
        clidata->players[3].plate.y,
        clidata->players[3].plate.direction,
        clidata->players[3].plate.speed,
        clidata->players[3].plate.wall,
        clidata->players[3].plate.specials.wide,
        clidata->players[3].plate.specials.stuck,
        clidata->players[3].plate.specials.fast
    );
    return 0;
}
EPongRet pong_Basic_waitAllJoinAndSendF(void *opaque)
{
    SConnectionInfo *_this=(SConnectionInfo *)opaque;
    Snetw_game_start_msg *msg;
    /*
#define NETW_GAME_START_MSG 0xF00BAF00
typedef struct Snetw_game_start_msg
{
//SGameData?? Or just some collection? Think.
  //SClientGameData
    SClientGameData client_game_data;
}Snetw_game_start_msg;
*/
    int i;
    printf
    (
        "Game '%s' - waiting for all clients (%u) to join. %u joined now\n",
            _this->gameData->game_name,
            _this->gameData->player_amnt,
            _this->gameData->players_joined
    );
    pong_assert(NULL==opaque,"NULL opaque in pong_Basic_waitAllJoinAndSendF()\n");
    for(i=1;_this->gameData->player_amnt>_this->gameData->players_joined;i++)
    {
        printf("%d players expected, %d joined - Waiting (...%u)\n",_this->gameData->player_amnt,_this->gameData->players_joined,i);
        sleep(1);
    }
    if(_this->gameData->player_amnt!=_this->gameData->players_joined)
    {
        printf
        (
            "pong_Basic_waitAllJoinAndSendF: ERROR: player amnt %u, joined players %u\ni (should be equal)",
            _this->gameData->player_amnt,
            _this->gameData->players_joined
        );
        return EPongRet_ERROR;
    }
    printf("All %u players joined!\n",_this->gameData->player_amnt);
    printf
    (
        "Game 0x%x, Sending NETW_GAME_START_MSG to client 0x%x (%s)\n",
        _this->gameData->gameId,
        _this->own_player_id,
        _this->gameData->players[_this->own_playerdata_index].playerName
    );

    msg = msg_create
    (
        _this->gameData->gameId,
        _this->own_player_id, 
        SERVER_ID, 
        NETW_GAME_START_MSG,
        sizeof(Snetw_game_start_msg)
    );
    pong_assert(NULL==msg,"msg_create() failed!! fo NETW_GAME_START_MSG\n");
            
    //TODO: build "sudden game kill mechanism" for situations where one client dies. Eg. inform othr clients, cleanup & disconnect
    if(fill_initial_gamedata(msg,_this->gameData))
    {
        printf("Failed to fill gamedata to be sent to client\n");
        return EPongRet_ERROR;
    }
    msg->client_game_data.myplayer_index=_this->own_playerdata_index;

    pong_assert( EPongRet_ERROR == msg_send(_this->sock,msg, sizeof(Snetw_game_start_msg)),"MsgSending failed!\n");
    printf("TODO: when all NETW_GAME_START_MSG is sento to all clients, start position updater thread\n");
    printf("Ponder if all clients should be messaged from same thread's context to ensure equal starting time. Scheduling latencies may give visible advance to first client to whom this is sent\n");
    return EPongRet_SUCCESS;
}

EPongRet pong_Basic_parsehelloF(void *opaque)
{
    SConnectionInfo *_this=(SConnectionInfo *)opaque;
    pong_assert(NULL==_this->msg,"NULL msg in pong_Basic_parsehelloF()\n");
    if(msg_getSize(_this->msg)!=sizeof(Snetw_hello_from_client))
    {
        printf("Expexted game_init message (size 0x%x), but received ill-sized package (0x%x), internal hdr (0x%x)!\n",sizeof(Snetw_hello_from_client),msg_getSize(_this->msg),sizeof(Snetw_internal_msg_header));
        return EPongRet_ERROR;
    }
    if(EPongRet_SUCCESS!=parseAndReplyHelloMsg(_this))
    {
        //TODO: add error reply to be sent to client when something fails.
        printf("HelloMsg parsing failed! at %s:%d\n",__FILE__,__LINE__);
        return EPongRet_ERROR;
    }
	return EPongRet_SUCCESS;
}
EPongRet pong_Basic_parseinitF(void *opaque)
{
	SConnectionInfo *_this=(SConnectionInfo *)opaque;
    Snetw_game_init_req *initReqMsg=_this->msg;
	//TODO: build "sudden game kill mechanism" for situations where one client dies. Eg. inform othr clients, cleanup & disconnect
    pong_assert( NULL==initReqMsg,"Did not receive game init req\n");
    printf("Game INIT msg from client received\n");
	GAME_DATA_LOCK(_this->gameData);
	_this->gameData->player_amnt=initReqMsg->number_of_players;
	strncpy(_this->gameData->game_passwd,initReqMsg->passwd,PASSWD_MAX);
    printf("Game INIT msg gave playeramount %d, passwd %s\n",_this->gameData->player_amnt,_this->gameData->game_passwd);
    if(0>=_this->gameData->player_amnt || 4<_this->gameData->player_amnt)
    {
        printf("Invalid player amount in request, discarding!\n");
        return EPongRet_ERROR;
    }
    printf("Initializing server queue\n");
    if(server_queue_init(&(_this->gameData->serverqueue)))
    {
        printf("server_queue_init FAILED\n");
        exit(1);
    }
    printf("Changing _this->gameData->handshakeStatus.init_data_recvd to 1  for game '%s'and letting other dogs in\n",_this->gameData->game_name);
	_this->gameData->handshakeStatus.init_data_recvd=1;
	_this->expected_msg_id=NETW_GAME_JOIN_REQ;
	GAME_DATA_UNLOCK(_this->gameData);
    printf("Game init req received and successfully parsed!\n");
	return EPongRet_SUCCESS;
}
static pthread_mutex_t playeramntmutex=PTHREAD_MUTEX_INITIALIZER;
EPongRet pong_Basic_parsejoinreqF(void *opaque)
{
	//store last pieces of information (client name) && verify password.
	//generate random plate position for client and reply it.
	//After last client joins, generate inital values for balls and walls and everything.
	//launch tick, tick, ticker thread which handles and calculates movements
	//send NETW_GAME_START_MSG with initial game data info...
	//
	//int playerindex;
	SConnectionInfo *_this=(SConnectionInfo *)opaque;
    Snetw_game_join_req *joinReqMsg=_this->msg;
    pong_assert( !joinReqMsg,"NULL game join msg\n");
	if(msg_getSize(joinReqMsg) != sizeof(Snetw_game_join_req))
	{
		printf("Ill sized JOIN req : correct size (0x%x), recvd (0x%x)\n",sizeof(Snetw_game_join_req),msg_getSize(joinReqMsg));
		return EPongRet_ERROR;
	}
    printf("Game JOIN msg from client received - game ID 0x%x, name '%s'\n",_this->gameData->gameId,_this->gameData->game_name);
	if(memcmp(_this->gameData->game_passwd,joinReqMsg->passwd,PASSWD_MAX))
	{
		joinReqMsg->passwd[PASSWD_MAX-1]='\0';
		printf("Wrong passwd '%s' given for game '%s'",joinReqMsg->passwd,_this->gameData->game_name);
		return EPongRet_ERROR;
	}
	/* LOCK NEEDED HERE*/
	pthread_mutex_lock(&playeramntmutex);
    if(!_this->gameData->players_joined)
    {
        if(SERVER_setup_game_generics(_this->gameData))
        {
            printf("Failed to setup game generic datas\n");
            pthread_mutex_unlock(&playeramntmutex);
            return EPongRet_ERROR;
        }
    }
    printf("Game '%s' generic data setup done\n",_this->gameData->game_name);
    if(SERVER_add_player_to_gamedata(_this->gameData,joinReqMsg->player_name))
    {
        printf("Failed to add player to gamedata!\n");
        pthread_mutex_unlock(&playeramntmutex);
        return EPongRet_ERROR;
    }
    _this->own_playerdata_index=_this->gameData->players_joined-1;
    printf("Game '%s' Player data setup done for player '%s'  at index %u\n",_this->gameData->game_name,_this->gameData->players[_this->own_playerdata_index].playerName,_this->own_playerdata_index);
	pthread_mutex_unlock(&playeramntmutex);
	/* LOCK SHOULD BE RELEASED HERE*/
	printf("player (id=%d) '%s' JOINED\n",_this->own_player_id,_this->gameData->players[_this->own_playerdata_index].playerName);
    return EPongRet_SUCCESS;
}

EPongRet parseAndReplyHelloMsg( SConnectionInfo *_this)
{
    char *game_name;
    //int game_id;
    //int playerId;
    Snetw_hello_from_client *hello=(Snetw_hello_from_client *)_this->msg;
//    Snetw_game_init_req *initReqMsg;
//    int player_id;
    pong_assert(NULL==hello,"ERR: NULL hello msg given to parseHelloMsg!\n");
    game_name=hello->game_name;
    Snetw_hello_short_resp *lilreply;
    Snetw_hello_full_resp *fullreply;

    printf("Handling HELLO message: getting gamedata...\n");    

    if(NULL==(_this->gameData=gamedata_lookup(game_name)))
    {
        printf("Failed to init gamedata at %s:%d",__FILE__,__LINE__);

        return EPongRet_ERROR;
    }
    GAME_DATA_LOCK(_this->gameData);
	//Create some "add helloplayerdata" func.
	_this->own_player_id=create_player_id(_this->gameData);
	if(0==_this->own_player_id)
	{
		printf("PlayerDataSorage failure at %s:%d",__FILE__,__LINE__);
		GAME_DATA_UNLOCK(_this->gameData);
		return EPongRet_ERROR;
	}
	_this->gameData->players[_this->own_player_id-1].sock=_this->sock;
    if(1!=_this->gameData->handshakeStatus.init_data_recvd&&0==_this->gameData->handshakeStatus.helloed && 1==_this->own_player_id)
    {
        //game was not inited yet, and this is the first player => Create game (assign first playerId??). 
        //Create Reply:
        _this->gameData->handshakeStatus.helloed=1;
		GAME_DATA_UNLOCK(_this->gameData);
        lilreply = msg_create(_this->gameData->gameId,_this->own_player_id, SERVER_ID, NETW_GAME_HELLO_SRV_SHORT, sizeof(Snetw_hello_short_resp));
        pong_assert(NULL==lilreply,"msg_create() failed!!\n");
        lilreply->req_init_data=1;
			//TODO: build "sudden game kill mechanism" for situations where one client dies. Eg. inform othr clients, cleanup & disconnect
        pong_assert( EPongRet_ERROR == msg_send(_this->sock,lilreply,sizeof(Snetw_hello_short_resp)),"MsgSending failed!\n");
		_this->expected_msg_id=NETW_GAME_INIT_REQ;
/*        
*/	
	}
    else
    {
		//Wait for a while so init_data_recvd is changed by first helloer
		if(1!=_this->gameData->handshakeStatus.init_data_recvd)
		{
			GAME_DATA_UNLOCK(_this->gameData);
			while(1!=_this->gameData->handshakeStatus.init_data_recvd)
			{
				//TODO: make some fallback mechanism, so that error is returned if init_data is not recvd in decent amount of time.
				pong_delay(100);
			}
			GAME_DATA_LOCK(_this->gameData);
		}
		_this->gameData->handshakeStatus.helloed++;
        //game was created => add playerId; etc.
        fullreply = msg_create(_this->gameData->gameId,_this->own_player_id, SERVER_ID, NETW_GAME_HELLO_SRV_FULL, sizeof(Snetw_hello_full_resp));
        fullreply->num_of_players=_this->gameData->player_amnt;
        fullreply->req_init_data=0;
		pong_assert( EPongRet_ERROR == msg_send(_this->sock,fullreply,sizeof(Snetw_hello_full_resp)),"MsgSending failed!\n");
		_this->expected_msg_id=NETW_GAME_JOIN_REQ;
    }
    return EPongRet_SUCCESS;
}
