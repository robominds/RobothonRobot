
#include "asm.h"
#include "stdio.h"
#include "stddef.h"
#include "pwm.h"
#include "nav.h"
#include "drive.h"

#define PWMFRQ 80

int drive_init(void) {
  int i;

  pwm_init(3,PWMFRQ,PWMFRQ);
  pwm_init(5,PWMFRQ,PWMFRQ);
  dio_init(4,1);
  dio_init(6,1);
}

int drive_set_speed(int vel, int ang_rate) {
  int i;
  int pwmpos;
  int pwmang;
  int pwmcmd;

  pwmpos = (vel-nav_get_vel())/10;
  pwmang = (ang_rate-nav_get_ang_rate())/2;

  pwmpos = max(-PWMFRQ,pwmpos);
  pwmpos = min(pwmpos,PWMFRQ);
  pwmang = max(-(PWMFRQ-abs(pwmpos)),pwmang);
  pwmang = min(pwmang,(PWMFRQ-abs(pwmpos)));
   
  pwm_update(3,PWMFRQ-pwmpos+pwmang,PWMFRQ+pwmpos-pwmang);
  pwm_update(5,PWMFRQ+pwmpos+pwmang,PWMFRQ-pwmpos-pwmang);
}

int drive_set_torque(int t1, int t2) {
  ;
}
