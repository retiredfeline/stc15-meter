#include "pt.h"

typedef	unsigned char	uint8_t;
#ifndef	STM8S	// already defined in STM8 stdint.h
typedef unsigned int	uint16_t;
typedef signed int	int16_t;
#endif	// STM8S
typedef	unsigned long	uint32_t;

#define	TICKDIV		16			// 3840 Hz -> 240 Hz
#define	TICK		4			// ms, roughly
#define	DEPMIN		(100 / TICK)		// debounce period
#define	RPTTHRESH	((400 / TICK) + 1)	// repeat threshold after debounce
#define	RPTPERIOD	(250 / TICK)		// repeat period
#define	BUTTON_TIMEOUT	(16000u / TICK)		// revert to Tem mode after 16 seconds in other modes

extern volatile uint8_t tickdiv;
extern enum Mode { Tem = 0, Hum = 1, TemHum = 2 } mode;

#ifdef	QX
#include "qx.h"
#endif	// QX

#ifdef	STC15F
#include "stc15f.h"
#endif	// STC15F

#ifdef	STM8S
#include "stm8s.h"
#include "mcu.h"
#include "button.h"
#endif	// STM8S
