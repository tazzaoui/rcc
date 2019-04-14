#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../src/pairs.h"
#include "../src/rcc.h"
#include "../src/utils.h"
#include "tests.h"

#define DEBUG 0
#define NUM_PROGS 1024

int main(int argc, char *argv[]) {
  int count = 0, res, rand_depth, res_opt, res_uniq, res_rco, res_econ, res_ul,
    res_si, res_ar, res_pi, res_mp, res_ui, res_bi, res_cg, full_count = 0;
  R_Expr *expr, *expr_opt, *uniq, *simple;
  C_Tail *c_tail;
  X_Program *ul, *bi, *ar, *pi, *mp, *cg, *xp;
  C_Program *cp, *cp_uncovered;

  srand(time(0));
  list_t new_vars, labels, vars = list_create();

  printf("Testing Full Pass on %d programs...\n", NUM_PROGS);

  for (size_t i = 0; i < NUM_PROGS; ++i) {
    rand_depth = rand() % 10;
    expr = randp(vars, rand_depth);
    uniq = unique(expr);
    new_vars = list_create();
    simple = rco(uniq, &new_vars);
    c_tail = econ_expr(simple);
    labels = list_create();
    list_insert(labels, new_lbl_tail_pair("body", c_tail));
    cp = new_c_program(NULL, labels);
    cp_uncovered = uncover_locals(cp);
    xp = select_instr(cp_uncovered);
    ul = uncover_live(xp);
    bi = build_interferences(ul);
    cg = color_graph(bi);
    ar = assign_registers(xp);
    pi = patch_instrs(ar);
    mp = main_pass(pi);
    res = r_int_interp(expr, NULL);
    res_uniq = r_int_interp(uniq, NULL);
    res_rco = r_int_interp(simple, NULL);
    res_econ = c_t_interp(c_tail, list_create(), NULL);
    res_ul = c_p_interp(cp_uncovered);
    res_si = x_interp(xp);
    res_ui = x_interp(ul);
    res_bi = x_interp(bi);
    res_cg = x_interp(cg);
    res_ar = x_interp(ar);
    res_pi = x_interp(pi);
    res_mp = x_compile(mp);
    assert(res == res_uniq);
    assert(res_uniq == res_rco);
    assert(res_rco == res_econ);
    assert(res_econ == res_ul);
    assert(res_ul == res_si);
    assert(res_si == res_ui);
    assert(res_ui == res_bi);
    assert(res_bi == res_cg);
    assert(res_cg == res_ar);
    assert(res_ar == res_pi);
    assert(res_pi == res_mp);
    expr_opt = r_optimize(expr, NULL);
    res_opt = r_int_interp(expr_opt, NULL);
    assert(res_opt == res);
    count += (res_opt == res);
    full_count += (res_opt == res && expr_opt->type == R_EXPR_NUM);
    printf("%s TEST %d PASSED %s\n", GRN, count, NRM);
  }

  printf("%sSuccessfully Compiled %d/%d Programs.%s\n", GRN, count, NUM_PROGS,
         NRM);
  printf("Fully optimized %d/%d Programs\n", count, NUM_PROGS);
  assert(count == NUM_PROGS);

  return 0;
}
