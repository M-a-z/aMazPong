/* ************************************************************************** */
/*                                                                            *
 *      This file offers functions for creating messages to be delivered      *
 *      other processe(s).                                                    *
 *                                                                            *
 *                                                                            *
 *      Revision History:                                                     *
 *																			  *
 *      -0.0.5  06.01.2009/Maz  Added msgId getting and added game_id in      *
 *								msg_create + fixed bugs.                      *
 *      -0.0.4  26.09.2008/Maz  A few bugs removed from msging funcs.         *
 *      -0.0.3  18.09.2008/Maz  Ironed out some compile errors.               *
 *      -0.0.2  29.06.2008/Maz Added a few functions                          *
 *      -0.0.1  23.06.2008/Maz First Draft                                    *
 *                                                                            *
 *      PasteLeft Maz @2008                                                   *
 *                                                                            */
/* ************************************************************************** */

#include "pong_netw.h"
#include "network_messages.h"
//#include "definitions.h"
#include "pong_misc_func.h"
#define DBGPRINT
#ifdef DBGPRINT
    #define PONG_DATAPRINT(dataptr,datasize,explanation) dbg_dataprint((dataptr),(datasize),(explanation),__FILE__,__LINE__)
#else
	#define PONG_DATAPRINT(dataptr,datasize,explanation) ;
#endif


void dbg_dataprint(void *data,size_t size,char *explanation,char *file, int line)
{
	int i;
	printf("%s\n",explanation);
	printf("At %s:%d\n",file,line);
	for(i=1;i<=size;i++)
	{
		printf("%02x",*(unsigned char *)data++);
		if(!(i%2))
			printf(" ");
		if(!(i%4))
			printf(" ");
		if(!(i%16))
			printf("\n");
	}
	printf("\n");
	fflush(stdout);
}
//TODO: Modify header to contain both game and player Id as unsigned ints.
void * msg_create(unsigned int game_id, unsigned int receiver_id, unsigned int own_id, int MSG_ID, size_t msg_size)
{
//     char own_ip[16];
     char *msg;
     msg=malloc(msg_size+sizeof(Snetw_internal_msg_header));
     printf("Msg allocated, start at %p\n",msg);
     pong_assert(NULL==msg,"null ptr given for filling in fill_msg_header!");
     Snetw_internal_msg_header *pong_header = (Snetw_internal_msg_header *)msg;
     //printf("msg internal header starts at %p\n",pong_header);
	 pong_header->game_id=game_id;
     pong_header->msg_id=MSG_ID;
     pong_header->receiver_id=receiver_id;
     pong_header->sender_id=own_id;
     //obtain ip
     pong_header->msgSize=msg_size;
	 //PONG_DATAPRINT(msg,msg_size+sizeof(Snetw_internal_msg_header),"Message Created, printing msg");
     //printf("returning msg payload at %p\n",(void *)(msg+sizeof(Snetw_internal_msg_header)));
     return (void *)(msg+sizeof(Snetw_internal_msg_header));
}
unsigned int msg_getSender(void *replymsg)
{
    Snetw_internal_msg_header *hdr;
    hdr=(Snetw_internal_msg_header *)(((char *)replymsg)-sizeof(Snetw_internal_msg_header));
	PONG_DATAPRINT(hdr,sizeof(Snetw_internal_msg_header),"Msg Sender got, printing header");
    return hdr->sender_id;
}
unsigned int msg_getReceiver(void *replymsg)
{
    Snetw_internal_msg_header *hdr;
    hdr=(Snetw_internal_msg_header *)(((char *)replymsg)-sizeof(Snetw_internal_msg_header));
	PONG_DATAPRINT(hdr,sizeof(Snetw_internal_msg_header),"Msg Receiver got, printing header");
    return hdr->receiver_id;
}
void msg_free(void **msg)
{
    pong_assert(NULL==msg||NULL==*msg,"NULL ptr given to msg_free");
    free(((char *)*msg)-sizeof(Snetw_internal_msg_header));
    *msg=NULL;     
}

EPongRet msg_send(SOCKET sock, void *msg, size_t msgsize)
{
       int retval;
       pong_assert(!msg,"Null pointer given to send");
	   //PONG_DATAPRINT((char *)msg-sizeof(Snetw_internal_msg_header),msgsize+sizeof(Snetw_internal_msg_header),"Sending MSG:");
       printf("msg being sent, handle (payload start) is %p\n",msg);
       printf("msg being sent, calculated msg head at %p\n",((char *)msg)-sizeof(Snetw_internal_msg_header));
       retval=send( sock, ((char *)msg)-sizeof(Snetw_internal_msg_header), msgsize+sizeof(Snetw_internal_msg_header),0);
       return (retval!=msgsize+sizeof(Snetw_internal_msg_header))?EPongRet_ERROR:EPongRet_SUCCESS;
}
       
