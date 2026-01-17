#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "binary.h"

int bin_write_(struct binary_file_struct *bin_struct) {

  bin_open_(bin_struct);

  bin_write_header_(bin_struct);

  bin_close_(bin_struct);

  return(1);
}

int bin_open_(struct binary_file_struct *bin_struct) {

  bin_struct->file_num = open(bin_struct->file_name, O_WRONLY | O_CREAT,
                              S_IREAD | S_IWRITE);
  return(1);
}

int bin_close_(struct binary_file_struct *bin_struct) {

  close(bin_struct->file_num);
  return(1);
}

int is_bin_open_(struct binary_file_struct *bin_struct) {
  return (bin_struct->file_num != 0);
}
int bin_write_header_(struct binary_file_struct *bin_struct) {
  int    i;
  int    reclen;
  char   string[8+4*MAX_NUM_PARAMS];
  char   spaces[MAX_PARAM_NAME_LEN];

  for(i=0;i<MAX_PARAM_NAME_LEN;i++) spaces[i] = ' ';

  reclen = 8+4*bin_struct->num_params;
  for(i=0;i<(MAX_NUM_PARAMS+2);i++) *(long*)&(string[i*4]) = 0;
  *(long*)&(string[0]) = 6;
  *(long*)&(string[4]) = bin_struct->num_params;
  write(bin_struct->file_num,string,reclen);
  lseek(bin_struct->file_num,2*reclen,SEEK_SET);
  strncpy(string,"TIMEREF ",MAX_PARAM_NAME_LEN);
  *(long*)&(string[MAX_PARAM_NAME_LEN]) = 2;
  write(bin_struct->file_num,string,reclen);
  for(i=0;i<bin_struct->num_params;i++) {
    strncat(strncpy(string,bin_struct->param_name[i],MAX_PARAM_NAME_LEN),
        spaces,MAX_PARAM_NAME_LEN-strlen(string));
    *(long*)&(string[MAX_PARAM_NAME_LEN]) = bin_struct->param_type[i];
    write(bin_struct->file_num,string,reclen);
  }
  bin_struct->last_record = 3+bin_struct->num_params;
  bin_update_num_rec_(bin_struct);
  return(1);
}

int bin_update_num_rec_(struct binary_file_struct *bin_struct) {
  long i;
  long reclen;
  char string[8+4*MAX_NUM_PARAMS];

  reclen = 8+4*bin_struct->num_params;
  for(i=0;i<(MAX_NUM_PARAMS+2);i++) *(long*)&(string[i*4]) = 0;
  lseek(bin_struct->file_num,1*reclen,SEEK_SET);
  for(i=0;i<(MAX_NUM_PARAMS+2);i++) *(long*)&(string[i*4]) = 0;
  *(long*)&(string[0]) = bin_struct->last_record;
  write(bin_struct->file_num,string,reclen);
  return(1);
}

int bin_write_data_rec_(struct binary_file_struct *bin_struct, long *data) {
  long reclen;

  reclen = 8+4*bin_struct->num_params;
  lseek(bin_struct->file_num,bin_struct->last_record*reclen,SEEK_SET);
  write(bin_struct->file_num,data,reclen);
  bin_struct->last_record++;
  bin_struct->current_record = bin_struct->last_record;
  return(1);
}

struct binary_file_struct *bin_define_(char *file_name,
     char param_names[][MAX_PARAM_NAME_LEN],
     long param_types[],struct binary_file_struct *bin_struct) {
  int i;
  int num_params = 0;
  char spaces[MAX_PARAM_NAME_LEN];

  for(i=0;i<MAX_PARAM_NAME_LEN;i++) spaces[i] = ' ';

  bin_struct->file_type = 6;
  bin_struct->file_num  = 0;
  strncpy(bin_struct->file_name,file_name,MAX_FILE_NAME_LEN-1);
  while((num_params < MAX_NUM_PARAMS) && (param_names[num_params][0] != '\0')) {
    strncpy(bin_struct->param_name[num_params],param_names[num_params],
        MAX_PARAM_NAME_LEN);
    strncat(bin_struct->param_name[num_params],spaces,
        MAX_PARAM_NAME_LEN-strlen(bin_struct->param_name[num_params]));
    bin_struct->param_type[num_params] = param_types[num_params];
    num_params++;
  }
  bin_struct->num_params     = num_params;
  bin_struct->first_record   = num_params + 3;
  bin_struct->last_record    = num_params + 2;
  bin_struct->current_record = bin_struct->first_record;
  return(bin_struct);
}

