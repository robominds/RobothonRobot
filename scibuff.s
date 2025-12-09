	.file	"scibuff.c"
gcc2_compiled.:
__gnu_compiled_c:
.stabs "e:/prog/robot/rob332/",100,0,0,.Ltext0
.stabs "scibuff.c",100,0,0,.Ltext0
.text
.Ltext0:
.stabs "int:t(0,1)=r(0,1);-2147483648;2147483647;",128,0,0,0
.stabs "char:t(0,2)=r(0,2);0;127;",128,0,0,0
.stabs "long int:t(0,3)=r(0,3);-2147483648;2147483647;",128,0,0,0
.stabs "unsigned int:t(0,4)=r(0,4);0;-1;",128,0,0,0
.stabs "long unsigned int:t(0,5)=r(0,5);0;-1;",128,0,0,0
.stabs "long long int:t(0,6)=r(0,1);01000000000000000000000;0777777777777777777777;",128,0,0,0
.stabs "long long unsigned int:t(0,7)=r(0,1);0000000000000;01777777777777777777777;",128,0,0,0
.stabs "short int:t(0,8)=r(0,8);-32768;32767;",128,0,0,0
.stabs "short unsigned int:t(0,9)=r(0,9);0;65535;",128,0,0,0
.stabs "signed char:t(0,10)=r(0,10);-128;127;",128,0,0,0
.stabs "unsigned char:t(0,11)=r(0,11);0;255;",128,0,0,0
.stabs "float:t(0,12)=r(0,1);4;0;",128,0,0,0
.stabs "double:t(0,13)=r(0,1);8;0;",128,0,0,0
.stabs "long double:t(0,14)=r(0,1);12;0;",128,0,0,0
.stabs "complex int:t(0,15)=s8real:(0,1),0,32;imag:(0,1),32,32;;",128,0,0,0
.stabs "complex float:t(0,16)=r(0,16);4;0;",128,0,0,0
.stabs "complex double:t(0,17)=r(0,17);8;0;",128,0,0,0
.stabs "complex long double:t(0,18)=r(0,18);12;0;",128,0,0,0
.stabs "void:t(0,19)=(0,19)",128,0,0,0
.stabs "asm.h",130,0,0,0
.stabn 162,0,0,0
.stabs "qsm.h",130,0,0,0
.stabn 162,0,0,0
.stabs "scibuff.h",130,0,0,0
.stabs "word:t(3,1)=(0,9)",128,0,7,0
.stabs "queue_struct:t(3,2)=(3,3)=s52in:(0,1),0,32;out:(0,1),32,32;q:(3,4)=ar(0,0);0;42;(0,2),64,344;;",128,0,13,0
.stabn 162,0,0,0
	.even
.stabs "qinit:F(0,19)",36,0,10,qinit
.stabs "qvar:p(0,20)=*(3,2)",160,0,10,8
.globl qinit
qinit:
.stabn 68,0,10,.LM1-qinit
.LM1:
	link.w %a6,#-4
	movm.l #0x3c00,-(%sp)
.stabn 68,0,11,.LM2-qinit
.LM2:
.LBB2:
.stabn 68,0,13,.LM3-qinit
.LM3:
	move.l 8(%a6),%a0
	clr.l (%a0)
.stabn 68,0,14,.LM4-qinit
.LM4:
	move.l 8(%a6),%a0
	clr.l 4(%a0)
.stabn 68,0,15,.LM5-qinit
.LM5:
	clr.l -4(%a6)
.L2:
	moveq.l #42,%d5
	cmp.l -4(%a6),%d5
	jbge .L5
	jbra .L3
	.even
.L5:
	move.l 8(%a6),%a0
	move.l -4(%a6),%d0
	move.l -4(%a6),%d1
	move.l %d1,%d2
	muls.l #1717986919,%d3:%d2
	move.l %d3,%d2
	asr.l #2,%d2
	move.l %d1,%d3
	add.l %d3,%d3
	subx.l %d3,%d3
	sub.l %d3,%d2
	move.l %d2,%d4
	move.l %d4,%d3
	lsl.l #2,%d3
	add.l %d2,%d3
	move.l %d3,%d2
	add.l %d2,%d2
	sub.l %d2,%d1
	move.b %d1,%d5
	add.b #48,%d5
	move.b %d5,8(%a0,%d0.l)
