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

#define _POSIX_SOURCE

#include "movedb.h"
#include "normalize.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>


uint64_t hash(state st) {
  return st.turn ^ (st.sheep * st.tiger) ^ st.setsheep;
}

movedb *create_movedb(uint64_t hashsize, uint64_t bsize) {
  movedb *db = (movedb *) malloc(sizeof(movedb));

  db->base = NULL;
  db->hashsize = hashsize;
  db->hash = (hashbucket **) malloc(db->hashsize * sizeof(void *));

  for (uint64_t i = 0; i < db->hashsize; i++) {
    db->hash[i] = (hashbucket *)malloc(sizeof(hashbucket) + bsize * sizeof(movedb_entry));
    db->hash[i]->size = 0;
    db->hash[i]->capacity = bsize;
    memset(db->hash[i]->entries, '\0', sizeof(movedb_entry) * bsize);
  }

  return db;
}


movedb *load_movedb(const char *filename) {
  FILE *file = fopen(filename, "rb");

  if (file == NULL) {
    return NULL;
  }

  movedb *db = (movedb *) malloc(sizeof(movedb));

  db->base = NULL;
  fread(&db->hashsize, sizeof(uint64_t), 1, file);
  db->hash = (hashbucket **) malloc(db->hashsize * sizeof(hashbucket *));

  for (uint64_t i = 0; i < db->hashsize; i++) {
    hashbucket proto, *res;
    fread(&proto, sizeof(hashbucket), 1, file);

    res = (hashbucket *)malloc(sizeof(hashbucket) + proto.capacity * sizeof(movedb_entry));
    memcpy(res, &proto, sizeof(hashbucket));
    fread(res->entries, sizeof(movedb_entry), proto.capacity, file);
    db->hash[i] = res;
  }

  fclose(file);
  return db;
}

int *rehash_movedb(movedb *rehashed, movedb *old) {
  for (uint64_t i = 0; i < old->hashsize; i++) {
    hashbucket *bucket = &old->hash[i];
    for (uint64_t j = 0; j < bucket->size; j++) {
      // TODO ... remap insert
      // need better hash algorithm to avoid clustering
      // at small values
    }
  }
}

movedb *mmap_movedb(const char *filename) {
  // XXX to implement
  // rationale: faster starup time, less
  // RAM usage facilitating the use of a
  // vast move db (especially on 64 bit machines)
  // problem: extension will be difficult
  return NULL;
}

int save_movedb(movedb * db, const char *filename) {
  FILE *file = fopen(filename, "wb");

  fwrite(&db->hashsize, sizeof(uint64_t), 1, file);

  for (int i = 0; i < db->hashsize; i++) {
    fwrite(db->hash[i], sizeof(hashbucket), 1, file);
    fwrite(db->hash[i]->entries, sizeof(movedb_entry), db->hash[i]->capacity, file);
  }

  return 0;
}

movedb_entry *lookup(movedb *db, state st) {
  st = normalize(st);
  hashbucket *b = db->hash[hash(st) % db->hashsize];
  for (uint64_t i = 0; i < b->size; i++) {
    if (memcmp(&b->entries[i].st, &st, sizeof(state)) == 0) {
      return &b->entries[i];
    }
  }
  return NULL;
}

movedb_entry *insert(movedb *db, state st) {
  // TODO: search by binary search for the insertion point
  // TODO: implement the mmapped case
  // TODO: implement expansion
  st = normalize(st);
  hashbucket *b = db->hash[hash(st) % db->hashsize];

  if (b->size == b->capacity) {
    b->capacity += 128;
    b = db->hash[hash(st) % db->hashsize] = (hashbucket *)realloc(b, b->capacity*sizeof(movedb_entry));
  }

  b->entries[b->size].st = st;
  b->entries[b->size].param = 0;
  b->entries[b->size].score = 0;
  return &b->entries[b->size++];
}

// operations for relative assessment
int update_win(movedb *db, state st) {
  movedb_entry *item = lookup(db, st);
  if (item == NULL) {
    item = insert(db, st);
    if (item == NULL)
      return 1;
  }

  item->param++;
  item->score++;

  return 0;
}

int update_loss(movedb *db, state st) {
  movedb_entry *item = lookup(db, st);
  if (item == NULL) {
    item = insert(db, st);
    if (item == NULL)
      return 1;
  }

  item->param++;
  item->score--;

  return 0;
}

// operations for storing absolute strength
// not used currently
int update_score(movedb *db, state st, int32_t strength) {
  movedb_entry *item = lookup(db, st);
  if (item == NULL) {
    item = insert(db, st);
    if (item == NULL)
      return 1;
  }

  item->param = 0;
  item->score = strength;

  return 0;
}
