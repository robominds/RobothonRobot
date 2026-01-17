
#include "stddef.h"
#include "tpu_reg.h"
#include "tpu.h"
#include "pwm.h"

int pwm_init(int chan, int on, int off) {

  tpu_set_cpr(chan, 0);
  tpu_set_cfsr(chan, 14);

  (*(short int *)(TPU_RAM + chan*16 + 0)) = chan*16+3*2+0;
  (*(short int *)(TPU_RAM + chan*16 + 2)) = 0;
  (*(short int *)(TPU_RAM + chan*16 + 4)) = off*2+0;
  (*(short int *)(TPU_RAM + chan*16 + 6)) = on*2+1;
  tpu_set_hsqr(chan, 2);
  tpu_set_hsrr(chan, 2);
  tpu_set_cpr(chan, 3);

  return 0;
}

int pwm_update(int chan, int on, int off) {

  on  = max(0,on);
  off = max(0,off);
  if(off == 0) {
    (*(short int *)(TPU_RAM + chan*16 + 4)) = off*2+0;
  } else {
    (*(short int *)(TPU_RAM + chan*16 + 4)) = off*2+0;
  }
  if(on == 0) {
    (*(short int *)(TPU_RAM + chan*16 + 6)) = on*2+1;
  } else {
    (*(short int *)(TPU_RAM + chan*16 + 6)) = on*2+1;
  }

  return 0;
}

int dio_init(int chan, int state) {

  tpu_set_cpr(chan, 0);
  tpu_set_cfsr(chan, 14);

  state = state & 0x1;
  (*(short int *)(TPU_RAM + chan*16 + 0)) = chan*16+2*2+0;
  (*(short int *)(TPU_RAM + chan*16 + 2)) = 0;
  (*(short int *)(TPU_RAM + chan*16 + 4)) = 10+state;
  tpu_set_hsqr(chan, 0);
  tpu_set_hsrr(chan, 1);
  tpu_set_cpr(chan, 1);

  return 0;
}

int dio_update(int chan, int state) {

  state = state & 0x1;
  (*(short int *)(TPU_RAM + chan*16 + 2)) = 0;
  (*(short int *)(TPU_RAM + chan*16 + 4)) = 10+state;
  tpu_set_hsrr(chan, 1);

  return 0;
}
