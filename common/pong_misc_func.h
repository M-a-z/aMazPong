/* ************************************************************************** */
/*
 *    This will be a collection of misc functions. 
 *
 *    Revision history:
 *
 *    -0.0.2  23.09.2008/Maz Added playerId related funcs
 *    -0.0.1  12.06.2008/Maz  First Draft. (winsock init func)
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */

#ifndef PONG_MISC_FUNC_H
#define PONG_MISC_FUNC_H


#include "general.h"
#include <pthread.h>
#include <sys/time.h>

//int create_player_id(short gameId,short playerNo);
void godawfulwinsockinit(void);
void pong_delay(uint_64 delay_msec);
void explode_playerId(int playerId, short *gameId, short *playerno);
short playerIdToNo(int playerId);
short playerIdToGameId(int playerId);
#ifdef __WIN32__
    #ifndef CLOCK_REALTIME
        #define CLOCK_REALTIME 0
    #endif
    #define clock_gettime(foo,bar) mva_gettime(bar)
    void mva_gettime(struct timespec *tm);
#endif

#endif // PONG_MISC_FUNC_H
