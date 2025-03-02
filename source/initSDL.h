#ifndef INITSDL_H
#define INITSDL_H

#include <SDL.h>
#include <stdbool.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include <SDL_image.h>
#include <stdlib.h> 
#include <stdio.h>
#include <string.h>

#define STARTING_WINDOW_WIDTH 800 
#define STARTING_WINDOW_HEIGHT 600

typedef struct{
    //controls 
    //and more for the game 
    SDL_Window* pWindow;
    SDL_Renderer *pRenderere;
    bool keys[SDL_NUM_SCANCODES];
    SDL_Point mouse_pos;
    bool programIsRunning;
}Game;


int initialize_window(Game *pGame);


void input(SDL_Event event,Game* pGame);
void render(Game *pGame);
void update(Game *pGame);

void close_SDL(SDL_Window* pWindow,SDL_Renderer *pRenderer);


#endif