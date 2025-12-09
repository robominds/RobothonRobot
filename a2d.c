
#include "stddef.h"
#include "a2d.h"

static int a2d_chans[8];
static int a2d_chans_f[8];

int a2d_init(void) {

  return 0;
}

int a2d_read(void) {
  int i;

  for(i=1;i<9;i++) {
    a2d_chans[i-1] = (int)*(unsigned char*)(0x100000+(i%8));
    a2d_chans_f[i-1] += (a2d_chans[i-1]-a2d_chans_f[i-1]+sign(a2d_chans[i-1]-a2d_chans_f[i-1])*3)/4;
  }
  return 0;
}

int a2d_chan(int chan) {

  return a2d_chans[chan];
}

int a2d_chan_f(int chan) {

  return a2d_chans_f[chan];
}

