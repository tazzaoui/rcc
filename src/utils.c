#include <time.h>
#include <sys/timeb.h>
#include <string.h>
#include "utils.h"
#include "list.h"

Info *new_info(list_t vars, list_t live, list_t graph) {
  Info *info = malloc_or_die(sizeof(Info));
  info->vars = vars;
  info->live = live;
  info->i_graph = graph;
  return info;
}

char *get_time_stamp() {
  struct timeb start;
  char *buf = malloc_or_die(512 * sizeof(char));
  char append[100];
  if (buf) {
    ftime(&start);
    strftime(buf, 100, "%H:%M:%S", localtime(&start.time));
    sprintf(append, ":%03u", start.millitm);
    strcat(buf, append);
  }
  return buf;
}
