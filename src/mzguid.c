
#include "stddef.h"
#include "mzguid.h"
#include "nav.h"
#include "dist.h"
#include "nav.h"

#define wptol 3000
#define dtol 0x50

static struct wp_str {
  int heading[4];
  int visitflg;
} wp[21][21];
static int wpinit=0;
static int twpx=11;
static int twpy=10;
static int owpx=10;
static int owpy=10;
static int deadendflg=0;
static int done=0;

int mzguid_init(void) {
  int i,j,k;

  twpx=11;
  twpy=10;
  owpx=10;
  owpy=10;
  deadendflg=1;
  done = 0;
  if((int)(*(unsigned char*)0xfffa13&0x80)) wpinit = 0;
  if(!wpinit) {
    for(i=0;i<21;i++) {
      for(j=0;j<21;j++) {
        for(k=0;k<4;k++) wp[i][j].heading[k] = 1;
        wp[i][j].visitflg = 0;
      }
    }
    wp[owpx][owpy].heading[0] = -2;
  }
  wpinit=1;
  /*
  for(i=0;i<21;i++) wp[i][ 0].heading[3] = 0;
  for(i=0;i<21;i++) wp[i][20].heading[1] = 0;
  for(i=0;i<21;i++) wp[ 0][i].heading[0] = 0;
  for(i=0;i<21;i++) wp[ 0][i].heading[2] = 0;
  */
  nav_set_pos(owpx*24000+12000,owpy*24000);

  return 0;
}

int mzguid_update(void) {
  int heading;
  int iheading;
  int idir=0;

  if((abs(twpx*24000-nav_get_pos(0)) < wptol) &&
     (abs(twpy*24000-nav_get_pos(1)) < wptol)) {
    heading = nav_get_ang();
    if(heading >= 0) {
      iheading = ((heading+450)/900)%4;
    } else {
      iheading = ((3600+heading+450)/900)%4;
    }
    wp[twpx][twpy].heading[(iheading+2)%4] = -1;
    if(deadendflg) wp[twpx][twpy].heading[(iheading+2)%4] = 0;
    deadendflg = 0;
    if(wp[twpx][twpy].heading[iheading] == 1) {
      if(dist_get(0) > dtol) wp[twpx][twpy].heading[iheading] = 0;
    }
    if(wp[twpx][twpy].heading[(iheading+1)%4] == 1) {
      if(dist_get(2) > dtol) wp[twpx][twpy].heading[(iheading+1)%4] = 0;
    }
    if(wp[twpx][twpy].heading[(iheading+3)%4] == 1) {
      if(dist_get(1) > dtol) wp[twpx][twpy].heading[(iheading+3)%4] = 0;
    }
    if(wp[twpx][twpy].heading[(iheading+0)%4] == 1) {
      idir = iheading;
    } else if(wp[twpx][twpy].heading[(iheading+1)%4] == 1) {
      idir = (iheading+1)%4;
    } else if(wp[twpx][twpy].heading[(iheading+3)%4] == 1) {
      idir = (iheading+3)%4;
    } else {
      if(wp[twpx][twpy].heading[(iheading+0)%4] == -1) {
        idir = iheading;
      } else if(wp[twpx][twpy].heading[(iheading+1)%4] == -1) {
        idir = (iheading+1)%4;
      } else if(wp[twpx][twpy].heading[(iheading+3)%4] == -1) {
        idir = (iheading+3)%4;
      } else if(wp[twpx][twpy].heading[(iheading+2)%4] == -1) {
        idir = (iheading+2)%4;
      } else {
	done = 1;
      }
      deadendflg = 1;
    }
    owpx = twpx;
    owpy = twpy;
    wp[owpx][owpy].heading[idir] = -2;
    switch(idir) {
      case 0:
        twpx++;
        break;
      case 1:
        twpy++;
        break;
      case 2:
        twpx--;
        break;
      case 3:
        twpy--;
        break;
      default:
        break;
    }
  }
  return 0;
}

int mzguid_get_velcmd(void) {
  int velcmd;

  velcmd = max(abs(twpx*24000-nav_get_pos(0)),
               abs(twpy*24000-nav_get_pos(1)))/40;
  velcmd = min(300,velcmd)+50;
  if(done) velcmd = 0;
  return velcmd;
}

int mzguid_get_headingcmd(void) {
  int heading0,heading,headingcmd;
  int dx,dy;

  dx = twpx*24000 - owpx*24000;
  dy = twpy*24000 - owpy*24000;
  heading0 = (900*dy)/(abs(dx)+abs(dy)+1);
  if(dx < 0) heading0 = 1800*sign(dy) - heading0;

  dx = twpx*24000 - nav_get_pos(0);
  dy = twpy*24000 - nav_get_pos(1);
  heading = (900*dy)/(abs(dx)+abs(dy)+1);
  if(dx < 0) heading = 1800*sign(dy) - heading;

  headingcmd = heading0 - 3*nav_ang_err(heading0,heading)/2;

  if(done) headingcmd = 0;
  if(headingcmd > 1800) headingcmd -= 3600;
  if(headingcmd <-1800) headingcmd += 3600;

  return headingcmd;
}

int mzguid_way_point_done(void) {

  return 0;
}

