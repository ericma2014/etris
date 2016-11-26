# etris - Embeddable Tetris Engine

Use the embeddable Tetris gaming engine to add a game wherever one is needed, there is always space left for Tetris :)

It has been designed to be really easy to use and integrate. Just implement one or two function hooks, user input and timer ticks logic, and you're done!

## Status

Stable. Is running on low-end ARM Cortex-M3 devices as well as high-end x86 workstations.

Was originally hosted on [https://code.google.com/p/etris/] and haven't been touched since Jul 2, 2012.

## Credits

etris is written and maintained by Jonas Romfelt simply because it is fun.  

# Examples

## Playable demo using SDL

Included for demonstration purposes is a playable Tetris game using SDL as back-end for screen drawing, font rendering and user input event handling.

![etris SDL screen shot](https://github.com/romfelt/etris/raw/master/img/etris-sdl.png "etris")

## Basic game template

Below a simple example that could be used as template for new users. In most examples error handling has been left out for simplicity. Just implement the hooks and play!

```c
#include <stdio.h>
#include "etris.h"

#define FIELD_WIDTH 10
#define FIELD_HEIGHT 20
#define FIELD_BORDER 1

void draw_block(int x, int y, int c)
{
  /* draw a block to screen at coordinate (x,y) with color c */
}

void update_score(int score, int lines, int figures)
{
  /* draw score string */
}

int ticks(void)
{
  /* return number of milliseconds */
}

void flip(void)
{
  /* flip screen and redraw, not always necessary */
}

char get_key(void)
{
  /* get input from user */
}

int main(int argc, char **argv)
{
  ETRIS E;
  int rc, next_tick;
  char key;

  E = etris_create(FIELD_WIDTH, FIELD_HEIGHT, FIELD_BORDER, draw_block, update_score);

  next_tick = ticks();

  /* main game loop */
  while (1) {
    if (ticks() > next_tick) {
      next_tick = ticks() + 10;
      if (etris_tick(E) == ETRIS_OK_REDRAW)
        flip();
    } 
    else if ((c = get_key()) != 0) {
      switch (c) {
        case UP: 
          rc = etris_rotate(E); 
          break;
        case DOWN:
          rc = etris_drop(E); 
          break;
        case RIGHT:
          rc = etris_right(E); 
          break;
        case LEFT:
          rc = etris_left(E); 
          break;
        case ESCAPE:
          continue;
      }
      if (rc == ETRIS_OK_REDRAW)
        flip();
    }
  }

  return 0;
}
```
