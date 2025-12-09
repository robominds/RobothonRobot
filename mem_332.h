/***************************************************************************
	This source code is part of Anarchboot Multitasking Kernal,
	Copyright 1994, Anne Wright, anarch@ai.mit.edu
	This copy is not to be distributed in any form
	and is to be used only with express permission of the author.
****************************************************************************/
	
/* MEMORY MAPS AND TYPES for 68332, version 2.1 */

#ifndef __mem_332__
#define __mem_332__

/* memory access functions */
#define bget(x) *(unsigned volatile char*)(x)
#define bput(x,y) *(unsigned char*)(x)=((unsigned char)y)

#define mget(x) *(unsigned volatile short*)(x)
#define mput(x,y) *(unsigned short*)(x)=((unsigned short)y)

#define lget(x) *(unsigned volatile long*)(x)
#define lput(x,y) *(unsigned long*)(x)=((unsigned long)y)

#define error(msg) printf("Error: line %d, file %s = %s\n",\
				__LINE__,__FILE__,msg)

/* there are 180 bytes of registers on a 68332 w/68881      */
/* many of the fpa registers are 12 byte (96 bit) registers */
#define GDB_NUMREGBYTES 180
#define NUMREGBYTES 72

enum regnames {D0,D1,D2,D3,D4,D5,D6,D7, 
               A0,A1,A2,A3,A4,A5,A6,A7, 
               PS,PC,
               FP0,FP1,FP2,FP3,FP4,FP5,FP6,FP7,
               FPCONTROL,FPSTATUS,FPIADDR
              };

/****************************** setjmp *****************************/
#define _SETJMPLEN 14

typedef int jmp_buf[_SETJMPLEN];

/******************** system integration module *********************/
/* the following map copied from 68332 users manual page b-1 */
typedef volatile struct simmap {
  word mcr;
  word simtr;
  struct syncr_struct {
    uint w :1;
    uint x :1;
    uint y :6;
    uint ediv :1;
    uint   :2;
    volatile uint slimp :1;
    volatile uint slock :1;
    uint rsten :1;
    uint stsim :1;
    uint stext :1;} syncr;
  byte u1;  byte rsr;
  word simtre;
  uint u2;

  word u3;
  byte u4;   volatile byte porte;
  byte u5;   volatile byte eport;
  byte u6;   byte ddre;
  byte u7;   byte pepar;
  byte u8;   volatile byte portf;
  byte u9;   volatile byte fport;
  byte u10;   byte ddrf;
  byte u11;   byte pfpar;
  byte u12;   byte sypcr;
  struct picr_struct {
    uint     :5;
    uint pirq:3;
    uint piv :8;} picr;
  struct pitr_struct {
    uint     :7;
    uint ptp :1;
    uint pitr:8;} pitr;
  byte u13;   byte swsr;
  word ua28;
  word ua2a;
  word ua2c;
  word ua2e;
  word tstmsra; /* a30 */
  word tstmsrb;
  byte tstsca;   byte tstscb;
  word tstrc;
  word creg;
  word dreg;
  uint u15;
  byte u16;   byte cspdr;} *simptr;

extern const simptr sim;
#define sim ((simptr)0xfffa00)

/*************************** chip select module *****************/
/*************************** chip select module *****************/
enum cs_pin {output,default_config,cs_8bit,cs_16bit};
enum cs_size {s2k,s8k,s16k,s64k,s128k,s256k,s512k,s1m}; /* block_size */
enum cs_mode {async,sync};
enum cs_byte {off,lower,upper,both};
enum cs_rw {read=1,write,rw};
enum cs_strobe {as,ds};
enum cs_space {cpu,user,supervisor,u_s};
enum cs_avec {avec_off,avec_on};

struct chip_select_channel {
    uint addr :13;
    enum cs_size blksz :3;
    enum cs_mode mode :1;
    enum cs_byte byte :2;
    enum cs_rw rw :2;
    enum cs_strobe strb :1;
    uint dsack :4;
    enum cs_space space :2;
    uint ipl :3;
    enum cs_avec avec :1;};

typedef struct chip_select_map {
  struct cspar_struct {
    uint   :2;
    uint cs5 :2;
    uint cs4 :2;
    uint cs3 :2;
    uint cs2 :2;
    uint cs1 :2;
    uint swsr1 :2;
    uint swsr2 :2;  /* end of first word */
    
    uint   :6;
    uint cs10 :2;
    uint cs9 :2;
    uint cs8 :2;
    uint cs7 :2;
    uint cs6 :2;} cspar;
  
  struct chip_select_channel boot;
  struct chip_select_channel csch[10];} *csmptr;

extern const csmptr csm;
#define csm ((csmptr)0xfffa44)

/**************************** serial and baud control*****************/
/* serial module defines from 68332 user manual page c1 */
struct synch_ser_comnd {
  uint cont   :1;
  uint bitse  :1;
  uint dt     :1;
  uint dsck   :1;
  uint psc3   :1;
  uint psc2   :1;
  uint psc1   :1;
  uint psc0ss :1;};

