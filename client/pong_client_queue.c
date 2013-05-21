#include "pong_client_queue.h"
//#include "pong_queue.h"
#include "pong_misc_func.h"
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>

int poll_clientaction_startsend_internal(SClientSendQueue *_this,size_t mutexoffset);
int poll_clientaction_endsend_internal(SClientSendQueue *_this,size_t mutexoffset);
void *poll_clientaction_getdata_internal(SClientSendQueue *_this, size_t *msgsize, size_t offset, unsigned int queuesize);

int poll_clientaction_startsend_internal(SClientSendQueue *_this,size_t mutexoffset);

int client_queue_init(SClientSendQueue *_this)
{
    if(!_this)
    {
        printf("NULL ptr in client_queue_init\n");
        return -1;
    }
    memset(_this,0,sizeof(SClientSendQueue));
    return init_queue(&(_this->genqueue));
}
void *rx_poll_clientaction_getdata(SClientSendQueue *_this, size_t *msgsize)
{
    return queue_getrxdata(&(_this->genqueue),_this->rxdata_array,msgsize,RX_QUEUELEN);
//    return poll_clientaction_getdata_internal(_this, msgsize, offsetof(SClientSendQueue,rxfull),RX_QUEUELEN);
}


void *poll_clientaction_getdata(SClientSendQueue *_this, size_t *msgsize)
{
    return queue_gettxdata(&(_this->genqueue),_this->data_array,msgsize,QUEUELEN);
    //return poll_clientaction_getdata_internal(_this, msgsize, offsetof(SClientSendQueue,full),QUEUELEN);
}
/*
void *poll_clientaction_getdata_internal(SClientSendQueue *_this, size_t *msgsize, size_t offset, unsigned int queuesize)
{
    struct helper { int full; int current; int last; SClientQueueItem *darra; } *helper;

    if(!_this || !msgsize)
    {
        printf("NULL ptr in poll_clientaction_getdatai_internal!\n");
        return NULL;
    }
    helper=(struct helper*)(((char *)_this)+offset);
    if(helper->current==helper->last)
        return NULL;
    helper->current++;
    helper->full=0;
    if(helper->current==queuesize)
        helper->current=0;
    *msgsize=helper->darra[_this->current].datasize;
    return helper->darra[helper->current].data;
}
*/
int rx_poll_clientaction_endsend(SClientSendQueue *_this)
{
    return queue_unlock_rx(&(_this->genqueue));
}

int poll_clientaction_endsend(SClientSendQueue *_this)
{
    return queue_unlock_tx(&(_this->genqueue));
    //return poll_clientaction_endsend_internal(_this,offsetof(SClientSendQueue,queuemutex));
}
/*
int poll_clientaction_endsend_internal(SClientSendQueue *_this,size_t mutexoffset)
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
*/
int poll_clientaction_startsend(SClientSendQueue *_this)
{
    return queue_lock_tx(&(_this->genqueue));
    //return poll_clientaction_startsend_internal(_this,offsetof(SClientSendQueue,queuemutex));
}

int rx_poll_clientaction_startsend(SClientSendQueue *_this)
{
    return queue_lock_rx(&(_this->genqueue));
//    return poll_clientaction_startsend_internal(_this,offsetof(SClientSendQueue,rxqueuemutex));
}
/*
int poll_clientaction_startsend_internal(SClientSendQueue *_this,size_t mutexoffset)
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
*/
int poll_clientaction_to_send(SClientSendQueue *_this, unsigned int timeout)
{
    return queue_poll(&(_this->genqueue),timeout);
/*    struct timespec tm;
    //struct timespec tmp;
    int rval;
    
    if(!_this)
    {
        printf("Vituiksi meni!\n");
        return -1;
    }
    clock_gettime(CLOCK_REALTIME, &tm);
    //mva_gettime(&tm);
    tm.tv_nsec+=1000000*timeout;

    if (tm.tv_nsec > 999999999)
    {
        tm.tv_sec ++;
        tm.tv_nsec -= 1000000000;
    }

    if(pthread_mutex_lock(&(_this->condmutex)))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    while(_this->current==_this->last && _this->rxcurrent==_this->rxlast)
        if((rval=pthread_cond_timedwait(&(_this->condvar),&(_this->condmutex),&tm)))
        {
            if(pthread_mutex_unlock(&(_this->condmutex)))
            {
                printf("%s:%d mutex\n",__FILE__,__LINE__);
                exit(1);
            }
            if(ETIMEDOUT==rval)
            {
                return 0;
            }
            printf("tm.sec=%u, tm.nsec=%ld\n",tm.tv_sec,tm.tv_nsec);
            printf("pthread_cond_timedwait() returned error %d!\n",rval);
            return -1;
        }
    if(pthread_mutex_unlock(&(_this->condmutex)))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    return 1;
    */
}
void signal_sender(SClientSendQueue *_this)
{
    return queue_signal(&(_this->genqueue));
}
int client_queue_add_data(SClientSendQueue *_this,void *data,size_t datasize)
{
    int addindex;
    if(0==datasize || !data)
    {
        printf("client_queue_add_data() invalid data\n");
        return -1;
    }
    /*
    if(queue_lock_tx(&(_this->genqueue)))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    */
    while(queue_tx_lock_if_space(&(_this->genqueue)))
    {
        queue_signal(&(_this->genqueue));
    }
    addindex=queue_tx_add_unsafe(&(_this->genqueue),QUEUELEN);
    _this->data_array[addindex].datasize=datasize;
    _this->data_array[addindex].data=data;
    if(queue_unlock_tx(&(_this->genqueue)))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    if(queue_txfull(&(_this->genqueue)))
        queue_signal(&(_this->genqueue));
    return 0;
}

int rx_client_queue_add_data(SClientSendQueue *_this,void *data,size_t datasize)
{
    int addindex;
    if(0==datasize || !data)
    {
        printf("rx_client_queue_add_data() invalid data\n");
        return -1;
    }
    while(queue_rx_lock_if_space(&(_this->genqueue)))
    {
        queue_signal(&(_this->genqueue));
    }

    addindex=queue_rx_add_unsafe(&(_this->genqueue),RX_QUEUELEN);
    printf("Adding client data to index %d\n",addindex);
    _this->rxdata_array[addindex].datasize=datasize;
    _this->rxdata_array[addindex].data=data;
    if(queue_unlock_rx(&(_this->genqueue)))
    {
        printf("%s:%d mutex\n",__FILE__,__LINE__);
        exit(1);
    }
    if(queue_rxfull(&(_this->genqueue)))
        queue_signal(&(_this->genqueue));
    return 0;
}


