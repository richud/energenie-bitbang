/*
Depends libftdi-dev libftdi1 libusb-dev

Energenie remote code sender via FTDI bitbang
by Richud.com 2015

max 24 bit number 111111111111111111111111  = 16'777'215
 on 12345 000000000011000000111001
off 12344 000000000011000000111000 ,always one less

Remote '4514' code table
	ON			OFF
1	Received 12363535	Received 12363534
2	Received 12363527	Received 12363526
3	Received 12363531	Received 12363530
4	Received 12363523	Received 12363522
ALL	Received 12363533	Received 12363532


Default pin codes, 0x00 all off, others OR'd for on.
#define PIN_TX  0x01
#define PIX_RX  0x02
#define PIN_RTS 0x04
#define PIN_CTS 0x08
#define PIN_DTR 0x10
#define PIN_DSR 0x20
#define PIN_DCD 0x40
#define PIN_RI  0x80
*/

#include <time.h>
#include <stdio.h>
#include <limits.h>
#include <ftdi.h>

/* DEFINE are only things that should be changed */
#define DATA 0x02  /* This pin is the data line (DATA = ATAD on FS1000A, lol) */
#define VCC 0x80  /* This pin is used to power the device (VCC 3.3v) */


/* Generate a 1 or 0 pulse bit sequence and put it in array buffer */
void gen(char *buf, char **pos, int *t, char d)
{
	buf = *pos;	//set buffer array to next free row
	int x;
	for ( x = 0 ; x < *t; x++ )	//pulse
		*buf++ = d;
	*pos = buf;	//update pointer to next free row of buffer array
}

/* Convert integer to 32bit binary stored in an char array */
void dec2bin(char *dst, int x)
{
	int i;
	for (i = sizeof x * CHAR_BIT - 1; i >= 0; i--)
		*dst++ = (x >> i & 1) + '0';
}

/* Setup ftdi device */
struct ftdi_context ftdic;
void setupftdi(char **argv)
{
	ftdi_init(&ftdic); //initialize context

	if(ftdi_usb_open_desc(&ftdic, 0x0403, 0x6001, 0, argv[2]?argv[2]:0) < 0) {
		printf("Can't open device with Serial Number:%s\nTry running with sudo?\n", argv[2]);
		exit(1);
	}

	ftdi_usb_reset(&ftdic); //reset usb
	ftdi_usb_purge_buffers(&ftdic); //clean buffers
	ftdi_set_event_char(&ftdic, 0, 0); //disable event chars
	ftdi_set_error_char(&ftdic, 0, 0); //disable error chars
	ftdi_set_baudrate(&ftdic, 65536); //altering this value or moving it after setting bitbang mode will change timings!!
	ftdi_set_bitmode(&ftdic, VCC | DATA, BITMODE_BITBANG); //set bitbang mode on both pins
}


/* MAIN */
int main(int argc, char **argv)
{
	if (argc < 2) {
		puts("Usage: ./energenie code [serial]\ncode: 1 to 16777215\nserial: 8 character string");
		exit (1);
	}

	setupftdi(argv);

	char b[32]; //binary digits
	int n = atoi(argv[1]); //integer from command line
	dec2bin(b,n); //convert supplied integer to 32 bit binary

	/*
	The timings, these rely on set baudrate, 1 bit 'lasts' 4 us
	These times are 'relative' to OLS sample @ 50kHz. i.e. they changed when altering sample rate so not sure what true values are.
	They match very accurately what the remote generates, (which is about half what '433Utils/RPi_utils/codesend' produces, although both work!)
	*/
	int s = 220/4;					//short time, 220us
	int l = 660/4;					//long time, 660us [3x220]
	int g = 6380/4;					//inter packet gap, 6380us [9x220]
	int bsiz = ( 25 * (s + l) ) + g ;		//buffer size needed = 24 bits + 0 bit + interpacket gap
	char buffer[bsiz];				//buffer to send to ftdi_write_data
	char *buf = buffer;
	char *pos = buf;


	/* Generate each 24 binary digit to correct 0 or 1 pulse */
	int x;
	printf("Sending %d as ", n);
	for (x = 8; x < sizeof n * CHAR_BIT; x++) {	//24bit number so lop off first 8 0's from the 32bit dec2bin encoding
		printf("%i", b[x]-'0');			//print binary out
		if (b[x]-'0') {				/* pulse 1 */ 
			gen(buf, &pos, &l, VCC | DATA);	//long on
			gen(buf, &pos, &s, VCC);	//short off
		} else { 				/* pulse 0 */ 
			gen(buf, &pos, &s, VCC | DATA);	//short on
			gen(buf, &pos, &l, VCC);	//long off
		}
	}

	/* Generate the trailing 0 pulse, aka the 25th bit */
	gen(buf, &pos, &s, VCC | DATA);
	gen(buf, &pos, &l, VCC);
	
	/* Generate interpacket gap */
	gen(buf, &pos, &g, VCC);

	/* Send 7 repeat packets as per real remote */
	for ( x = 0; x < 7; x++ ) {
		ftdi_write_data(&ftdic, buf, bsiz);
	}
	puts("\nOk!\n");

	//power off xmitter
	char off = 0x00;
	ftdi_write_data(&ftdic, &off , 1);
}
