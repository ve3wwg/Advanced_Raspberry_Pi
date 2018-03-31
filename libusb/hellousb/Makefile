BUS	?= 001
DEV	?= 003

hellousb:
	$(CC) $(CFLAGS) hellousb.c

# Bus 001 Device 003: ID 04b4:8613 (unconfigured FX2)
list:
	$(PROG) --list # [-d=BUS.DEV]

prog:
	@echo "*** Make sure you do sudo rmmod usbtest ***"
	# or sudo sh -c "echo 'blacklist usbtest' >> /etc/modprobe.d/blacklist.conf"
	sudo $(PROG) -d=$(BUS).$(DEV) prg:hellousb.ihx run delay:10 dbulk:6,-512,5

more:
	sudo $(PROG) -d=$(BUS).$(DEV) dbulk:6,-512,5

reset:
	sudo $(PROG) -d=$(BUS).$(DEV) reset

clobber: clean
	rm -f hellousb.ihx phellousb *.o

include ../ezusb/Makefile.incl

