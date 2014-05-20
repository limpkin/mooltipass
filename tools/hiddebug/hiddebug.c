#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <hidapi/hidapi.h>
#include <signal.h>
#include <sys/time.h>

static int keepRunning = 1;

/* usb mooltipass hid commands */
#define CMD_DEBUG	0x01
#define CMD_PING	0x02
#define CMD_VERSION 0x03

hid_device *handle;

void intHandler() 
{
    keepRunning = 0;
}

void pingHandler()
{
	// send ping
	unsigned char buf[65];
	buf[0] = 0x00;	// endpoint 0
	buf[1] = 0x00;	// no data
	buf[2] = CMD_PING;	// send ping
	hid_write(handle, buf, 65);	

	// restart timer
  	struct itimerval tout_val;
  	tout_val.it_interval.tv_sec = 0;
  	tout_val.it_interval.tv_usec = 0;
  	tout_val.it_value.tv_sec = 10;
  	tout_val.it_value.tv_usec = 0;
  	setitimer(ITIMER_REAL, &tout_val, 0);

}

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[65];

	signal(SIGINT, intHandler);

	wprintf( L"USB Debug Client\n" );


	// Initialize the hidapi library
	res = hid_init();

	// Open the device using the VID, PID,
	handle = hid_open(0x16D0, 0x09A0, NULL);

  	struct itimerval tout_val;
  
  	tout_val.it_interval.tv_sec = 0;
  	tout_val.it_interval.tv_usec = 0;
  	tout_val.it_value.tv_sec = 10;
  	tout_val.it_value.tv_usec = 0;
  	setitimer(ITIMER_REAL, &tout_val,0);

	signal(SIGALRM,pingHandler);

	// enable non-blocking
	hid_set_nonblocking(handle, 1);

	buf[0] = 0x00;	// endpoint 0
	buf[1] = 0x00;	// no data
	buf[2] = CMD_VERSION;	// send ping
	res = hid_write(handle, buf, 65);

	while (keepRunning == 1)
	{
		res = hid_read(handle, buf, 65);

		if ( res > 2 )
		{
			int size = buf[0];
			int cmd = buf[1];
			buf[size + 2] = '\0';

			switch( cmd )
			{
				case CMD_DEBUG:	
				{
					unsigned char *msg = buf + 2;
					wprintf(L"dbg: %s\n", msg);
					break;
				}

				case CMD_PING:	
				{
					wprintf(L"cmd: Ping\n");
					break;
				}

				case CMD_VERSION: 	
				{
					wprintf(L"cmd: Version %i.%i\n", buf[2], buf[3]);
					break;
				}

				default:	
				{
					wprintf(L"unknown cmd");
					break;
				}

			}
		}
	}

	// exit
	wprintf( L"Good Bye\n" );
	res = hid_exit();

	return 0;
}
