/* ************************************************************************** */
/*
 *    The purpose of this file is to create a thread which calculates the ball
 *    and paddle positions && updates the data structures in commP singleton
 *    class.
 *
 *    Revision history:
 *
 *    
 *    -0.0.1  04.06.2008/Maz  First Draft.
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */


#ifndef PONG_PLACE_CTR_H
#define PONG_PLACE_CTR_H
#include <pthread.h>
#include "pong_dataStorage.h"

EPongRet launch_timerthread(SGameData *gameData);
//void * schedfunc(void *arg);
#endif //PONG_PLACE_CTR_H
