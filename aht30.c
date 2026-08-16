/*
Sensor Reading Process

1. After power-on, wait for ≥100ms. Before reading the temperature and humidity value, get a byte of status word
by sending 0x71. If the status word and 0x18 are not equal to 0x18, initialize the 0x1B, 0x1C, 0x1E registers,
details Please refer to our official website routine for the initialization process; if they are equal, proceed to the next
step.

2. Wait 10ms to send the 0xAC command (trigger measurement). This command parameter has two bytes, the first
byte is 0x33, and the second byte is 0x00.

3. Wait 80ms for the measurement to be completed, if the read status word Bit[7] is 0, it means the measurement
is completed, and then six bytes can be read continuously; otherwise, continue to wait.

4. After receiving six bytes, the next byte is CRC check data, which the user can read as needed. If the receiver
needs CRC check, it will send an ACK reply after receiving the sixth byte, otherwise it will send a NACK reply. The
initial value of CRC is 0xFF, and the CRC8 check polynomial is:

CRC [7:0] = 1+X4+X5+X8

5. Calculate the temperature and humidity value

Note: The calibration status check in the first step only needs to be checked when the power is turned on.No
operation is required during the acquisition process.
*/

#include "meter.h"
#include "aht30.h"
#include "i2c.h"

struct measurement tem, hum;
uint8_t valid, updated;
static uint16_t sensor_tick;

#define	AHTX0_CMD_TRIGGER	0xAC
#define	AHTX0_STATUS_BUSY	0x80
#define AHTX0_STATUS_CALIBRATED	0x08
#define	AHTX0_CMD_CALIBRATE	0xE1

void aht30_init(void)
{
	i2c_init();
	valid = 1;
	updated = 1;
	sensor_tick = SENSOR_START;
	tem.whole = 24;
	tem.frac = 8;
	hum.whole = 56;
	hum.frac = 4;
}

static uint8_t aht30_status(void)
{
	uint8_t status[7];

	i2c_readdata(status, sizeof status);
	return status[0];
}

static void aht30_trigger(void)
{
	const static uint8_t cmd[3] = { AHTX0_CMD_TRIGGER, 0x33, 0 };
	
	i2c_senddata(cmd, 3);
}

/* Taken from AHT30 datasheet */
static uint8_t aht30_calc_crc(uint8_t *data, uint8_t len)
{
	uint8_t crc = 0xFF;

	for (uint8_t byte = 0; byte < len; byte++) /* len times */
	{
		crc ^= data[byte];			/* xor byte */
		for (uint8_t i = 8; i > 0; --i) /* one byte */
		{
			if ((crc & 0x80) != 0)		 /* if high*/
				crc = (crc << 1) ^ 0x31; /* xor 0x31 */
			else
				crc = crc << 1;		 /* skip */
		}
	}
	return crc;
}

static uint8_t aht30_getmeas(void)
{
#undef	TESTDATA
#ifdef	TESTDATA
	const static uint8_t data[] = { 0x18, 0x9B, 0x7C, 0x15, 0xA1, 0x9E, 0xDC };
	// Taken from actual AHT sensor
	// T: 20.4 °C
	// H: 60.7 % rH
#else
	uint8_t data[7];

	i2c_readdata(data, sizeof data);
#endif
	if (aht30_calc_crc(data, 6) != data[6])
		return 0;

	uint32_t d = data[1];
	d <<= 8;
	d |= data[2];
	d <<= 4;
	d |= data[3] >> 4;
	d *= 100;
	uint16_t w = d >> 16;
	hum.frac = w & 0x0F;
	hum.whole = w >> 4;

	d = data[3] & 0x0F;
	d <<= 8;
	d |= data[4];
	d <<= 8;
	d |= data[5];
	d *= 200;
	w = d >> 16;
	tem.frac = w & 0x0F;
	tem.whole = (w >> 4) - 50;
	if (tem.whole < 0) {
		tem.whole++;
		tem.frac = 0x10 - tem.frac;
	}
	return 1;
}

PT_THREAD(sensorhandler(struct pt *pt))
{
	PT_BEGIN(pt);
	for (;;) {
		PT_WAIT_UNTIL(pt, --sensor_tick <= 0);
		// send measure command
		aht30_trigger();
		sensor_tick = MEASURE_WAIT;
		PT_WAIT_UNTIL(pt, --sensor_tick <= 0 || !(aht30_status() & AHTX0_STATUS_BUSY));
		// collect data
		valid = aht30_getmeas();
		updated = 1;
		sensor_tick = SENSOR_POLL;
	}
	PT_END(pt);
}