typedef volatile struct queued_serial_module {
  struct qmcr_struct {
    uint stop :1;
    uint frz1 :1;
    uint frz0 :1;
    uint      :5;
    uint supv :1;
    uint      :3;
    uint iarb :4;} qmcr;
  word qtest;
  struct qilr_struct {
    uint        :2;
    uint ilqspi :3;
    uint ilsci  :3;
    uint qivr   :8;} qir;
  word r0;
  word scbr;  /* baud control register */
  struct sccr_struct {
    uint    :1;
    uint loops :1;
    uint woms  :1;
    uint ilt   :1;
    uint pt    :1;
    uint pe    :1;
    uint m     :1;
    uint wake  :1;
    uint tie   :1;
    uint tcie  :1;
    uint rie   :1;
    uint ilie  :1;
    uint te    :1;
    uint re    :1;
    uint rwu   :1;
    uint sbk   :1;} sccr;
  volatile struct scsr_struct {
    uint      :7;
    volatile uint tdre :1;
    volatile uint tc   :1;
    volatile uint rdrf :1;
    volatile uint raf  :1;
    volatile uint idle :1;
    volatile uint or   :1;
    volatile uint nf   :1;
    volatile uint fe   :1;
    volatile uint pf   :1;} scsr;
  volatile word scdr;
  uint r1;
  byte r2;    byte qpdr;
  union qpar_qddr_union {
    struct qpar_struct {
      uint       :1;
      uint pcs  :3;
      uint pcs0ss :1;
      uint       :1;
      uint mosi  :1;
      uint miso  :1;
      uint       :8;} qpar;
    struct qddr_struct {
      uint       :8;
      uint txd   :1;
      uint pcs   :3;
      uint pcs0ss :1;
      uint sck   :1;
      uint mosi  :1;
      uint miso  :1;} qddr;
  } qpar_qddr;
  struct spcr0_struct {
    uint mstr    :1;
    uint womq    :1;
    uint bits    :4;
    uint cpol    :1;
    uint cpha    :1;
    uint spbr    :8;} spcr0;
  struct spcr1_struct {
    uint spe     :1;
    uint dsckl   :7;
    uint dtl     :8;} spcr1;
  struct spcr2_struct {
    uint spifie  :1;
    uint wren    :1;
    uint wrto    :1;
    uint         :1;
    uint endqp   :4;
    uint         :4;
    uint newqp   :4;} spcr2;
  struct spcr3_spsr_struct {
      uint       :5;
      uint loopq :1;
      uint hmie  :1;
      uint halt  :1;
      uint spif  :1;
      uint modf  :1;
      uint halta :1;
      uint       :1;
      uint cptqp :4;} spcr3_spsr;
  byte r3[0xe0];
  word rec_ram[16];
  word tran_ram[16];
  byte comd_ram[16];} *qsmptr;

extern const qsmptr qsm;
#define qsm ((qsmptr)0xfffc00)

/*************************   tpu control   *********************************/
typedef struct tpu_control_struct {
  struct tmcr_struct {
    uint stop :1;
    uint tcr1_prescale :2;
    uint tcr2_prescale :2;
    uint emu :1;
    uint t2cg :1;
    uint stf :1;
    uint supv :1;
    uint psck :1;
    uint      :2;
    uint interrupt_arbitration :4;} tmcr;
  word ttcr;
  word dscr;
  word dssr;
  struct ticr_struct {
    uint      :5; 
    uint interrupt_request_level :3;
    uint interrupt_base_vector :8;} ticr;
  word cier;
  word cfs[4];
  word hsf[2];
  volatile word hsrf[2];
  word cpr[2];
  volatile word cisr;} *tpu_control_ptr;

extern const tpu_control_ptr tpu_control;
#define tpu_control ((tpu_control_ptr)0xfffe00)

/* DOCUMENTATION
*********************************************************************
332 hardware registers:		(* mem.h *)

The hardware registers for the 332 are memory mapped as bitfield
structures with the same names as in the Motorola documentation.  See
the file mem.h in the kernal subdirectory of the main 332 tree for
details.  Basically you have the following pointers to the main blocks
of 332 hardware registers, which work in both your programs and in
gdb:

simptr sim;
     Pointer to the system integration module memory map.  
     Some examples are:
     sim->syncr  - the clock control register
     sim->porte  - the data byte for port E
     sim->picr.piv - the periodic control register interrupt vector number
     
     Generally, if you are tempted to use these registers in your
     programs you might want to check sys.h and sys.c to see if there
     is a function that already does what you want.

csmptr csm;
     Pointer to the chip select module memory map.
     Some examples are:
     csm->boot   - the complete chip select settings for CSBOOT
     csm->csch[3].dsack - the data strobe acknowdge delay for CS3
     
     Functions for setting up chip selects and changing their dsack
     delay can be found in sys.h and sys.c.

qsmptr qsm;
     Pointer to the queued serial module memory map.  This includes
     the control and data registers for the queued serial module and
     the asynchronous serial module both.
     Some examples are:
     qsm->tmcr.qmcr.iarb - the interrupt arbitration value for qsm
     qsm->scbr    - the asynchronous serial baud rate conrol register

     Functions for controlling the asynchronous serial module can be
     found in sys.h, sys.c, stdio.h, and stdio.c.  The queued serial
     module is not curently built into kernal in any way.  This is a
     planned improvement.

tpu_control_ptr tpu_control;
     Pointer to the timer processor unit configuration and control
     registers. 
     Some examples are:
     tpu_control->cier - channel interrupt enable register
     tpu_control->tmcr.stop - bit for stopping the entire TPU

     Functions for controlling the TPU are found in TPU.h and TPU.c.

*/ 

#endif



