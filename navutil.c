
#define sincostbl 0

#include "navtbl.h"
#include "navutil.h"

int navsin(int ang, int side) {

  ang = ang%(numentries);
  if(ang < 0) ang += numentries;
  if(side == 0) {
    return lsintbl[ang];
  } else {
    return rsintbl[ang];
  }
}

int navcos(int ang, int side) {

  ang = (ang-numentries/4)%(numentries);
  if(ang < 0) ang += numentries;
  if(side == 0) {
    return -lsintbl[ang];
  } else {
    return -rsintbl[ang];
  }
}
