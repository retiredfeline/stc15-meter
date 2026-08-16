#ifndef QX_H
#define QX_H

#define	_SDCC_

// Crystal frequency 11.0592 MHz

#define	COUNTDIV	240			// 11.0592 MHz -> 3840 Hz

#define COUNT_TL0	(256 - COUNTDIV)

#include <8052.h>
/* Port mode bits */

/*  BYTE Register  */
__sfr __at (0x91) P1M1 ;
__sfr __at (0x92) P1M0 ;
__sfr __at (0x93) P0M1 ;
__sfr __at (0x94) P0M0 ;
__sfr __at (0x95) P2M1 ;
__sfr __at (0x96) P2M0 ;
__sfr __at (0xB1) P3M1 ;
__sfr __at (0xB2) P3M0 ;

// Signals
#define	SER		P1_2
#define	SRCLK		P1_3

#define	SDA		P1_4
#define	SCL		P1_5
#define	SDA_L()		P1_4 = 0
#define	SDA_H()		P1_4 = 1
#define	SCL_L()		P1_5 = 0
#define	SCL_H()		P1_5 = 1
#define	READ_SDA()	SDA
#define	button_state()	P3

#define	CONST	__code
#define	mcu_init()
#define	mcu_enable_interrupts()
#define	watchdog_check()
#define	watchdog_reload()
#define	button_init()
#define	led_toggle()	P1_0 ^= 1

#endif

extern void timer0(void) __interrupt(TF0_VECTOR);	// to make sure vector is initialised
