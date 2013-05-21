/* ************************************************************************** */
/*
 *    This will be a collection of misc functions. 
 *
 *    Revision history:
 *
 *    
 *    -0.0.3  23.09.2008/Maz Added playerId related funcs
 *    -0.0.2  18.09.2008/Maz  Ironed out some compile errors.    
 *    -0.0.1  12.06.2008/Maz  First Draft. (winsock init func)
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */



#include "pong_misc_func.h"
//#include "general.h"
//#include <stdio.h>
//#include "pong_server_accept_client.h"
#include <general.h>
#include <sys/types.h>
#ifndef __LINUX__
    void godawfulwinsockinit()
    { 
            WSADATA wsaData;  
            if (WSAStartup(MAKEWORD(1, 1), &wsaData) != 0) 
            { 
                    fprintf(stderr, "WSAStartup failed.\n"); 
                    exit(1); 
            }
    }

    void mva_gettime(struct timespec *tm)
    {
        struct timeval tv;
        gettimeofday(&tv,NULL);
        tm->tv_sec=tv.tv_sec;
        tm->tv_nsec=tv.tv_usec*1000UL;
    }
            
#else 
#include <unistd.h>
        void godawfulwinsockinit()
        { 
                ;
        } 
#endif


void explode_playerId(int playerId, short *gameId, short *playerno)
{
    pong_assert((NULL==gameId || NULL==playerno),"NULL value given to explode_playerId!");
    *playerno=(playerId&0x0000FFFF);
    *gameId=( (playerId&0xFFFF0000) >> 16 );
}
/*
int create_player_id(short gameId,short playerNo)
{
    int playerId=0;
    playerId=gameId;
    playerId=(playerId<<16);
    return playerId+=playerNo;
}
*/
short playerIdToNo(int playerId)
{
    return (short)(playerId&0x0000FFFF);
}

short playerIdToGameId(int playerId)
{
    return (short)( ( playerId&0xFFFF0000 ) << 16 );      
}
      
void pong_delay(uint_64 delay_msec)
{
#ifdef __LINUX__
    int smallscale=(int)(delay_msec%1000);
    if(delay_msec>1000)
    {
        sleep((int)delay_msec/1000);
        //TODO: Check if nanosleep is safer!
    }
    usleep(smallscale*1000);
#else
    //TODO: check what options windows gives for high precision sleep
    Sleep((double)delay_msec/1000);
#endif
}
