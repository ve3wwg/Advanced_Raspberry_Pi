//////////////////////////////////////////////////////////////////////
// controlusb.cpp -- Communicate with EZ-USB using Bulk EP
// Date: Fri Mar 30 09:54:18 2018   (C) ve3wwg@gmail.com
///////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <poll.h>
#include <termios.h>
#include <assert.h>

#include <usb.h>

class Tty {
	struct termios	saved,
			current;
	bool		rawf = false;
public:	Tty();
	~Tty();
	void raw_mode(bool enablef=true);
	int getc(int timeout_ms);
};

Tty::Tty() {
	int rc = tcgetattr(0,&saved);		// Get current serial attributes
	assert(rc != -1);
	current = saved;
}

Tty::~Tty() {
	if ( rawf ) {
		tcsetattr(0,TCSADRAIN,&saved);	// Restore original tty settings
		rawf = true;
	}
}

void
Tty::raw_mode(bool enablef) {
	if ( enablef ) {
		cfmakeraw(&current);			// Modify for raw I/O
		current.c_oflag |= ONLCR | OPOST;	// Translate LF as CRLF
		tcsetattr(0,TCSADRAIN,&current);	// Set stdin for raw I/O
		rawf = true;
	} else	{
		tcsetattr(0,TCSADRAIN,&saved);		// Restore tty
	}
}

int
Tty::getc(int timeout_ms) {
	struct pollfd polls[1];
	char ch;
	int rc;
	
	polls[0].fd	= 0;			// Standard input
	polls[0].events	= POLLIN;
	polls[0].revents = 0;
	rc = ::poll(&polls[0],1,timeout_ms);
	assert(rc >= 0);
	if ( rc == 0 )
		return -1;			// No data
	rc = ::read(0,&ch,1);
	if ( rc <= 0 )
		return -1;
	return ch;				// Return char
}

struct usb_device *
find_usb_device(unsigned id_vendor,unsigned id_product) {
	static struct usb_bus *busses = nullptr;
	struct usb_bus *bus = nullptr;
	struct usb_device *dev = nullptr;
	struct usb_device_descriptor *desc = nullptr;
	
	if ( !busses ) {
		usb_init();
		usb_find_busses();
		usb_find_devices();
		busses = usb_get_busses();
	}

	for ( bus=busses; bus; bus = bus->next ) {
		for ( dev=bus->devices; dev; dev = dev->next ) {
			desc = &dev->descriptor;

			// Looking for: Bus 001 Device 003: ID 04b4:8613 (unconfigured FX2)
			if ( desc->idVendor == id_vendor && desc->idProduct == id_product )
				return dev;
		}
	}
	return nullptr;	// Not found
}

int
main(int argc,char **argv) {
	Tty tty;
	int rc, ch;
	char buf[513];
	unsigned id_vendor = 0x04b4, id_product = 0x8613;
	struct usb_device *dev = find_usb_device(id_vendor,id_product);
	struct usb_dev_handle *hdev = nullptr;
	unsigned state = 0b0011;

	if ( !dev ) {
		fprintf(stderr,"Device not found. Vendor=0x%04X Product=0x%04X\n",id_vendor,id_product);
		return 1;
	}

	hdev = usb_open(dev);
	if ( !hdev ) {
		fprintf(stderr,"%s: opening USB device 0x%04X.0x%04X\n",usb_strerror(),id_vendor,id_product);
		return 1;
	}

	rc = usb_claim_interface(hdev,0);
	if ( rc != 0 ) {
		fprintf(stderr,"%s: Claiming interface 0.\n",usb_strerror());
		usb_close(hdev);
		return 2;
	}

	printf("Interface CLAIMED:\n");

	if ( usb_set_altinterface(hdev,1) < 0 ) {
		fprintf(stderr,"%s: usb_set_altinterface(h,1)\n",usb_strerror());
		return 3;
	}

	tty.raw_mode();

	for (;;) {
		if ( (ch = tty.getc(500)) == -1 ) {
			// Timed out: Try to read EP6
			rc = usb_bulk_read(hdev,0x86,buf,512,10/*ms*/);
			if ( rc < 0 ) {
				printf("%s: usb_bulk_read()\n\r",usb_strerror());
				break;
			} else	{
				assert(rc < int(sizeof buf));
				buf[rc] = 0;
				printf("Read %d bytes: %s\n\r",rc,buf);
			}
			if ( !isatty(0) )
				break;
		} else	{
			if ( ch == 'q' || ch == 'Q' || ch == 0x04 /*CTRL-D*/ )
				break;
			if ( ch == '0' || ch == '1' ) {
				unsigned mask = 1 << (ch & 1);

				state ^= mask;
				buf[0] = state;
				rc = usb_bulk_write(hdev,0x02,buf,1,10/*ms*/);
				if ( rc < 0 )
					printf("%s: write bulk to EP 2\n",usb_strerror());
				else	printf("Wrote %d bytes: 0x%02X  (state 0x%02X)\n",rc,unsigned(buf[0]),state);
			} else	{
				printf("Press q to quit, else 0 or 1 to toggle LED.\n");
			}
		}
	}

	rc = usb_release_interface(hdev,0);
	usb_close(hdev);
	return 0;
}

// End controlusb.cpp
