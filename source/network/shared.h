/* ========== shared.h ========== */
#ifndef SHARED_H
#define SHARED_H

#include <SDL.h>
#include <SDL_net.h>

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
    MSG_DISCOVER_RESPONSe = 4
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

Client initNet();   //constructer of client struct

// simplefide send funtion
void SEND(Client aClient,MessageType msg,int id,int one,int two,int three);

#endif
