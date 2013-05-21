#ifndef PONG_SERVER_QUEUE_H
#define PONG_SERVER_QUEUE_H
#include <pthread.h>
#include "pong_queue.h"
#include "pong_dataStorage.h"


int server_queue_init(SServerSendQueue *_this);
int server_lock_tx(SServerSendQueue *_this);
int server_unlock_tx(SServerSendQueue *_this);
void server_data_free(void *data);
void *server_queue_get_tx_data(SServerSendQueue *_this, size_t *datalen,int *imlast);
int poll_serverdata_to_send(SServerSendQueue *_this, unsigned int timeout);
int dec_senderamnt(char *store);
void add_senderamnt(char *store,unsigned char senderamnt);
int server_queue_add_tx_data(SServerSendQueue *_this,void *data,size_t datasize,unsigned char senderamnt);
/*
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
*/
#endif

