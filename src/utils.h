#ifndef UTILS_H
#define UTILS_H

#define NRM "\x1B[0m"
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define I32MIN -2147483648
#define RAND_RANGE 1024
#define GET_RAND() (rand() % (2 * RAND_RANGE)) - RAND_RANGE

#include <stdio.h>
#include <stdlib.h>

typedef const char* label_t;

static inline void *malloc_or_die(size_t size) {
  void *mem = malloc(size);
  if (mem == NULL) {
    fprintf(stderr, "Unable to allocate memory!");
    exit(-1);
  }
  return mem;
}

static inline int die(const char *err_msg) {
  fprintf(stderr, "%sERROR: %s%s\n", RED, err_msg, NRM);
  exit(-1);
}

#endif /* UTILS_H */
