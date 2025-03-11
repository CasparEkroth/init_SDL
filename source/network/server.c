#include "server.h"

TCPsocket initNet(){
    SDLNet_Init();
    
    IPaddress ip;
    SDLNet_ResolveHost(&ip,NULL,PORT);
    TCPsocket server = SDLNet_tcp_open(&ip);
    //TCPsocket client;
    return server;
}


void confirmation(TCPsocket server, char str[LENGTH]){
    TCPsocket client;
    while (1){
        client = SDLNet_TCP_Accept(server);
        if(client){
            //comunecate white client
            SDLNet_TCP_Send(client,str,strlen(str)+1);
            SDLNet_TCP_Close(client);
            break;
        }
    }
    SDLNet_TCP_Close(server);
    SDLNet_Quit();
}