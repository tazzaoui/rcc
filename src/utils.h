#ifndef UTILS_H
#define UTILS_H

static inline void *malloc_or_die(size_t size) {
  void *mem = malloc(size);
  if (mem == NULL) {
    fprintf(stderr, "Unable to allocate memory!");
    exit(-1);
  }
  return mem;
}

#endif /* UTILS_H */