.L4:
	addq.l #1,-4(%a6)
	jbra .L2
	.even
.L3:
.stabn 68,0,16,.LM6-qinit
.LM6:
	move.l 8(%a6),%a0
	move.b #13,50(%a0)
.stabn 68,0,17,.LM7-qinit
.LM7:
	move.l 8(%a6),%a0
	move.b #10,49(%a0)
.stabn 68,0,18,.LM8-qinit
.LM8:
.LBE2:
.stabn 68,0,18,.LM9-qinit
.LM9:
.L1:
	movm.l (%sp)+,#0x3c
	unlk %a6
	rts
.stabs "i:(0,1)",128,0,11,-4
.stabn 192,0,0,.LBB2-qinit
.stabn 224,0,0,.LBE2-qinit
.Lscope0:
.stabs "",36,0,0,.Lscope0-qinit
	.even
.stabs "qstat:F(0,1)",36,0,20,qstat
.stabs "qvar:p(0,20)",160,0,20,8
.globl qstat
qstat:
.stabn 68,0,20,.LM10-qstat
.LM10:
	link.w %a6,#0
	move.l %d2,-(%sp)
.stabn 68,0,22,.LM11-qstat
.LM11:
	move.l 8(%a6),%a0
	move.l 8(%a6),%a1
	move.l (%a0),%d2
	cmp.l 4(%a1),%d2
	jble .L7
.stabn 68,0,23,.LM12-qstat
.LM12:
	move.l 8(%a6),%a0
	move.l 8(%a6),%a1
	move.l (%a0),%d1
	sub.l 4(%a1),%d1
	move.l %d1,%d0
	jbra .L6
	.even
.stabn 68,0,24,.LM13-qstat
.LM13:
	jbra .L8
	.even
.L7:
	move.l 8(%a6),%a0
	move.l 8(%a6),%a1
	move.l (%a0),%d2
	cmp.l 4(%a1),%d2
	jbge .L9
.stabn 68,0,25,.LM14-qstat
.LM14:
	move.l 8(%a6),%a0
	move.l 8(%a6),%a1
	moveq.l #-43,%d0
	add.l 4(%a1),%d0
	move.l (%a0),%d1
	sub.l %d0,%d1
	move.l %d1,%d0
	jbra .L6
	.even
.stabn 68,0,26,.LM15-qstat
.LM15:
	jbra .L10
	.even
.L9:
.stabn 68,0,27,.LM16-qstat
.LM16:
	clr.l %d0
	jbra .L6
	.even
.L10:
.L8:
.stabn 68,0,29,.LM17-qstat
.LM17:
	clr.l %d0
	jbra .L6
	.even
.stabn 68,0,30,.LM18-qstat
.LM18:
.L6:
	move.l (%sp)+,%d2
	unlk %a6
	rts
.Lscope1:
.stabs "",36,0,0,.Lscope1-qstat
	.even
.stabs "rx_byte:F(0,2)",36,0,32,rx_byte
.stabs "rxbyte:p(0,21)=*(0,2)",160,0,32,8
.globl rx_byte
rx_byte:
.stabn 68,0,32,.LM19-rx_byte
.LM19:
	link.w %a6,#0
.stabn 68,0,34,.LM20-rx_byte
.LM20:
	move.l rxbuff,%d1
	cmp.l rxbuff+4,%d1
	jbeq .L12
.stabn 68,0,35,.LM21-rx_byte
.LM21:
	move.l 8(%a6),%a0
	move.l rxbuff+4,%d0
	lea rxbuff,%a1
	move.b 8(%a1,%d0.l),(%a0)
.stabn 68,0,36,.LM22-rx_byte
.LM22:
	addq.l #1,rxbuff+4
.stabn 68,0,37,.LM23-rx_byte
.LM23:
	moveq.l #42,%d1
	cmp.l rxbuff+4,%d1
	jbge .L13
	clr.l rxbuff+4
.L13:
.stabn 68,0,38,.LM24-rx_byte
.LM24:
	moveq.l #1,%d0
	jbra .L11
	.even
.stabn 68,0,39,.LM25-rx_byte
.LM25:
	jbra .L14
	.even
.L12:
.stabn 68,0,40,.LM26-rx_byte
.LM26:
	move.l 8(%a6),%a0
	move.b #88,(%a0)
