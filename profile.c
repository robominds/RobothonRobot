#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "profile.h"
#include "message.h"

#define COMMAND     0
#define SUB         1
#define LAB         2
#define END         3

static int dbg = 0;
static int profile_wait_flag = 0;
static struct profile_data_type profile_data;

int profile_init(char *filename, int debug) {
  int   i,j,k;
  int   ierr    = 0;
  int   linenum = 0;
  char  line[MAXLINE];
  char  currentlabel[MAXLABEL] = ""; 
  char  currentsub[MAXLABEL]   = "";
  char  templabel[MAXLABEL];
  char *tok;
  FILE *infile;

  dbg = debug;
  for(i=0;i<strlen(filename);i++) filename[i] = tolower(filename[i]);
  message("Loading profile %s",filename);

  profile_data.numlines    = 0;
  profile_data.numsubs     = 0;
  profile_data.subs[0].numlabels = 0;
  profile_data.stack_index = 0;
  profile_data.stack[0].line = 0;
  profile_data.stack[0].num_params = 0;
  profile_data.step_flag   = 0;

  strcpy(profile_data.subs[0].name,currentsub);
  strcpy(profile_data.subs[0].labels[0].name,currentlabel);

  if((infile = fopen(filename,"r")) == NULL) {
    msgperr("profile : fopen failure");
    ierr = -1;
    return ierr;
  }
  while((fgets(profile_data.lines[linenum].text,MAXLINE,infile) != NULL) && 
        (linenum < MAXLINES)) {
    strtok(profile_data.lines[linenum].text,"\n\r");
    linenum++;  
  }
  profile_data.numlines = linenum;
  if(profile_data.numlines >= MAXLINES) {
    message("profile : Too many lines in profile");
    ierr = -1;
    return ierr;
  }
  for(i=0;i<profile_data.numlines;i++) {
    strcpy(line,profile_data.lines[i].text);
    for(j=0;line[j]!='\0';j++) line[j] = toupper(line[j]);
    profile_data.lines[i].type = COMMAND;
    if(line[0] == '#') {
      if(strncmp(&line[1],"SUB",3) == 0) {
        profile_data.lines[i].type = SUB; 
        if(strcmp(currentsub,"") != 0) {
          message("profile - SUB statement in subroutine %s",
              profile_data.subs[profile_data.numsubs-1].name);
          ierr = -1;
          return ierr;
        }
        tok = strtok(&line[5],"( \n\t");
        strcpy(templabel,tok);
        for(k=0;k<profile_data.numsubs;k++) {
          if(strcmp(templabel,profile_data.subs[k].name) == 0) {
            message("profile : Duplicate subroutine name '%s'",templabel);
            ierr = -1;
            return ierr;
          }
        }
        strcpy(profile_data.subs[profile_data.numsubs].name,templabel);
        strcpy(profile_data.subs[profile_data.numsubs].fmt,strtok(0,0));
        profile_data.subs[profile_data.numsubs].numlabels = 0;
        profile_data.subs[profile_data.numsubs].line      = i;
        strcpy(profile_data.subs[profile_data.numsubs].labels[0].name,
            templabel);
        strcpy(currentsub,templabel);
        strcpy(currentlabel,"");
        profile_data.numsubs++;
        profile_data.subs[profile_data.numsubs].numlabels = 0;
      } else if(strncmp(&line[1],"END",3) == 0) {
        profile_data.lines[i].type = END; 
        if(strcmp(currentsub,"") == 0) {
          message("profile - END of undefined subroutine");
          ierr = -1;
          return ierr;
        }
        strcpy(currentsub,"");
        strcpy(currentlabel,"");
      } else {
        profile_data.lines[i].type = LAB; 
        tok = strtok(&line[1]," \n\t");
        strcpy(templabel,tok);
        for(k=0;k<profile_data.subs[profile_data.numsubs-1].numlabels;k++) {
          if(strcmp(templabel,profile_data.subs[profile_data.numsubs-1].
              labels[k].name) == 0) {
            message("profile : Duplicate label '%s'",templabel);
            ierr = -1;
            return ierr;
          }
        }
        strcpy(profile_data.subs[profile_data.numsubs-1].
            labels[profile_data.subs[profile_data.numsubs-1].numlabels].name,
                templabel);
        profile_data.subs[profile_data.numsubs-1].numlabels++;
        strcpy(currentlabel,templabel);
      }
    } else if(line[0] != '!') {
      if(strcmp(currentsub,"") == 0) {
        message("profile - command in undefined subroutine");
        ierr = -1;
        return ierr;
      }
    }
    if(dbg != 0) {
      printf("%4d %2d : %s\n",i,profile_data.lines[i].type,
          profile_data.lines[i].text);
    }
  }
  fclose(infile);
  return 0;
}

