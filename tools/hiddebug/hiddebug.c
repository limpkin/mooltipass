#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <hidapi/hidapi.h>
#include <signal.h>

static int keepRunning = 1;

void intHandler() {
    keepRunning = 0;
}

int main(int argc, char* argv[])
{
	int res;
	unsigned char buf[65];
	hid_device *handle;

	signal(SIGINT, intHandler);

	wprintf( L"USB Debug Client\n" );

	// Initialize the hidapi library
	res = hid_init();

	// Open the device using the VID, PID,
	handle = hid_open(0x16C0, 0x047C, NULL);

	// enable non-blocking
	hid_set_nonblocking(handle, 1);

	while (keepRunning == 1)
	{

		// Request state (cmd 0x81). The first byte is the report number (0x0).
		buf[0] = 0x0;
		buf[1] = 0x81;
		res = hid_write(handle, buf, 65);

		// Read requested state
		res = hid_read(handle, buf, 65);

		if ( res > 2 )
		{
			int size = buf[0];
			int cmd = buf[1];
			buf[size + 2] = '\0';

			if ( cmd == 0x01 )	// debug msg
			{
				unsigned char *msg = buf + 2;
				wprintf(L"dbg: %s\n", msg);
			}
		}
	}

	// exit
	wprintf( L"Good Bye\n" );
	res = hid_exit();

	return 0;
}
