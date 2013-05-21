#include "pong_queue.h"
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <errno.h>
#include "pong_misc_func.h"

static int queue_lock(SSendQueue *_this,size_t mutexoffset);

int init_queue(SSendQueue *_this)
{
    int err;
    pthread_mutexattr_t attr;
    if(pthread_mutexattr_init(&attr))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    if(pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_ERRORCHECK))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    if(pthread_cond_init(&(_this->condvar),NULL))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
//    printf("condvar after init %p\n",&(_this->condvar));
    if((err=pthread_mutex_init(&(_this->condmutex),&attr)))
    {
        printf("%s:%d mutex %p err %d\n",__FILE__,__LINE__,&(_this->condmutex),err);
        exit(1);
    }
    if(pthread_mutex_init(&(_this->rxqueuemutex),&attr))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    if(pthread_mutex_init(&(_this->queuemutex),&attr))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    return 0;
}
void queue_txdata_consumed(SSendQueue *_this,unsigned int queuesize)
{
    if(_this->current==_this->last)
        return;
    _this->full=0;
    _this->current++;
    if(_this->current==queuesize)
        _this->current=0;
}
int queue_peek_txdata(SSendQueue *_this,unsigned int queuesize)
{
    if(_this->current==_this->last)
        return -1;
    if(_this->current+1==queuesize)
        return 0;
    return _this->current+1;
}

static void *queue_getdata_internal(SSendQueue *_this,SQueueItem* array,size_t *msgsize, size_t offset, unsigned int queuesize)
{
    struct helper { int full; int current; int last; } *helper;
    helper=(struct helper*)(((char *)_this)+offset);

    if(helper->current==helper->last)
        return NULL;
    helper->current++;
    helper->full=0;
    if(helper->current==queuesize)
        helper->current=0;
    *msgsize=array[_this->current].datasize;
    return array[helper->current].data;
}

void *queue_getrxdata(SSendQueue *_this,SQueueItem* array,size_t *msgsize ,unsigned int queuelen)
{
    if(!_this || !msgsize)
    {
        printf("NULL ptr in %s!\n",__FUNCTION__);
        return NULL;
    }
    return queue_getdata_internal(_this,array,msgsize,offsetof(SSendQueue,rxfull) ,queuelen);
}
void *queue_gettxdata(SSendQueue *_this,SQueueItem* array,size_t *msgsize ,unsigned int queuelen)
{
    if(!_this || !msgsize)
    {
        printf("NULL ptr in %s!\n",__FUNCTION__);
        return NULL;
    }
    return queue_getdata_internal(_this,array,msgsize,offsetof(SSendQueue,full) ,queuelen);
}
static int queue_unlock(SSendQueue *_this,size_t mutexoffset)
{
    pthread_mutex_t *mutex;
    if(!_this)
        return -1;
    mutex=(pthread_mutex_t *)(((char *)_this)+mutexoffset);
    if(pthread_mutex_unlock(mutex))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    return 0;
}
int queue_rxfull(SSendQueue *_this)
{
    return _this->rxfull;
}
int queue_txfull(SSendQueue *_this)
{
    return _this->full;
}
/* These must be called mutex successfully locked via lock_if_space - returns index to store data*/
int queue_rx_add_unsafe(SSendQueue *_this,unsigned int queuelen)
{
    _this->rxlast++;
    if(queuelen==_this->rxlast)
        _this->rxlast=0;
    printf("Add unsafe updating rxdata (last: %d => %d)\n",_this->rxlast-1,_this->rxlast);
    if(_this->rxlast == _this->rxcurrent-1 || (_this->rxlast==queuelen-1 && 0==_this->rxcurrent))
    {
        printf("Add unsafe: rxfull=%d\n",_this->rxfull);
        _this->rxfull=1;
    }
    printf("Add unsafe: returning add index %d\n",_this->rxlast);
    return _this->rxlast;
}

