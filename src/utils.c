#include "utils.h"
#include "list.h"

Info* new_info(list_t vars){
    Info *info = malloc_or_die(sizeof(Info));
    info->vars = vars;
    return info;
}
