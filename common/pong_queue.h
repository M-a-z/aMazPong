#ifndef PONG_QUEUE_H
#define PONG_QUEUE_H
#include <pthread.h>

typedef struct SQueueItem
{
    size_t datasize;
    void *data;
}SQueueItem;


typedef struct SSendQueue
{
    pthread_cond_t condvar;
    pthread_mutex_t condmutex;
    pthread_mutex_t queuemutex;
    pthread_mutex_t rxqueuemutex;
    int full;
    int current;
    int last;
    int rxfull;
    int rxcurrent;
    int rxlast;
}SSendQueue;

int init_queue(SSendQueue *_this);

void *queue_gettxdata(SSendQueue *_this,SQueueItem* array,size_t *msgsize ,unsigned int queuelen);
void *queue_getrxdata(SSendQueue *_this,SQueueItem* array,size_t *msgsize, unsigned int queuelen);
int queue_lock_rx(SSendQueue *_this);
int queue_lock_tx(SSendQueue *_this);
int queue_unlock_rx(SSendQueue *_this);
int queue_unlock_tx(SSendQueue *_this);
int queue_poll(SSendQueue *_this, unsigned int timeout);
void queue_signal(SSendQueue *_this);
/* This must be called mutex successfully locked via lock_if_space - returns index to store data*/
int queue_tx_add_unsafe(SSendQueue *_this,unsigned int queuelen);
int queue_rx_add_unsafe(SSendQueue *_this,unsigned int queuelen);
int queue_txfull(SSendQueue *_this);
int queue_rxfull(SSendQueue *_this);
int queue_tx_lock_if_space(SSendQueue *_this);
int queue_rx_lock_if_space(SSendQueue *_this);
void queue_txdata_consumed(SSendQueue *_this,unsigned int queuesize);
int queue_peek_txdata(SSendQueue *_this,unsigned int queuesize);

#endif
