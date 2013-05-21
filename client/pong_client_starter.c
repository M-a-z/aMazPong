/* ************************************************************************** */
/*                                                                            *
 *      The purpose of this file is to (temporary) startup scenario for       *
 *      client. This simple version works just by first starting the server   *
 *      process, and then the client and specifying the game name, whether    *
 *      to 'init' or 'join' in game, and server's IP address. Later we need   *
 *      nice startup menu after client is started, and manage the server      *
 *      process launching from client if server is needed.                    *
 *                                                                            *
 *                                                                            *
 *                                                                            *
 *      Revision history:                                                     *
 *                                                                            *
 *      -0.0.2  18.09.2008/Maz  Ironed out some compile errors.    
 *      -0.0.l  14.06.2008/Maz  First draft                                   *
 *                                                                            *
 *      PasteLeft 2008 Maz                                                    *
 *                                                                            */
/******************************************************************************/

#include "pong_misc_func.h"
#include "general.h"
#include "pong_netw.h"
#include "pong_client_netwIF.h" 
#include "pong_drawer.h"

#define OPTSTRING "vhp:g:s:w:"
#define HELPTEXT "USAGE:\n" \
    "./pongClient [ -v -h -p playeramount -g gamename -s serverip -w passWord]\n" \
    "-v \t\tDisplays version and exits\n" \
    "-h \t\tPrints this help and exits\n" \
    "-p num\tNumber of players if you're hosting game\n" \
    "-g text\tName of the game ro host/join\n" \
    "-s ip\tIP address of server hosting the game\n" \
    "-w text\tPassword for joining the game\n" 


static char G_serverip[16];
static struct in_addr G_serverip_bin;
static char G_game_name[16];
static char G_passwd[15];

SClientSendQueue Gsendqueue;

void init_globals();
void pong_temp_cmdline_parse( int argc, char *argv[]);
int G_argc;
char **G_argv;

void init_globals()
{
    memset(&(G_serverip[0]),0,sizeof(G_serverip));
    strcpy(G_serverip,"127.0.0.1");
    memset(&(G_game_name[0]),0,sizeof(G_game_name));
    strcpy(G_game_name,"local_game_1");
    memset(&(G_passwd[0]),0,sizeof(G_passwd));
}


