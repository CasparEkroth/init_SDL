/* ========== shared.h ========== */
#ifndef SHARED_H
#define SHARED_H

#include <SDL.h>
#include <SDL_net.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#ifdef _WIN32
    #include <windows.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#elif __APPLE__
    #include <unistd.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <ifaddrs.h>
    #include <net/if.h>
    #include <arpa/inet.h>
    #include <arpa/inet.h>
#endif

#define MAX_PLAYERS 4
#define PORT 1234
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define RECT_SIZE 50

// Simple message types
typedef enum {
    MSG_MOVE = 0,       // dx, dy movement
    MSG_KEEPALIVE = 1,  // not used in this demo, but an example
    MSG_DISCONNECT = 2,  // client signals it is disconnecting
    MSG_DISCOVER = 3,
    MSG_DISCOVER_RESPONSE = 4
} MessageType;

typedef struct {
    int messageType;  // One of MessageType
    int playerID;     // Which player is sending
    int dx;           // Movement X
    int dy;           // Movement Y
    int health;       // Just an example extra field
} PacketData;

struct client;
typedef struct client *Client;

struct server;
typedef struct server *Server;

int initSDL(SDL_Window **pWindow,SDL_Renderer **pRenderer);
void closeSDLElement(SDL_Renderer *pRen,SDL_Window *pWin);

//constructer of client struct
Client initNet();   
// simplefide send funtion
void SEND(Client aClient,MessageType msg,int id,int one,int two,int three);

bool broadcastServer(Client aClient);


//returns your local ip
char* get_local_ip();

//returns your local brodcast ip
char* get_local_broadcast();


//win only
#if _WIN32
    void open_console();
#endif



#endif
