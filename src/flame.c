
#include "flame.h"
#include "a2d.h"

int flame_init(void) {

  return 0;
}

int flame_mag(void) {

  //return a2d_chan_f(2)-0x83;
  return a2d_chan_f(1)-0x83;
}

int fan_on(void) {

  return 0;
}

int fan_off(void) {

  return 0;
}
