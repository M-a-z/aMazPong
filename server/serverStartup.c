/* ************************************************************************** */
/*
 * The purpose of this file is to start the server and provide listening socket
 * for pong clients to connect to server. First player connecting should send
 * game initialization data. (at least number of players).
 * 
 * Each connection handling is started in it's own thread. Basically
 * these connections will end up in infinite recv loop, listening keypress
 * messages, and ball/paddle position queries and responding to these.
 *
 * All position updates/queries will use same datacontainer, wrapped in a
 * singleton class (Yeah, I believe I'll use c++ for that part.)
 *
 *    Revision history:
 *
 *    -0.0.2  06.01.2009/Maz Moved max conn check to pong_server_accept_client.c which
 *                           will be modified to be a foreverloop just parsing
 *                           incoming network messages. This way when some client drops
 *                           connection, recv() will fail, and the connection amnt can
 *                           be reasonably decreased.
 *    -0.0.1  26.09.2008/Maz Separated this startup from actual handshake 
 *                           messaging
 *
 *
 *    PasteLeft Maz 2008.
 */
/* ************************************************************************** */

#include "serverStartup.h"
#include "pong_server_accept_client.h"
#include "pong_misc_func.h"
//#include "place_counter_thread.h"
//#include "definitions.h"
#include "network_messages.h"
#include "pong_dataStorage.h"
#include <time.h>
//#include "pong_commP_CppToC_wrapper.h"



/*
typedef struct SserverInternalGameData
{
    int game_id;        
    short client_ids[4];
    int isrunning;
    char game_name[GAME_NAME_MAX];    
}SserverInternalGameData;
*/      

static SserverInternalClientData G_clients[CLIENTS_MAX];
//static SserverInternalGameData G_games[GAMES_MAX];


//static int G_first_player_connected=0;
//static int G_num_of_players=0;
//static int G_connections_alive=0;

/*
void * datalistener(void *arg);
*/
int main()
{
    return (int)start_server();
}

void server_startup_hooks()
{
    srand(time(NULL));
    init_dataStorage();
}

EPongRet start_server()
{
    SOCKET sock, new_sock;
    struct sockaddr_in my_addr;    
    struct sockaddr_in their_addr;
    size_t sin_size;
//    struct sigaction sa;

    server_startup_hooks();

    
    godawfulwinsockinit();
    memset( (void *)G_clients,0,sizeof(G_clients));
//    memset( (void *)G_games,0,sizeof(G_games));
    if( (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP) ) < 0 )
    {
        printf("Socket creation failed! retval=%d\n",sock);
        return EPongRet_ERROR;
    }
    printf("TCP/IP Socket %d created\n",sock);
    memset(&my_addr, 0, sizeof(my_addr));  
    my_addr.sin_family      = AF_INET;           
    my_addr.sin_addr.s_addr = INADDR_ANY;
    memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero); 
    my_addr.sin_port        = htons((short)SERVER_POS_UPD_PORT); 

    if (bind(sock, (struct sockaddr *)&my_addr, sizeof my_addr) == -1)
    {
        perror("bind\n");
        exit(1);
    }

    if (listen(sock, BACKLOG) == -1)
    {
        perror("listen\n");
        exit(1);
    }
/*
    We do not use fork for the sake of windows compatibility - we use pthreads.
    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("sigaction");
        exit(1);
    }
*/
    while(1)
    {  // main accept() loop
        sin_size = sizeof their_addr;
        if(
            (new_sock = 
                accept(
                    sock, 
                    (struct sockaddr *)&their_addr, 
                    &sin_size
                )
            ) == INVALID_SOCKET
        )
        {
            perror("Error in accept\n");
            continue;
        }
        printf(
            "server: got connection from %s\n",
            inet_ntoa(their_addr.sin_addr)
        );
        new_sock_conn(new_sock,their_addr);
//        G_connections_alive--;
//        exit(0);
//        }
        //pong_sock_close(new_sock);  
        //Is this done in child thread???
    }
}
