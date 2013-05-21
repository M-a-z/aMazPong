
#include "pong_server_queue.h"
#include "pong_misc_func.h"
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stddef.h>
/*
int poll_clientaction_startsend_internal(SClientSendQueue *_this,size_t mutexoffset);
int poll_clientaction_endsend_internal(SClientSendQueue *_this,size_t mutexoffset);
void *poll_clientaction_getdata_internal(SClientSendQueue *_this, size_t *msgsize, size_t offset, unsigned int queuesize);

int poll_clientaction_startsend_internal(SClientSendQueue *_this,size_t mutexoffset);
*/
int server_queue_init(SServerSendQueue *_this)
{
    if(!_this)
    {
        printf("NULL ptr in %s\n",__FUNCTION__);
        return -1;
    }
    memset(_this,0,sizeof(SServerSendQueue));
    return init_queue(&(_this->genqueue));
}
#if 0
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
#endif
int server_lock_tx(SServerSendQueue *_this)
{
    return queue_lock_tx(&(_this->genqueue));
}
int server_unlock_tx(SServerSendQueue *_this)
{
    return queue_unlock_tx(&(_this->genqueue));
}
void server_data_free(void *data)
{
    free(data);
}
void *server_queue_get_tx_data(SServerSendQueue *_this, size_t *datalen,int *imlast)
{
    int index;
    //SQueueItem *itm;
    index=queue_peek_txdata(&(_this->genqueue),SERVERQUEUELEN);
    if(!dec_senderamnt((char *)&(_this->data_array[index].datasize)))
    {
        *imlast=1;
        queue_txdata_consumed(&(_this->genqueue),SERVERQUEUELEN);
    }
    else
        *imlast=0;
    *datalen=_this->data_array[index].datasize&0x00FFFFFF;
    return _this->data_array[index].data;

}
int poll_serverdata_to_send(SServerSendQueue *_this, unsigned int timeout)
{
    return queue_poll(&(_this->genqueue),timeout);
}
#if 0
void signal_sender(SClientSendQueue *_this)
{
    return queue_signal(&(_this->genqueue));
}
#endif
int dec_senderamnt(char *store)
{
    short foo=1;
    char *bar=(char *)&foo;
    if(*bar)
    {
        store[3]--;
        return (int)store[3];
    }
    else
    {
        store[0]--;
        return (int)store[0];
    }
    return -1;
}
void add_senderamnt(char *store,unsigned char senderamnt)
{
    short foo=1;
    char *bar=(char *)&foo;
    if(*bar)
    {
        store[3]=senderamnt;
    }
    else
    {
        store[0]=senderamnt;
    }
}
int server_queue_add_tx_data(SServerSendQueue *_this,void *data,size_t datasize,unsigned char senderamnt)
{
    int addindex;
    if(0==datasize || !data)
    {
        printf("server_queue_add_tx_data() invalid data\n");
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
    addindex=queue_tx_add_unsafe(&(_this->genqueue),SERVERQUEUELEN);
    _this->data_array[addindex].datasize=datasize;
    add_senderamnt((char *)&(_this->data_array[addindex].datasize),senderamnt);
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
#if 0
int rx_client_queue_add_data(SClientSendQueue *_this,void *data,size_t datasize)
{
    if(0==datasize || !data)
    {
        printf("rx_client_queue_add_data() invalid data\n");
        return -1;
    }
    while(queue_rx_lock_if_space(&(_this->genqueue)))
    {
        queue_signal(&(_this->genqueue));
    }

    addindex=queue_rx_add_unsafe(&(_this->genqueue),SERVER_RX_QUEUELEN);
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
#endif

