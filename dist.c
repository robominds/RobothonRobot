
#include "asm.h"
#include "stdio.h"
#include "scibuff.h"
#include "dist.h"

static int distance[8];

int dist_init(void) {

  *(char*)(0xfffa15) = 0x55;
  *(char*)(0xfffa13) = 0x55;
}

int dist_start(void) {

  if((*(char*)(0xfffa13) & 0x2a)) {
    *(char*)(0xfffa13) = 0x00;
    return 1;
  } else {
    return 0;
  }
}

int dist_ready(void) {

  if((*(char*)(0xfffa13) & 0x2a)) {
    return 1;
  } else {
    return 0;
  }
}

int dist_read(void) {
  int i,j;
  int range[8]={0,0,0,0,0,0,0,0};

  if((*(char*)(0xfffa13) & 0x2a)) {
    *(char*)(0xfffa13) = 0x15;
    for(j=0;j<0;j++);
    for(i=0;i<8;i++) {
      *(char*)(0xfffa13) = 0x00;
      *(char*)(0xfffa13) = 0x15;
      for(j=0;j<0;j++);
      range[0] = range[0]<<1 | (*(char*)(0xfffa13) & 0x20);
      range[1] = range[1]<<1 | (*(char*)(0xfffa13) & 0x08);
      range[2] = range[2]<<1 | (*(char*)(0xfffa13) & 0x02);
    }
    range[0] = range[0]>>5;
    range[1] = range[1]>>3;
    range[2] = range[2]>>1;
    distance[0] = range[0];
    distance[1] = range[1];
    distance[2] = range[2];
  } else {
    distance[0] = -1;
    distance[1] = -1;
    distance[2] = -1;
  }
  return 0;
}

int dist_get(int sensor_num) {

  return distance[sensor_num];
}
