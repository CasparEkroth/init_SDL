#include "toolSDL.h"


int main(void){
    Game g = {0};
    SDL_Event event;
    if(initialize_window(&g)){
    g.programIsRunning = true;
    }else g.programIsRunning = false;

    while (g.programIsRunning){
        input(event,&g);
        update(&g);
        render(&g);
    }

    close_SDL(g.pWindow,g.pRenderere);

    return 0;
}

