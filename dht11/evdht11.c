/* Report from the DHT11:
 * Warren W. Gay ve3wwg
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <poll.h>
#include <assert.h>

#include "libgp.h"

static int gpio_pin = 22;

static inline void
timeofday(struct timespec *t) {

	clock_gettime(CLOCK_MONOTONIC,t);
}

static int
ms_diff(struct timespec *t0,struct timespec *t1) {
	int dsec = (int)(t1->tv_sec - t0->tv_sec);
	int dms = (t1->tv_nsec - t0->tv_nsec) / 1000000L;

	assert(dsec >= 0);
	dms += dsec * 1000;
	return dms;
}

static inline long
ns_diff(struct timespec *t0,struct timespec *t1) {
	int dsec = (int)(t1->tv_sec - t0->tv_sec);
	long dns = t1->tv_nsec - t0->tv_nsec;

	assert(dsec >= 0);
	dns += dsec * 1000000000L;
	return dns;
}

static void
wait_ready(void) {
	static struct timespec t0 = {0L,0L};
	struct timespec t1;

	if ( !t0.tv_sec ) {
		timeofday(&t0);
		--t0.tv_sec;
	}

	for (;;) {
		timeofday(&t1);
		if ( ms_diff(&t0,&t1) >= 1000 ) {
			t0 = t1;
			return;
		}
		usleep(100);
	}
}

static void
wait_ms(int ms) {
	struct pollfd p[1];
	int rc;

	rc = poll(&p[0],0,ms);
	assert(!rc);
}

static inline int
wait_change(long *nsec) {
	int b1;
	struct timespec t0, t1;
	int b0 = gpio_read(gpio_pin);

	timeofday(&t0);

	while ( (b1 = gpio_read(gpio_pin)) == b0 )
		;
	timeofday(&t1);
	*nsec = ns_diff(&t0,&t1);
	return b1;
}

static inline int
read_bit(void) {
static int flipper = 0;
	long nsec0, nsec1;
	int b0, b1;

flipper ^= 1;
gpio_write(4,flipper);
	b0 = wait_change(&nsec0);
	b1 = wait_change(&nsec1);
	return nsec1 > 35000;
}

static unsigned
read_40bits(void) {
	short sx = 40;
	uint64_t acc = 0;

	while ( sx-- > 0 )
		acc |= (uint64_t)read_bit() << sx;

	unsigned cksum = acc & 0xFF;
	unsigned rh = acc >> 32;
	unsigned temp = (acc >> 16) & 0xFF;
	unsigned comp = (rh + temp) & 0xFF;

	if ( comp != cksum )
		return 0;

	return (rh << 8) | temp;
}

static void
usage(const char *cmd) {
	
	printf(
		"Usage:\t%s [-g gpio] [-h]\n"
		"where:\n"
		"\t-g gpio\tSpecify GPIO pin (22 is default)\n"
		"\t-h\tThis help\n",
		cmd);	
}

int
main(int argc,char **argv) {
	static char options[] = "hg:";
	long nsec;
	int oc, b;

	while ( (oc = getopt(argc,argv,options)) != -1 ) {
		switch ( oc ) {
		case 'g':
			gpio_pin = atoi(optarg);
			if ( gpio_pin <= 0 ) {
				fprintf(stderr,"Invalid gpio: -g %s\n",
					optarg);
				exit(1);
			}
			break;
		case 'h':
			usage(argv[0]);
			exit(0);
		default:
			usage(argv[0]);
			exit(1);
		}		
	}
	
	gpio_open();

	gpio_configure_io(4,Output);
	gpio_write(4,0);

	gpio_configure_io(gpio_pin,Output);
	gpio_write(gpio_pin,1);

	for (;;) {
		wait_ready();

		gpio_write(gpio_pin,1);
		wait_ms(3);
		gpio_configure_io(gpio_pin,Output);

		gpio_write(gpio_pin,0);
		wait_ms(30);
		gpio_configure_io(gpio_pin,Input);

		b = wait_change(&nsec);

		long nsec0 = nsec;
		int b0 = b;

		if ( b || nsec > 20000 )
			continue;

		long nsec1;
		int b1 = wait_change(&nsec1);

		if ( nsec1 < 40000 || nsec1 > 90000 )
			continue;

		long nsec2;
		int b2 = wait_change(&nsec2);

		if ( nsec2 < 40000 || nsec2 > 90000 )
			continue;

		unsigned resp = read_40bits();
		unsigned char *uc = (unsigned char *)&resp;

		printf("b0=%d, %6ld nsec\n",b0,nsec0);
		printf("b1=%d, %6ld nsec\n",b1,nsec1);
		printf("b2=%d, %6ld nsec\n",b2,nsec2);
	
		printf("word = %X\n",resp);
	}

	return 0;
}

// End evdht11.c
