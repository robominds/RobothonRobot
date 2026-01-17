
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "valset.h"

#define MAX_NUM_VALS 200
#define FLOAT  0
#define INT    1
#define LONG   2

static int vals_num = 0;
static struct val_type {
  int  type;
  void *addr;
  union {
    int    ival;
    long   lval;
    float  fval;
  } min,max;
  char name[20];
} vals[MAX_NUM_VALS],temp;

int val_set_init(int dbg) {

  return 0;
}

int val_set_def_int(int *addr, char *name, int min, int max) {
  int i=0;

  if(vals_num < MAX_NUM_VALS) {
    vals[vals_num].type = INT;
    vals[vals_num].addr = (void*)addr;
    vals[vals_num].min.ival = min;
    vals[vals_num].max.ival = max;
    while((name[i] != '\0') && (i<20)) {
      vals[vals_num].name[i] = toupper(name[i]);
      i++;
    }
    vals[vals_num].name[20] = '\0';
    vals_num++;
  } else {
    printf("Max number of VAL_SET variables exceeded\n");
    return -1;
  }
  return 0;
}

int val_set_def_long(long *addr, char *name, long min, long max) {
  int i=0;

  if(vals_num < MAX_NUM_VALS) {
    vals[vals_num].type = LONG;
    vals[vals_num].addr = (void*)addr;
    vals[vals_num].min.lval = min;
    vals[vals_num].max.lval = max;
    while(name[i] != '\0') {
      vals[vals_num].name[i] = toupper(name[i]);
      i++;
    }
    vals_num++;
  } else {
    printf("Max number of VAL_SET variables exceeded\n");
    return -1;
  }
  return 0;
}

int val_set_def_float(float *addr, char *name, float min, float max) {
  int i=0;

  if(vals_num < MAX_NUM_VALS) {
    vals[vals_num].type = FLOAT;
    vals[vals_num].addr = (void*)addr;
    vals[vals_num].min.fval = min;
    vals[vals_num].max.fval = max;
    while(name[i] != '\0') {
      vals[vals_num].name[i] = toupper(name[i]);
      i++;
    }
    vals_num++;
  } else {
    printf("Max number of VAL_SET variables exceeded\n");
    return -1;
  }
  return 0;
}

int val_set_prc(int mode, char *line) {
  int i=0,j=0;
  char *name;
  char *val;
  char str[132];

  j = 0;
  for(i=0;(i<132)&&(line[i]!=NULL);i++) {
    if(!isspace(line[i])) {
      str[j] = line[i];
      j++;
    }
  }
  str[j] = '\0';
  name = strtok(str,"=");
  val  = strtok(0,"\n\r\0");
  for(i=0;i<vals_num;i++) {
    if(strcmp(vals[i].name,name) == 0) {
      switch(vals[i].type) {
	case FLOAT:
	  if((atof(val) >= vals[i].min.fval) &&
	     (atof(val) <= vals[i].max.fval)) {
            *(float*)vals[i].addr = atof(val);
	    if(mode != 0) printf("Variable '%s' = %f\n",name,atof(val));
	  } else {
	    printf("Value %f out of range <%f,%f> for variable '%s'\n",
	        atof(val),vals[i].min.fval,vals[i].max.fval,vals[i].name);
          }
	  break;
	case INT:
	  if((atoi(val) >= vals[i].min.ival) &&
	     (atoi(val) <= vals[i].max.ival)) {
            *(int*)vals[i].addr = atoi(val);
	    if(mode != 0) printf("Variable '%s' = %d\n",name,atoi(val));
	  } else {
	    printf("Value %d out of range <%d,%d> for variable '%s'\n",
	        atoi(val),vals[i].min.ival,vals[i].max.ival,vals[i].name);
          }
	  break;
	case LONG:
	  if((atoi(val) >= vals[i].min.lval) &&
	     (atoi(val) <= vals[i].max.lval)) {
            *(long*)vals[i].addr = atol(val);
	    if(mode != 0) printf("Variable '%s' = %d\n",name,atol(val));
	  } else {
	    printf("Value %d out of range <%d,%d> for variable '%s'\n",
	        atol(val),vals[i].min.lval,vals[i].max.lval,vals[i].name);
          }
	  break;
	default:
	  printf("VAL_SET type not known\n");
	  break;
      }
      break;
    }
  }
  if(i >= vals_num) printf("Variable '%s' not found\n",name);
  return 0;
}

int val_set_list(char *name) {
  int i,j;
  char *str;
  char namemin[80];
  char namemax[80];

  str = strtok(name,"*");
  if(str == NULL) {
    namemin[0] = '\0';
  } else {
    strcpy(namemin,str);
  }
  strcpy(namemax,namemin);
  strcat(namemax,"zzzzzzzzzz");

  for(i=0;i<vals_num;i++) {
    for(j=0;j<vals_num-1;j++) {
      if(strcmp(vals[j].name,vals[j+1].name) > 0) {
	temp = vals[j+1];
	vals[j+1] = vals[j];
	vals[j] = temp;
      }
    }
  }
  for(i=0;i<vals_num;i++) {
    if((strcmp(vals[i].name,namemin) >= 0) &&
       (strcmp(vals[i].name,namemax) <= 0)) {
      switch(vals[i].type) {
	case INT:
          printf("%-20s %8d (i) <%8d,%8d>\n",vals[i].name,*(int*)vals[i].addr,vals[i].min.ival,vals[i].max.ival);
	  break;
	case LONG:
          printf("%-20s %8d (l) <%8d,%8d>\n",vals[i].name,*(int*)vals[i].addr,vals[i].min.lval,vals[i].max.lval);
	  break;
	case FLOAT:
          printf("%-20s %8.5g (f) <%8.5g,%8.5g>\n",vals[i].name,*(float*)vals[i].addr,vals[i].min.fval,vals[i].max.fval);
	  break;
	default:
	  break;
      }
    }
  }
  return 0;
}

int val_set_save(char *filename) {
  int i,j;
  FILE *outfile;

  if((outfile=fopen(filename,"w")) == NULL) {
    perror(filename);
    return -1;
  }
  for(i=0;i<vals_num;i++) {
    for(j=0;j<vals_num-1;j++) {
      if(strcmp(vals[j].name,vals[j+1].name) > 0) {
	temp = vals[j+1];
	vals[j+1] = vals[j];
	vals[j] = temp;
      }
    }
  }
  for(i=0;i<vals_num;i++) {
    switch(vals[i].type) {
      case INT:
        fprintf(outfile,"%-20s=%8d (i) <%8d,%8d>\n",vals[i].name,*(int*)vals[i].addr,vals[i].min.ival,vals[i].max.ival);
	break;
      case LONG:
        fprintf(outfile,"%-20s=%8d (l) <%8d,%8d>\n",vals[i].name,*(int*)vals[i].addr,vals[i].min.lval,vals[i].max.lval);
	break;
      case FLOAT:
        fprintf(outfile,"%-20s=%8.5g (f) <%8.5g,%8.5g>\n",vals[i].name,*(float*)vals[i].addr,vals[i].min.fval,vals[i].max.fval);
	break;
      default:
        break;
    }
  }
  fclose(outfile);
  return 0;
}

int val_set_load(char* filename) {
  char str[132];
  FILE *infile;

  if((infile=fopen(filename,"r")) == NULL) {
    perror(filename);
    return -1;
  }
  while(fgets(str,130,infile) != NULL) {
    val_set_prc(0, strtok(str,"()\n\r"));
  }
  fclose(infile);
  return 0;
}

