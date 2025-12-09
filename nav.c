#define STARTFILE_SPEC

#include "asm.h"
#include "stdio.h"
#include "qsm_reg.h"
#include "scibuff.h"
#include "fqd.h"
#include "nav.h"
#include "navtbl.h"
#include "stddef.h"

static int ang      = 0;
static int pos[2]   = {0,0};
static int vel      = 0;
static int ang_rate = 0;
static short int count_old[2];
static rotcnt       = 1;
static n[2]         = {0,0};

void navint(void);

int nav_init(void) {

  fqd_init(11);
  fqd_init(13);
  count_old[0] = fqd_get_count(13);
  count_old[1] = fqd_get_count(11);
  ang = 0;
  pos[0] = 0;
  pos[1] = 0;
  vel = 0;
  ang_rate = 0;
  rotcnt = 1;

  return 0;
}

int nav_update(void) {
  short int count[2];
  int delta[2];
  int angtemp;

  count[0] = fqd_get_count(13);
  count[1] = fqd_get_count(11);

  delta[0] = count[0]-count_old[0];
  if(delta[0] > 32767) delta[0] -= 65536;
  if(delta[0] < -32768) delta[0] += 65536;
  delta[1] = count_old[1]-count[1];
  if(delta[1] > 32767) delta[1] -= 65536;
  if(delta[1] < -32768) delta[1] += 65536;
  angtemp = ang + (delta[0]*lres-delta[1]*rres)/2;
  if(angtemp < 0) angtemp += 2*ipi;
  if(angtemp >= 2*ipi) angtemp -= 2*ipi;
  pos[0] = pos[0]+(delta[0]*navcos(angtemp/angres,0)+delta[1]*navcos(angtemp/angres,1))/2;
  pos[1] = pos[1]+(delta[0]*navsin(angtemp/angres,0)+delta[1]*navsin(angtemp/angres,1))/2;
  ang = ang + (delta[0]*lres-delta[1]*rres);
  if(ang < 0) {
    ang += 2*ipi;
    rotcnt -= 2;
  }
  if(ang >= 2*ipi) {
    ang -= 2*ipi;
    rotcnt += 2;
  }
  count_old[0] = count[0];
  count_old[1] = count[1];
  vel += ((delta[0]+delta[1])*lres/2/1000 - vel-sign(vel)*9)/10;
  ang_rate += ((delta[0]*lres-delta[1]*rres) - ang_rate)/3;
  n[0] = delta[0];
  n[1] = delta[1];
}

int nav_get_pos(int axis) {

  return pos[axis]/1000;
}

int nav_get_ang(void) {

  if(ang < ipi) {
    return ang/angres;
  } else {
    return (ang - 2*ipi)/angres;
  }
}

int nav_get_vel(void) {

  return vel;
}

int nav_get_ang_rate(void) {

  return 5*ang_rate/angres;
}

int nav_ang_err(int ang1, int ang2) {
  int angerr;

  angerr = ang1-ang2;
  if(angerr < -ipi/angres) angerr += 2*ipi/angres;
  if(angerr >  ipi/angres) angerr -= 2*ipi/angres;
  return angerr;
}

int nav_get_ipi(void) {

  return ipi;
}

int nav_get_n(int chan) {

  return n[chan];
}

int nav_get_rotcnt(void) {

  return rotcnt;
}

int nav_set_pos(int x, int y) {

  pos[0] = x*1000;
  pos[1] = y*1000;

  return 0;
}
