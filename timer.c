#include "meter.h"
#include "timer.h"

void timer0(void) __interrupt(TF0_VECTOR)
{
	tickdiv--;	// TL0 will reload from TH0
}

void timer_init(void)
{
	TMOD = T0_M1 | T1_M0;		// mode 2 on T0, mode 1 on T1
	TH0 = COUNT_TL0;		// load recurring divisor
	TL0 = COUNT_TL0;		// overflow next cycle
	ET0 = 1;			// enable T0 interrupts
	TR0 = 1;			// turn on T0
	EA = 1;				// enable global interrupts
}

#ifdef	NOP_DELAY

void delay5us(void) __naked
{
	// call and ret take 2 us each, add a nop for 5 us total
	// add more nops if crystal > 12 MHz
	__asm__("nop\nnop\nret");
}

#else

#define	T1DIV		(65536-5)

void delay5us(void) __naked {
	TH1 = T1DIV >> 8;
	TL1 = T1DIV & 0xff;
	TR1 = 1;	// start T1
	while (TF1 == 0)
		;	// wait for overflow
	TR1 = 0;	// turn off T1
	TF1 = 0;	// clear overflow
	__asm__("ret");
}

#endif	// NOP_DELAY
