
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

state normalize(state st);
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