int queue_tx_add_unsafe(SSendQueue *_this,unsigned int queuelen)
{
    _this->last++;
    if(queuelen==_this->last)
        _this->last=0;
    if(_this->last == _this->current-1 || (_this->last==queuelen-1 && 0==_this->current))
    {
        _this->full=1;
    }
    return _this->last;
}

int queue_tx_lock_if_space(SSendQueue *_this)
{
    queue_lock(_this,offsetof(SSendQueue,queuemutex));
    if(_this->full)
    {
        queue_unlock(_this,offsetof(SSendQueue,queuemutex));
        return -1;
    }
    return 0;
}

int queue_rx_lock_if_space(SSendQueue *_this)
{
    queue_lock(_this,offsetof(SSendQueue,rxqueuemutex));
    if(_this->rxfull)
    {
        queue_unlock(_this,offsetof(SSendQueue,rxqueuemutex));
        return -1;
    }
    return 0;
}



static int queue_lock(SSendQueue *_this,size_t mutexoffset)
{
    pthread_mutex_t *mutex;
    if(!_this)
        return -1;
    mutex=(pthread_mutex_t *)(((char *)_this)+mutexoffset);
    if(pthread_mutex_lock(mutex))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    return 0;
}
int queue_lock_rx(SSendQueue *_this)
{
    return queue_lock(_this,offsetof(SSendQueue,rxqueuemutex));
}
int queue_lock_tx(SSendQueue *_this)
{
    return queue_lock(_this,offsetof(SSendQueue,queuemutex));
}
int queue_unlock_rx(SSendQueue *_this)
{
    return queue_unlock(_this,offsetof(SSendQueue,rxqueuemutex));
}
int queue_unlock_tx(SSendQueue *_this)
{
    return queue_unlock(_this,offsetof(SSendQueue,queuemutex));
}
void queue_signal(SSendQueue *_this)
{
    int err;
    if((err=pthread_mutex_lock(&(_this->condmutex))))
    {
        printf("%s:%d mutex %p err %d\n",__FILE__,__LINE__,&(_this->condmutex),err);
        exit(1);
    }

    if(pthread_cond_broadcast(&(_this->condvar)))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    if((err=pthread_mutex_unlock(&(_this->condmutex))))
    {
        printf("%s:%d mutex %p err %d\n",__FILE__,__LINE__,&(_this->condmutex),err);
        exit(1);
    }

}
int queue_poll(SSendQueue *_this, unsigned int timeout)
{
    struct timespec tm;
    int rval;
    int err;
    if(!_this)
    {
        printf("Vituiksi meni!\n");
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &tm);
    //mva_gettime(&tm);
    tm.tv_nsec+=1000000*timeout;

    while (tm.tv_nsec > 999999999 || -1 > tm.tv_nsec )
    {
        tm.tv_sec ++;
        tm.tv_nsec -= 1000000000;
    }

    if((err=pthread_mutex_lock(&(_this->condmutex))))
    {
        printf("%s:%d mutex %p err %d\n",__FILE__,__LINE__,&(_this->condmutex),err);
        exit(1);
    }
    while(_this->current==_this->last && _this->rxcurrent==_this->rxlast)
    {
        if((rval=pthread_cond_timedwait(&(_this->condvar),&(_this->condmutex),&tm)))
        {
            if((err=pthread_mutex_unlock(&(_this->condmutex))))
            {
                printf("%s:%d mutex %p err %d\n",__FILE__,__LINE__,&(_this->condmutex),err);
                exit(1);
            }
            if(ETIMEDOUT==rval)
            {
                return 0;
            }
            printf("tm.sec=%u, tm.nsec=%ld\n",(unsigned)tm.tv_sec,tm.tv_nsec);
            printf("pthread_cond_timedwait() returned error %d!\n",rval);
            return -1;
        }
    }
    /* We have been signaled */
    if((err=pthread_mutex_unlock(&(_this->condmutex))))
    {
        printf("%s:%d mutex %p err %d\n",__FILE__,__LINE__,&(_this->condmutex),err);
        exit(1);
    }
    return 1;
}



