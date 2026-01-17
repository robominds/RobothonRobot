
#include <stdarg.h>

#include "stdio.h"
#include "stddef.h"
#include "scibuff.h"

int printf(const char *format, ...) {
  int i=0;
  char c,*c_p;
  int  j;
  va_list argp;
  int  longflag;

  va_start(argp, format);
  while(format[i] != '\0') {
    switch(format[i]) {
      case '%':
        longflag = 0;
        i++;
	if(format[i] == 'l') {
	  longflag = 1;
	  i++;
	}
	switch(format[i]) {
	  case 'c':
            c = (char)va_arg(argp, int);
            while(!tx_byte(c));
	    break;
	  case 's':
            c_p = (char*)va_arg(argp, int);
            while(*c_p != '\0') {
	      while(!tx_byte(*c_p));
	      c_p++;
	    }
	    break;
	  case 'x':
            j = va_arg(argp, int);
	    if(longflag) {
  	      while(!tx_byte(hextochar((j>>28)&0xf)));
	      while(!tx_byte(hextochar((j>>24)&0xf)));
	      while(!tx_byte(hextochar((j>>20)&0xf)));
	      while(!tx_byte(hextochar((j>>16)&0xf)));
	    }
	    while(!tx_byte(hextochar((j>>12)&0xf)));
	    while(!tx_byte(hextochar((j>> 8)&0xf)));
	    while(!tx_byte(hextochar((j>> 4)&0xf)));
	    while(!tx_byte(hextochar((j)&0xf)));
	    break;
	  case 'd':
            j = va_arg(argp, int);
            while(!tx_byte('?'));
	    break;
	  default:
            va_arg(argp, int);
            while(!tx_byte('?'));
	    break;
	}
        break;
      default:
        while(!tx_byte(format[i]));
        break;
    } 
    i++;
  }
  return 0;
}
