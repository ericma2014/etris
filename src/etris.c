/* etris.c -- an embeddable Tetris gaming engine.
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

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE
#define WIN32_LEAN_AND_MEAN
#endif

#include <stdlib.h>

#include "etris.h"

#define E_MINIMUM_HEIGHT 4
#define E_MINIMUM_WIDTH 4
#define E_MAXIMUM_ROTATION 3

#define E_LEFT 1
#define E_RIGHT 2
#define E_ROTATE 3
#define E_DROP 4
#define E_TICK 5

/* TODO be user configurable */
#define ETRIS_TICKS_NORMAL 50
#define ETRIS_TICKS_DROPPING 1
#define ETRIS_TICKS_SHOWING_HIGHLIGHT 10
#define ETRIS_TICKS_SHOWING_BLANK 6
#define ETRIS_TICKS_REMOVING 2

/* TODO be user configurable */
#define ETRIS_SCORE_PER_LINE_MULTIPLIER 5
#define ETRIS_SCORE_PER_NEW_FIGURE 5
#define ETRIS_SCORE_PER_LINE_DROPPED 1

enum e_state {E_NORMAL, E_DROPPING, E_SHOWING_HIGHLIGHT, E_SHOWING_BLANK, E_REMOVING, E_GAME_OVER};

struct e_etris {
  struct {
    int width;
    int height;
    int border;
    char **data;
    int lines[4];
  } field;
  struct {
    int x;
    int y;
    int n;
    int r;
  } figure;
  struct {
    unsigned int figures;
    unsigned int lines;
    unsigned int score;
    unsigned int drops;
  } stats;
  struct {
    void (*draw_block)(int x, int y, int c);
    void (*update_score)(int score, int lines, int figures);
  } hooks;
  enum e_state state;
  int ticks;
  int speed;
};

typedef struct _e_figure {
  char offset;
  char color;
  char blocks[4][4];
} e_figure;

static e_figure figures[] = {
  {
    /* 00#0  0000
     * 00#0  ####
     * 00#0  0000
     * 00#0  0000 */
    0x00, 3, {{0x20, 0x21, 0x22, 0x23}, {0x01, 0x11, 0x21, 0x31}, {0x20, 0x21, 0x22, 0x23}, {0x01, 0x11, 0x21, 0x31}}
  },
  {
    /* 0000  0000
     * 0#00  0##0
     * 0##0  ##00
     * 00#0  0000 */
    0x00, 4, {{0x11, 0x12, 0x22, 0x23}, {0x11, 0x21, 0x02, 0x12}, {0x11, 0x12, 0x22, 0x23}, {0x11, 0x21, 0x02, 0x12}}
  },
  {
    /* 0000  0000
     * 00#0  0##0
     * 0##0  00##
     * 0#00  0000 */
    0x00, 5, {{0x21, 0x12, 0x22, 0x13}, {0x11, 0x21, 0x22, 0x32}, {0x21, 0x12, 0x22, 0x13}, {0x11, 0x21, 0x22, 0x32}}
  },
  {
    /* 0000
     * 0000
     * 0##0
     * 0##0 */
    0x00, 6, {{0x12, 0x22, 0x13, 0x23}, {0x12, 0x22, 0x13, 0x23}, {0x12, 0x22, 0x13, 0x23}, {0x12, 0x22, 0x13, 0x23}}
  },
  {
    /* 0000  0000  0000  0000
     * 00#0  00#0  0000  00#0
     * 0###  00##  0###  0##0
     * 0000  00#0  00#0  00#0 */
    0x01, 7, {{0x21, 0x12, 0x22, 0x32}, {0x21, 0x22, 0x32, 0x23}, {0x12, 0x22, 0x32, 0x23}, {0x21, 0x12, 0x22, 0x23}}
  },
  {
    /* 0000  0000  0000  0000
     * 00#0  0#00  00##  0000
     * 00#0  0###  00#0  0###
     * 0##0  0000  00#0  000# */
    0x00, 8, {{0x21, 0x22, 0x23, 0x13}, {0x11, 0x12, 0x22, 0x32}, {0x21, 0x31, 0x22, 0x23}, {0x12, 0x22, 0x32, 0x33}}
  },
  {
    /* 0000  0000  0000  0000 
     * 00#0  0000  0##0  000#
     * 00#0  0###  00#0  0###
     * 00##  0#00  00#0  0000 */
    0x00, 9, {{0x21, 0x22, 0x23, 0x33}, {0x12, 0x22, 0x32, 0x13}, {0x11, 0x21, 0x22, 0x23}, {0x31, 0x12, 0x22, 0x32}}
  }
};

#define E_NUMBER_OF_FIGURES (sizeof(figures) / sizeof(e_figure))

/* Draw game field */
static void e_draw_game_field(ETRIS e)
{
  int i, j;

  for (i = 0; i < (e->field.width + e->field.border * 2); i++)
    for (j = 0; j < (e->field.height + e->field.border); j++)
      e->hooks.draw_block(i, j, e->field.data[i][j]);
}

