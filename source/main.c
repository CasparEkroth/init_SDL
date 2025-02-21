#include "initSDL.h"
int main(void){
    SDL_Window* pWindow;
    SDL_Renderer*pRenderer;
    initSDL(&pWindow,&pRenderer);
    while (true){
        //run game 
    }
    close_SDL(&pWindow,&pRenderer);
    return 0;
}