.stabn 68,0,41,.LM27-rx_byte
.LM27:
	clr.l %d0
	jbra .L11
	.even
.L14:
.stabn 68,0,43,.LM28-rx_byte
.LM28:
	clr.l %d0
	jbra .L11
	.even
.stabn 68,0,44,.LM29-rx_byte
.LM29:
.L11:
	unlk %a6
	rts
.Lscope2:
.stabs "",36,0,0,.Lscope2-rx_byte
	.even
.stabs "tx_byte:F(0,1)",36,0,46,tx_byte
.stabs "txbyte:p(0,1)",160,0,46,8
.globl tx_byte
tx_byte:
.stabn 68,0,46,.LM30-tx_byte
.LM30:
	link.w %a6,#0
.stabn 68,0,48,.LM31-tx_byte
.LM31:
	move.l txbuff,%d1
	cmp.l txbuff+4,%d1
	jbeq .L16
	clr.l %d0
	jbra .L15
	.even
.L16:
.stabn 68,0,51,.LM32-tx_byte
.LM32:
	moveq.l #47,%d1
	cmp.l 8(%a6),%d1
	jbge .L19
	moveq.l #58,%d1
	cmp.l 8(%a6),%d1
	jblt .L19
	jbra .L18
	.even
.L19:
#APP
	bgnd
#NO_APP
.L18:
.stabn 68,0,53,.LM33-tx_byte
.LM33:
	moveq.l #40,%d1
	cmp.l txbuff,%d1
	jblt .L20
	move.l txbuff,%d0
	lea txbuff,%a0
	move.b 11(%a6),8(%a0,%d0.l)
.L20:
.stabn 68,0,55,.LM34-tx_byte
.LM34:
	move.l txbuff,%d0
	addq.l #1,%d0
	moveq.l #42,%d1
	cmp.l %d0,%d1
	jbge .L21
.stabn 68,0,56,.LM35-tx_byte
.LM35:
	clr.l txbuff
.stabn 68,0,57,.LM36-tx_byte
.LM36:
	jbra .L22
	.even
.L21:
.stabn 68,0,58,.LM37-tx_byte
.LM37:
	addq.l #1,txbuff
.L22:
.stabn 68,0,61,.LM38-tx_byte
.LM38:
	moveq.l #1,%d0
	jbra .L15
	.even
.stabn 68,0,62,.LM39-tx_byte
.LM39:
	jbra .L23
	.even
.L17:
.stabn 68,0,63,.LM40-tx_byte
.LM40:
	clr.l %d0
	jbra .L15
	.even
.L23:
.stabn 68,0,65,.LM41-tx_byte
.LM41:
	clr.l %d0
	jbra .L15
	.even
.stabn 68,0,66,.LM42-tx_byte
.LM42:
.L15:
	unlk %a6
	rts
.Lscope3:
.stabs "",36,0,0,.Lscope3-tx_byte
	.even
.stabs "sciinit:F(0,19)",36,0,68,sciinit
.globl sciinit
sciinit:
.stabn 68,0,68,.LM43-sciinit
.LM43:
	link.w %a6,#0
.stabn 68,0,70,.LM44-sciinit
.LM44:
	move.w #135,-1024.w
.stabn 68,0,71,.LM45-sciinit
.LM45:
	move.w #1622,-1020.w
.stabn 68,0,72,.LM46-sciinit
.LM46:
	move.w #53,-1016.w
.stabn 68,0,73,.LM47-sciinit
.LM47:
	move.w #44,-1014.w
.stabn 68,0,74,.LM48-sciinit
.LM48:
.L24:
	unlk %a6
	rts
.Lscope4:
.stabs "",36,0,0,.Lscope4-sciinit
	.even
.stabs "sciint:F(0,19)",36,0,76,sciint
.globl sciint
sciint:
.stabn 68,0,76,.LM49-sciint
.LM49:
	link.w %a6,#0
	move.l %d2,-(%sp)
.stabn 68,0,78,.LM50-sciint
.LM50:
	clr.l %d0
	move.w -1012.w,%d0
	move.l %d0,status
.stabn 68,0,80,.LM51-sciint
.LM51:
	moveq.l #64,%d0
	and.l status,%d0
	tst.l %d0
	jbeq .L26
.stabn 68,0,81,.LM52-sciint
.LM52:
	clr.l %d0
	move.w -1010.w,%d0
	move.l %d0,scidata
