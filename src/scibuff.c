
#include "asm.h"
#include "qsm_reg.h"
#include "scibuff.h"

static queue_struct rxbuff,txbuff;

void sci_int(void);

void qinit(void) {

  rxbuff.in   = 0x0000;
  rxbuff.out  = 0x0000;

  txbuff.in   = 0x0000;
  txbuff.out  = 0x0000;
}

int qstat(int que) {
  queue_struct *qvar;

  if(que == RX) {
    qvar = &rxbuff;
  } else {
    qvar = &txbuff;
  }
  if(qvar->in > qvar->out) {
    return(qvar->in-qvar->out);
  } else if(qvar->in < qvar->out) {
    return(qvar->in+qsize-qvar->out);
  } else {
    return(0);
  }
  return 0;
}

char rx_byte(char *rxbyte) {

  if(rxbuff.in != rxbuff.out) {
    *rxbyte = rxbuff.q[rxbuff.out];
    rxbuff.out++;
    if(rxbuff.out >= qsize) rxbuff.out = 0;
    return 1;
  } else {
    *rxbyte = '\0';
    return 0;
  }
  return 0;
}

int tx_byte(int txbyte) {

  if((((txbuff.in+1) != txbuff.out) && ((txbuff.in+1) != (txbuff.out+qsize)))) {
    txbuff.q[txbuff.in] = txbyte;
    if(txbuff.in+1 >= qsize) {
      txbuff.in = 0;
    } else {
      txbuff.in++;
    }
    QSM_SCCR1 = 0x00ac;
    return 1;
  } else {
    return 0;
  }
  return 0;
}

void sciinit(void) {
    
  qinit();

  *(long*)((scivec+0)*4) = (long)sci_int;
  *(long*)((scivec+1)*4) = (long)sci_int;

  QSM_MCR = 0x0087;
  QSM_QILR_QIVR = scivec + 0x0600;
  QSM_SCCR0 = 0x0035;
  QSM_SCCR1 = 0x002c;
}

void sci_handler(void) {
  int status,scidata;

  status = QSM_SCSR;

  if(status & 0x0040) {
    scidata = QSM_SCDR;
    if(status & 0x0008) {
      ;	// Overrun code here
    } else {
      if((rxbuff.in+1 != rxbuff.out) && (rxbuff.in+1 != rxbuff.out+qsize)) {
	rxbuff.q[rxbuff.in] = (char)scidata;
	rxbuff.in++;
	if(rxbuff.in >= qsize) rxbuff.in = 0;
      } else {
	; // buffer overrun code here
      }
    }
  } else if(status & 0x0008) {
    scidata = QSM_SCDR;
  } else if(status & 0x0100) {
    if(txbuff.in != txbuff.out) {
      scidata = txbuff.q[txbuff.out];
      if(txbuff.out+1 >= qsize) {
	txbuff.out = 0;
      } else {
        txbuff.out++;
      }
      QSM_SCDR = scidata;
    } else {
      QSM_SCCR1 = 0x002c;
    }
  } else {
    sciinit();
  }
}

asm(".global sci_int;sci_int:link %a6,#0");
asm("movel %d0,-(%a7);movel %d1,-(%a7);movel %a0,-(%a7)");
asm("jsr sci_handler");
asm("movel (%a7)+,%a0;movel (%a7)+,%d1;movel (%a7)+,%d0");
asm("unlk %a6;rte");
