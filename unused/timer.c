/* TIMER.C
 *
 *      Sample program for Micro/sys MPC550
 *      Timer I/O
 *
 */

#include <dos.h>
#include <stdio.h>
#include "MPC550.h"

int MPC550BASE = 0x200;

int setuptimer(int timer, int mode, long hertz)
{
    int control = 0x30;
    int lsb = 0;
    int msb = 0;
        if ((timer<0)||(timer>2))
           return(-1);
        if ((mode<0)||(mode>5))
           return(-1);
        if ((hertz<110)||(hertz>7159000L))
           return(-1);
        lsb = (7159000/hertz) & 0x00FF;
        msb = (7159000/hertz) & 0xFF00;
        msb = msb>>8;
        control |= (mode<<1);
        control |= (timer<<6);
        outp(TMRCNTRL550, control);
        outp(TMRCNTR0550+timer, lsb);
        outp(TMRCNTR0550+timer, msb);
        return(0);
}

test82C54()
{
        printf("Outputting 1kHz square wave on Timer 0 (J6 Pin 0)\n");
        setuptimer(0, 3, 1000L);
        
        printf("Outputting 2kHz square wave on Timer 1 (J6 Pin 6)\n");
        setuptimer(1, 3, 2000L);
        
        printf("Outputting 3kHz square wave on Timer 2 (J6 Pin 9)\n\n");
        setuptimer(2, 3, 3000L);

        for(;;);
}


/************************************ main *********************************/

main() 
{
        test82C54();
}



