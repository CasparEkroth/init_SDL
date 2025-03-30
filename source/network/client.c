/* ========== client.c ========== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_net.h>

#include "shared.h"
#define SERVER_IP "127.0.0.1"
#define SERVER_IP_EKERO "10.0.0.8"
#define SERVER_BRODCAST "255.255.255.255"
#define SERVER_IP_EDURUM "130.229.140.39"
#define SERVER_IP_EDURUM2 "130.229.134.176"
#define SERVER_BRODCAST_LOCAL "10.0.0.255"

// om du vill köra endbart på din enhet använd
// "127.0.0.1" - kör med dig själv
// "255.255.255.255" ksk brodcast
// tanken är att bordcasta efter en server om en server finns 
// kommer den att svara och clienten kommer att gå med i LANet
// om de inte finns en server kommer client programet köra 
// kommandot system("make run_server"); och alltså börja hosta 
// från sin egna dator 


// static UDPsocket udpSocket = NULL;
// static UDPpacket *outPacket = NULL;
// static UDPpacket *inPacket = NULL;

struct client{
    int player_id[MAX_PLAYERS];
    UDPsocket udp_socket;
    UDPpacket *out_packet,*in_packet;
    IPaddress serverAddr;
};

int initSDL(SDL_Window **pWindow,SDL_Renderer **pRenderer);
Client initNet();
void closeSDLElement(SDL_Renderer *pRen,SDL_Window *pWin);
void SEND(Client aClient,MessageType msg,int id,int one,int two,int three);
bool pingServerDirectly(Client aClient);

void open_console();

int main(int argc, char *argv[]) {
    (void)argc; (void)argv;
    open_console();
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;
    if (initSDL(&window, &renderer) == 1) {
        return 1;
    }
    if(window == NULL || renderer == NULL){
        printf("Erorr seting upp the renderaer or window: %s",SDL_GetError());
        closeSDLElement(renderer,window);
        return 1;
    }
    Client theClient;
    theClient = initNet();
    if(theClient == NULL){
        closeSDLElement(renderer,window);
    }  // Resolve the server address (change IP if needed)
    /*if (SDLNet_ResolveHost(&theClient->serverAddr,SERVER_IP, PORT) < 0) {
        printf("SDLNet_ResolveHost failed: %s\n", SDLNet_GetError());
        closeSDLElement(renderer,window);
        return 1;
    }*/
    if (!pingServerDirectly(theClient)){
        if(!broadcastServer(theClient)){
            if (SDLNet_ResolveHost(&theClient->serverAddr,SERVER_IP_EKERO, PORT) < 0) {
                printf("SDLNet_ResolveHost failed: %s\n", SDLNet_GetError());
                closeSDLElement(renderer,window);
                return 1;
            }
        }
    }
    int numPlayers = 0; 
    int playerPosX[MAX_PLAYERS]={0};
    int playerPosY[MAX_PLAYERS]={0};

    bool keys[SDL_NUM_SCANCODES] ={0};
    bool running = true;
    while (running) {
        SDL_Event event;
        int dx = 0, dy = 0;
        while (SDL_PollEvent(&event)){
            switch (event.type){
            case SDL_QUIT:
                running = false;
                break;
            case SDL_MOUSEBUTTONDOWN:
                keys[event.button.button] = SDL_PRESSED;
                break;
            case SDL_MOUSEBUTTONUP:
                keys[event.button.button] = SDL_RELEASED;
                break;
            case SDL_KEYDOWN:
                keys[event.key.keysym.scancode]  = true;
                break;
            case SDL_KEYUP:
                keys[event.key.keysym.scancode]  = false;
                break;
            default:
                break;
            }
        }
        if(keys[SDL_SCANCODE_UP]) dy = -5;
        if(keys[SDL_SCANCODE_DOWN]) dy = 5;
        if(keys[SDL_SCANCODE_LEFT]) dx = -5;
        if(keys[SDL_SCANCODE_RIGHT]) dx = 5;
        if (dx != 0 || dy != 0) { // If there was a movement, send it
            SEND(theClient,MSG_MOVE,0,dx,dy,100);
        }
        // Receive updated positions from server
        while (SDLNet_UDP_Recv( theClient->udp_socket, theClient->in_packet)) {
            // We’re sending an int for connectedPlayers, then x,y for each
            // So total length = sizeof(int) + 2*sizeof(int)*connectedPlayers
            // We can parse it accordingly
            char *buf = (char *) theClient->in_packet->data;
            int offset = 0;
            // Check we at least have an int
            if ( theClient->in_packet->len >= (int)sizeof(int)) {
                memcpy(&numPlayers, buf + offset, sizeof(int));
                offset += sizeof(int);

                // For each player
                for (int i = 0; i < numPlayers; i++) {
                    if (offset + 2 * (int)sizeof(int) <=  theClient->in_packet->len) {
                        memcpy(&playerPosX[i], buf + offset, sizeof(int));
                        offset += sizeof(int);
                        memcpy(&playerPosY[i], buf + offset, sizeof(int));
                        offset += sizeof(int);
                    }
                }
            }
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        // Draw each player
        for (int i = 0; i < numPlayers; i++) {
            // Just alternate color for fun
            if (i % 2 == 0) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            }
            SDL_Rect rect = { playerPosX[i], playerPosY[i], RECT_SIZE, RECT_SIZE };
            SDL_RenderFillRect(renderer, &rect);
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }
    // If we’re disconnecting, let server know
    SEND(theClient,MSG_DISCONNECT,0,0,0,0);
    // Cleanup
    SDLNet_FreePacket(theClient->out_packet);
    SDLNet_FreePacket(theClient->in_packet);
    SDLNet_UDP_Close(theClient->udp_socket);
    free(theClient);
    closeSDLElement(renderer,window);
    return 0;
}

