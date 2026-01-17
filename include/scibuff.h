

#define scivec 0x56
#define qsize  160
#define RX     0
#define TX     1

typedef unsigned short int word;

typedef struct {
  int in;
  int out;
  char q[qsize];
} queue_struct;

void qinit(void);
int  qstat(int que);
void sciinit(void);
void sci_handler(void);
void sci_int(void);
char rx_byte(char *b);
int  tx_byte(int b);
