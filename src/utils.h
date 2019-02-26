#ifndef UTILS_H
#define UTILS_H

#define NRM "\x1B[0m"
#define RED "\x1B[31m"
#define GRN "\x1B[32m"
#define I32MIN -2147483648
#define RAND_RANGE 1024
#define GET_RAND() (rand() % (2 * RAND_RANGE)) - RAND_RANGE
#define QUIET_READ 1

#if QUIET_READ == 1
#define READ_INT "read_int_debug"
#else
#define READ_INT "read_int"
#endif

#include <stdio.h>
#include <stdlib.h>
#include "list.h"

typedef const char *label_t;

/* Program Info */
typedef struct Info {
  list_t vars;     // list of live locals : <X_Arg*>
  list_t live;     // list of live after sets : <X_Instr -> <X_Arg*>>
  list_t i_graph;  // Adj. List rep. of interference graph
} Info;

/* Return a new Info */
Info *new_info(list_t, list_t, list_t);

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
