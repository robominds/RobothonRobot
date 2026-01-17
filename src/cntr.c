
#include "mssndef.h"
#include "stddef.h"
#include "pwm.h"
#include "nav.h"
#include "guid.h"
#include "mzguid.h"
#include "line.h"
#include "draw.h"
#include "cntr.h"

#define PWMFRQ 240

int cntr_init(void) {

  pwm_init(3,PWMFRQ,PWMFRQ);
  pwm_init(5,PWMFRQ,PWMFRQ);
  dio_init(4,1);
  dio_init(6,1);
  return 0;
}

int cntr_update(void) {
  int pwmpos;
  int pwmang;
  int velcmd=100;
  int dx,dy;
  int headingcmd;
  int headingerr;
  int pwmleft,pwmleftlim;
  int pwmright,pwmrightlim;

  dx = guid_get_pos(0) - nav_get_pos(0);
  dy = guid_get_pos(1) - nav_get_pos(1);

  headingcmd = (900*dy)/(abs(dx)+abs(dy)+1);
  if(dx < 0) headingcmd = 1800*sign(dy) - headingcmd;

  if(mssnguid) {
    headingerr = nav_ang_err(guid_get_headingcmd(),nav_get_ang());
    velcmd = guid_get_velcmd();
    if(guid_get_scanmode() == 0) {
      pwmang = abslim(250,headingerr)*4-3*nav_get_ang_rate()/1;
    } else {
      pwmang = abslim(70,headingerr)*4-3*nav_get_ang_rate()/2;
    }
    if(guid_way_point_done()) {
      pwmpos = -(abslim(200,(guid_get_pos(0)-nav_get_pos(0)))-nav_get_vel());
    } else {
      pwmpos = -(60*signzero(velcmd)+velcmd+(velcmd-nav_get_vel())*5);
    }
  }
  if(mssnmzguid) {
    headingerr = nav_ang_err(mzguid_get_headingcmd(),nav_get_ang());
    velcmd = mzguid_get_velcmd();
    pwmang = abslim(250,headingerr)*4-3*nav_get_ang_rate()/1;
    pwmpos = -(60*signzero(velcmd)+velcmd+(velcmd-nav_get_vel())*5);
  } 
  if(mssnline) {
    headingerr = line_ang_err();
    velcmd = line_velcmd();
    pwmang = abslim(250,headingerr)*4-3*nav_get_ang_rate()/1;
    pwmpos = -(60*signzero(velcmd)+velcmd+(velcmd-nav_get_vel())*5);
  } 
  if(mssndraw) {
    headingerr = nav_ang_err(draw_get_headingcmd(),nav_get_ang());
    velcmd = draw_get_velcmd();
    pwmang = abslim(150,headingerr)*4-3*nav_get_ang_rate()/1;
    pwmpos = -(0*signzero(velcmd)+velcmd+(velcmd-nav_get_vel())*5);
  } 
  if(abs(headingerr) > 300) pwmpos = pwmpos/2;
  if(abs(headingerr) > 900) pwmpos = 0;

  pwmpos = max(-PWMFRQ,pwmpos);
  pwmpos = min(pwmpos,PWMFRQ);
  pwmang = max(-PWMFRQ,pwmang);
  pwmang = min(pwmang,PWMFRQ);
   
  pwmleft     = pwmpos-(pwmang+sign(pwmang)*60);
  pwmleftlim  = 120-sign(pwmleft)*15*nav_get_n(0);
  if((abs(pwmleft) > pwmleftlim) && (pwmleft*nav_get_n(0) > 0)) pwmleft = sign(pwmleft)*pwmleftlim;
  pwmright    = pwmpos+pwmang;
  pwmrightlim = 120-sign(pwmright)*15*nav_get_n(1);
  if((abs(pwmright) > pwmrightlim) && (pwmright*nav_get_n(1) > 0)) pwmright = sign(pwmright)*pwmrightlim;

  pwm_update(3,PWMFRQ-pwmleft,PWMFRQ+pwmleft);
  pwm_update(5,PWMFRQ+pwmright,PWMFRQ-pwmright);
  //pwm_update(3,PWMFRQ-pwmpos+pwmang,PWMFRQ+pwmpos-pwmang);
  //pwm_update(5,PWMFRQ+pwmpos+pwmang,PWMFRQ-pwmpos-pwmang);

  return 0;
}
