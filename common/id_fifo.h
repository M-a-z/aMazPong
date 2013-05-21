/* ************************************************************************** */
/*
 *    This header will define an IdFifo, usable for producing unique ID numbers
 *
 *    Revision history:
 *
 * 
 *    -0.0.1  06.01.2009/Maz  First Draft.
 *
 *
 *    PasteLeft Maz 2009.
 */
/* ************************************************************************** */



#ifndef PoNG_ID_FIFo
#define PoNG_ID_FIFo

#ifdef __CPP
extern "C"
{
#endif

#include <stdio.h>
#include <pthread.h>
//free_ids[1] reserves space for one id => invalid Id (0) is reserved.
#define IdPool_calcAllocSize(idamnt) (sizeof(SIdFifo)+(idamnt)*sizeof(unsigned int))
#define IdPoolNonFreeId 0xFFFFFFFF
#define IDFIFO_ID_INVALID 0
typedef struct SIdFifo
{
    pthread_mutex_t idpoolLock;
    unsigned int next;
    unsigned int amnt;
    unsigned int base;
    unsigned int free_ids[1];
}SIdFifo;


void fifopool_init(SIdFifo *pool, unsigned int base, unsigned int id_amnt);
unsigned int idpool_idReserve(SIdFifo *pool);
void idpool_idRelease(SIdFifo *pool,unsigned int id);
void idpool_uninit(SIdFifo *pool);


#ifdef __CPP
}
#endif

#endif // PoNG_ID_FIFo
