/* ************************************************************************** */
/*
 *    This file will implement an IdFifo, usable for producing unique ID numbers
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


#include "id_fifo.h"
#include <stdlib.h>

void find_and_print_dublicates(SIdFifo *pool);
void fifopool_init(SIdFifo *pool, unsigned int base, unsigned int id_amnt)
{
    int i=0;
    if(pool==NULL)
    {
        printf("IdFifo: Error, pool is NULL at init! %s:%d",__FILE__,__LINE__);
        fflush(stdout);
	    exit(1);
    }
    for(i=0;i<=id_amnt;i++)
    {
        pool->free_ids[i]=i;
    }
    pool->amnt=id_amnt;
    pool->base=base;
    pool->next=0;
    pthread_mutex_init(&(pool->idpoolLock),NULL);
}

unsigned int idpool_idReserve(SIdFifo *pool)
{
    unsigned int id=IDFIFO_ID_INVALID;
    unsigned int i;
    pthread_mutex_lock(&(pool->idpoolLock));
    //ensure pool->next is indeed free
    if(pool->free_ids[pool->next]!=IdPoolNonFreeId)
    {
        id=pool->free_ids[pool->next];
        pool->next++;
    }
    else
    {
#ifdef DEBUGPRINTS
        printf("IdPool out of sync!! Resyncing...");
#endif
        //odd... fallback
        for(i=pool->next;i<=pool->amnt;i++)
        {
            if(pool->free_ids[i]==IdPoolNonFreeId)
                continue;
            id=pool->free_ids[i];
            pool->free_ids[i]=IdPoolNonFreeId;
            pool->next=i+1;
            break;
        }
        if(id==IdPoolNonFreeId)
            for(;i<pool->next;i++)
            {
                if(pool->free_ids[i]==IdPoolNonFreeId)
                    continue;
                id=pool->free_ids[i];
                pool->free_ids[i]=IdPoolNonFreeId;
                pool->next=i+1;
                break;
            }
    }
    if(pool->next>pool->amnt)
        pool->next=0;
    pthread_mutex_unlock(&(pool->idpoolLock));
#ifdef DEBUGPRINTS
    if(id==IdPoolNonFreeId)
    {
        printf("All Ids Wasted, cannot allocate! %s, %s:%d",__FUNCTION__,__FILE__,__LINE__);
    }
#endif
    return (id==IdPoolNonFreeId)?id:id+pool->base;
}

void idpool_idRelease(SIdFifo *pool,unsigned int id)
{
    unsigned int internal_id = id-pool->base;
    unsigned int i;
    unsigned int place=0;
    pthread_mutex_lock(&(pool->idpoolLock));
    for(i=pool->next-2;i>0;i--)
    {
        if(pool->free_ids[i]!=IdPoolNonFreeId)
        {
            if(pool->free_ids[i+1]==IdPoolNonFreeId)
            {
                place=i+1;
                break;
            }
#ifdef DEBUGPRINTS
            else
                printf("IdPool out Of Sync at FREE!!, %s:%d",__FILE__,__LINE__);
#endif
        }
    }
    if(place==0)
    {
        for(i=pool->amnt;i>pool->next-1;i--)
        {
            if(pool->free_ids[i]!=IdPoolNonFreeId)
            {
                if(pool->free_ids[ (i+1<pool->amnt)?i+1:0 ]==IdPoolNonFreeId)
                {
                    place=(i+1<pool->amnt)?i+1:0;
                    break;
                }
#ifdef DEBUGPRINTS
                else
                    printf("IdPool out Of Sync at FREE!!, %s:%d",__FILE__,__LINE__);
#endif
            }
        }
    }    
    if(place==0)
    {
        printf("IdPool - Invalid Release, all Ids already released");
        find_and_print_dublicates(pool);
        fflush(stdout);
        exit(1);
    }
    pool->free_ids[place]=internal_id;
    pthread_mutex_unlock(&(pool->idpoolLock));
}

void find_and_print_dublicates(SIdFifo *pool)
{
    //TODO: toDo.
    return ;
}

void idpool_uninit(SIdFifo *pool)
{
    pthread_mutex_destroy(&(pool->idpoolLock));
}
