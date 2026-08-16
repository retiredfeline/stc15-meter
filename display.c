#include "meter.h"
#include "display.h"
#include "timer.h"
#include "aht30.h"

#include <string.h>

static uint8_t display_buffer[4];
#define	HYPHEN	0x40	// to indicate invalid data
#define	CHAR_c	0x58	// c
#define	PERCENT	0x24	// %
const static uint8_t font[] =
{ 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
const static uint8_t sixteenths[16] = { 0, 1, 1, 2, 3, 3, 4, 4, 5, 6, 6, 7, 8, 8, 9, 9 };

static void setBrightness(uint8_t);

void display_init(void)
{
	SER = 0;
	SRCLK = 0;
	RCLK = 0;
	setBrightness(0x8f);
}

static void digits_update(uint8_t *buffer, uint8_t value)
{
	uint8_t	tens = value / 10;
	uint8_t units = value % 10;

	buffer[0] = font[tens];
	buffer[1] = font[units];
}

static void startXfer(void)
{
	SRCLK = 1;
	SER = 1;
	delay5us();
	SER = 0;
	SRCLK = 0;
	delay5us();
}

static void stopXfer(void)
{
	SRCLK = 0;
	SER = 0;
	delay5us();
	SRCLK = 1;
	SER = 1;
	delay5us();
}

static uint8_t writeByte(uint8_t value)
{
	for (uint8_t i = 0; i < 8; i++) {
		SRCLK = 0;
		delay5us();
		SER = value & 0x1;
		delay5us();
		SRCLK = 1;
		delay5us();
		value >>= 1;
	}
	SRCLK = 0;
	delay5us();
	SER = 1;
	SRCLK = 1;
	delay5us();
	return 1;
}

static void setBrightness(uint8_t val)
{
	startXfer();
	writeByte(val);
	stopXfer();
}

void display_update(void)
{
	memset(display_buffer, HYPHEN, sizeof display_buffer);
	switch (mode) {
	case Tem:
		if (valid) {
			digits_update(&display_buffer[0], tem.whole);
			display_buffer[2] = font[sixteenths[tem.frac]];
		}
		display_buffer[1] |= 0x80;	// DP on units digit
		display_buffer[3] = CHAR_c;
		break;
	case Hum:
		if (valid) {
			digits_update(&display_buffer[0], hum.whole);
			display_buffer[2] = font[sixteenths[hum.frac]];
		}
		display_buffer[1] |= 0x80;	// DP on units digit
		display_buffer[3] = PERCENT;
		break;
#ifdef	TEMHUM
	case TemHum:
		if (valid) {
			digits_update(&display_buffer[0], tem.whole + (tem.frac < 8 ? 0 : 1));
			digits_update(&display_buffer[2], hum.whole + (hum.frac < 8 ? 0 : 1));
		}
#endif
	}
	startXfer();
	writeByte(0x40);
	stopXfer();
	startXfer();
	writeByte(0xc0);
	for (uint8_t i = sizeof(display_buffer), *p = display_buffer; i > 0; i--, p++)
		writeByte(*p);
	stopXfer();
}
