/* etris-sdl.c -- a Tetris clone to demonstrate etris (the embeddable Tetris 
 * gaming engine) using SDL (http://www.libsdl.org).
 *
 * Copyright (c) 2011-2012, Jonas Romfelt <jonas at romfelt dot se>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of etris nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "etris.h"

#include "SDL.h"
#include "SDL_ttf.h"

#define DEPTH 32
#define BPP ((DEPTH)/8)

#define FONT_FILE_PATH "/usr/share/fonts/truetype/freefont/FreeSans.ttf"

#define X_OFFSET 150
#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define FIELD_BORDER 1
#define BLOCK_SIZE 17

SDL_Surface *screen = NULL;
SDL_Surface *text_surface = NULL;
TTF_Font *font = NULL;

static int color[] = {
  0xff444444,
  0xffaaaaaa,
  0xffffffff,
  0xffff4444,
  0xff44ff44,
  0xff4444ff,
  0xffffff44,
  0xffff44ff,
  0xff44ffff,
  0xff7f7f7f
};

void filled_rectangle(SDL_Surface *screen, int x, int y, int w, int h, int color)
{
  SDL_Rect rect = {x, y, w, h};

  if (SDL_MUSTLOCK(screen))
    if (SDL_LockSurface(screen) < 0)
      return;

  SDL_FillRect(screen, &rect, color);

  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
}

void draw_block(int x, int y, int c)
{
  filled_rectangle(screen, X_OFFSET + x * BLOCK_SIZE, y * BLOCK_SIZE, 
		   BLOCK_SIZE, BLOCK_SIZE, color[c]);
}

void update_score(int score, int lines, int figures)
{
  SDL_Color color = {0xff, 0xff, 0xff};
  SDL_Color bg_color = {0, 0, 0};
  SDL_Rect rect;
  char text[3][32];
  int w, h, i;

  sprintf(text[0], "Score: %d", score);
  sprintf(text[1], "Lines: %d", lines);
  sprintf(text[2], "Figures: %d", figures);

  TTF_SizeText(font, text[0], &w, &h); 
  h *= 1.2;

  filled_rectangle(screen, 0, 0, X_OFFSET, h * 3, 0);

  rect.x = 5;
  rect.y = rect.x;
  rect.h = h;
  rect.w = X_OFFSET - rect.x;
  for (i = 0; i < 3; i++) {
    if (text_surface)
      SDL_FreeSurface(text_surface);

    text_surface = TTF_RenderText_Shaded(font, text[i], color, bg_color);
    SDL_BlitSurface(text_surface, NULL, screen, &rect);
    rect.y += h;
  }
}

int main(void)
{
  SDL_Event event;
  ETRIS E;
  int rc, next_tick, pause = 0;

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) == -1) {
    printf("SDL_Init: %s\n", SDL_GetError());
    exit(1);
  }

  screen = SDL_SetVideoMode(X_OFFSET * 2 + (FIELD_WIDTH + FIELD_BORDER * 2) * BLOCK_SIZE, 
			    (FIELD_HEIGHT + FIELD_BORDER * 2) * BLOCK_SIZE, 
			    0, SDL_HWSURFACE | SDL_DOUBLEBUF);
  if (!screen) {
    printf("SDL_SetVideoMode: %s\n", SDL_GetError());
    exit(1);
  }

  if (TTF_Init() == -1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    exit(1);
  }

  font = TTF_OpenFont(FONT_FILE_PATH, 16);
  if (!font) {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    exit(1);
  }

  E = etris_create(FIELD_WIDTH, FIELD_HEIGHT, FIELD_BORDER, draw_block, update_score);
  if (!E) {
    printf("Failed to create etris instance\n");
    exit(1);
  }

  SDL_WM_SetCaption("etris + SDL = Tetris", 0);
  SDL_Flip(screen);

  next_tick = SDL_GetTicks();

  while (1) {
    if (SDL_GetTicks() > next_tick) {
      next_tick = SDL_GetTicks() + 10;
      if (!pause && etris_tick(E) == ETRIS_OK_REDRAW)
        SDL_Flip(screen);
    } 
    else if (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_KEYDOWN: 
        switch (event.key.keysym.sym) {
        case SDLK_UP: 
          rc = etris_rotate(E); 
          break;
        case SDLK_DOWN:
          rc = etris_drop(E); 
          break;
        case SDLK_RIGHT:
          rc = etris_right(E); 
          break;
        case SDLK_LEFT:
          rc = etris_left(E); 
          break;
        case SDLK_SPACE:
          rc = etris_tick(E);
          break;
        case SDLK_p:
	  pause ^= 1;
          break;
        case SDLK_r:
          etris_redraw(E);
          rc = ETRIS_OK_REDRAW;
          break;
        case SDLK_ESCAPE:
          etris_destroy(E);
	  TTF_Quit();
          SDL_Quit();
          exit(0);
          break;
        }
        if (rc == ETRIS_OK_REDRAW)
          SDL_Flip(screen);
        break;

      case SDL_QUIT:
        etris_destroy(E);
	TTF_Quit();
        SDL_Quit();
        exit(0);
      }
    }
  }

  return 0;
}