int profile_get_next_cmd(char *next_cmd) {
  int   i;
  int   pos;
  char *tok;
  char  stmp1[132],stmp2[132];

  profile_data.step_flag = profile_data.stack[profile_data.stack_index].
      step_flag;
  if((profile_data.step_flag != 0) && (profile_wait_complete() != 0)) {
    if(profile_data.step_flag == 1) profile_data.step_flag = 0;
    if(profile_data.stack[profile_data.stack_index].line < 
        profile_data.numlines) {
      switch(profile_data.lines[profile_data.stack[profile_data.stack_index].
          line].type) {
        case SUB:
        case LAB:
          profile_data.stack[profile_data.stack_index].line++;
          break;
        case END:
          profile_return();
          break;
        default:
          strcpy(next_cmd,profile_data.lines[profile_data.
              stack[profile_data.stack_index].line].text);
          for(i=1;i<=profile_data.stack[profile_data.stack_index].num_params;
              i++) {
            sprintf(stmp1,"$%d",i);
            while((pos=(int)strstr(next_cmd,stmp1)) != NULL) {
              pos = pos-(int)next_cmd;
              strcpy(stmp2,&next_cmd[pos+2]);
              strcpy(&next_cmd[pos],profile_data.
                  stack[profile_data.stack_index].params[i-1]);
              strcat(next_cmd,stmp2);
            }
          }
          profile_data.stack[profile_data.stack_index].line++;
          if(profile_data.stop_at ==
              profile_data.stack[profile_data.stack_index].line) {
            profile_data.stack[profile_data.stack_index].step_flag = 0;
          }
          strtok(next_cmd,"\n\r");
          return 1;
          break;
      }
    }
  }
  return 0;
}

int profile_call(char *sub, char *arglist) {
  int   i,j; 
  int   done = 0;
  int   ierr = 0;
  char *tok;

  for(i=0;(i<profile_data.numsubs) && (done==0);i++) {
    if(strcmp(profile_data.subs[i].name,sub) == 0) {
      profile_data.stack_index++;
      profile_data.stack[profile_data.stack_index].line = 
          profile_data.subs[i].line;
      profile_data.stack[profile_data.stack_index].num_params = 0;
      profile_data.stack[profile_data.stack_index].step_flag  = 
          profile_data.step_flag;
      tok = strtok(arglist," ");
      while(tok != NULL) {
        strcpy(profile_data.stack[profile_data.stack_index].
            params[profile_data.stack[profile_data.stack_index].num_params],
            tok);
        profile_data.stack[profile_data.stack_index].num_params++;
        tok = strtok(0," ");
      }
      done = 1;
    }
  }
  if(done == 0) {
    message("profile - Subroutine '%s' not defined",sub);
    message("          Defined subroutines are:");
    for(j=0;j<profile_data.numsubs;j++)
        message("                %s",profile_data.subs[j].name);
    profile_stop();
    ierr = -1;
  }
  return ierr;
}
 
int profile_return(void) {

  if(profile_data.stack_index > 0) {
    profile_data.stack_index--;
    profile_data.step_flag = profile_data.stack[profile_data.stack_index].
        step_flag;
  } else {
    profile_reset(); 
  }
  return 0;
}

int profile_listsub(char *sub) {
  int i,j;
  int itemp;
  int done = 0;
  int ierr = 0;

  if(strcmp(sub,"CURRENTSUB") == 0) {
    i = profile_data.stack[profile_data.stack_index].line;
    while((profile_data.lines[i].type != SUB) && (i > 0)) i--;
    itemp = i;
    while(done == 0) {
      message("%4d : %s",i-itemp,profile_data.lines[i].text);
      if((profile_data.lines[i].type == END) || (i == profile_data.numlines))
          done = 1; 
      i++;
    }
  } else {
    for(j=0;(j<profile_data.numsubs) && (done==0);j++) {
      if(strcmp(profile_data.subs[j].name,sub) == 0) {
        itemp = profile_data.subs[j].line;
        i = itemp;
        while(done == 0) {
          message("%4d : %s",i-itemp,profile_data.lines[i].text);
          if(profile_data.lines[i].type == END || i == profile_data.numlines)
              done = 1;
          i++;
        }
      }
    }
  }
  if(done == 0) {
    message("profile - Subroutine '%s' not defined",sub);
    message("          Defined subroutines are:");
    for(j=0;j<profile_data.numsubs;j++)
        message("                %s",profile_data.subs[j].name);
    profile_stop();
    ierr = -1;
  }
  return ierr;
}  