bool broadcastServer(Client aClient){
    IPaddress broadcastAddr;
    if (SDLNet_ResolveHost(&broadcastAddr,SERVER_BRODCAST_LOCAL,PORT) < 0) {
        printf("SDLNet_ResolveHost failed: %s\n", SDLNet_GetError());
        return false;
    }
    PacketData discoverPkg;
    discoverPkg.messageType = MSG_DISCOVER;
    discoverPkg.playerID = 0;

    memcpy(aClient->out_packet->data, &discoverPkg, sizeof(discoverPkg));
    aClient->out_packet->len = sizeof(PacketData);
    aClient->out_packet->address = broadcastAddr;
    SDLNet_UDP_Send(aClient->udp_socket,-1,aClient->out_packet);
    // wait for respons
    Uint32 startTime = SDL_GetTicks();
    bool serverFound = false;
    while (SDL_GetTicks() - startTime < 2000 ){//2 sec
        if(SDLNet_UDP_Recv(aClient->udp_socket,aClient->in_packet)){
            PacketData response;
            memcpy(&response, aClient->in_packet->data, sizeof(PacketData));
            if(response.messageType == MSG_DISCOVER_RESPONSE){
                serverFound = true;
                aClient->serverAddr = aClient->in_packet->address;  // Uppdatera serverns adress
                printf("client resevde MSG_DISCOVER_RESPONSE\n");
                break;
            }
        }
        SDL_Delay(10);
    }
    if(!serverFound){
        #ifdef _WIN32
            //for OS to get acses to termial
            #include <direct.h>
            system("start cmd /k \"cd /d %CD% && make run_server\"");
        #elif __APPLE__
            // for OS to get acses to termial
            #include <unistd.h>
            #include <stdio.h>
            #include <stdlib.h>
            #include <limits.h>
            #include "TargetConditionals.h"
            #if TARGET_OS_MAC
                char cwd[PATH_MAX];
                if(getcwd(cwd, sizeof(cwd)) != NULL){ // Hämta aktuell katalog
                    char command[2048];
                    snprintf(command, sizeof(command),
                        "osascript -e 'tell application \"Terminal\" to do script \"cd %s && make run_server\"'",
                        cwd);
                    system(command);
                } else {
                    perror("getcwd() error");
                }
            #endif
        #endif
        printf("NO Server fund\n");
        return false;
    }
    return true;
}