.stabn 68,0,82,.LM53-sciint
.LM53:
	moveq.l #8,%d0
	and.l status,%d0
	tst.l %d0
	jbeq .L27
.stabn 68,0,84,.LM54-sciint
.LM54:
	jbra .L28
	.even
.L27:
.stabn 68,0,85,.LM55-sciint
.LM55:
	move.l rxbuff,%d0
	addq.l #1,%d0
	cmp.l rxbuff+4,%d0
	jbeq .L28
	move.l rxbuff,%d0
	addq.l #1,%d0
	moveq.l #43,%d1
	add.l rxbuff+4,%d1
	cmp.l %d0,%d1
	jbeq .L28
.stabn 68,0,86,.LM56-sciint
.LM56:
	move.l rxbuff,%d0
	lea rxbuff,%a0
	move.b scidata+3,8(%a0,%d0.l)
.stabn 68,0,87,.LM57-sciint
.LM57:
	addq.l #1,rxbuff
.stabn 68,0,88,.LM58-sciint
.LM58:
	moveq.l #42,%d2
	cmp.l rxbuff,%d2
	jbge .L30
	clr.l rxbuff
.L30:
.stabn 68,0,89,.LM59-sciint
.LM59:
	jbra .L28
	.even
.L29:
.L31:
.L28:
.stabn 68,0,93,.LM60-sciint
.LM60:
	jbra .L32
	.even
.L26:
	moveq.l #8,%d0
	and.l status,%d0
	tst.l %d0
	jbeq .L33
.stabn 68,0,94,.LM61-sciint
.LM61:
	clr.l %d0
	move.w -1010.w,%d0
	move.l %d0,scidata
.stabn 68,0,95,.LM62-sciint
.LM62:
	jbra .L32
	.even
.L33:
	move.l #256,%d0
	and.l status,%d0
	tst.l %d0
	jbeq .L35
.stabn 68,0,96,.LM63-sciint
.LM63:
	move.l txbuff,%d2
	cmp.l txbuff+4,%d2
	jbeq .L36
.stabn 68,0,97,.LM64-sciint
.LM64:
	move.l txbuff+4,%d0
	lea txbuff,%a0
	move.b 8(%a0,%d0.l),%d2
	extb.l %d2
	move.l %d2,scidata
.stabn 68,0,98,.LM65-sciint
.LM65:
	move.l txbuff+4,%d0
	addq.l #1,%d0
	moveq.l #42,%d2
	cmp.l %d0,%d2
	jbge .L37
.stabn 68,0,99,.LM66-sciint
.LM66:
	clr.l txbuff+4
.stabn 68,0,100,.LM67-sciint
.LM67:
	jbra .L38
	.even
.L37:
.stabn 68,0,101,.LM68-sciint
.LM68:
	addq.l #1,txbuff+4
.L38:
.stabn 68,0,103,.LM69-sciint
.LM69:
	move.w scidata+2,-1010.w
.stabn 68,0,104,.LM70-sciint
.LM70:
	jbra .L39
	.even
.L36:
.stabn 68,0,105,.LM71-sciint
.LM71:
	move.w #44,-1014.w
.L39:
.stabn 68,0,107,.LM72-sciint
.LM72:
	jbra .L32
	.even
.L35:
.stabn 68,0,108,.LM73-sciint
.LM73:
	jsr sciinit
.L40:
.L34:
.L32:
.stabn 68,0,110,.LM74-sciint
.LM74:
	clr.l scidata
.stabn 68,0,111,.LM75-sciint
.LM75:
	clr.l status
.stabn 68,0,112,.LM76-sciint
.LM76:
.L25:
	move.l -4(%a6),%d2
	unlk %a6
	rts
.Lscope5:
.stabs "",36,0,0,.Lscope5-sciint
#APP
	.global scixxx;scixxx:link %a6,#-8;jsr sciint;unlk %a6;rte
#NO_APP
.stabs "rxbuff:G(3,2)",32,0,6,0
.comm rxbuff,52
.stabs "txbuff:G(3,2)",32,0,6,0
.comm txbuff,52
.stabs "status:G(0,1)",32,0,8,0
.comm status,4
.stabs "scidata:G(0,1)",32,0,8,0
.comm scidata,4
	.text
	.stabs "",100,0,0,Letext
Letext:
