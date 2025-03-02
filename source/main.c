#include "toolSDL.h"
int main(void){
    Game *pGame;
    SDL_Event event;
    initSDL(pGame->pWindow,pGame->pRenderere);
    pGame->programIsRunning = true;
    while (pGame->programIsRunning){
        //run game 
        input(event,pGame);
    }
    close_SDL(pGame->pWindow,pGame->pRenderere);
    return 0;
}

