
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/timers.h>
#include <sys/cintrio.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "marktime.h"

float  mt_threshold[MAXOPS];
float  mt_err_threshold[MAXOPS];
int    mt_thres_exceeded = 0;
int    mt_error_exceeded = 0;
int    mt_wrap           = 0;
int    mt_freq           = 0;
int    mt_max_freq       = 0;
int    mt_savecnt        = 0;
int    mt_maxindex       = 1;
double mt_timeref0       = 0.0;
double mt_time0          = 0.0;
double mt_time1          = 0.0;
char   mt_file_name[40]  = "time.bin";

struct times_type time_mon;

struct save_time_type {
  double time;
  float  timeref;
  int    frame;
  float  deltime[MAXOPS];
} save_time[MAXCNTS];

int mark_time_(int clock,int operation) {
  static int ifirst   = 1;
  int    i,j;
  struct timestruc time;

  if(ifirst <= 0) {
    if((operation < MAXOPS) && (operation > 0)) {
      if(operation > mt_maxindex) mt_maxindex = operation;
      gettimer(TIMESINCEBOOT,&time);
      mt_time1 = time.tv_sec*1000.0 + time.tv_nsec/1000000.0;
      save_time[mt_savecnt].deltime[operation-1] = mt_time1 - mt_time0;
      if(mt_threshold[operation-1] >= 0.0) {
        if((save_time[mt_savecnt].deltime[operation-1] >
            mt_threshold[operation-1]) ||
           (save_time[mt_savecnt].deltime[operation-1] < 0)) {
          mt_thres_exceeded = 1;
        }
      }
      if(mt_err_threshold[operation-1] >= 0.0) {
        if((save_time[mt_savecnt].deltime[operation-1] > 
            mt_err_threshold[operation-1]) || 
           (save_time[mt_savecnt].deltime[operation-1] < 0)) {
          mt_error_exceeded = 1;
        }
      }
    }
  }
  if(operation == 1) {
    if(ifirst == 0) {
      mt_update_(&(save_time[mt_savecnt].deltime[0]),clock,mt_maxindex);
    }
    gettimer(TIMESINCEBOOT,&time);
    mt_time0 = time.tv_sec*1000.0 + time.tv_nsec/1000000.0;
    if (ifirst == 1) mt_timeref0 = mt_time0;
    if((mt_thres_exceeded == 1) || (ifirst == 1)) {
      mt_thres_exceeded = 0;
      if(mt_savecnt <  MAXCNTS) {
        mt_savecnt++;
      } else {
        if(mt_wrap > 0) {
          mt_savecnt = 0;
          mt_wrap++;
        }
      }
    }
    if(ifirst > 0) ifirst--;
    save_time[mt_savecnt].frame   = clock % mt_freq;
    save_time[mt_savecnt].time    = ((float)clock)/(float)mt_freq;
    save_time[mt_savecnt].timeref = mt_time0 - mt_timeref0;
    for(i=0;i<mt_maxindex;i++) {
      save_time[mt_savecnt].deltime[i] = 0.0;
    }
  }
  if(mt_error_exceeded == 1) {
    mt_error_exceeded = 0;
    return 1;
  } else {
    return 0;
  }
}

void mt_write_(void) {
  int    i;
  int    recordnum;
  int    reclen;
  int    binfile;
  char   string[16+4*MAXOPS];

  if((binfile = creat(mt_file_name,0644)) < 0) {
    msgperr("%s create failure",mt_file_name);
    exit(0);
  }
  reclen = 16+4*mt_maxindex;
  for(i=0;i<(MAXOPS+4);i++) *(int*)&(string[i*4]) = 0;
  *(int*)&(string[0]) = 6;
  *(int*)&(string[4]) = mt_maxindex+2;
  write(binfile,string,reclen);
  lseek(binfile,2*reclen,SEEK_SET);
  sprintf(string,"TIME    ");
  *(int*)&(string[8]) = 2;
  write(binfile,string,reclen);
  sprintf(string,"TIMEREF ");
  *(int*)&(string[8]) = 0;
  write(binfile,string,reclen);
  sprintf(string,"FRAME   ");
  *(int*)&(string[8]) = 1;
  write(binfile,string,reclen);
  for(i=1;i<=mt_maxindex;i++) {
    sprintf(string,"TDELT%2.2d ",i);
    *(int*)&(string[8]) = 0;
    write(binfile,string,reclen);
  }
  recordnum = 5+mt_maxindex;
  if(mt_wrap > 1) {
    for(i=mt_savecnt+1;i<MAXCNTS;i++) {
      recordnum = recordnum+1;
      write(binfile,&(save_time[i]),reclen);
    }
    for(i=0;i<mt_savecnt;i++) {
      recordnum = recordnum+1;
      write(binfile,&(save_time[i]),reclen);
    }
  } else {
    for(i=0;i<mt_savecnt;i++) {
      recordnum = recordnum+1;
      write(binfile,&(save_time[i]),reclen);
    }
  }
  lseek(binfile,reclen,SEEK_SET);
  *(int*)&(string[0]) = recordnum;
  *(int*)&(string[4]) = 0;
  write(binfile,string,reclen);
/*
  close(binfile);
*/
}

void mt_setup_(char *name, int freq, int max_freq, int wrap) {
  int i;

  strncpy(mt_file_name,name,40);
  mt_wrap     = wrap;
  mt_freq     = freq;
  mt_max_freq = max_freq;
  for(i=0;i<MAXOPS;i++) {
    mt_threshold[i]     = -1;
    mt_err_threshold[i] = -1;
  }
}

void mt_set_threshold_(int operation, float threshold, float err_threshold) {

  mt_threshold[operation-1]     = threshold;
  mt_err_threshold[operation-1] = err_threshold;
}

void mt_update_(float times[], int frame, int numops) {
  static int saveat = 0;
  static int max_fram[MAXOPS],min_fram[MAXOPS];
  static float max_time[MAXOPS],min_time[MAXOPS];
  static float time_save[MAXOPS][MAXSAVE],time_total[MAXOPS];
  int i;

  saveat = (saveat%MAXSAVE)+1;

  for(i=0;i<numops;i++) {
    time_mon.cur_time[i] = times[i];
    time_total[i] = time_total[i] - time_save[i][saveat] + times[i];
    time_mon.ave_time[i] = time_total[i]/MAXSAVE;
    time_save[i][saveat] = times[i];
    if((times[i] > time_mon.max_rst_time[i]) && (frame != 1)) {
      time_mon.max_rst_time[i] = times[i];
      time_mon.max_rst_fram[i] = frame;
    }
    if(times[i] > max_time[i]) {
      max_time[i] = times[i];
      max_fram[i] = frame % mt_freq;
    }
    if(times[i] < min_time[i]) {
      min_time[i] = times[i];
      min_fram[i] = frame % mt_freq;
    }
    if((frame % mt_max_freq) == 0) {
      time_mon.max_time[i] = max_time[i];
      time_mon.max_fram[i] = max_fram[i];
      time_mon.min_time[i] = min_time[i];
      time_mon.min_fram[i] = min_fram[i];
      max_time[i] = 0.0;
      max_fram[i] = -1;
      min_time[i] = 999999.9;
      min_fram[i] = -1;
    }
  }
}

void mt_reset_max_(void) {
  int i;

  for(i=0;i<MAXOPS;i++) {
    time_mon.max_rst_time[i] = 0.0;
    time_mon.max_rst_fram[i] = 0;
  }
}

void mt_out_(struct times_type *time_mon_out) {

  *time_mon_out = time_mon;
}
