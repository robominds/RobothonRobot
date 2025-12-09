#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "cmdparse.h"
#include "message.h"

#define CMDMAXNUM   500
#define CMDMAXLEVEL  10
#define MAXNUMARGS   20

#define TRUE   1
#define FALSE  0

int   cmd_dbg = 0;
int   cmd_items[CMDMAXNUM];
int   cmd_max;
char *cmd_array[CMDMAXNUM][CMDMAXLEVEL];
char *cmd_args[CMDMAXNUM];
int   command_src = 0;
static float fvalue[MAXNUMARGS];
static int   ivalue[MAXNUMARGS];

int cmd_init(char *commands[], int dbg) {
  int   i=0;
  int   j;
  int   cmdnum=0,cmdlevel=0;
  int   del,del2;

  cmd_dbg = dbg;

  while(commands[i][0] != '\0') {
    del = strcspn(commands[i],":(;");
    switch(commands[i][del]) {
      case ':':
        cmd_array[cmdnum][cmdlevel] = strtok(commands[i],":");
        cmdlevel++;
        break;
      case '(':
        cmd_array[cmdnum][cmdlevel] = strtok(commands[i],"%(%");
        if((del2=strcspn(&commands[i][del],")"))!=strlen(&commands[i][del])) {
          message("args error");
        } else {
          cmd_args[cmdnum] = strtok(&commands[i][del+1],")");
        }
        i++;
        cmd_array[cmdnum][cmdlevel+1] = commands[i];
        cmd_items[cmdnum] = cmdlevel;
        if(cmd_dbg != 0) {
          for(j=0;j<cmdlevel+1;j++) message("%s ",cmd_array[cmdnum][j]);
          message("%d",cmd_array[cmdnum][cmdlevel+1]);
        }
        cmdnum++;
        cmd_max = cmdnum;
        for(j=0;j<cmdlevel;j++)
          cmd_array[cmdnum][j] = cmd_array[cmdnum-1][j];
        break;
      case ';':
        if(cmdlevel > 0) {
          cmdlevel--;
        } else {
          message("cmd_init parse error : item %d - %s",i,commands[i]);
        }
        break;
      default:
        message("cmd_init commands syntax : item %d - %s",i,commands[i]);
    }
    i++;
  }
  return 0;
}
 
int cmd_prc(int cmdsrc, char *cmd) {
  int  i,j;
  int  ierr = 0;
  int  ifirst = 0;
  int  not_here;
  int  done = FALSE;
  char cmd_temp[80];
  char *args_string;
  static void *args[20];
  char cmd_arg_temp[512];
  char *temp;

  command_src = cmdsrc;
  if(cmd[strspn(cmd," ")] != '!') {
    for(i=0;(!done) && (i<cmd_max);i++) {
      not_here = FALSE;
      ifirst   = 1;
      strcpy(cmd_temp,cmd);
      for(j=0;cmd_temp[j]!='\0';j++) cmd_temp[j] = toupper(cmd_temp[j]);
      if(cmd_dbg != 0) message("<%s>",cmd_temp);
      for(j=0;!not_here && (j<cmd_items[i]+1);j++) {
        if(cmd_dbg != 0) message("<%s> ",cmd_array[i][j]);
	if(ifirst == 1) {
          temp = strtok(cmd_temp," \t\n");
	} else {
          temp = strtok(0," \t\n");
	}
        if(cmd_dbg != 0) printf("<%s>\n",temp);
        if(strcmp(cmd_array[i][j],temp)  != 0) not_here = TRUE;
        ifirst = 0;
      }
      if(cmd_dbg != 0) message("");
      if(!not_here) {
        args_string = strtok(0,"");
        strcpy(cmd_arg_temp,cmd_args[i]);
        ierr = cmd_prc_args(args,cmd_arg_temp,args_string);
        if(ierr == 0) {
          if(cmd_array[i][cmd_items[i]+1] == 0) {
            ierr = -1;
            message("Function not implemented:");
            message(">>> %s",cmd);
          } else {
            ierr = (*(int (*)(void*))(cmd_array[i][cmd_items[i]+1]))(args);
          }
        }
        done = TRUE;
      }
    }
    if(not_here) {
      message("Incorrect command syntax");
      message("%s",cmd);
      ierr = -1;
    }
  }
  return ierr;
}

