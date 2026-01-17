
#define MAXLINES 5000
#define MAXLINE   132
#define MAXLABEL  132
#define MAXSTACK   20
#define MAXLABELS 100
#define MAXSUBS   100
#define MAXPARAMS  10

struct profile_data_type {
  int stack_index;
  int step_flag;
  int stop_at;
  struct stack_type {
    int  line;
    int  step_flag;
    int  num_params;
    char params[MAXPARAMS][32];
  } stack[MAXSTACK];
  int numsubs;
  struct sub_type {
    char name[MAXLABEL];
    char fmt[MAXLINE];
    int  line;
    int  numlabels;
    struct label_type {
      char name[MAXLABEL];
      int  line;
    } labels[MAXLABELS];
  } subs[MAXSUBS];
  int numlines;
  struct line_type {
    char text[MAXLINE];
    int  type;
  } lines[MAXLINES];
};

int profile_init(char *filename, int dbg); 
int profile_get_next_cmd(char *next_cmd); 
int profile_call(char *sub, char *args);
int profile_return(void);
int profile_listsub(char *sub);
int profile_list(void);
int profile_wait_complete(void);
int profile_inc_wait_flag(void);
int profile_dec_wait_flag(void);
int profile_clr_wait_flag(void);
int profile_stop(void);
int profile_skip(int lines);
int profile_startat(int line);
int profile_stopat(int line);
int profile_step(void);
int profile_run(void);
int profile_reset(void);
int profile_continue(void);

