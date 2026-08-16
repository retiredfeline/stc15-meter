//
// I2C communication functions using bit-banging
// I have implemented only what I needed for my project
// In particular, clock stretching is not implemented
// Feel free to improve
//

#include "meter.h"
#include "i2c.h"
#include "timer.h"
#include "aht30.h"

#define	ASMRET()	__asm__("ret")

void i2c_init(void) __naked
{
#ifdef	STC15F
	// set SCL and SDA to open drain, need pullup
        P3M0 = 0x0A;            // P1M0.2,3 = 1, open drain
        P3M1 = 0x0A;            // P1M1.2,3 = 1, open drain
#endif	// STC15F
	SDA_H();
	delay5us();
	SCL_H();
	delay5us();
	ASMRET();
}

void i2c_start(void) __naked
{
	SDA_L();
	delay5us();
	SCL_L();
	delay5us();
	ASMRET();
}

void i2c_restart(void) __naked
{
	SDA_H();
	delay5us();
	SCL_H();
	delay5us();
	SDA_L();
	delay5us();
	SCL_L();
	delay5us();
	ASMRET();
}

void i2c_stop(void) __naked
{
	SCL_L();
	delay5us();
	SDA_L();
	delay5us();
	SCL_H();
	delay5us();
	SDA_H();
	delay5us();
	ASMRET();
}

void i2c_ack(void) __naked
{
	SDA_L();
	delay5us();
	SCL_H();
	delay5us();
	SCL_L();
	delay5us();
	SDA_H();
	delay5us();
	ASMRET();
}

void i2c_nak(void) __naked
{
	SDA_H();
	delay5us();
	SCL_H();
	delay5us();
	SCL_L();
	delay5us();
	SDA_H();
	delay5us();
	ASMRET();
}

uint8_t i2c_send(uint8_t data)
{
	uint8_t i;

	for (i = 0; i < 8; i++) {
		if (data & 0x80)
			SDA_H();
		else
			SDA_L();
		delay5us();
		SCL_H();
		delay5us();
		SCL_L();
		delay5us();
		data <<= 1;
	}
	SDA_H();
	delay5us();
	SCL_H();
	i = READ_SDA();
	delay5us();
	SCL_L();
	delay5us();
	return i;
}

uint8_t i2c_sendaddr(void)
{
	return i2c_send(I2C_ADDR << 1);
}

uint8_t i2c_readaddr(void)
{
	return i2c_send((I2C_ADDR << 1) | 1);
}

uint8_t i2c_read(void)
{
	uint8_t i;
	uint8_t data = 0;

	for (i = 0; i < 8; i++) {
		data <<= 1;
		data |= READ_SDA();
		SCL_H();
		delay5us();
		SCL_L();
		delay5us();
	}
	return data;
}

void i2c_senddata(uint8_t *data, uint8_t len)
{
	i2c_start();
	i2c_sendaddr();
	for (uint8_t i = len; i > 0; i--)
		i2c_send(*data++);
	i2c_stop();
}

void i2c_readdata(uint8_t *data, uint8_t len)
{
	i2c_start();
	i2c_readaddr();
	for (uint8_t i = len; i > 0; i--) {
		*data++ = i2c_read();
		i > 1 ? i2c_ack() : i2c_nak();
	}
	i2c_stop();
}
