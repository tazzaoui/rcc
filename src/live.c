#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "live.h"
#include "list.h"
#include "pairs.h"


list_t live_l(list_t all, list_t mem, label_t lbl){
    if(!all) all = list_create();
    if(!mem) mem = list_create();

    Node *m;
    list_t la;
    Info *info;
    X_Block *x_blk;
    lbl_blk_pair_t *lbp;

    lbp = new_lbl_blk_pair(lbl, NULL);
    m = list_find(mem, lbp, lbl_blk_pair_cmp); 

    if(m){
        lbp = m->data;
        return lbp->block->instrs;
    } else if(strcmp(lbl, "END"))
        return list_create(); 
      
    m = list_find(all, lbp, lbl_blk_pair_cmp); 
    assert(m);
    lbp = m->data;
    x_blk = lbp->block;
    info = x_blk->info;

    la = live_is(all, mem, x_blk->instrs);

    list_concat(info->live, la);
    list_insert(mem, lbp);
    return la;
}