//TODO: add timeout arg
//timeout is set in millisecs
EpongPollRetVal dataIsComing(SOCKET sock, unsigned long int tmo)
{
	fd_set fdset_ready;
	int retval=-1;
	struct timeval tim;
	FD_ZERO(&fdset_ready);
	FD_SET(sock,&fdset_ready);


	memset(&tim,0,sizeof(tim));
	if(tmo!=0)
	{
        tim.tv_sec=SECS_IN_MILLISEC(tmo);
//      ((tmo-((unsigned long int) tim.tv_sec)*1000)*1000)        
        tim.tv_usec=tmo-tim.tv_sec*1000;
        tim.tv_usec*=tim.tv_usec;
    }
	retval=select(sock+1,&fdset_ready,NULL,NULL,&tim);
	if(retval==-1)
	{		
		printf("select failed on socket!\n");
		PRINT_S_ERR();
		return EpongPollRetVal_Error;
	}
	if(FD_ISSET(sock,&fdset_ready))
	{
		return EpongPollRetVal_NewData;
	}
	return EpongPollRetVal_NothingNew;
}
//tmo in msecs
void *msg_receive(SOCKET sock,int tmo,EPongRecvRet *status)
{
    void *buff;
    void *payload;
    size_t payload_size;
    EPongRecvRet *foo;
    EPongRecvRet bar;
    if(status)
        foo=status;
    else
        foo=&bar;
    buff=malloc(sizeof(Snetw_internal_msg_header));
    if(buff==NULL)
    {
        printf("malloc failed in %s:%d\n",__FILE__,__LINE__);
        return NULL;
    }
    if(EPongRecvRet_SUCCESS != (*foo=dataNonBlockReceive(sock,buff,sizeof(Snetw_internal_msg_header),tmo)))
    {
        if(*foo!=EPongRecvRet_EAGAIN)
            printf("Receiving MsgHeader failed, returning NULL!\n");
        return NULL;
    }
	PONG_DATAPRINT(buff,sizeof(Snetw_internal_msg_header),"Received header:");
    payload_size=((Snetw_internal_msg_header *)buff)->msgSize;
    printf("Msg Header received, payload size 0x%x\n",payload_size);
    if( payload_size>0)
    {
        buff=realloc( buff, sizeof(Snetw_internal_msg_header)+payload_size);
        payload=(((char *)buff)+sizeof(Snetw_internal_msg_header));
        printf("buff=%p, payload=%p, inthdrsize=0x%x\n",buff,payload,sizeof(Snetw_internal_msg_header));
        if(EPongRecvRet_SUCCESS != (*foo=dataNonBlockReceive(sock,payload,payload_size,0)))
        {
            printf("No payload received although such was accepted (pl size 0x%x)\n",payload_size);
            return NULL;
        }            
    }
	 PONG_DATAPRINT(payload,payload_size,"Received payload");
    return payload;
}
int msg_getSize(void *buff)
{
    Snetw_internal_msg_header *tmp = (Snetw_internal_msg_header *) (((char *)buff)-sizeof(Snetw_internal_msg_header));
	PONG_DATAPRINT(tmp,sizeof(Snetw_internal_msg_header),"Msg size got, printing header");
    printf("in getsize, ptr=%p, payl=%p\n",tmp,buff);
    printf("size=%d\n",tmp->msgSize);
    return tmp->msgSize;
}
unsigned int msg_getId(void *buff)
{
    Snetw_internal_msg_header *tmp = (Snetw_internal_msg_header *) (((char *)buff)-sizeof(Snetw_internal_msg_header));
	PONG_DATAPRINT(tmp,sizeof(Snetw_internal_msg_header),"Msg Id got, printing header");
    return tmp->msg_id;
}
int msg_getGameId(void *buff)
{
    Snetw_internal_msg_header *tmp = (Snetw_internal_msg_header *) (((char *)buff)-sizeof(Snetw_internal_msg_header));
	PONG_DATAPRINT(tmp,sizeof(Snetw_internal_msg_header),"Game Id from Msg got, printing header");
    return tmp->game_id;
}


//TODO: add timeout arg
EPongRecvRet dataNonBlockReceive(SOCKET sock, void *buff,unsigned int size, int tmo)
{
	int retval=0, amnt=0;
	int attemptcounter=0;

	EpongPollRetVal sockstat;
		pong_assert(0==size,"0 bytes requested to be recv from socket!");

	buff=memset(buff,0,(size_t)size);
	sockstat=dataIsComing(sock,tmo);
	if(sockstat==EpongPollRetVal_Error)
	{
		return EPongRecvRet_ERROR;
	}
	if(EpongPollRetVal_NewData==sockstat)
	{
		for(attemptcounter=0;amnt<size && attemptcounter<100;attemptcounter++)
		{
			retval=recv(sock,(void *)((long)buff+(long)amnt),(size_t)(size-amnt),0);
			amnt+=retval;
	  
			if((int)size!=(int)retval&&errno!=EAGAIN && errno != EWOULDBLOCK)
			{
				if(0>retval)
				{
					amnt+=retval;
					continue;
				}
				if(retval==0)
				{
					printf("recv returned 0 => target has closed connection\n");
					buff=memset(buff,0,(size_t)size);
					return EPongRecvRet_ERROR;
				}
			}
		}
		if(amnt<(int)size)
		{
            
			pong_delay(25);
            printf("Not enough data recv from socket, aborting\n");
			buff=memset(buff,0,(size_t)size);
			return EPongRecvRet_ERROR;
		}
		else if(amnt==size)
		{
			return EPongRecvRet_SUCCESS;
		}
		else
		{
			pong_assert(amnt>size,"Something is really bad wrong as kari says. Too much data obtained from socket!!!");
		}
	}
	return EPongRecvRet_EAGAIN;
}
