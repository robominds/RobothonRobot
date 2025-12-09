
#define MAX_NUM_PARAMS     32
#define MAX_PARAM_NAME_LEN 8
#define MAX_FILE_NAME_LEN  32

struct binary_file_struct {
  long file_type;
  long file_num;
  char file_name[MAX_FILE_NAME_LEN];
  long first_record;
  long last_record;
  long current_record;
  long num_params;
  long param_type[MAX_NUM_PARAMS];
  char param_name[MAX_NUM_PARAMS][MAX_PARAM_NAME_LEN];
  void* data_ptr;
};

struct msg_open_bin_type {
  char file_name[MAX_FILE_NAME_LEN];
  char param_names[MAX_NUM_PARAMS][MAX_PARAM_NAME_LEN];
  long param_types[MAX_NUM_PARAMS];
};

struct bin_data_type {
  double timeref;
  long data[MAX_NUM_PARAMS];
};

struct msg_write_bin_type {
  struct bin_data_type data;
};

struct bin_id_type {
  long pid;
  long num;
};

int bin_write_(struct binary_file_struct *bin_struct);
int bin_write_header_(struct binary_file_struct *bin_struct);
int bin_update_num_rec_(struct binary_file_struct *bin_struct);
int bin_write_data_rec_(struct binary_file_struct *bin_struct, long *data);
struct binary_file_struct *bin_define_(char *filename, 
    char param_names[][MAX_PARAM_NAME_LEN],
    long param_types[],struct binary_file_struct *bin_struct);
int bin_open_(struct binary_file_struct *bin_struct);
int bin_close_(struct binary_file_struct *bin_struct);
int is_bin_open_(struct binary_file_struct *bin_struct);