int main( int argc, char *argv[])
{
    SOCKET sock;
    int players=0;
    int game_id;
    int gname_given=0;
    char c;
    int errors=0;
    pthread_t draweId;
    int initiator=0;
    int ownId;
    char myname[PLAYER_NAME_MAX];
    void *ingamearg[2];
    pthread_t tid;
    //pthread_t guiId;
    G_argc=argc;
    G_argv=argv;
    godawfulwinsockinit();
    init_globals();
    Snetw_game_start_msg gameInfo;
    if(client_queue_init(&Gsendqueue))
    {
        printf("FAILED to initialize client_send_queue!\n");
        return -1;
    }
    //parse cmdline args and set globals
//    pong_temp_cmdline_parse( argc, argv);
    sock = connect_to_server(G_serverip, SERVER_TCP_PORT);
    ingamearg[0]=&sock;
    ingamearg[1]=&Gsendqueue;

/*    options.. */
    
    while(-1 != (c = getopt(argc, argv, OPTSTRING)))
    {
        switch(c)
        {
            case 's':
            {
                unsigned int len;
#ifndef __LINUX__
                unsigned long tmp;
#endif
                if(!optarg)
                    return -1;
                len=strlen(optarg);
#ifdef __LINUX__
                if(len>=16 || 0==len || !inet_pton(AF_INET,optarg,&G_serverip_bin))
#else
                if(len>=16 || 0==len )
                    goto invalidIP;
                tmp=inet_addr(optarg);
                if(tmp==INADDR_NONE || tmp == INADDR_ANY)
                    goto invalidIP;
                G_serverip_bin=*(struct in_addr *)&tmp;
              if(0)
#endif
                {
invalidIP:
                    printf("Invalid server IP %s\n",optarg);
                    return -1;
                }
                strcpy(G_serverip,optarg);
                break;
            }    
            case 'g':
                if(optarg)
                {
                    unsigned int len;

                    len=strlen(optarg);
                    if(len>=16 || 0==len)
                    {
                        printf("Invalid game name '%s' specified! (should be 1-16 chars)\n",optarg);
                        return -1;
                    }
                    strcpy(G_game_name,optarg);
                    printf("Game name set to %s\n",G_game_name);
                    gname_given++;
                }
                break;
            case 'h':
                printf("%s\n",HELPTEXT);
                return 0;
                break;
            case 'v':
                printf("MazPongV4.%s\n",G_CLIENTVER);
                return 0;
                break;
            case 'p':
                initiator=atoi(optarg);
                if(1>initiator || 4<initiator)
                {
                    printf("You must init game with 1...4 Players\n");
                    return -1;
                }
                break;
        }
    }

   /*sock =*/  connect_to_server(G_serverip, SERVER_POS_UPD_PORT);

    if(EPongRet_ERROR==netw_client_hello_send(G_game_name,&players,&game_id,initiator,&ownId))
    {
        printf("Error in handshake with server, aborting!\n");
        return 1;
    }
    if(!initiator)
        DEBUGPRINT("Server gave: %u players, %u gameId\n",players,game_id);
    else
    {
        DEBUGPRINT("Server gave:  %u gameId\n",game_id);
        DEBUGPRINT("Going sending INIT data...\n");
    }
    if(initiator)
    {
        //send game init req
         if(EPongRet_ERROR == pong_initiate_game(game_id, initiator, G_passwd))   
         {
             printf("Game Init Req failed!\n");
             return 1;                  
         }
		 else
			 printf("Game init req successfull, TODO: continue handshake and start game\n");
    }
	else
		printf("game is already inited...\n");
    printf("Sending game JOIN req\n");
    snprintf(myname,PLAYER_NAME_MAX-1,"player_%u",ownId);
    myname[PLAYER_NAME_MAX-1]='\0';
    printf("Client name now '%s'",myname);
    if( EPongRet_ERROR == pong_join_inited_game(game_id,myname,G_passwd))
    {
        printf("Joining game FAILED\n");
        return 1;
    }
    if( EPongRet_ERROR == pong_wait_game_start(&gameInfo))
    {
        printf("Game start failed\n");
        return -1;
    }

    if(init_and_launch_maindrawer(&(gameInfo.client_game_data),&draweId))
    {
        printf("Failed to launch drawer process!\n");
        exit(-1);
    }
    if
    (
        pthread_create
        (
            &tid,
            NULL,
            ingameloop,
            ingamearg
        )
    )
    {
        printf("Failed to create RX thread\n");
        exit(1);
    }
    while(1)
    {
        int rval;
        if((rval=poll_clientaction_to_send(&Gsendqueue,POLLTIMEQUANTA)))
        {
            if(rval==-1)
            {
                printf("Error in signaling!\n");
                errors++;
                if(errors>50)
                    goto end;
            }
            else 
            {
                rval=send_clientaction(&Gsendqueue);
                if(rval)
                {
                    printf("Client exiting (%d)!\n",rval);
                    goto end;
                }
            }
        }
        if(peek_server_for_gameupdate(&Gsendqueue,&(gameInfo.client_game_data)))
        {
            printf("Client exiting due to server!\n");
            goto end;
        }
    }
end:
    return 0;
}

#if 0
void pong_temp_cmdline_parse( int argc, char *argv[])
{
     int i;
     for(i=1;i<argc;i++)
     {
         if(!strncmp( argv[i],"-host_game=",12))
         {
             printf(
                 "game hosting set to %d",
                 (
                     G_host_game=(argv[i][13]=='1') ? 1:0
                 )
             );
             continue;
         }
         if(!strncmp( argv[i],"-amnt_of_players=",17))
         {
             switch((int)argv[i][18])
             {
                 case '2':
                     G_amnt_of_players=2;
                     break;
                 case '3':
                     G_amnt_of_players=3;
                     break;
                 case '4':
                     G_amnt_of_players=4;
                     break;
                 default:
                     pong_assert(0,"Invalid player count!");
                     break;                        
             }
                                     
             printf(
                 "amnt_of_players set to %d",
                 G_amnt_of_players
             );
             
             strncpy(G_game_name,"default",15);
             continue;
         }
         if(!strncmp( argv[i],"-server_ip=",11))
         {
             printf( "server ip set to %s",&(argv[i][12]));
             memset(&(G_serverip[0]),0,sizeof(G_serverip));
             strncpy( &(G_serverip[0]), &(argv[i][12]), 15);
             continue;
         }
         if(!strncmp( argv[i],"-game_name=",11))
         {
             printf( "game name set to %s",&(argv[i][12]));
             memset(&(G_game_name[0]),0,sizeof(G_game_name));
             strncpy( &(G_game_name[0]), &(argv[i][12]), 15);
             continue;
         }                      
         if(!strncmp( argv[i],"-passwd=",8))
         {
             printf("game passwd set to %s",&(argv[i][9]));
                 
             memset(&(G_passwd[0]),0,sizeof(G_passwd));
             strncpy( &(G_passwd[0]), &(argv[i][9]), 15);
             continue;
         }
                         
     }
}
#endif
