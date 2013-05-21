/* ************************************************************************** */
/*
 *    The purpose of this file is to provide message definitions for messaging
 *    between client and server. Generally the first 32 (size_t, can it be 
 *    more/less than 32 bits?) bits in message will tell the size of the 
 *    message. Next 32 bits will tell the message ID (type) and then there's
 *    sender's and receiver's ids (short integers, server will always have ID 
 *    PONG_SERVER_ID). This will form the 'header'. 
 *    Rest of the message is the 'payload', which depends on the type of the 
 *    message.
 *
 *    Game startup scenario (messages:)
 *
 *    Server listens for client connections. When client establishes connection,
 *    following negotiation is accomplished:
 *    
 *    1. Client sends a 'hello' msg. (header and game name.
 *    2. Server responds with either short or full reply.
 *        Short is used when client was first who joined to game with specified
 *        name. In reply there is assigned client ID for client, and ID for 
 *        game. After sending short reply, server is waiting for client to send
 *        game init data (see 3). If server sends full reply, it means that game
 *        init data was already sent by other client, and server tells the 
 *        assigned game id number, number of players, and header contains 
 *        assigned client ID. Next server waits for game join req message. 
 *        (see 5).
 *    3. The first client who initiates a game, sends game init req to server,
 *        when server asks for it. game init req consists of number of player,
 *        game password and game id which server told in hello msg.
 *    4. Server sends game init resp message back to client who sent game init
 *        req.
 *    5. All clients send game join req to server. Client who sent game init 
 *        data sends this req after game init resp from server. Other clients
 *        send it right after full hello resp.
 *        game join req contains game_id, password and user's user name.
 *    6. Server sends game join responce with success code.
 *
 *    7. Server may or may not send ping messages at this point.
 *
 *    8. After all clients have joined, server sends game_ready msg with initial
 *        plate informations.
 *
 *
 *
 *    TODO: consider. Could it be funny to allow different game mode, where
 *    opponent's plates were not shown? If so, this should be added in game
 *    init req, and full hello resp. In no opponents visible mode, the game data
 *    does not need to contain info about other plates.
 *
 *
 *
 *
 *    Revision history:
 *
 *    -0.0.5  06.01.2009/Maz  placed game_id to header, and made passwd fields 
 *    						  to static sized instead of using placeholder and
 *    						  added game_start_msg defines (which still are incomplete)
 *    -0.0.4  22.06.2008/Maz  Added some messages, and specified the startup
 *                            messaging scenario in comments abowe.
 *    -0.0.3  19.06.2008/Maz  Added EplatePosition in game_init_resp
 *    -0.0.2  11.06.2008/Maz  Added the comment section + header struct which 
 *                            will be in each message struct.
 *    -0.0.1  04.06.2008/Maz  First Draft.
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */


#ifndef NETWORK_MESSAGES_H
#define NETWORK_MESSAGES_H

#include "general.h"
#include "pong_dataStorage.h"
//size_t(4)+int(4)+unsigned int(4)+unsigned long long(8) = 20
#define NETW_MSG_PING_REQ_SIZE 20

//Message header
/*TODO: Change ips to some ID numbers so it is possible to have many players on
same computer */






typedef struct Snetw_internal_msg_header
{
    size_t msgSize;
    int msg_id;
	unsigned int game_id;
    unsigned int sender_id; //ints are better??
    unsigned int receiver_id; //ints are better??
}Snetw_internal_msg_header;

//Msgs inited by client and server's replies.


#define NETW_GAME_HELLO_CLI 0xBADDAD01
typedef struct Snetw_hello_from_client
{
//    Snetw_internal_msg_heaser header;
    char game_name[GAME_NAME_MAX];
}Snetw_hello_from_client;

#define NETW_GAME_HELLO_SRV_SHORT 0xDEEB5C17
typedef struct Snetw_hello_short_resp
{
//    Snetw_internal_msg_header header;
    int req_init_data;
}Snetw_hello_short_resp;

#define NETW_GAME_HELLO_SRV_FULL 0xDEEB5C27
typedef struct Snetw_hello_full_resp
{
//    Snetw_internal_msg_header header;
    int req_init_data;
    int num_of_players;
}Snetw_hello_full_resp;

#define NETW_GAME_JOIN_REQ 0xADD00001
typedef struct Snetw_game_join_req
{
//    Snetw_internal_msg_header header;
    char player_name[PLAYER_NAME_MAX];
    char passwd[PASSWD_MAX];
}Snetw_game_join_req;

#define NETW_GAME_JOIN_RESP 0xADD00002
typedef struct Snetw_game_join_resp
{
//    Snetw_internal_msg_header header;
    int success_code;
    char player_name[PLAYER_NAME_MAX];
//    EplatePosition pos;
}Snetw_game_join_resp;

#define NETW_GAME_INIT_REQ 0xB00BBABE
typedef struct Snetw_game_init_req
{
//    Snetw_internal_msg_header header;
    int number_of_players;
    char passwd[PASSWD_MAX]; //I'd rather not use placeholder since it spoils the sizeof usage.
}Snetw_game_init_req;

#define NETW_GAME_INIT_RESP 0xBABEB00B
typedef struct Snetw_game_init_resp
{
//    Snetw_internal_msg_header header;
    int success_code;
}Snetw_game_init_resp;

//Msgs inited by server and client's replies.
#define NETW_GAME_START_MSG 0xF00BAF00
typedef struct Snetw_game_start_msg
{
//SGameData?? Or just some collection? Think.
  //SClientGameData
    SClientGameData client_game_data;
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

#define NETW_PING_RESP 0xBEEFFEED
typedef struct Snetw_ping_resp
{
    size_t msgSize;
    int clientIP;
    unsigned int trid;
    unsigned long long server_gametime;        
}Snetw_ping_resp;

#define NETW_GAMEDATA_UPDATE_MSG 0xFA700A55
typedef struct Snetw_gamedata_update_msg
{
    unsigned long long time_from_gamestart; /* currently unused, may be needed in future */
    SClientGameData client_game_data;
}Snetw_gamedata_update_msg;

#define NETW_GAME_KILL_MSG 0xD1ED1E11
typedef struct Snetw_game_kill_msg
{
    unsigned int reason;
    char message[50];
}Snetw_game_kill_msg;

#define NETW_GAME_CLIENT_KEY_PRESSED_MSG 0x0DDBABE5
typedef struct Snetw_client_dir_change_pressed_msg
{
    /* currently unused, may be needed in future */
    unsigned long long time_from_gamestart; 
    int newdirection;
}Snetw_client_dir_change_pressed_msg;

#endif //


