#ifndef UTILS_H
#define UTILS_H

#define NRM "\x1B[0m"
#define RED "\x1B[31m"
#define GRN "\x1B[32m"

#include <stdio.h>
#include <stdlib.h>

static inline void *malloc_or_die(size_t size) {
  void *mem = malloc(size);
  if (mem == NULL) {
    fprintf(stderr, "Unable to allocate memory!");
    exit(-1);
  }
  return mem;
}

int die(const char *err_msg);

#endif /* UTILS_H */
