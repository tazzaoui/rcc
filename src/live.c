#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "live.h"
#include "x.h"
#include "list.h"
#include "pairs.h"
#include "utils.h"
#include "rcc.h"

list_t live_l(list_t all, list_t mem, label_t lbl) {
  if (!all)
    all = list_create();
  if (!mem)
    mem = list_create();

  Node *m;
  list_t la;
  Info *info;
  X_Block *x_blk;
  lbl_blk_pair_t *lbp;

  lbp = new_lbl_blk_pair(lbl, NULL);
  m = list_find(mem, lbp, lbl_blk_pair_cmp);

  if (m) {
    lbp = m->data;
    return lbp->block->instrs;
  } else if (strcmp(lbl, "END"))
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


list_t live_is(list_t all, list_t mem, list_t instrs) {
  if (!all)
    all = list_create();
  if (!mem)
    mem = list_create();
  if (!instrs)
    instrs = list_create();

  if (list_size(instrs) == 0)
    return list_create();

  list_t res, la, wv = list_create(), rv = list_create(),
    la0 = list_create(), f = list_create(), r = list_create();
  label_t lbl;
  Node *first = *instrs;
  Node *rest = first->next;

  X_Instr *fi = first->data;
  list_insert(f, fi);
  list_insert(r, rest->data);

  la = live_is(all, mem, r);
  if (fi->type == JMP || fi->type == JMPIF) {
    if (fi->type == JMP)
      lbl = ((X_Jmp *) fi->instr)->label;
    else
      lbl = ((X_Jmpif *) fi->instr)->label;
    la = live_l(all, mem, lbl);
    list_insert(la0, *la);
    list_concat(la0, instrs);
    return la0;
  }

  instrs_read(fi, rv);
  instrs_written(fi, wv);
  list_insert(la0, *la);
  res = list_subtract(la0, wv, cmp_x_args);
  list_concat(res, rv);
  list_concat(res, la);
  return res;
}
