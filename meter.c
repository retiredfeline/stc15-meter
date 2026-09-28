// AHT30 based thermometer and humidity meter
// See README.md and LICENSE.md
//

#include "meter.h"
#include "timer.h"
#include "display.h"
#include "aht30.h"

// Switches
#define	SWMASK		(MODEBUTTON)

uint8_t volatile tickdiv;
enum Mode mode;

static uint8_t swstate, swtent, swmin, swrepeat;		// switch handling
static uint16_t button_timeout;
static struct pt pt, ahtpt;

CONST uint8_t builddate[] = __DATE__;
#define	xstring(x)	string(x)
#define	string(x)	#x
CONST uint8_t buildflags[] = "BUILDFLAGS=" xstring(BUILDFLAGS);

static void switchaction(void)
{
	switch(~swstate & SWMASK) {
	case MODEBUTTON:
		switch (mode) {
		case Tem:
			mode = Hum;
			break;
		case Hum:
#ifndef	TEMHUM
			mode = Tem;
			break;
#else
			mode = TemHum;
			break;
		case TemHum:
			mode = Tem;
			break;
#endif
		}
		display_update();
		break;
	}
	if (mode == Hum)
		button_timeout = BUTTON_TIMEOUT;
	else
		button_timeout = 0;
}

static inline void reinitstate(void)
{
	swtent = swstate;
	swmin = DEPMIN;
	swrepeat = RPTTHRESH;
}

static
PT_THREAD(switchhandler(struct pt *pt, uint8_t oneshot))
{
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, swstate != swtent);
	swtent = swstate;
	PT_WAIT_UNTIL(pt, --swmin <= 0 || swstate != swtent);
	if (swstate != swtent) {		// changed, restart
		reinitstate();
		PT_RESTART(pt);
	}
	switchaction();
	if (oneshot) {
		reinitstate();
		PT_RESTART(pt);
	}
	PT_WAIT_UNTIL(pt, --swrepeat <= 0 || swstate != swtent);
	if (swstate != swtent) {		// changed, restart
		reinitstate();
		PT_RESTART(pt);
	}
	switchaction();
	for (;;) {
		swrepeat = RPTPERIOD;
		PT_WAIT_UNTIL(pt, --swrepeat <= 0 || swstate == SWMASK);
		if (swstate == SWMASK) {	// released, restart
			reinitstate();
			PT_RESTART(pt);
		}
		switchaction();
	}
	PT_END(pt);
}

void main(void)
{
	watchdog_check();
	mcu_init();
	timer_init();
	button_init();
	swstate = SWMASK;		// all buttons up
	tickdiv = TICKDIV;
	aht30_init();
	display_init();
	mcu_enable_interrupts();
	display_update();		// load segments
	reinitstate();			// switch handler
#ifdef	STARTMODE
	mode = STARTMODE;
#else
	mode = Tem;
#endif
	for (;;) {
		while (tickdiv > 0)
			;
		tickdiv = TICKDIV;
		watchdog_reload();
		uint8_t counter;
		if (counter-- <= 0) {
			led_toggle();
			counter = 125;	// ½ second
		}
		if (button_timeout != 0)
			button_timeout--;
		else if (mode == Hum) {
#ifdef	STARTMODE
			mode = STARTMODE;
#else
			mode = Tem;
#endif
			display_update();
		}
		swstate = button_state() & SWMASK;
		PT_SCHEDULE(switchhandler(&pt, (~swstate & MODEBUTTON)));
		PT_SCHEDULE(sensorhandler(&ahtpt));
		if (updated) {
			display_update();
			updated = 0;
		}
	}
}
