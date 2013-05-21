#ifndef PONG_CLIENT_QUEUE_H
#define PONG_CLIENT_QUEUE_H
#include <pthread.h>
#include "pong_queue.h"

#define QUEUELEN 255
#define RX_QUEUELEN 20

typedef struct SClientSendQueue
{
    SSendQueue genqueue;
    SQueueItem data_array[QUEUELEN];
    SQueueItem rxdata_array[RX_QUEUELEN];
}SClientSendQueue;

int client_queue_init(SClientSendQueue *_this);
void *poll_clientaction_getdata(SClientSendQueue *_this, size_t *msgsize);
int poll_clientaction_endsend(SClientSendQueue *_this);
int poll_clientaction_startsend(SClientSendQueue *_this);
int poll_clientaction_to_send(SClientSendQueue *_this, unsigned int timeout);
void signal_sender(SClientSendQueue *_this);
int client_queue_add_data(SClientSendQueue *_this,void *data,size_t datasize);
int rx_client_queue_add_data(SClientSendQueue *_this,void *data,size_t datasize);
int rx_poll_clientaction_startsend(SClientSendQueue *_this);
int rx_poll_clientaction_endsend(SClientSendQueue *_this);
void *rx_poll_clientaction_getdata(SClientSendQueue *_this, size_t *msgsize);

#endif

