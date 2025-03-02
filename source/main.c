#include "toolSDL.h"


int main(void){
    Game pGame = {0};
    SDL_Event event;
    initialize_window(&pGame);
    pGame.programIsRunning = true;

    while (pGame.programIsRunning){
        input(event,&pGame);
        update(&pGame);
        render(&pGame);
    }

    close_SDL(pGame.pWindow,pGame.pRenderere);

    return 0;
}

