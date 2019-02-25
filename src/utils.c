#include "utils.h"
#include "list.h"

Info* new_info(list_t vars, list_t live){
    Info *info = malloc_or_die(sizeof(Info));
    info->vars = vars;
    info->live = live;
    return info;
}
