
#define MSG_TXT 1*256
#define MSG_TXT_NO_TIME MSG_TXT+1
#define MSG_BIN 2*256
#define MAX_PROC_NAME        20
#define MAX_BIN_MSG_SIZE   1000
#define MAX_NUM_BIN_FILES    20
#define BIN_OPEN              1
#define BIN_CLOSE             2
#define BIN_WRITE             3

#ifndef TRUE
  #define TRUE  1
  #define FALSE 0
#endif

#define MAX_LOG_MSG_LEN   255 
#define MAX_PROC_NAME_LEN  32

struct msg_txt_type {
  long   mtype;
  long   mtyp2;
  long   mmode;
  long   mmiss;
  double mtime;
  char   mprog[MAX_PROC_NAME_LEN];
  char   mtext[MAX_LOG_MSG_LEN];
};

struct msg_bin_type {
  long mtype;
  char proc_name[MAX_PROC_NAME];
  long pid;
  long num;
  long data[MAX_BIN_MSG_SIZE];
};


int   msg_init(char *prg, double *time, int out_enable, int debug);
int   msg_txt_init(char *logfile);
int   msg_close(void);        
int   msgperr(char *fmt, ...);
int   message(char *fmt, ...);
int   message_(char *fmt, ...);
int   msgout(char *string);
char* msg_txt_proc(int wait);

