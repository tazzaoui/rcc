#include <stdio.h>

int read_int() {
  int x = 0;
  scanf("%d", &x);
  return x;
}

int print_int(int x) {
  printf("%d\n", x);
  return 0;
}