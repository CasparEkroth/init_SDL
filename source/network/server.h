#include "initSDL.h"
#include <SDL_net.h>

#define PORT 1234
#define LENGTH 50
TCPsocket initNet();
void confirmation(TCPsocket server, char str[LENGTH]);
