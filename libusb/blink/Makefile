blink:
	$(CC) $(CFLAGS) blink.c

# Bus 001 Device 003: ID 04b4:8613 (unconfigured FX2)
list:
	$(PROG) --list # [-d=BUS.DEV]

prog:
	sudo $(PROG) -d=001.003 reset prg:blink.ihx run

reset:
	sudo $(PROG) -d=001.003 reset

clobber: clean
	rm -f blink.ihx

include ../ezusb/Makefile.incl

