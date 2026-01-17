#define STARTFILE_SPEC

#include "mssndef.h"
#include "asm.h"
#include "stddef.h"
#include "qsm_reg.h"
#include "scibuff.h"
#include "pwm.h"
#include "tpu.h"
#include "dist.h"
#include "nav.h"
#include "guid.h"
#include "mzguid.h"
#include "cntr.h"
#include "line.h"
#include "draw.h"
#include "a2d.h"
#include "stdio.h"

void __vector_default(void);
void illegal_instruction_int(void);
void hz100_int(void);

#define pa0 1314
#define pa1 1539
#define pa2 680
#define pb0 1000
#define pb1 1539
#define pb2 994

static int frame = 0;
static int hz100_flag = 0;
static int hz100_overrun = 0;
static int hz10_flag = 0;
static int hz10_overrun = 0;
static int hz10_step = 0;
static int hz10_inhibit = 0;
static int as=0,s=0,s1=0,s2=0;
static int start=0;

int main(void) {
  int j;
  char rxchar;

  // Initialize all interrrupt vectors to default_int()
  for(j=2;j<256;j++) *(long*)(j*4) = (long)__vector_default;
  *(long*)(4*4) = (long)illegal_instruction_int;
  // Set clock speed to 16.78Mhz
  *(word*)(0xfffa04) = (word)0x7d08;

  frame = -1;
  hz100_flag = 0;
  hz100_overrun = 0;
  hz10_flag = 0;
  hz10_overrun = 0;
  hz10_step = 0;
  hz10_inhibit = 0;
  as=s=s1=s2=0;
  start = 0;

  tpu_init();
  qinit();
  sciinit();
  dist_init();
  //drive_init();
  nav_init();
  if(mssnguid) guid_init();
  if(mssnmzguid) mzguid_init();
  cntr_init();
  if(mssnline) line_init();
  if(mssndraw) draw_init();

  // Inizialize periodic timer for 100 hz interrupt (level 7)
  *(long*)(64*4) = (long)hz100_int;
  *(word*)0xfffa24 = 82;
  *(word*)0xfffa24 = 1;
  *(word*)0xfffa22 = 0x700+64;

  // Enable level 6&7 interrupts
  asm("move.w #0x2500,%sr");


  // Background tasks

  while(1) {

    printf("%x %x %x %x %x ", line_chan(0),line_chan(1),line_chan(2),line_chan(3),line_chan(4));
    //printf("%x %x %x %x ",
    //  dist_get(0),dist_get(1),dist_get(2),(int)(*(unsigned char*)0xfffa13&0x80));
    printf("%x %x %x ",
       nav_get_ang(),nav_get_pos(0),nav_get_pos(1));
    /*
    printf("%x %lx %lx %lx %lx %lx %lx\n %x\r",
      flame_mag(),guid_get_pos(0),guid_get_pos(1),
      nav_get_ang(),nav_get_pos(0),nav_get_pos(1),nav_get_vel(),guid_get_way_point());
      */
    /*printf("%x %x %x %x %x\n\r",line_chan(0),line_chan(1),
        line_chan(2),line_chan(3),line_chan(4));
	*/
    printf("\n\r");
    //drive_set_speed(min(500,50000-nav_get_pos(0)),nav_get_ang_err(0,nav_get_ang())/5);
    if(rx_byte(&rxchar)) {
      while(!tx_byte(rxchar));
      if(rxchar == '\r') while(!tx_byte('\n'));
    }
  }
}

void hz100_handler(void) {
  int ss;

  if(hz100_flag) {
    hz100_overrun++;
    return;
  }
  hz100_flag = 1;

  if(!start) {
    //ss = (int)*(unsigned char*)0x100001-0x80;
    ss = (int)*(unsigned char*)0x100000-0x80;
    s  = s1+ss*pa0;
    s1 = s2+ss*pa1-(s*pb1)/pb0;
    s2 = ss*pa2-(s*pb2)/pb0;
    as += abs(s)-as/100;
    if(as > 0x01800000) {
      start = 1;
      *(word*)0xfffa24 = 82;
    }
    hz100_flag = 0;
    return;
  }

  frame = (frame+1)%100;
  if(frame%10 == 0) hz10_inhibit = 0;

  nav_update();

  hz100_flag = 0;

  if(hz10_inhibit) return;

  a2d_read();

  switch(frame%10) {
    case 0:
      if(hz10_step) {
	hz10_overrun++;
	hz10_inhibit = 1;
	return;
      }
      hz10_step = 0x3ff;
      dist_start();
      break;
    case 1:
    case 5:
    case 3:
    case 7:
    case 9:
      cntr_update();
      break;
    case 4:
      if(mssnguid) guid_update();
      if(mssnmzguid) mzguid_update();
      if(mssndraw) draw_update();
      break;
    case 6:
      if(mssnmzguid) dist_read();
      break;
    default:
      break;
  }
  hz10_step = hz10_step & ~(1<<(frame%10));

}

// default interrupt
//asm(".global default_int;default_int:link %a6,#0;loop:bgnd;jmp loop;unlk %a6;rte");
asm(".global __vector_default;__vector_default:loop:bgnd;jmp loop;rte");
asm(".global illegal_instruction_int;illegal_instruction_int:ill_loop:jmp ill_loop;rte");

// hz100 interrupt
asm(".global hz100_int;hz100_int:link %a6,#0");
asm("moveml %a6-%d0,-(%a7)");
asm("jsr hz100_handler");
asm("moveml (%a7)+,%d0-%a6");
asm("unlk %a6;rte");
