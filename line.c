
#include "line.h"
#include "a2d.h"
#include "stddef.h"

static int angerr=0;
static int lineref[5];
static int velcmd=0;
static int linedone=0;
static int countdown=0;
static int headingcmd=-1;
static int angerrcmd=0;

int line_init(void) {
  int i;

  for(i=0;i<20;i++) a2d_read();
  for(i=0;i<5;i++) lineref[i] = a2d_chan(i+2);
  angerr=0;
  velcmd=0;
  linedone=0;
  countdown=25;
  headingcmd=-1;
  angerrcmd=0;

  return 0;
}

int line_chan(int chan) {

  return a2d_chan(chan+2)-lineref[chan];
}

int line_ang_err(void) {
  int i,j;
  int linechan[5];
  int temp;
  int linemax=0x8;
  int index=-1;
  int ldone = 1;

  for(i=0;i<5;i++) linechan[i] = max(0,line_chan(i));
  for(i=0;i<5;i++) {
    if(linechan[i] < 0x08) ldone = 0;
    temp    = linechan[i];
    if(temp > linemax) index=i;
    linemax = max(linemax,temp);
  }
  if(ldone) linedone = 1;
  switch(index) {
    case 0:
      angerr=-200+((100*linechan[1])/(linechan[0]+linechan[1]));
      break;
    case 1:
      angerr= -100+(50*(linechan[2]-linechan[0]))/linechan[1];
      break;
    case 2:
      angerr= (50*(linechan[3]-linechan[1]))/linechan[2];
      break;
    case 3:
      angerr= 100+(50*(linechan[4]-linechan[2]))/linechan[3];
      break;
    case 4:
      angerr= 200-(100*linechan[3])/(linechan[3]+linechan[4]);
      break;
    default:
      angerr=sign(angerr)*200;
      break;
  }
  velcmd = 400;
  if(linedone) {
    if(headingcmd < 0) headingcmd=nav_get_ang();
    return nav_ang_err(headingcmd,nav_get_ang());
  }
  return angerr/2;
}

int line_velcmd(void) {

  if(linedone) countdown--;
  if(countdown < 0) return 0;
  return velcmd;
}