/* Draw current figure stored in `e' with "color" `c'. */
static void e_draw_figure(ETRIS e, char c)
{
  int i, bx, by;
  unsigned short b;

  for (i = 0; i < 4; i++) {
    b = figures[e->figure.n].blocks[e->figure.r][i];
    bx = e->figure.x + ((b >> 4) & 0xf);
    by = e->figure.y + (b & 0xf);
    if (by >= 0)
      e->hooks.draw_block(bx, by, c);
  }
}

/* Save current figure to game field. 
 * Returns greater than 0 if a block was save on top row or 
 * higher (i.e. game over), else 0 is returned. */
static int e_save_figure(ETRIS e)
{
  int i, bx, by, rc = 0;
  unsigned short b;

  for (i = 0; i < 4; i++) {
    b = figures[e->figure.n].blocks[e->figure.r][i];
    bx = e->figure.x + ((b >> 4) & 0xf);
    by = e->figure.y + (b & 0xf);
    if (by >= 0)
      e->field.data[bx][by] = figures[e->figure.n].color;
    if (by <= 0)
      rc++;
  }

  return rc;
}

/* Check if wanted figure position `x',`y' or rotation `r' is possible.
 * Returns 0 if it is. */
static int e_check_figure(ETRIS e, int x, int y, int r)
{
  int i, bx, by;
  unsigned short b;

  for (i = 0; i < 4; i++) {
    b = figures[e->figure.n].blocks[r][i];
    bx = x + ((b >> 4) & 0xf);
    by = y + (b & 0xf);
    if (bx < e->field.border || bx > (e->field.width + e->field.border - 1) ||
	by > (e->field.height - 1) ||
	e->field.data[bx][by] != ETRIS_BLOCK_BACKGROUND)
      return -1;
  }

  return 0;
}

/* Check if there are complete lines.
 * Returns the number of complete lines. */
static int e_check_lines(ETRIS e, int start)
{
  int x, y, l=0;

  for (y = start; y < e->field.height && y < (start + 4); y++) {
    for (x = e->field.border; x < (e->field.width + e->field.border); x++)
      if (e->field.data[x][y] == ETRIS_BLOCK_BACKGROUND)
	break;
    if (x == (e->field.width + e->field.border)) 
      e->field.lines[l++] = y;
  }

  /* clear row indexes for non-complete lines */
  for (y = l; y < 4; y++) 
    e->field.lines[y] = 0;
  
  return l;
}

/* Highlight complete lines. */
static void e_highlight_lines(ETRIS e, char c)
{
  int l, x, y;

  for (l = 0; l < 4; l++) {
    if ((y = e->field.lines[l]) == 0)
      break;
    for (x = e->field.border; x < (e->field.width + e->field.border); x++) {
      e->field.data[x][y] = c;
      e->hooks.draw_block(x, y, c);
    }
  }
}

/* Remove complete lines. */
static void e_remove_lines(ETRIS e)
{
  int l, x, y;

  for (l = 0; l < 4; l++) {
    for (y = e->field.lines[l]; y > 0; y--)
      for (x = e->field.border; x < (e->field.width + e->field.border); x++)
	e->field.data[x][y] = e->field.data[x][y - 1];
    e->field.lines[l] = 0;
  }
}

/* Prepare next figure. */
static void e_next_figure(ETRIS e)
{
  /* TODO add random() hook? */
  if (++e->figure.n >= E_NUMBER_OF_FIGURES)
    e->figure.n = 0;

  e->figure.x = e->field.width / 2 + e->field.border - 2 + ((figures[e->figure.n].offset >> 4) & 0xf);
  e->figure.y = (figures[e->figure.n].offset & 0xf) - 3;
  e->figure.r = 0;

  e->state = E_NORMAL;
  e->ticks = e->speed;

  e->stats.figures++;
  e->stats.score += ETRIS_SCORE_PER_NEW_FIGURE;

  e_draw_figure(e, figures[e->figure.n].color);
}

void etris_reset(ETRIS e)
{
  int i, j;

  e->speed = ETRIS_TICKS_NORMAL;

  e->stats.figures = 0;
  e->stats.lines = 0;
  e->stats.score = 0;
  e->stats.drops = 0;

  for (i = 0; i < (e->field.width + 2 * e->field.border); i++)
    for (j = 0; j < (e->field.height + e->field.border); j++)
      e->field.data[i][j] = ETRIS_BLOCK_BORDER;

  for (i = 0; i < e->field.width; i++)
    for (j = 0; j < e->field.height; j++)
      e->field.data[e->field.border + i][j] = ETRIS_BLOCK_BACKGROUND;

  e_next_figure(e);
  etris_redraw(e);
  e->hooks.update_score(e->stats.score, e->stats.lines, e->stats.figures);
}

void etris_destroy(ETRIS e) 
{
  int i;

  if (e != NULL) {
    if (e->field.data != NULL) {
      for (i = 0; i < (e->field.width + e->field.border * 2); i++) {
        if(e->field.data[i] == NULL) 
          break;
        free(e->field.data[i]);
      }
      free(e->field.data);
    }
    free(e);
  }
}

