#include <stdio.h>
#include "../src/utils.h"
#include "tests.h"

int main(int argc, char *argv[]) {
  printf("Testing Linked List...\n");

  test_list();

  printf("%sAll List Tests Passed.\n%s", GRN, NRM);

  printf("===================================================\n");

  printf("Interpreting a dozen r0 programs....\n");

  test_dozen_r0();

  printf("Testing a dozen R1 programs...\n");

  test_dozen_r1();

  printf("===================================================\n");

  printf("Testing X0 emitter...\n");

  test_x0_emit();

  printf("===================================================\n");

  printf("Testing a Dozen X0 Progs...\n");

  test_dozen_x0();

  printf("===================================================\n");

  printf("Testing a Dozen C0 Progs...\n");

  test_dozen_c0();

  printf("===================================================\n");

  printf("testing uniquify...\n\n");

  test_uniquify();

  printf("===================================================\n");

  printf("testing rco...\n\n");

  test_rco();

  printf("===================================================\n");

  printf("testing econs...\n\n");

  test_econ();

  printf("===================================================\n");

  printf("testing uncover-locals...\n\n");

  test_uncover_locals();

  printf("===================================================\n");

  printf("testing select-instr...\n\n");

  test_select_instr();

  printf("===================================================\n");

  printf("testing assign-homes...\n\n");

  test_assign_homes();

  printf("===================================================\n");

  printf("testing patch-instrs...\n\n");

  test_patch_instrs();

  printf("===================================================\n");

  printf("testing uncover-live...\n\n");

  test_uncover_live();

  printf("===================================================\n");

  printf("testing build_interferences...\n\n");

  test_build_interferences();

  printf("===================================================\n");

  printf("testing color_graph...\n\n");

  test_color_graph();

  printf("===================================================\n");

  printf("testing assign_registers...\n\n");

  test_assign_registers();

  printf("===================================================\n");

  printf("testing allocate_registers...\n\n");

  test_allocate_registers();

  printf("===================================================\n");

  printf("testing move biasing...\n\n");

  test_move_biasing();

  printf("===================================================\n");

  printf("testing r2...\n\n");

  test_r2();

  printf("===================================================\n");

  printf("testing type checker...\n\n");

  test_type_checker();

  printf("===================================================\n");

  printf("testing r2 optimizer...\n\n");

  test_r2_optimizer();

  printf("===================================================\n");

  printf("testing c2...\n\n");

  test_c2();

  printf("===================================================\n");

  printf("testing x1...\n\n");

  test_x1();

  printf("===================================================\n");

  printf("testing uncover-live...\n\n");

  test_uncover_live();

  return 0;
}
