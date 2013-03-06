/*
 * This file is part of baghchal an implementation of the Bagh-Chal
 * board game.
 *
 * Copyright 2012 Sebastian Riese
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies
 * of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef BHAGCHAL_H
#define BHAGCHAL_H

#include <stdint.h>

// strength parameters for the AI
// twiddle around with these, but be warned
// they may turn the game indefinitley slow
#define AI_DEPTH_DEFAULT 2
#define RECURSE_LIMIT_SHEEP 5000
#define RECURSE_LIMIT_TIGER 1000
#define RECURSE_INC 2
#define LIMIT_MULT 2

// the board is represented by
// the sheep bitmap, the tiger bitmap
// and the turn bit (0 sheep, 1 tiger)
struct state {
  uint64_t sheep: 25;
  uint64_t tiger: 25;
  uint64_t setsheep: 5;
  uint64_t turn: 1;
};
typedef struct state state;

#define TURN_SHEEP 0
#define TURN_TIGER 1

// initial board
static const state START = {
  0, // no sheep
  0x1100011, // the four tigers in the corners
  0, // no sheep set
  TURN_SHEEP // sheep's turn
};

// constants for moves
static const uint64_t MAXSHEEP = 20;
static const uint64_t BOARDLEN = 5;
static const uint64_t BOARDPLACES = 25;

// AI calibration
static const int SHEEPWEIGHT = 131;
static const int TRAPPEDWEIGHT = 26;
static const int LOCKEDWEIGHT = 1;
static const int MAXSCORE = 2749; // XXX: update according to SHEEPWEIGHT * MAXSHEEP + TRAPPEDWEIGHT * 4 + LOCKEDWEIGHT * 25 + 1

// board connections - each byte contains the possible moves
// by (-1,-1), (-1,0), (-1,1), ..
// perhaps one day the move generation code could exploit symmetries
// that I don't see right now to use mainly bitwise operations on the entire word
// or be created from the data
static const uint8_t CONNECTIONS[25] = {
  0x1c, 0x15, 0x1f, 0x15, 0x07,
  0x54, 0xff, 0x55, 0xff, 0x45,
  0x7c, 0x55, 0xff, 0x55, 0xc7,
  0x54, 0xff, 0x55, 0xff, 0x45,
  0x70, 0x51, 0xf1, 0x51, 0xc1
};

// the relative index of the next field, indexed by the
// number of the bit, which encodes this direction in
// the CONNECTIONS array
static const int8_t SHIFT[8] = {-1, 4, 5, 6, 1, -4, -5, -6};

// querying the board
static inline int sheep_at(state st, int i, int j) {
  return (st.sheep >> (i + j * 5)) & 1;
}

static inline int tiger_at(state st, int i, int j) {
  return (st.tiger >> (i + j * 5)) & 1;
}

// not optimal, but fast enough for my purposes for now
// if it gets critical, write a code generator, precalculating
// the hamming weight of bytes, finally there is always one alternative: magic
static const int HAMMING_NYBBLE[16] = {
// 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
   0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
};

// calculate the hamming weight of the argument
static inline int hamming(uint64_t n) {
  int res = 0;
  for (int i = 0; i < 16; i++) {
    res += HAMMING_NYBBLE[n & 0xf];
    n >>= 4;
  }
  return res;
}

static inline int max(int a, int b)
{
    return (a > b) ? a : b;
}

static inline int min(int a, int b)
{
    return (a < b) ? a : b;
}

#endif