ETRIS etris_create(int width, int height, int border, 
		   void (*func_draw_block)(int x, int y, int c), 
		   void (*func_update_score)(int score, int lines, int figures))
{
  ETRIS e;
  int i;

  if (!func_draw_block || !func_update_score || 
      width < E_MINIMUM_WIDTH || height < E_MINIMUM_HEIGHT)
    return NULL;

  if ((e = calloc(sizeof(struct e_etris), 1)) == NULL ||
      (e->field.data = (char **)malloc((width + border * 2) * sizeof(char *))) == NULL) {
    etris_destroy(e);
    return NULL;   
  }

  for (i = 0; i < (width + border * 2); i++) {
    if ((e->field.data[i] = (char *)malloc(height + border)) == NULL) {
      etris_destroy(e);
      return NULL;   
    }
  }

  e->field.width = width;
  e->field.height = height;
  e->field.border = border;

  e->hooks.draw_block = func_draw_block;
  e->hooks.update_score = func_update_score;

  etris_reset(e);
  
  return e;
}

void etris_redraw(ETRIS e)
{
  e_draw_game_field(e);
  if (e->state == E_NORMAL || e->state == E_DROPPING) 
    e_draw_figure(e, figures[e->figure.n].color);
}

static int e_input(ETRIS e, int input)
{
  int x, y, r, l, rc;

  if (e->state == E_GAME_OVER)
    return ETRIS_GAME_OVER;

  x = e->figure.x;
  y = e->figure.y;
  r = e->figure.r;

  switch (input) {
  case E_LEFT: 
    x--; 
    break;
  case E_RIGHT: 
    x++; 
    break;
  case E_ROTATE: 
    if(++r > E_MAXIMUM_ROTATION)
      r = 0;
    break;
  case E_DROP:
    if (e->state == E_NORMAL) {
      e->state++;
      e->ticks = ETRIS_TICKS_DROPPING;
      e->stats.drops++;
    }
    return ETRIS_OK;
  case E_TICK:
    if (--e->ticks <= 0) {
      switch (e->state) {
      case E_SHOWING_HIGHLIGHT :
        e->state++;
        e->ticks = ETRIS_TICKS_SHOWING_BLANK;
        e_highlight_lines(e, ETRIS_BLOCK_BACKGROUND);
        return ETRIS_OK_REDRAW;
      case E_SHOWING_BLANK :
        e_remove_lines(e);
        e_draw_game_field(e);
        e->state++;
        e->ticks = ETRIS_TICKS_REMOVING;
        return ETRIS_OK_REDRAW;
      case E_REMOVING :
        e_next_figure(e);
        return ETRIS_OK_REDRAW;
      case E_NORMAL :
        e->ticks = e->speed;
        y++;
        break;
      case E_DROPPING :
        e->ticks = ETRIS_TICKS_DROPPING;
	e->stats.score += ETRIS_SCORE_PER_LINE_DROPPED;
        y++;
        break;
      }
    }
    else
      return ETRIS_OK;
    break;
  default:
    return ETRIS_ERR;
  }

  if (e->state == E_NORMAL || e->state == E_DROPPING) {
    if (e_check_figure(e, x, y, r) == 0) {
      e_draw_figure(e, ETRIS_BLOCK_BACKGROUND);
      e->figure.x = x;
      e->figure.y = y;
      e->figure.r = r;
      e_draw_figure(e, figures[e->figure.n].color);
      return ETRIS_OK_REDRAW;
    }
    else if (input == E_TICK) {
      if (e_save_figure(e) > 0) {
        e->state = E_GAME_OVER;
        return ETRIS_GAME_OVER;
      }
      else if ((rc = e_check_lines(e, e->figure.y)) > 0) {
        e->stats.lines += rc;
	e->stats.score += (ETRIS_SCORE_PER_LINE_MULTIPLIER * (2 << rc));

        e_highlight_lines(e, ETRIS_BLOCK_HIGHLIGHT);
        e->state = E_SHOWING_HIGHLIGHT;
        e->ticks = ETRIS_TICKS_SHOWING_HIGHLIGHT;
      }
      else
        e_next_figure(e);
      return ETRIS_OK_REDRAW;
    }
  }

  return ETRIS_OK;
}

static int e_run(ETRIS e, int input)
{
  int score = e->stats.score;
  int rc = e_input(e, input);
  if (score != e->stats.score)
    e->hooks.update_score(e->stats.score, e->stats.lines, e->stats.figures);

  return rc;
}

int etris_left(ETRIS e)
{
  return e_run(e, E_LEFT);
}

int etris_right(ETRIS e)
{
  return e_run(e, E_RIGHT);
}

int etris_rotate(ETRIS e)
{
  return e_run(e, E_ROTATE);
}

int etris_drop(ETRIS e)
{
  return e_run(e, E_DROP);
}

int etris_tick(ETRIS e)
{
  return e_run(e, E_TICK);
}
