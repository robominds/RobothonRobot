
#include "stddef.h"
#include "tpu_reg.h"
#include "tpu.h"
#include "fqd.h"

int fqd_init(int chan) {

  tpu_set_cpr(chan+0, 0);
  tpu_set_cpr(chan+1, 0);
  tpu_set_cfsr(chan+0, 6);
  tpu_set_cfsr(chan+1, 6);

  (*(short int *)(TPU_RAM + (chan+0)*16 + 0x8)) = (chan+1)*16+6;
  (*(short int *)(TPU_RAM + (chan+1)*16 + 0x8)) = (chan+0)*16+6;
  (*(short int *)(TPU_RAM + (chan+0)*16 + 0xa)) = chan*16+1;
  (*(short int *)(TPU_RAM + (chan+1)*16 + 0xa)) = chan*16+1;
  (*(short int *)(TPU_RAM + (chan+0)*16 + 0x2)) = 0;
  tpu_set_hsqr(chan+0, 0);
  tpu_set_hsqr(chan+1, 1);
  tpu_set_hsrr(chan+0, 3);
  tpu_set_hsrr(chan+1, 3);
  tpu_set_cpr(chan+0, 3);
  tpu_set_cpr(chan+1, 3);

  return 0;
}

short int fqd_get_count(int chan) {

  return (*(short int *)(TPU_RAM + chan*16 + 2));
}

