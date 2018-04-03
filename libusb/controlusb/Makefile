BUS	?= 001
DEV	?= 005

ezusb:
	$(CC) $(CFLAGS) ezusb.c

list:
	$(PROG) --list 

prog:
	# sudo rmmod usbtest, if necessary
	# or sudo sh -c "echo 'blacklist usbtest' >> /etc/modprobe.d/blacklist.conf"
	sudo $(PROG) -d=$(BUS).$(DEV) prg:ezusb.ihx run delay:10 dbulk:6,-512,5

more:
	sudo $(PROG) -d=$(BUS).$(DEV) dbulk:6,-512,5

reset:
	sudo $(PROG) -d=$(BUS).$(DEV) reset

clobber: clean
	rm -f ezusb.ihx pezusb *.o .errs.t

include ../ezusb/Makefile.incl

