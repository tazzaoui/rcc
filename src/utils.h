#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include "../src/rcc.h"

static inline void *malloc_or_die(size_t size) {
  void *mem = malloc(size);
  if (mem == NULL) {
    fprintf(stderr, "Unable to allocate memory!");
    exit(-1);
  }
  return mem;
}

typedef struct env_pair_t {
  Expr *var;
  int val;
} env_pair_t;

#endif /* UTILS_H */
