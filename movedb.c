
#include "movedb.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

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

      if (tiger_at(st, i, j)) {
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
      if (sheep_at(st, i, BOARDLEN - i - 1)) {
        res.sheep |= 1ULL << (i + j * 5);
      }

      if (tiger_at(st, i, j)) {
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

movedb *mmap_movedb(const char *filename) {
  // XXX to implement
  // rationale: faster starup time, less
  // RAM usage facilitating the use of a 
  // vast move db
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
