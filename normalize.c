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

#include "normalize.h"

// rotate the board 90 degrees counter-clockwise
static state rotate(state st) {
  state res = st;
  res.sheep = 0ULL;
  res.tiger = 0ULL;
  for (int i = 0; i < BOARDLEN; i++) {
    for (int j = 0; j < BOARDLEN; j++) {
      if (sheep_at(st, BOARDLEN - j - 1, i)) {
        res.sheep |= 1ULL << (i + j * 5);
      }

      if (tiger_at(st, BOARDLEN - j - 1, i)) {
        res.tiger |= 1ULL << (i + j * 5);
      }
    }
  }
  return res;
}

// mirror the board horizontally
static state mirror(state st) {
  state res = st;
  res.sheep = 0ULL;
  res.tiger = 0ULL;
  for (int i = 0; i < BOARDLEN; i++) {
    for (int j = 0; j < BOARDLEN; j++) {
      if (sheep_at(st, i, BOARDLEN - j - 1)) {
        res.sheep |= 1ULL << (i + j * 5);
      }

      if (tiger_at(st, i, BOARDLEN - j - 1)) {
        res.tiger |= 1ULL << (i + j * 5);
      }
    }
  }
  return res;
}

// consider all rotated and
// mirrored boards, choose
// the one where the tigers
// bitmap interpreted as integer
// is smallest, and if it is the same
// for two, choose the one, with the
// smalltest sheep integer
state normalize(state st) {
  state cand[7];
  cand[0] = mirror(st);
  cand[1] = rotate(st);
  cand[2] = rotate(cand[0]);
  cand[3] = rotate(cand[1]);
  cand[4] = rotate(cand[2]);
  cand[5] = rotate(cand[3]);
  cand[6] = rotate(cand[4]);

  state res = st;
  for (int i = 0; i < 7; i++) {
    if (cand[i].tiger <= res.tiger) {
      if (cand[i].tiger < res.tiger) {
        res = cand[i];
      } else if (cand[i].sheep < res.sheep) {
        res = cand[i];
      }
    }
  }

  return res;
}
