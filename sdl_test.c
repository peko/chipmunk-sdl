//   $ clang -g -lSDL_gfx -lSDLmain -lSDL -o sdl_test sdl_test.c
//
#include <SDL/SDL.h>
#include <SDL/SDL_gfxPrimitives.h>

#define SCREEN_W  800
#define SCREEN_H  600

int main(int argc, char *argv[]) {
  SDL_Event evt; 
  int i;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    return -1;
  }

  SDL_Surface *screen = SDL_SetVideoMode(
    SCREEN_W, 
    SCREEN_H, 
    24, SDL_SWSURFACE | SDL_DOUBLEBUF);

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
    }

    SDL_LockSurface(screen);

    SDL_FillRect(screen, NULL, 0x000080); 

    for (i = 0; i < 100; ++i) {
      int x0 = rand() % SCREEN_W;
      int y0 = rand() % SCREEN_H;
      int x1 = rand() % SCREEN_W;
      int y1 = rand() % SCREEN_H;
      
      // http://www.ferzkopp.net/Software/SDL_gfx-2.0/
      lineColor(screen, x0, y0, x1, y1, 0xFFFFFFFF);
    }

    SDL_FreeSurface(screen);
    SDL_Flip(screen);
  }
  
finish:
  SDL_FreeSurface(screen);
  SDL_Quit();

  return 0;
}