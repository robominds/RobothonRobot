
#include "mssndef.h"
#include "draw.h"
#include "stddef.h"

static int sweep = 0;
static int osc = 0;
static int done = 0;
static int targx[2]={24000,-5000};
static int targy[2]={0,0};
static int count = 0;

int draw_init(void) {


  sweep = 1;
  osc   = 1;
  done  = 0;
  count = 10;
  pwm_init(0,0,7000);
  return 0;
}

int draw_update(void) {

  if(abs(nav_ang_err(draw_get_headingcmd(),nav_get_ang())) < 125) sweep = (sweep+1)%2;
  if(nav_get_pos(0) > 19000) osc = 0;
  if(nav_get_pos(0) < 0) {
    osc = 1;
    count--;
  }
  if(count < 0) done = 1;
  return 0;
}

int draw_get_velcmd(void) {
  int velcmd;

  /*
  velcmd = max(abs(twpx*24000-nav_get_pos(0)),
               abs(twpy*24000-nav_get_pos(1)))/40;
  velcmd = min(300,velcmd)+50;
  */
  if(osc) {
    velcmd = 10;
  } else {
    velcmd =-10;
  }
  if(done) velcmd = 0;
  return velcmd;
}

int draw_get_headingcmd(void) {
  int heading0,heading,headingcmd;
  int dx,dy;

  dx = targx[0] - targx[1];
  dy = targy[0] - targy[1];
  heading0 = (900*dy)/(abs(dx)+abs(dy)+1);
  if(dx < 0) heading0 = 1800*sign(dy) - heading0;

  dx = targx[0] - nav_get_pos(0);
  dy = targy[0] - nav_get_pos(1);
  heading = (900*dy)/(abs(dx)+abs(dy)+1);
  if(dx < 0) heading = 1800*sign(dy) - heading;

  if(osc) {
    headingcmd = heading0 - 3*nav_ang_err(heading0,heading)/2;
  } else {
    headingcmd = heading0;
  }

  if(sweep) {
    headingcmd -= 375;
  } else {
    headingcmd += 375;
  }
  if(done) headingcmd = 0;
  if(headingcmd > 1800) headingcmd -= 3600;
  if(headingcmd <-1800) headingcmd += 3600;

  return headingcmd;
}

int draw_down(void) {

  pwm_update(0,20,240);
  pwm_update(0,0,240);
  return 0;
}
