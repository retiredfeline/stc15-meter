CC=sdcc
# BUILDFLAGS:
#  STC15F
#  QX			// for testing
#  TEMHUM		// 3rd mode: show temperature and humidity
#  STARTMODE=		// if defined start in this mode
#  NOP_DELAY		// use nops for delay instead of timer1
#
# Example:
# BUILDFLAGS=-DQX
#

ifeq "$(STC15F)" "Y"
# suggested options for STC15F from another project
# sdcc -mmcs51 --iram-size 128 --xram-size 0 --code-size 4096 \
# --nooverlay --noinduction --verbose --debug -V --std-sdcc89 \
# --model-small
BUILDFLAGS=-DSTC15F
LDFLAGS=--iram-size 128 --code-size 4096
STCGALARGS=-p $(PORT) -P stc15 -t $(FREQ) -b $(BAUD) -D # -o low_voltage_reset=True
else
BUILDFLAGS=-DQX
LDFLAGS=
STCGALARGS=-p $(PORT)
endif

CFLAGS=-mmcs51 -Ipt-1.4 $(BUILDFLAGS) -DBUILDFLAGS="$(BUILDFLAGS)" --opt-code-speed
INCLUDES=
LIBS=
PORT?=/dev/ttyUSB0
BAUD?=38400
FREQ?=11059

meter.hex:	meter.rel timer.rel display.rel aht30.rel i2c.rel
		$(CC) $(LDFLAGS) -o $@ $^

meter.rel:	meter.c qx.h stc15f.h
		$(CC) -c $(CFLAGS) $(INCLUDES) meter.c

%.asm:		%.c
		$(CC) $(CFLAGS) $(INCLUDES) -S $<

%.rel:		%.c %.h
		$(CC) -c $(CFLAGS) $(INCLUDES) $< -o $(<:.c=.rel)

%.hex:		%.rel
		$(CC) $(CFLAGS) -o $@ $<

%.flash:	%.hex
		stcgal $(STCGALARGS) $<

%.check:
		stcgal $(STCGALARGS)

clean:
		rm -f *.{asm,sym,lst,rel,rst,sym,lk,map,mem}