bool pingServerDirectly(Client aClient){
    // Try direct connection using SERVER_IP.
    if (SDLNet_ResolveHost(&aClient->serverAddr, SERVER_IP, PORT) < 0) {
        printf("SDLNet_ResolveHost failed: %s\n", SDLNet_GetError());
        return false;
    }
    PacketData discoverPkg = { .messageType = MSG_DISCOVER, .playerID = 0 };
    memcpy(aClient->out_packet->data, &discoverPkg, sizeof(discoverPkg));
    aClient->out_packet->len = sizeof(PacketData);
    aClient->out_packet->address = aClient->serverAddr;
    SDLNet_UDP_Send(aClient->udp_socket, -1, aClient->out_packet);
    
    Uint32 startTime = SDL_GetTicks();
    while (SDL_GetTicks() - startTime < 2000) { // 2 second timeout
        if (SDLNet_UDP_Recv(aClient->udp_socket, aClient->in_packet)) {
            PacketData response;
            memcpy(&response, aClient->in_packet->data, sizeof(PacketData));
            if (response.messageType == MSG_DISCOVER_RESPONSE) {
                aClient->serverAddr = aClient->in_packet->address;
                printf("Direct ping: received MSG_DISCOVER_RESPONSE\n");
                return true;
            }
        }
        SDL_Delay(10);
    }
    return false;
}


void SEND(Client aClient,MessageType msg,int id,int one,int two,int three){
        PacketData pkg;
        pkg.messageType = msg;
        pkg.playerID = id;
        pkg.dx = one;
        pkg.dy = two;
        pkg.health = three;
    memcpy(aClient->out_packet->data, &pkg, sizeof(PacketData));
    aClient->out_packet->len = sizeof(PacketData);
    aClient->out_packet->address =  aClient->serverAddr;
    SDLNet_UDP_Send(aClient->udp_socket, -1, aClient->out_packet);
}

Client initNet(){
    Client aClient = malloc(sizeof(struct client));
    // Open a local UDP port (0 picks any available)
    aClient->udp_socket = SDLNet_UDP_Open(0);
    if (!aClient->udp_socket) {
        printf("SDLNet_UDP_Open failed: %s\n", SDLNet_GetError());
        SDLNet_UDP_Close(aClient->udp_socket);
        return NULL;
    }
       // Allocate packets
    aClient->in_packet = SDLNet_AllocPacket(512);
    aClient->out_packet = SDLNet_AllocPacket(512);
    if (! aClient->out_packet || !aClient->in_packet) {
        printf("Failed to allocate packets.\n");
        if (aClient->out_packet) SDLNet_FreePacket(aClient->out_packet);
        if (aClient->in_packet) SDLNet_FreePacket(aClient->in_packet);
        SDLNet_UDP_Close(aClient->udp_socket);
        return NULL;
    }
    return aClient;
}

int initSDL(SDL_Window **pWindow, SDL_Renderer **pRenderer) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }
    if (SDLNet_Init() < 0) {
        printf("SDLNet_Init failed: %s\n", SDLNet_GetError());
        SDL_Quit();
        return 1;
    }
    *pWindow = SDL_CreateWindow("UDP Client",
                                SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED,
                                SCREEN_WIDTH, SCREEN_HEIGHT,
                                SDL_WINDOW_SHOWN);
    if (!(*pWindow)) {
        printf("Failed to create window: %s\n", SDL_GetError());
        SDLNet_Quit();
        SDL_Quit();
        return 1;
    }
    *pRenderer = SDL_CreateRenderer(*pWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!(*pRenderer)) {
        printf("Failed to create renderer: %s\n", SDL_GetError());
        SDL_DestroyWindow(*pWindow);
        SDLNet_Quit();
        SDL_Quit();
        return 1;
    }
    return 0;
}

void closeSDLElement(SDL_Renderer *pRen,SDL_Window *pWin){
    SDL_DestroyRenderer(pRen);
    SDL_DestroyWindow(pWin);
    SDLNet_Quit();
    SDL_Quit();
}

void open_console(){
    AllocConsole();
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
}