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

#ifndef BHAGCHAL_MOVEDB_H
#define BHAGCHAL_MOVEDB_H

#include "baghchal.h"
#include <stdint.h>

struct movedb_entry {
  state st;
  int32_t param;
  int32_t score;
};
typedef struct movedb_entry movedb_entry;

struct hashbucket {
  int size;
  int capacity;
  movedb_entry entries[];
};
typedef struct hashbucket hashbucket;

struct movedb {
  void *base;
  uint64_t hashsize;
  hashbucket **hash;
};
typedef struct movedb movedb;

// for the hashtable
uint64_t hash(state st);

movedb *mmap_movedb(const char *file);
movedb *load_movedb(const char *file);
movedb *create_movedb(uint64_t hashsize, uint64_t bucketsize);
int save_movedb(movedb *db, const char *file);
void free_movedb();

movedb_entry *lookup(movedb *db, state st);

// operations for relative assessment
int update_win(movedb *db, state st);
int update_loss(movedb *db, state st);

// operations for storing absolute strength
int update_score(movedb *db, state st, int32_t strenth);

#endif
