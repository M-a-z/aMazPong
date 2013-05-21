/* ************************************************************************** */
/*
 * The purpose of this file is to provide generic definitions and includes for 
 * using unix and winsock sockets. 
 *
 *    Revision history:
 *
 *    -0.0.2  18.09.2008/Maz  Ironed out some compile errors.    
 *    -0.0.1  19.06.2008/Maz
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */

#ifndef PONG_NETW_H
#define PONG_NETW_H

#include <errno.h>
#ifdef __LINUX__
     #include <sys/select.h>
     #include <sys/socket.h>
     #include <netinet/in.h>
     #include <arpa/inet.h>
     #define PRINT_S_ERR() printf("Err=%d\n",errno)
//     typedef int SOCKET;
#else
     #include <winsock2.h>
//     #define EAGAIN WSAEWOULDBLOCK 
     #define EWOULDBLOCK WSAEWOULDBLOCK 
     #define PRINT_S_ERR() printf("Err=%d\n",WSAGetLastError());
#endif
#include "general.h"
#define SERVER_POS_UPD_PORT 51003
#define SERVER_TCP_PORT 51003
#define SERVER_ID 0xB00BABE
/*
typedef enum EHelloMsgRet
{
    EHelloMsgRet_Error     = 0,
    EHelloMsgRet_Inited    = 1,
    EHelloMsgRet_NotInited = 2
}EHelloMsgRet;
*/

typedef enum EPongRecvRet
{
    EPongRecvRet_ERROR = 0,
    EPongRecvRet_SUCCESS = 1,
    EPongRecvRet_EAGAIN = 3
}EPongRecvRet;
typedef enum EpongPollRetVal
{
        EpongPollRetVal_Error = 0,
        EpongPollRetVal_NewData = 1,
        EpongPollRetVal_NothingNew = 2
}EpongPollRetVal;
        
int msg_getSize(void *buff);
void fill_msg_header(void *msg, char*receiver, int MSG_ID, size_t msg_size);
unsigned int msg_getReceiver(void *replymsg);
unsigned int msg_getSender(void *replymsg);
unsigned int msg_getId(void *buff);
int msg_getGameId(void *buff);
void * msg_create(unsigned int game_id, unsigned int receiver_id, unsigned int own_id, int MSG_ID, size_t msg_size);
void msg_free(void **msg);
EPongRet msg_send(SOCKET sock, void *msg, size_t msgsize);
EpongPollRetVal dataIsComing(SOCKET sock, unsigned long int tmo);
void *msg_receive(SOCKET sock,int tmo,EPongRecvRet *status);
EPongRecvRet dataNonBlockReceive(SOCKET sock, void *buff, unsigned int size, int tmo);
//Actually this is same as abowe function :/
//void *blocking_sock_data_receive(SOCKET sock, int tmo);

#endif //PONG_NETW_H
