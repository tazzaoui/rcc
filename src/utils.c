#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

int die(const char *err_msg){
    fprintf(stderr, "%sERROR: %s%s\n", RED, err_msg, NRM);
    exit(-1);
}
