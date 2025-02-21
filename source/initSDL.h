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

SDL_Window* initialize_window(void);
SDL_Renderer* initialize_renderer(SDL_Window* pWindow);
void initSDL(SDL_Window *pWindow,SDL_Renderer* pRenderer);


void close_SDL(SDL_Window* pWindow,SDL_Renderer *pRenderer);


#endif