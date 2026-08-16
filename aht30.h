#define	I2C_ADDR	0x38U
#define	SENSOR_START	(1000u / TICK)		// wait 1 second before first measurement
#define	SENSOR_POLL	(10000u / TICK)		// poll every 10 seconds
#define	MEASURE_WAIT	(80u / TICK)		// wait 80 ms to collect data

struct measurement {
  int16_t whole;
  uint16_t frac;
};

extern struct measurement tem, hum;
extern uint8_t valid, updated;

extern void aht30_init(void);
extern PT_THREAD(sensorhandler(struct pt *pt));
