#include <stdio.h>

#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#include <chipmunk/chipmunk_private.h>
#include <chipmunk/chipmunk.h>

#include "space.h"

#define SCREEN_W  640
#define SCREEN_H  480

static void DrawImpl(cpSpace *space);

static cpSpaceDebugColor
ColorForShape(cpShape *shape, cpDataPointer data);

static inline cpSpaceDebugColor RGBAColor(float r, float g, float b, float a){
  cpSpaceDebugColor color = {r, g, b, a};
  return color;
}

static inline cpSpaceDebugColor LAColor(float l, float a){
  cpSpaceDebugColor color = {l, l, l, a};
  return color;
}

static SDL_Surface *screen = NULL;
cpSpace *space = NULL;

int main(void){
  
  space = space_init(SCREEN_W, SCREEN_H);

  SDL_Event evt; 

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    return -1;
  }

  screen = SDL_SetVideoMode(
    SCREEN_W, 
    SCREEN_H, 
    24, SDL_HWSURFACE | SDL_DOUBLEBUF);

  if (screen == NULL) {
    return -1;
  }

  while(1) {
    while(SDL_PollEvent(&evt)) {
      if(evt.type == SDL_QUIT) {
        goto finish;
      }
      if (evt.type == SDL_KEYUP && evt.key.keysym.sym == SDLK_ESCAPE) {
        goto finish;
      }

      if (evt.type == SDL_MOUSEMOTION    ) space_mouse_move(space, evt.motion.x, evt.motion.y);
      if (evt.type == SDL_MOUSEBUTTONDOWN) space_mouse_down(space);
      if (evt.type == SDL_MOUSEBUTTONUP  ) space_mouse_up  (space);
    }


    SDL_LockSurface(screen);
    SDL_FillRect(screen, NULL, 0x000080); 
    space_update(space, 0.02);

    DrawImpl(space);

    SDL_FreeSurface(screen);
    SDL_Flip(screen);
  }
  
finish:
  
  space_destroy(space);  
  SDL_FreeSurface(screen);
  SDL_Quit();

  return 0;
}

#include "sdl_draw.c"