#include <fx2regs.h>
#include <fx2sdly.h>

static void
initialize(void) {

	CPUCS = 0x10;		// 48 MHz, CLKOUT output disabled.
	IFCONFIG = 0xc0;	// Internal IFCLK, 48MHz; A,B as normal ports.
	SYNCDELAY;		
	REVCTL = 0x03;		// See TRM...
	SYNCDELAY;		
	EP6CFG = 0xE2;		// 1110 0010 bulk IN, 512 bytes, double-buffered
	SYNCDELAY;
	EP2CFG = 0xA2;		// 1010 0010 bulk OUT, 512 bytes, double-buffered
	SYNCDELAY;
	FIFORESET = 0x80;	// NAK all requests from host.
	SYNCDELAY;	
	FIFORESET = 0x82;	// Reset individual EP (2,4,6,8)
	SYNCDELAY;	
	FIFORESET = 0x84;
	SYNCDELAY;
	FIFORESET = 0x86;
	SYNCDELAY;
	FIFORESET = 0x88;
	SYNCDELAY;
	FIFORESET = 0x00;	// Resume normal operation.
	SYNCDELAY;		
	EP2FIFOCFG = 0x00;	// AUTOOUT=0
	SYNCDELAY;		
	OUTPKTEND = 0x82;	// Clear the 2 buffers
	SYNCDELAY;		
	OUTPKTEND = 0x82;	// ..both of them
	SYNCDELAY;		
	
}

static void
accept_cmd(void) {
	__xdata const unsigned char *src = EP2FIFOBUF;
	unsigned len = ((unsigned)EP2BCH)<<8 | EP2BCL;

	if ( len < 1 )
		return;		// Nothing to process
	PA0 = *src & 1;		// Set PA0 LED
	PA1 = *src & 2;		// Set PA1 LED

	OUTPKTEND = 0x82;	// Release buffer
}

static void
send_state(void) {
	// First, copy the data into the EP6 buffer.
	__xdata unsigned char *dest = EP6FIFOBUF;
	const char *msg1 = PA0 ? "PA0=1" : "PA0=0";
	const char *msg2 = PA1 ? "PA1=1" : "PA1=0";
	unsigned char len=0;

	while ( *msg1 ) {
		*dest++ = *msg1++;
		++len;  
	}
	*dest++ = ',';
	++len;
	while ( *msg2 ) {
		*dest++ = *msg2++;
		++len;
	}

	// Arm the endpoint. Be sure to set BCH before BCL because BCL access
	// actually arms the endpoint.
	SYNCDELAY;  
	EP6BCH=0;
	SYNCDELAY;  
	EP6BCL=len;
	
	// Data will be transmitted the next time
}

void
main(void) {

	OEA = 0x03;		// Enable PA0 and PA1 outputs
	initialize();
    
	PA0 = 1;
	PA1 = 1;

	for (;;) {
		if ( !(EP2CS & (1<<2)) )
			accept_cmd();

		if ( !(EP6CS & (1<<3)) ) {
			// EP6 buffer is not full
			send_state();
		}
	}
}
