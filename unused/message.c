#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "binary.h"
#include "message.h"

//extern int  errno;

static int     dbg     = 0;
static int     missed_msg = 0;
static int     output_enable = 1;
static char    program[80];
static char    log_name[80];
static double *time_ptr;
static FILE   *logfile;
struct binary_file_struct bin_struct[MAX_NUM_BIN_FILES];
struct bin_id_type bin_id[MAX_NUM_BIN_FILES];

int msg_init(char *prg, double *time, int out_enable, int debug) {

  strcpy(program,prg);
  time_ptr = time;
  dbg      = debug;
  output_enable = out_enable;
  return (0);
}

int msg_txt_init(char *logname) {
  int ierr = 0;

  strcpy(log_name,logname);
  if((logfile = fopen(log_name,"w")) == NULL) {
    msgperr("Error opening log file %s",log_name);
    ierr = -1;
  }
  return ierr;
}

int msg_term(void) {

  if(logfile != NULL) fclose(logfile);
  return 0;
}

int msgperr(char *fmt, ...) {
  char msg[132];
  va_list ap;

  va_start(ap, fmt);
  vsprintf(msg, fmt, ap);
  va_end(ap);
  strcat(strcat(msg,": "),strerror(errno));
  msg_txt_out(msg);
  return 0;
}

int message(char *fmt, ...) {
  char string[132];
  va_list ap;

  va_start(ap, fmt);
  vsprintf(string, fmt, ap);
  va_end(ap);
  msg_txt_out(string);
  return 0;
}

int message_(char *fmt, ...) {
  char string[132];
  va_list ap;

  va_start(ap, fmt);
  vsprintf(string, fmt, ap);
  va_end(ap);
  msg_txt_out(string);
  return 0;
}

int msg_txt_out(char *line) {
  char string[256];
  struct msg_txt_type msg;

  msg.mtype = MSG_TXT;
  msg.mmiss = missed_msg;
  if(time_ptr != 0) {
    msg.mtyp2 = 0;
    msg.mtime = *time_ptr;
  } else {
    msg.mtyp2 = 1;
    msg.mtime = 0.0;
  }
  strcpy(msg.mprog,program);
  strcpy(msg.mtext,line);
  sprintf(string,"%9.2f:%10s - %s",msg.mtime,msg.mprog,msg.mtext);
  printf("%s\n",string);
  if(logfile != 0) fprintf(logfile,"%s\n",string);
  return 0;
}

char* msg_txt_proc(int wait) {
  int    msg_size=0;
  int    ierr        = 0;
  static char   string[256] = "";
  static struct msg_txt_type txt_msg;

  if(msg_size > 0) { 
    if((txt_msg.mtyp2 == 1) && (time_ptr != NULL)) txt_msg.mtime = *time_ptr;
    if(txt_msg.mmiss > 0) {
      sprintf(string,"%9.2f:%10s - %d missed messages\n",txt_msg.mtime,
          txt_msg.mprog,txt_msg.mmiss);
      if(output_enable) printf(string);
      if(logfile != 0) fprintf(logfile,string);
    }
    sprintf(string,"%9.2f:%10s - %s\n",txt_msg.mtime,txt_msg.mprog,
        txt_msg.mtext);
    if(output_enable) printf(string);
    if(logfile != 0) fprintf(logfile,string);
    ierr = 1;
  }
  return ((msg_size > 0) ? string : NULL);
}

int msg_bin_proc(int wait) {
/*
  int  msg_size=0;
  struct msg_bin_type bin_msg;

  if(msg_size > 0) {
    if(bin_msg.mtype == 0) {
      printf("%s: Received termination message from %s\n",bin_msg.proc_name);
      fprintf(logfile,"%s: Received termination message from %s\n",
          bin_msg.proc_name);
    } else {
      bin_msg_handler(&bin_msg);
    }
  }
*/
  return 0;
}

void bin_close(void) {
  int  i;

  printf("%s: Terminating Binary Message process\n",program);
  for(i=0;i<MAX_NUM_BIN_FILES;i++) {
    if(is_bin_open_(&bin_struct[i]) == TRUE) bin_close_(&bin_struct[i]);
  }
}

int bin_msg_handler(struct msg_bin_type *bin_msg) {
  int i;
  int done=FALSE;

  switch(bin_msg->mtype) {
    case BIN_OPEN:
      for(i=0;(i<MAX_NUM_BIN_FILES) && (!done);i++) {
        if((bin_id[i].pid == bin_msg->pid) && (bin_id[i].num == bin_msg->num)) {
          printf("%s: PID = %d, NUM = %d - All ready allocated\n",
              bin_msg->pid,bin_msg->num);
          done = TRUE;
        }
      }
      if(!done) {
        for(done=FALSE,i=0;(i<MAX_NUM_BIN_FILES) && (!done);i++) {
          if(bin_struct[i].file_num == 0) {
            bin_define_(((struct msg_open_bin_type *)bin_msg)->file_name,
                        ((struct msg_open_bin_type *)bin_msg)->param_names,
                        ((struct msg_open_bin_type *)bin_msg)->param_types,
                        &bin_struct[i]);
            bin_open_(&bin_struct[i]);
            bin_write_header_(&bin_struct[i]);
            bin_id[i].pid = bin_msg->pid;
            bin_id[i].num = bin_msg->num;
            done = TRUE;
          }
        }
        if(!done) {
          printf("%s: Attempt to open too many Binary files by process %s\n",
            program,bin_msg->proc_name);
        }
      }
      break;
    case BIN_CLOSE:
      for(i=0;(i<MAX_NUM_BIN_FILES) && (!done);i++) {
        if((bin_id[i].pid == bin_msg->pid) && (bin_id[i].num == bin_msg->num)) {
          bin_id[i].pid = 0;
          bin_id[i].num = 0;
          bin_update_num_rec_(&bin_struct[i]);
          bin_close_(&bin_struct[i]);
          done = TRUE;
        }
      }
      break;
    case BIN_WRITE:
      for(i=0;(i<MAX_NUM_BIN_FILES) && (!done);i++) {
        if((bin_id[i].pid == bin_msg->pid) && (bin_id[i].num == bin_msg->num)) {
          bin_id[i].pid = 0;
          bin_id[i].num = 0;
          bin_update_num_rec_(&bin_struct[i]);
          bin_close_(&bin_struct[i]);
          done = TRUE;
        }
      }
      break;
    default:
      printf("%s: Unhandled bin record message, type = %ld\n",bin_msg->mtype);
      break;
  }
  return 0;
}
