
#define MAXCNTS 50000
#define MAXSAVE 6000
#define MAXOPS  20

struct times_type {
  float cur_time[MAXOPS];
  float ave_time[MAXOPS];
  float max_time[MAXOPS];
  int   max_fram[MAXOPS];
  float min_time[MAXOPS];
  int   min_fram[MAXOPS];
  float max_rst_time[MAXOPS];
  int   max_rst_fram[MAXOPS];
};

int  mark_time_(int clock, int operation);
void mt_write_(void);
void mt_setup_(char *name, int freq, int max_freq, int wrap);
void mt_set_threshold_(int operation, float threshold, float err_threshold);
void mt_reset_max_(void);
void mt_update_(float times[], int frame, int numops);
void mt_out_(struct times_type *time_mon_out);


