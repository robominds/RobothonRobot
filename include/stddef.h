
#define FALSE 0
#define TRUE  1

#define min(a,b) ((a)<(b)?(a):(b))
#define max(a,b) ((a)>(b)?(a):(b))
#define sign(a)  ((a)<0?(-1):(1))
#define signzero(a) ((a)<0?(-1):((a)==0?(0):(1)))
#define abs(a)   ((a)<0?(-(a)):(a))
#define abslim(a,b) ((b)>(a)?(a):((b)<(-(a))?(-(a)):(b)))

#define hextochar(a) ((a)>9?((a)-10+'A'):((a)+'0'))
