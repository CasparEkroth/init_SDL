/* ========== server.c ========== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL.h>
#include <SDL_net.h>

#include "shared.h"

// Each player data: position on screen
typedef struct {
    int x;
    int y;
} PlayerState;

struct server{
    PlayerState players[MAX_PLAYERS];
    IPaddress clients[MAX_PLAYERS];
    int connectedPlayers;
    UDPsocket serverSocket;
    UDPpacket *recvPacket;
    UDPpacket *sendPacket;
};

//static PlayerState players[MAX_PLAYERS];
//static IPaddress clients[MAX_PLAYERS];
//static int connectedPlayers = 0;

void broadcastPlayers(UDPsocket sock, UDPpacket *packet,Server aServer);
void removeClient(int index, IPaddress addr, Server aServer);
int findClient(IPaddress addr, Server aServer);


bool initForServer();
void destroyServer(Server aServer);


Server serverConstructer();

int main(int argc, char *argv[]){
    (void)argc; (void)argv; // Silence unused warnings if you like
    if(!initForServer()){
        return 1;
    }
    Server aServer;
    aServer = serverConstructer();
    if(aServer == NULL){
        destroyServer(aServer);
        SDLNet_Quit();
        SDL_Quit();
        return 1;
    }else printf("UDP server started on port %d\n", PORT);

    bool running = true;
    while (running) {
        // Check for packets
        while (SDLNet_UDP_Recv(aServer->serverSocket, aServer->recvPacket)){
                printf("Received packet: len=%d, from host=%u, port=%u\n",
                        aServer->recvPacket->len,
                        aServer->recvPacket->address.host,
                        aServer->recvPacket->address.port);
            // We expect a PacketData or a smaller/larger message
            if (aServer->recvPacket->len == sizeof(PacketData)){
                PacketData pkg;
                memcpy(&pkg, aServer->recvPacket->data, sizeof(PacketData));
                IPaddress sender = aServer->recvPacket->address;
                // Find or add the sender
                if (pkg.messageType == MSG_DISCOVER) {
                    // Skicka tillbaka ett svar på DISCOVER-meddelandet
                    PacketData response;
                    response.messageType = MSG_DISCOVER_RESPONSE;
                    response.playerID = 0;  // Du kan använda ett speciellt värde eller t.ex. server-ID
                    response.dx = 0;
                    response.dy = 0;
                    response.health = 0;
                    memcpy(aServer->sendPacket->data, &response, sizeof(PacketData));
                    aServer->sendPacket->len = sizeof(PacketData);
                    aServer->sendPacket->address = sender;
                    aServer->sendPacket->address = aServer->recvPacket->address;  // svara till avsändaren
                    SDLNet_UDP_Send(aServer->serverSocket, -1, aServer->sendPacket);
                    printf("DISCOVER-meddelande mottaget, svar skickat.\n");
                    continue;
                }
                int playerIndex = findClient(sender,aServer);
                // If unknown, add if we have capacity
                if (playerIndex == -1 && aServer->connectedPlayers < MAX_PLAYERS) {
                    playerIndex = aServer->connectedPlayers;
                    aServer->clients[playerIndex] = sender;
                    aServer->connectedPlayers++;
                    printf("New client added as player %d\n", playerIndex);
                }

                if (playerIndex >= 0) {
                    // Process the incoming message
                    if (pkg.messageType == MSG_MOVE) {
                        // Adjust that player's position
                        aServer->players[playerIndex].x += pkg.dx;
                        aServer->players[playerIndex].y += pkg.dy;
                        // Just clamp to screen boundaries
                        if (aServer->players[playerIndex].x < 0) aServer->players[playerIndex].x = 0;
                        if (aServer->players[playerIndex].x > SCREEN_WIDTH) aServer->players[playerIndex].x = SCREEN_WIDTH;
                        if (aServer->players[playerIndex].y < 0) aServer->players[playerIndex].y = 0;
                        if (aServer->players[playerIndex].y > SCREEN_HEIGHT) aServer->players[playerIndex].y = SCREEN_HEIGHT;
                    }
                    else if (pkg.messageType == MSG_DISCONNECT) {
                        // This client is disconnecting
                        printf("Player %d disconnected.\n", playerIndex);
                        removeClient(playerIndex,aServer->clients[playerIndex],aServer);
                        if (aServer->connectedPlayers == 0) {
                            printf("All clients disconnected. Shutting down server.\n");
                            running = false;
                            break;
                        }
                    } 
                }
            }
        }
        // Broadcast updated positions to all clients
        if (aServer->connectedPlayers > 0) {
            broadcastPlayers(aServer->serverSocket, aServer->sendPacket,aServer);
        }
        // Minimal delay
        SDL_Delay(16);
        if (!running && aServer->connectedPlayers == 0) {
            // break out of outer loop
            break;
        }
    }
    destroyServer(aServer);
    SDLNet_Quit();
    SDL_Quit();
    return 0;
}

bool initForServer(){
    if (SDL_Init(0) < 0) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        return false;
    }
    if (SDLNet_Init() < 0) {
        printf("SDLNet_Init failed: %s\n", SDLNet_GetError());
        SDL_Quit();
        return false;
    }
    return true;
}

void destroyServer(Server aServer){
    if(aServer->sendPacket) SDLNet_FreePacket(aServer->recvPacket);
    if(aServer->recvPacket) SDLNet_FreePacket(aServer->sendPacket);
    if(aServer->serverSocket) SDLNet_UDP_Close(aServer->serverSocket);
    free(aServer);
}

// Helper: find which player ID corresponds to an IP/port
// Return -1 if not found
int findClient(IPaddress addr, Server aServer){
    for (int i = 0; i < aServer->connectedPlayers; i++){
        if (aServer->clients[i].host == addr.host && 
            aServer->clients[i].port == addr.port) {
            return i;
        }
    }
    return -1;
}

// Remove client by index
void removeClient(int index, IPaddress addr, Server aServer){
    printf("Removing client at index %d\nIP: %d\n", index,(int)addr.host);
    // Shift everything down from the end
    for (int i = index; i < aServer->connectedPlayers - 1; i++) {
        aServer->clients[i] = aServer->clients[i + 1];
        aServer->players[i] = aServer->players[i + 1]; // Also shift position
    }
    aServer->connectedPlayers--;
}

// Broadcast all players' positions to each client
void broadcastPlayers(UDPsocket sock, UDPpacket *packet,Server aServer){
    // We’ll send the entire players[] array in one go
    // or we can send multiple messages, but let's do one shot.
    // We'll just do: 2 * int for each player: x, y
    // But let's do it very simply with a second buffer or struct.

    // We'll create a temporary buffer: each player is 2 ints => 8 bytes
    // With MAX_PLAYERS players, that's 8 * MAX_PLAYERS = 32 for 4 players,
    // but let's be safe and assume 8 * 16 or just 128.
    // For clarity, let's do a small known limit.
    char buffer[512];
    int offset = 0;

    // Copy connectedPlayers (so client knows how many are valid)
    memcpy(buffer + offset, &aServer->connectedPlayers, sizeof(int));
    offset += sizeof(int);

    // Now copy positions for each player slot in the first connectedPlayers
    for (int i = 0; i < aServer->connectedPlayers; i++) {
        memcpy(buffer + offset, &aServer->players[i].x, sizeof(int));
        offset += sizeof(int);
        memcpy(buffer + offset, &aServer->players[i].y, sizeof(int));
        offset += sizeof(int);
    }

    // Put this in the outgoing packet
    memcpy(packet->data, buffer, offset);
    packet->len = offset;

    // Send to each connected client
    for (int i = 0; i < aServer->connectedPlayers; i++) {
        packet->address = aServer->clients[i];
        SDLNet_UDP_Send(sock, -1, packet);
    }
}

Server serverConstructer(){
    Server aServer = malloc(sizeof(struct server));
    if(aServer == NULL) {
        fprintf(stderr, "Misslyckades att allokera minne för servern.\n");
        return NULL;
    }
    aServer->connectedPlayers = 0;
    // Open server UDP socket
    aServer->serverSocket = SDLNet_UDP_Open(PORT);
    if(!aServer->serverSocket){
        printf("Failde to open UDP socket on port %d\n",PORT);
        return NULL;
    }
    // Packets for sending/receiving
    aServer->recvPacket = SDLNet_AllocPacket(512);
    aServer->sendPacket = SDLNet_AllocPacket(512);
    if (!aServer->sendPacket || !aServer->recvPacket) {
        printf("Failed to allocate packets\n");
        return NULL;
    }
        // Initialize player positions
    for (int i = 0; i < MAX_PLAYERS; i++) {
        aServer->players[i].x = 100 + i * 50;
        aServer->players[i].y = 100 + i * 50;
    }
    return aServer;
}
