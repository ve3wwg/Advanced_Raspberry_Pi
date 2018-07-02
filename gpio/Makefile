CC	= gcc
OPTS	= -Wall
DBG	= -O0 -g
CFLAGS	= $(OPTS) $(DBG)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $*.o

OBJS	= gpio.o

all:	$(OBJS)
	$(CC) $(OBJS) -o gpio
	sudo chown root ./gpio
	sudo chmod u+s ./gpio

clean:
	rm -f *.o core errs.t

clobber: clean
	rm -f gpio