int profile_list(void) {
  int i;
  int itemp=0;

  for(i=0;i<profile_data.numlines;i++) {
    if(profile_data.lines[i].type == SUB) itemp = i;
    message("%4d : %s",i-itemp,profile_data.lines[i].text);
  }
  return 0;
}  

int profile_wait_complete(void) {

  return (profile_wait_flag==0); 
}

int profile_inc_wait_flag(void) {

  profile_wait_flag++;
  return 0;
}

int profile_dec_wait_flag(void) {
  int ierr = 0;

  profile_wait_flag--;
  if(profile_wait_flag < 0) {
    profile_wait_flag = 0;
    ierr = -1;
  }
  return ierr;
}

int profile_clr_wait_flag(void) {

  profile_wait_flag = 0;
  return 0;
}

int profile_stop(void) {
  int i;

  profile_data.step_flag = 0;
  for(i=0;i<profile_data.stack_index;i++) {
    profile_data.stack[i].step_flag  = profile_data.step_flag;
  }
  return 0;
}

int profile_skip(int lines) {
  int i;
  int istart,iend;

  i = profile_data.stack[profile_data.stack_index].line;
  while((profile_data.lines[i].type != SUB) && (i > 0)) i--;
  istart = i;
  while((profile_data.lines[i].type != END) && (i < profile_data.numlines)) i++;
  iend = i;
  i = profile_data.stack[profile_data.stack_index].line;
  message("i: %d  lines: %d  istart: %d  iend: %d",i,lines,istart,iend);
  if((lines > 0) && (i + lines < iend)) {
    for(i=0;i<lines;i++) profile_data.stack[profile_data.stack_index].line++;
  } else {
    message("Can not skip %d lines from %d.  Not in range [%d,%d]",lines,i,
        istart,iend);
  }
  return 0;
}

int profile_start_at(int line) {
  int i;
  int istart,iend;

  i = profile_data.stack[profile_data.stack_index].line;
  while((profile_data.lines[i].type != SUB) && (i > 0)) i--;
  istart = i;
  while((profile_data.lines[i].type != END) && (i < profile_data.numlines)) i++;
  iend = i;
  if((line > 0) && (line < iend-istart)) {
    profile_data.stack[profile_data.stack_index].line = istart+line;
  } else {
    message("Can not start at line %d.  Not in range [%d,%d]",line,istart,iend);
  }
  return 0;
}

int profile_stop_at(int line) {
  int i;
  int istart,iend;

  i = profile_data.stack[profile_data.stack_index].line;
  while((profile_data.lines[i].type != SUB) && (i > 0)) i--;
  istart = i;
  while((profile_data.lines[i].type != END) && (i < profile_data.numlines)) i++;
  iend = i;
  if((line > 0) && (line < iend-istart)) {
    profile_data.stop_at = istart+line;
  } else {
    message("Can not stop at line %d, Not in range [%d,%d]",line,istart,iend);
  }
  return 0;
}

int profile_step(void) {

  profile_data.step_flag = 1;
  return 0;
}

int profile_run(void) {
  int i;

  profile_data.step_flag = -1;
  for(i=0;i<=profile_data.stack_index;i++) {
    profile_data.stack[i].step_flag  = profile_data.step_flag;
  }
  return 0;
}

int profile_reset(void) {

  profile_data.step_flag   = 0;
  profile_data.stack_index = 0;
  profile_data.stack[profile_data.stack_index].line       = 0;
  profile_data.stack[profile_data.stack_index].num_params = 0;
  return 0;
}

int profile_continue(void) {

  profile_data.step_flag = -1;
  profile_data.stack[profile_data.stack_index].step_flag = 
      profile_data.step_flag;
  return 0;
}
