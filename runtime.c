#include <stdio.h>

int read_int() {
  int x = 0;
  scanf("%d", &x);
  return x;
}

int read_int_debug() {
  return 7;
}

int print_int(int x) {
  printf("%d\n", x);
  return 0;
}

int print_bool(int x) {
  printf("%s\n", x ? "<T>" : "<F>");
  return x;
}
