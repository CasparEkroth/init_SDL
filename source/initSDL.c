#include "initSDL.h"

void initSDL(SDL_Window *pWindow,SDL_Renderer* pRenderer){
    pWindow = initialize_window();
    pRenderer = initialize_renderer(pWindow);
}

SDL_Window* initialize_window(void){ // Initialiserar SDL och skapar fönster
    SDL_Window* pWindow;
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0 || TTF_Init() != 0 || SDL_Init(SDL_INIT_AUDIO) < 0){
        fprintf(stderr, "Error initializing SDL. %s\n", SDL_GetError());
        return false;
    }
    if (IMG_Init(IMG_INIT_PNG) == 0) {
        fprintf(stderr, "Error initializing SDL_image: %s\n", IMG_GetError());
        return false;
    }
    if(Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0){
        fprintf(stderr,"SDL_mixer could not initialize! Mix_Error: %s\n",Mix_GetError());
        return false;
    }
    pWindow = SDL_CreateWindow(
        NULL, // Titel
        SDL_WINDOWPOS_CENTERED, // x
        SDL_WINDOWPOS_CENTERED, // y 
        STARTING_WINDOW_WIDTH, 
        STARTING_WINDOW_HEIGHT, 
        SDL_WINDOW_RESIZABLE  // Flags
    );
    if (!pWindow) {
        fprintf(stderr, "Error creating SDL Window: %s\n", SDL_GetError());
        return NULL;
    }
    return pWindow;
}

SDL_Renderer* initialize_renderer(SDL_Window* pWindow){
    SDL_Renderer* pRenderer; 
    Uint32 render_flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
    pRenderer = SDL_CreateRenderer(pWindow, -1, render_flags);
    if (!pRenderer) {
        fprintf(stderr, "Error creating SDL Renderer: %s\n", SDL_GetError());
        return NULL;
    }
    return pRenderer;
}

void close_SDL(SDL_Window* pWindow,SDL_Renderer *pRenderer){
    if (pRenderer) SDL_DestroyRenderer(pRenderer);
    if (pWindow) SDL_DestroyWindow(pWindow);
    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}