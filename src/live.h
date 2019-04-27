#ifndef LIVE_H
#define LIVE_H

#include "x.h"
#include "list.h"

/* live after analysis for the entire program */
X_Program *live_p(X_Program *);

/*  Liveness analysis for instrs */
list_t live_is(list_t, list_t, list_t);

/*  Liveness analysis for a given label */
list_t live_l(list_t, list_t, label_t);
#endif                          /* LIVE_H */
