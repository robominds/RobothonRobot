#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stddef.h"
#include "shkcmds.h"
#include "cmdparse.h"
#include "message.h"
#include "valset.h"

extern int dbg;

int cmd_save_vals(void *params[]);
int cmd_load_vals(void *params[]);
int cmd_do(void *params[]);
int cmd_set_vals(void *params[]);
int cmd_list_vals(void *params[]);

char *commands[] = {
  "SET:",
    "VALS(%*)",
      (char*)cmd_set_vals,
    " ;",
  "SAVE:",
    "VALS(%*:vals.dat)",
      (char*)cmd_save_vals,
    ";",
  "LOAD:",
    "VALS(%*:Vals.dat)",
      (char*)cmd_load_vals,
    ";",
  "DO(*)",
    (char*)cmd_do,
  "LIST:",
    "VALS(%*:*)",
      (char*)cmd_list_vals,
  ";",
  ""
};

int cmd_save_vals(void *params[]) {

  val_set_save((char*)params[0]);
  return 0;
}

int cmd_load_vals(void *params[]) {

  val_set_load((char*)params[0]);

  return 0;
}

int cmd_do(void *params[]) {
  char str[132];
  FILE *infile;

  if((infile=fopen((char*)params[0],"r")) == NULL) {
    perror("DO Error");
    return -1;
  }
  while(fgets(str,80,infile) != NULL) {
    cmd_prc(0,str);
  }
  fclose(infile);
  return 0;
}

int cmd_set_vals(void *params[]) {

  val_set_prc(1,(char*)params[0]);
  return 0;
}

int cmd_list_vals(void *params[]) {

  val_set_list((char*)params[0]);
  return 0;
}