int cmd_prc_args(void *args[], char *cmd_args, char *args_string) {
  int  i;
  int   argnum=0;
  int   ierr = 0;
  int imin,imax;
  float fmin,fmax;
  char *tok;
  int  pos;
  char *arg[MAXNUMARGS];
  char *passed[MAXNUMARGS];
  char argtemp[512];
  int  passednum=0;
  static char tempargstring[512];

  strcpy(tempargstring,args_string);
  tok = strtok(cmd_args,";");
  while((tok != 0) && (argnum < MAXNUMARGS)) {
    arg[argnum] = tok;
    arg[argnum] = arg[argnum] + strspn(arg[argnum]," \t\n");
    tok = strtok(0,";");
    argnum++;
  }
  tok = gettok(args_string," \t\n");
  while((tok != 0) && (passednum < MAXNUMARGS)) {
    passed[passednum] = tok;
    passed[passednum] = passed[passednum] + strspn(passed[passednum]," \t\n");
    tok = strtok(0," \t\n");
    passednum++;
  }
  if(argnum < MAXNUMARGS) args[argnum] = 0;
  for(i=0;i<argnum;i++) {
    if(arg[i][0] == '%') {
      switch(arg[i][1]) {
        case 's':
          pos = strcspn(arg[i],":");
          if(arg[i][pos] == ':') {
            args[i]   = &arg[i][pos+1];
          }
          if(i < passednum) {
            args[i] = passed[i];
          }
          break;
        case '*':
          pos = strcspn(arg[i],":");
          if(arg[i][pos] == ':') {
            args[i]   = &arg[i][pos+1];
          }
          if(i < passednum) {
            args[i] = tempargstring + (passed[i]-args_string);
          }
          i = argnum;
          break;
        case 'f':
          pos = strcspn(arg[i],":");
          if(arg[i][pos] == ':') {
            fvalue[i] = atof(&arg[i][pos+1]);
            args[i]   = &fvalue[i];
          }
          if(i < passednum) {
            fvalue[i] = 0.0;
            args[i] = &fvalue[i];
            fvalue[i] = atof(passed[i]);
	    pos = strcspn(arg[i],"[");
            if(arg[i][pos]=='[') {
              if(sscanf(&arg[i][pos],"[%f,%f]",&fmin,&fmax) == 2) {
                if((fvalue[i] >= fmin) && (fvalue[i] <= fmax)) {
                  args[i] = &fvalue[i];
                } else {
                  message("Float value '%f' not in ranger [%f,%f]",
                    fvalue[i],fmin,fmax);
                  ierr = -1;
                }
              } else {
                message("Float field syntax error - '%s'",passed[i]);
                ierr = -1;
              }
            }
          }
          break;
        case 'i':
          pos = strcspn(arg[i],":");
          if(arg[i][pos] == ':') {
            ivalue[i] = atoi(&arg[i][pos+1]);
            args[i]   = &ivalue[i];
          }
          if(i < passednum) {
            ivalue[i] = atoi(passed[i]);
            args[i]   = &ivalue[i];
            pos = strcspn(arg[i],"[");
            if(arg[i][pos]=='[') {
              if(sscanf(&arg[i][pos],"[%d,%d]",&imin,&imax) == 2) {
                if((ivalue[i] >= imin) && (ivalue[i] <= imax)) {
                  args[i] = &ivalue[i];
                } else {
                  message("Integer value '%d' not in ranger [%d,%d]",
                    ivalue[i],imin,imax);
                  ierr = -1;
                }
              } else {
                message("Integer field syntax error - '%s'",passed[i]);
                ierr = -1;
              }
            }
          }
          break;
        case 'd':
          pos = strcspn(arg[i],":");
          if(arg[i][pos] == ':') {
            ivalue[i] = atoi(&arg[i][pos+1]);
            args[i]   = &ivalue[i];
          }
          if(i < passednum) {
	    pos = strcspn(arg[i],"[");
            strcpy(argtemp,&(arg[i][pos]));
            if(cmd_prc_disc(passed[i],argtemp,&(ivalue[i])) == 0) {
              args[i] = &ivalue[i];
            } else {
              message("Discrete list syntax error");
              ierr = -1;
            }
          }
          break;
        default:
          message("Type not supported");
          ierr = -1;
          break;
      }
    } else {
      message("Illegal format");
      ierr = -1;
    }
  }
  return ierr;
}

int cmd_prc_disc(char *str, char *arg_string, int *ivalue) {
  int i;
  int ierr = 0;
  int numdiscs = 0;
  int idone    = FALSE;
  int idisc;
  char *tok1,*tok2;
  char *distok;
  char *discs[20];
  char temp[512]="[";

  if(arg_string[0] == '[') {
    distok = strtok(&arg_string[1],",]");
    while(distok != NULL) {
      discs[numdiscs] = distok;
      numdiscs++;
      distok = strtok(0,",]");
    }
    for(i=0;(i<numdiscs) && (!idone) && (ierr ==0);i++) {
      tok1 = strtok(discs[i],":");
      if(tok1 != NULL) {
        strcat(temp,tok1);
        strcat(temp,",");
        tok2 = strtok(0,",]");
        if(sscanf(tok2,"%d",&idisc) == 1) {
          if(strcmp(str,tok1) == 0) {
            *ivalue = idisc;
            idone   = TRUE;
          }
        } else {
          message("Discrete list syntax error - %s",arg_string);
          ierr = -1;
        }
      } else {
        message("Discrete item list syntax error");
        ierr = -1;
      }
    }
    if(!idone) {
      temp[strlen(temp)-1] = ']';
      message("Discrete '%s' not defined, use %s",str,temp);
      ierr = -1;
    }
  }
  return ierr;
}

char *gettok(char *str, char *tok) {

  return strtok(str,tok);
}
