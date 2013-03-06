// small tool to get some information on a movedb
// mainly used to improve the hash algorithm
#include "movedb.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  FILE *hist = NULL;
  FILE *raw = NULL;
  int *histogram = NULL;

  if (argc < 2) {
    fputs("Please give the bcmoves files as argument!\n", stderr);
    return 1;
  }

  if (argc >= 3) {
    hist = fopen(argv[2], "w");
  }

  if (argc == 4) {
    raw = fopen(argv[3], "w");
  }

  movedb *db = load_movedb(argv[1]);

  printf("%d hashbuckets\n", db->hashsize);

  int entries = 0, max = 0;
  for (int i = 0; i < db->hashsize; i++) {
    entries += db->hash[i]->size;

    if (db->hash[i]->size > max)
      max = db->hash[i]->size;

    if (raw != NULL) {
      fprintf(raw, "%d %d\n", i, db->hash[i]->size);
    }
  }

  if (hist != NULL) {
    histogram = (int *) malloc((max + 1) * sizeof(int));
    if (histogram != NULL) {
      for (int i = 0; i < max; i++) {
        histogram[i] = 0;
      }
    }
  }
  
  double var = 0.0;
  double mean = entries / (float)db->hashsize;
  int min = entries;
  for (int i = 0; i < db->hashsize; i++) {
    var += (db->hash[i]->size - mean)*(db->hash[i]->size - mean);

    if (db->hash[i]->size < min)
      min = db->hash[i]->size;

    if (histogram != NULL) {
      histogram[db->hash[i]->size]++;
    }
  }
  var /= db->hashsize;

  printf("%d entries (%f per bucket, max/min/dev %d/%d/%f)\n", entries, mean, max, min, sqrt(var));

  movedb_entry *start = lookup(db, START);
  printf("Score and param for START: %d, %d\n", start->score, start->param);

  if (hist != NULL) {
    int sum = 0;
    if (histogram != NULL) {
      for (int i = 0; i < max; i++) {
        sum += histogram[i];
        fprintf(hist, "%d %d %d\n", i, histogram[i], sum);
      }

      free(histogram);
    }
    fclose(hist);
  }
}
