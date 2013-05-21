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
 *    -0.0.1  26.09.2008/Maz Separated this startup from actual handshake 
 *                           messaging
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */

#ifndef SERVER_STARTUP_H
#define SERVER_STARTUP_H

#ifdef __CPP
extern "C"
{
#endif

#include "general.h"
typedef enum EGameType
{
    EGameType_Normal = 0,
    EGameType_Invalid
}EGameType;

typedef struct SStartParams
{
    EGameType gametype;
}SStartParams;

EPongRet start_server();

#endif //SERVER_STARTUP_H

#ifdef __CPP
}
#endif
