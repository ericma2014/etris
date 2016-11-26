/* etris.h -- an embeddable Tetris gaming engine, public API.
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

#ifndef __ETRIS_H
#define __ETRIS_H

#ifdef __cplusplus
extern "C" {
#endif

/* handle to an etris instance */
typedef struct e_etris * ETRIS;

#define ETRIS_OK 0
#define ETRIS_OK_REDRAW 1
#define ETRIS_GAME_OVER 2
#define ETRIS_ERR -90
#define ETRIS_ERR_NOMEM -91

#define ETRIS_BLOCK_BACKGROUND 0
#define ETRIS_BLOCK_BORDER 1
#define ETRIS_BLOCK_HIGHLIGHT 2

/** 
 * Create new etris instance.
 *
 * @param width The playfield width as number of blocks
 * @param height The playfield height as number of blocks
 * @param border The size of playfield border as number of blocks
 * @param func_draw_block Function call hook for drawing a block at (x, y) with color (c)
 * @param func_update_score Function call hook for refreshing score display
 * @return Newly created etris instance or NULL on error
 */
ETRIS etris_create(int width, int height, int border, 
		   void (*func_draw_block)(int x, int y, int c), 
		   void (*func_update_score)(int score, int lines, int figures));

/** 
 * Destroy an etris instance and free allocated memory.
 *
 * @param e The etris instance to destroy
 */
void etris_destroy(ETRIS e);

/** 
 * Feed game engine with a time tick, speed of play has been adjusted to a
 * periodic tick of 10 ms.
 *
 * @param e The etris instance
 * @return 1 if there has been re-drawing, 0 otherwise
 */
int etris_tick(ETRIS e);

/** 
 * User input actions that controls the figure currently played. etris_left()
 * and etris_right() moves the figure one block left and right respectively.
 * etris_rotate() rotates the figure and etris_drop() drops the figure, that 
 * is free fall.
 *
 * @param e The etris instance
 * @return 1 if there has been re-drawing, 0 otherwise
 */
int etris_left(ETRIS e);
int etris_right(ETRIS e);
int etris_rotate(ETRIS e);
int etris_drop(ETRIS e);

/** 
 * Redraw all graphics, game field and current figure. Note that this function
 * should be only when it is needed to redraw all graphics. The gaming engine
 * makes necessary graphics updates.
 *
 * @param e The etris instance
 */
void etris_redraw(ETRIS e);

/** 
 * Reset gaming engine and game field graphics. Typically used when starting
 * a new game session and after game over.
 *
 * @param e The etris instance
 */
void etris_reset(ETRIS e);

#ifdef __cplusplus
}
#endif

#endif /* __ETRIS_H */
