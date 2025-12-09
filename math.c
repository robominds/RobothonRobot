
#include "math.h"
#include "sincostbl.h"

int cos(int ang) {

  return costbl[ang];
}

int sin(int ang) {

  return sintbl[ang];
}
