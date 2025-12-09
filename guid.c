
#include "stddef.h"
#include "guid.h"
#include "nav.h"
#include "waypoint.h"
#include "fireplan.h"

static int origin_way_point = 0;
static int target_way_point = 0;
static int way_point_done = 0;
static int way_point_mode = 0;
static int target_cord[2] = {0,0};
static int origin_cord[2] = {0,0};
static int next_type      = 0;
static int step,next_step;
static int scanmode = 0;
static int tmr = 0;
static int ftmr = 0;
static int flameang=0;
static int flamemag=0;
static int scanangs[20]={0,0,1,2,4,5,4,2,1,0,0,0,-1,-2,-4,-5,-4,-2,-1,0};

int guid_init(void) {

  step = 0;
  next_step = way_plan[step].next;
  origin_way_point = way_plan[step].way_point;
  target_way_point = way_plan[next_step].way_point;
  origin_cord[0] = way_points[origin_way_point].cord[0];
  origin_cord[1] = way_points[origin_way_point].cord[1];
  target_cord[0] = way_points[target_way_point].cord[0];
  target_cord[1] = way_points[target_way_point].cord[1];
  way_point_done = 0;
  way_point_mode = 0;
  next_type = way_plan[next_step].type;
  tmr = 0;
  ftmr = 0;
  scanmode = 0;
  dio_init(0,0);
}

int guid_update(void) {

  if((!way_point_done &&
      (abs(way_points[target_way_point].cord[0]-nav_get_pos(0)) < way_plan[next_step].tol) &&
      (abs(way_points[target_way_point].cord[1]-nav_get_pos(1)) < way_plan[next_step].tol)) ||
      (scanmode != 0)) {
    if((way_plan[next_step].type != 1) && (scanmode > -1)) {
      if((abs(flame_mag()) > 0x04) && (scanmode < 10)) {
	scanmode = 10;
	flamemag = abs(flame_mag());
	flameang = nav_get_ang();
      }
      if((abs(flame_mag()) > flamemag) /*|| (abs(flame_mag()) > 0x20)*/) {
	flamemag = max(0x04,3*abs(flame_mag()/4));
	flameang = nav_get_ang();
      }
      switch(scanmode) {
	case 0:
	  ftmr=0;
	  scanmode++;
	  break;
	case 1:
	/*
          if(ftmr++ > 40) {
	    scanmode++;
	    ftmr = 0;
	    scanmode = -1;
	  }
	  */
	  if(abs(nav_ang_err(guid_get_headingcmd(),nav_get_ang())) < 200) {
	    scanmode++;
	  }
	  break;
	case 2:
	  if(abs(nav_ang_err(guid_get_headingcmd(),nav_get_ang())) < 200) {
	    scanmode++;
	  }
	  break;
	case 3:
	  if(abs(nav_ang_err(guid_get_headingcmd(),nav_get_ang())) < 200) {
	    scanmode = -1;
	  }
	  break;
	case 10:
	  tmr = 0;
	  dio_update(0,0);
          if(abs(flame_mag()) > 0x4c) scanmode = 11;
	  ftmr++;
	  break;
	case 11:
	  if(tmr < 100) dio_update(0,1);
	  if(tmr > 100) {
	    dio_update(0,0);
	    if(flame_mag() > 0x10) scanmode=10;
	  }
	  tmr++;
	  ftmr++;
	  if(tmr > 150) scanmode = 12;
	  break;
	case 12:
	  dio_update(0,0);
	  scanmode = -1;
	  way_point_mode = 1;
	  break;
	default:
	  scanmode = -1;
	  break;
      }
    } else {
      scanmode = 0;
      step = next_step;
      if(way_point_mode == 0) {
        next_step = way_plan[step].next;
      } else {
        next_step = way_plan[step].next_ret;
      }
      if(next_step < 0) way_point_done = 1;
      origin_way_point = way_plan[step].way_point;
      target_way_point = way_plan[next_step].way_point;
      origin_cord[0] = way_points[origin_way_point].cord[0];
      origin_cord[1] = way_points[origin_way_point].cord[1];
      target_cord[0] = way_points[target_way_point].cord[0];
      target_cord[1] = way_points[target_way_point].cord[1];
    }
  }
}

int guid_get_pos(int axis) {

  return target_cord[axis];
}

int guid_get_velcmd(void) {
  int velcmd;

  velcmd = max(abs(target_cord[0]-nav_get_pos(0)),
               abs(target_cord[1]-nav_get_pos(1)))/40;
  velcmd = min(300,velcmd)+50;
  if(scanmode != 0) {
    switch(scanmode) {
      case 10:
        if(flame_mag() > 0x04) {
          return min(200,0x400/flamemag);
	} else {
          return 25;
	}
	break;
      case 11:
        return 0;
	break;
      default:
        return 0;
	break;
    }
  } else {
    return velcmd;
  }
}

int guid_get_way_point(void) {

  return step;
}

int guid_get_headingcmd(void) {
  int heading0,heading,headingcmd;
  int dx,dy;

  dx = target_cord[0] - origin_cord[0];
  dy = target_cord[1] - origin_cord[1];
  heading0 = (900*dy)/(abs(dx)+abs(dy)+1);
  if(dx < 0) heading0 = 1800*sign(dy) - heading0;

  dx = target_cord[0] - nav_get_pos(0);
  dy = target_cord[1] - nav_get_pos(1);
  heading = (900*dy)/(abs(dx)+abs(dy)+1);
  if(dx < 0) heading = 1800*sign(dy) - heading;

  headingcmd = heading0 - 3*nav_ang_err(heading0,heading)/2;

  if(way_point_done) {
    return 0;
  } else if(scanmode != 0) {
    switch(scanmode) {
      case 1:
        headingcmd = heading0-1100;
	if(way_plan[next_step].type == 3) headingcmd = heading0-1500;
        //headingcmd = heading0+scanangs[(ftmr%40)/2]*220;
	//if(way_plan[next_step].type == 3) headingcmd = heading0+scanangs[(ftmr%40)/2]*300;
	break;
      case 2:
        headingcmd = heading0;
	break;
      case 3:
        headingcmd = heading0+1100;
	if(way_plan[next_step].type == 3) headingcmd = heading0+1500;
	break;
      case 10:
        headingcmd = flameang+scanangs[ftmr%20]*40;
	break;
      case 11:
        headingcmd = flameang+scanangs[ftmr%20]*40;
        if(tmr>50) headingcmd = flameang+scanangs[ftmr%20]*80;
	break;
      case 12:
        headingcmd = flameang;
	break;
      default:
        headingcmd = heading0;
	break;
    }
  }
  if(headingcmd > 1800) headingcmd -= 3600;
  if(headingcmd <-1800) headingcmd += 3600;
  return headingcmd;
}

int guid_way_point_done(void) {

  return way_point_done;
}

int guid_get_scanmode(void) {

  return scanmode;
}
