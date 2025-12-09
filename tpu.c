
#include "tpu_reg.h"
#include "tpu.h"

int tpu_init(void) {

  TPU_TMCR = 0x00c0;

}

int tpu_set_cier(int chan, int cie) {

  if((chan < 0) || (chan > 15)) return -1;
  if((cie < 0) || (cie > 3)) return -1;
  TPU_CIER = TPU_CIER & (0xffff^(1<<(chan*2))) | (cie<<(chan*2));
}

int tpu_set_hsqr(int chan, int hsq) {

  if((chan < 0) || (chan > 15)) return -1;
  if((hsq < 0) || (hsq > 3)) return -1;
  if(chan > 7) {
    TPU_HSQR0 = TPU_HSQR0 & (0xffff^(3<<((chan-8)*2))) | (hsq<<((chan-8)*2));
  } else {
    TPU_HSQR1 = TPU_HSQR1 & (0xffff^(3<<((chan)*2))) | (hsq<<((chan)*2));
  }
}

int tpu_set_hsrr(int chan, int hsr) {

  if((chan < 0) || (chan > 15)) return -1;
  if((hsr < 0) || (hsr > 3)) return -1;
  if(chan > 7) {
    TPU_HSRR0 = TPU_HSRR0 & (0xffff^(3<<((chan-8)*2))) | (hsr<<((chan-8)*2));
  } else {
    TPU_HSRR1 = TPU_HSRR1 & (0xffff^(3<<((chan)*2))) | (hsr<<((chan)*2));
  }
}

int tpu_set_cpr(int chan, int pri) {

  if((chan < 0) || (chan > 15)) return -1;
  if((pri < 0) || (pri > 3)) return -1;
  if(chan > 7) {
    TPU_CPR0 = TPU_CPR0 & (0xffff^(3<<((chan-8)*2))) | (pri<<((chan-8)*2));
  } else {
    TPU_CPR1 = TPU_CPR1 & (0xffff^(3<<((chan-8)*2))) | (pri<<((chan)*2));
  }
}

int tpu_set_cfsr(int chan, int fn) {

  if((chan < 0) || (chan > 15)) return -1;
  if((fn < 0) || (fn > 15)) return -1;
  if(chan > 11) {
    TPU_CFSR0 = TPU_CFSR0 & (0xffff^(15<<((chan-12)*4))) | (fn<<((chan-12)*4));
  } else if(chan > 7) {
    TPU_CFSR1 = TPU_CFSR1 & (0xffff^(15<<((chan-8)*4))) | (fn<<((chan-8)*4));
  } else if(chan > 3) {
    TPU_CFSR2 = TPU_CFSR2 & (0xffff^(15<<((chan-4)*4))) | (fn<<((chan-4)*4));
  } else {
    TPU_CFSR3 = TPU_CFSR3 & (0xffff^(15<<((chan)*4))) | (fn<<((chan)*4));
  }
}

int tpu_get_cisr(int chan) {

  if((chan < 0) || (chan > 15)) return -1;
  return TPU_CISR & (1<<(chan*2));
}

