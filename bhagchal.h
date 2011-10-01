
#ifndef BHAGCHAL_H
#define BHAGCHAL_H

#include <stdint.h>

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
const state START = {
  0, // no sheep
  0x1100011, // the four tigers in the corners
  0, // no sheep set
  TURN_SHEEP // sheep's turn
};

// constants for moves
const uint64_t MAXSHEEP = 20;
const uint64_t BOARDLEN = 5;
const uint64_t BOARDPLACES = 25;

// board connections - each byte contains the possible moves
// by (-1,-1), (-1,0), (-1,1), ..
// perhaps one day the move generation code could exploit symmetries
// that I don't see right now to use mainly bitwise operations on the entire word
// or be created from the data
const uint8_t CONNECTIONS[25] = {
  0x1c, 0x15, 0x1f, 0x15, 0x07,
  0x54, 0xff, 0x55, 0xff, 0x45,
  0x7c, 0x55, 0xff, 0x55, 0xc7,
  0x54, 0xff, 0x55, 0xff, 0x45,
  0x70, 0x51, 0xf1, 0x51, 0xc1
};

/* const int8_t SHIFT[8] = {-6, -5, -4, 1, 6, 5, 4, -1}; */
const int8_t SHIFT[8] = {-1, 4, 5, 6, 1, -4, -5, -6};

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
const int HAMMING_NYBBLE[16] = {
// 0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
   0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4
};
static inline int hamming(uint64_t n) {
  int res = 0;
  for (int i = 0; i < 16; i++) {
    res += HAMMING_NYBBLE[n & 0xf];
    n >>= 4;
  }
  return res;
}

#endif
