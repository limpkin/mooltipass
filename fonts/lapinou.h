/*
 * Font lapin
 */

#define LAPIN_HEIGHT 12

const uint8_t lapin_0x21[] __attribute__((__progmem__)) = {   /* '!' width: 6 */
	  0xbf,  	     /* [. ......] */
};

const uint8_t lapin_0x22[] __attribute__((__progmem__)) = {   /* '"' width: 6 */
	  0xe0,  	    	     /* [...] */
	  0x00,  	    	     /* [   ] */
	  0xe0,  	    	     /* [...] */
};

const uint8_t lapin_0x23[] __attribute__((__progmem__)) = {   /* '#' width: 6 */
	  0x50,  	    	     /* [ . . ] */
	  0xf8,  	    	     /* [.....] */
	  0x50,  	    	     /* [ . . ] */
	  0xf8,  	    	     /* [.....] */
	  0x50,  	    	     /* [ . . ] */
};

const uint8_t lapin_0x24[] __attribute__((__progmem__)) = {   /* '$' width: 6 */
	  0x23,  0x00,  	     /* [  .   ..  ] */
	  0x44,  0x80,  	     /* [ .   .  . ] */
	  0xff,  0xc0,  	     /* [..........] */
	  0x44,  0x80,  	     /* [ .   .  . ] */
	  0x39,  0x00,  	     /* [  ...  .  ] */
};

const uint8_t lapin_0x25[] __attribute__((__progmem__)) = {   /* '%' width: 6 */
	  0xc6,  	     /* [..   .. ] */
	  0x29,  	     /* [  . .  .] */
	  0x77,  	     /* [ ... ...] */
	  0x99,  	     /* [.  ..  .] */
	  0x67,  	     /* [ ..  ...] */
};

const uint8_t lapin_0x26[] __attribute__((__progmem__)) = {   /* '&' width: 6 */
	  0x76,  	     /* [ ... .. ] */
	  0x89,  	     /* [.   .  .] */
	  0xb5,  	     /* [. .. . .] */
	  0x42,  	     /* [ .    . ] */
	  0xa0,  	     /* [. .     ] */
};

const uint8_t lapin_0x27[] __attribute__((__progmem__)) = {   /* ''' width: 6 */
	  0xe0,  	    	     /* [...] */
};

const uint8_t lapin_0x28[] __attribute__((__progmem__)) = {   /* '(' width: 6 */
	  0x3f,  0x00,  	     /* [  ......  ] */
	  0x40,  0x80,  	     /* [ .      . ] */
	  0x80,  0x40,  	     /* [.        .] */
};

const uint8_t lapin_0x29[] __attribute__((__progmem__)) = {   /* ')' width: 6 */
	  0x80,  0x40,  	     /* [.        .] */
	  0x40,  0x80,  	     /* [ .      . ] */
	  0x3f,  0x00,  	     /* [  ......  ] */
};

const uint8_t lapin_0x2a[] __attribute__((__progmem__)) = {   /* '*' width: 6 */
	  0x50,  	    	     /* [ . . ] */
	  0x20,  	    	     /* [  .  ] */
	  0xf8,  	    	     /* [.....] */
	  0x20,  	    	     /* [  .  ] */
	  0x50,  	    	     /* [ . . ] */
};

const uint8_t lapin_0x2b[] __attribute__((__progmem__)) = {   /* '+' width: 6 */
	  0x20,  	    	     /* [  .  ] */
	  0x20,  	    	     /* [  .  ] */
	  0xf8,  	    	     /* [.....] */
	  0x20,  	    	     /* [  .  ] */
	  0x20,  	    	     /* [  .  ] */
};

const uint8_t lapin_0x2c[] __attribute__((__progmem__)) = {   /* ',' width: 6 */
	  0xb0,  	    	     /* [. ..] */
	  0x70,  	    	     /* [ ...] */
};

const uint8_t lapin_0x2d[] __attribute__((__progmem__)) = {   /* '-' width: 6 */
	  0x80,  	    	     /* [.] */
	  0x80,  	    	     /* [.] */
	  0x80,  	    	     /* [.] */
	  0x80,  	    	     /* [.] */
	  0x80,  	    	     /* [.] */
};

const uint8_t lapin_0x2e[] __attribute__((__progmem__)) = {   /* '.' width: 6 */
	  0xc0,  	    	     /* [..] */
	  0xc0,  	    	     /* [..] */
};

const uint8_t lapin_0x2f[] __attribute__((__progmem__)) = {   /* '/' width: 6 */
	  0xc0,  0x00,  	     /* [..        ] */
	  0x30,  0x00,  	     /* [  ..      ] */
	  0x0c,  0x00,  	     /* [    ..    ] */
	  0x03,  0x00,  	     /* [      ..  ] */
	  0x00,  0xc0,  	     /* [        ..] */
};

const uint8_t lapin_0x30[] __attribute__((__progmem__)) = {   /* '0' width: 6 */
	  0x7e,  	     /* [ ...... ] */
	  0x91,  	     /* [.  .   .] */
	  0x89,  	     /* [.   .  .] */
	  0x85,  	     /* [.    . .] */
	  0x7e,  	     /* [ ...... ] */
};

const uint8_t lapin_0x31[] __attribute__((__progmem__)) = {   /* '1' width: 6 */
	  0x82,  	     /* [.     . ] */
	  0x82,  	     /* [.     . ] */
	  0xff,  	     /* [........] */
	  0x80,  	     /* [.       ] */
	  0x80,  	     /* [.       ] */
};

const uint8_t lapin_0x32[] __attribute__((__progmem__)) = {   /* '2' width: 6 */
	  0xc2,  	     /* [..    . ] */
	  0xa1,  	     /* [. .    .] */
	  0x91,  	     /* [.  .   .] */
	  0x89,  	     /* [.   .  .] */
	  0x86,  	     /* [.    .. ] */
};

const uint8_t lapin_0x33[] __attribute__((__progmem__)) = {   /* '3' width: 6 */
	  0x42,  	     /* [ .    . ] */
	  0x81,  	     /* [.      .] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x76,  	     /* [ ... .. ] */
};

const uint8_t lapin_0x34[] __attribute__((__progmem__)) = {   /* '4' width: 6 */
	  0x18,  	     /* [   ..   ] */
	  0x14,  	     /* [   . .  ] */
	  0x92,  	     /* [.  .  . ] */
	  0xff,  	     /* [........] */
	  0x90,  	     /* [.  .    ] */
};

const uint8_t lapin_0x35[] __attribute__((__progmem__)) = {   /* '5' width: 6 */
	  0x4f,  	     /* [ .  ....] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x71,  	     /* [ ...   .] */
};

const uint8_t lapin_0x36[] __attribute__((__progmem__)) = {   /* '6' width: 6 */
	  0x7e,  	     /* [ ...... ] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x70,  	     /* [ ...    ] */
};

const uint8_t lapin_0x37[] __attribute__((__progmem__)) = {   /* '7' width: 6 */
	  0x01,  	     /* [       .] */
	  0x01,  	     /* [       .] */
	  0xf1,  	     /* [....   .] */
	  0x09,  	     /* [    .  .] */
	  0x07,  	     /* [     ...] */
};

const uint8_t lapin_0x38[] __attribute__((__progmem__)) = {   /* '8' width: 6 */
	  0x76,  	     /* [ ... .. ] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x76,  	     /* [ ... .. ] */
};

const uint8_t lapin_0x39[] __attribute__((__progmem__)) = {   /* '9' width: 6 */
	  0x0e,  	     /* [    ... ] */
	  0x91,  	     /* [.  .   .] */
	  0x91,  	     /* [.  .   .] */
	  0x91,  	     /* [.  .   .] */
	  0x7e,  	     /* [ ...... ] */
};

const uint8_t lapin_0x3a[] __attribute__((__progmem__)) = {   /* ':' width: 6 */
	  0xcc,  	    	     /* [..  ..] */
	  0xcc,  	    	     /* [..  ..] */
};

const uint8_t lapin_0x3b[] __attribute__((__progmem__)) = {   /* ';' width: 6 */
	  0xb3,  	     /* [. ..  ..] */
	  0x73,  	     /* [ ...  ..] */
};

const uint8_t lapin_0x3c[] __attribute__((__progmem__)) = {   /* '<' width: 6 */
	  0x10,  	    	     /* [   .   ] */
	  0x28,  	    	     /* [  . .  ] */
	  0x44,  	    	     /* [ .   . ] */
	  0x82,  	    	     /* [.     .] */
};

const uint8_t lapin_0x3d[] __attribute__((__progmem__)) = {   /* '=' width: 6 */
	  0xa0,  	    	     /* [. .] */
	  0xa0,  	    	     /* [. .] */
	  0xa0,  	    	     /* [. .] */
	  0xa0,  	    	     /* [. .] */
	  0xa0,  	    	     /* [. .] */
};

const uint8_t lapin_0x3e[] __attribute__((__progmem__)) = {   /* '>' width: 6 */
	  0x82,  	    	     /* [.     .] */
	  0x44,  	    	     /* [ .   . ] */
	  0x28,  	    	     /* [  . .  ] */
	  0x10,  	    	     /* [   .   ] */
};

const uint8_t lapin_0x3f[] __attribute__((__progmem__)) = {   /* '?' width: 6 */
	  0x02,  	     /* [      . ] */
	  0x01,  	     /* [       .] */
	  0xa1,  	     /* [. .    .] */
	  0x11,  	     /* [   .   .] */
	  0x0e,  	     /* [    ... ] */
};

const uint8_t lapin_0x40[] __attribute__((__progmem__)) = {   /* '@' width: 6 */
	  0x7e,  	     /* [ ...... ] */
	  0x81,  	     /* [.      .] */
	  0x9d,  	     /* [.  ... .] */
	  0x95,  	     /* [.  . . .] */
	  0x9e,  	     /* [.  .... ] */
};

const uint8_t lapin_0x41[] __attribute__((__progmem__)) = {   /* 'A' width: 6 */
	  0xf8,  	     /* [.....   ] */
	  0x26,  	     /* [  .  .. ] */
	  0x21,  	     /* [  .    .] */
	  0x26,  	     /* [  .  .. ] */
	  0xf8,  	     /* [.....   ] */
};

const uint8_t lapin_0x42[] __attribute__((__progmem__)) = {   /* 'B' width: 6 */
	  0xff,  	     /* [........] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x76,  	     /* [ ... .. ] */
};

const uint8_t lapin_0x43[] __attribute__((__progmem__)) = {   /* 'C' width: 6 */
	  0x7e,  	     /* [ ...... ] */
	  0x81,  	     /* [.      .] */
	  0x81,  	     /* [.      .] */
	  0x81,  	     /* [.      .] */
	  0x42,  	     /* [ .    . ] */
};

const uint8_t lapin_0x44[] __attribute__((__progmem__)) = {   /* 'D' width: 6 */
	  0xff,  	     /* [........] */
	  0x81,  	     /* [.      .] */
	  0x81,  	     /* [.      .] */
	  0x81,  	     /* [.      .] */
	  0x7e,  	     /* [ ...... ] */
};

const uint8_t lapin_0x45[] __attribute__((__progmem__)) = {   /* 'E' width: 6 */
	  0xff,  	     /* [........] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x81,  	     /* [.      .] */
};

const uint8_t lapin_0x46[] __attribute__((__progmem__)) = {   /* 'F' width: 6 */
	  0xff,  	     /* [........] */
	  0x09,  	     /* [    .  .] */
	  0x09,  	     /* [    .  .] */
	  0x09,  	     /* [    .  .] */
	  0x01,  	     /* [       .] */
};

const uint8_t lapin_0x47[] __attribute__((__progmem__)) = {   /* 'G' width: 6 */
	  0x7e,  	     /* [ ...... ] */
	  0x81,  	     /* [.      .] */
	  0x81,  	     /* [.      .] */
	  0x91,  	     /* [.  .   .] */
	  0x72,  	     /* [ ...  . ] */
};

const uint8_t lapin_0x48[] __attribute__((__progmem__)) = {   /* 'H' width: 6 */
	  0xff,  	     /* [........] */
	  0x08,  	     /* [    .   ] */
	  0x08,  	     /* [    .   ] */
	  0x08,  	     /* [    .   ] */
	  0xff,  	     /* [........] */
};

const uint8_t lapin_0x49[] __attribute__((__progmem__)) = {   /* 'I' width: 6 */
	  0x81,  	     /* [.      .] */
	  0x81,  	     /* [.      .] */
	  0xff,  	     /* [........] */
	  0x81,  	     /* [.      .] */
	  0x81,  	     /* [.      .] */
};

const uint8_t lapin_0x4a[] __attribute__((__progmem__)) = {   /* 'J' width: 6 */
	  0x60,  	     /* [ ..     ] */
	  0x80,  	     /* [.       ] */
	  0x80,  	     /* [.       ] */
	  0x80,  	     /* [.       ] */
	  0x7f,  	     /* [ .......] */
};

const uint8_t lapin_0x4b[] __attribute__((__progmem__)) = {   /* 'K' width: 6 */
	  0xff,  	     /* [........] */
	  0x18,  	     /* [   ..   ] */
	  0x24,  	     /* [  .  .  ] */
	  0x42,  	     /* [ .    . ] */
	  0x81,  	     /* [.      .] */
};

const uint8_t lapin_0x4c[] __attribute__((__progmem__)) = {   /* 'L' width: 6 */
	  0xff,  	     /* [........] */
	  0x80,  	     /* [.       ] */
	  0x80,  	     /* [.       ] */
	  0x80,  	     /* [.       ] */
	  0x80,  	     /* [.       ] */
};

const uint8_t lapin_0x4d[] __attribute__((__progmem__)) = {   /* 'M' width: 6 */
	  0xff,  	     /* [........] */
	  0x02,  	     /* [      . ] */
	  0x0c,  	     /* [    ..  ] */
	  0x02,  	     /* [      . ] */
	  0xff,  	     /* [........] */
};

const uint8_t lapin_0x4e[] __attribute__((__progmem__)) = {   /* 'N' width: 6 */
	  0xff,  	     /* [........] */
	  0x02,  	     /* [      . ] */
	  0x04,  	     /* [     .  ] */
	  0x08,  	     /* [    .   ] */
	  0xff,  	     /* [........] */
};

const uint8_t lapin_0x4f[] __attribute__((__progmem__)) = {   /* 'O' width: 6 */
	  0x7e,  	     /* [ ...... ] */
	  0x81,  	     /* [.      .] */
	  0x81,  	     /* [.      .] */
	  0x81,  	     /* [.      .] */
	  0x7e,  	     /* [ ...... ] */
};

const uint8_t lapin_0x50[] __attribute__((__progmem__)) = {   /* 'P' width: 6 */
	  0xff,  	     /* [........] */
	  0x09,  	     /* [    .  .] */
	  0x09,  	     /* [    .  .] */
	  0x09,  	     /* [    .  .] */
	  0x06,  	     /* [     .. ] */
};

const uint8_t lapin_0x51[] __attribute__((__progmem__)) = {   /* 'Q' width: 6 */
	  0x3f,  0x00,  	     /* [  ...... ] */
	  0x40,  0x80,  	     /* [ .      .] */
	  0x60,  0x80,  	     /* [ ..     .] */
	  0x40,  0x80,  	     /* [ .      .] */
	  0xbf,  0x00,  	     /* [. ...... ] */
};

const uint8_t lapin_0x52[] __attribute__((__progmem__)) = {   /* 'R' width: 6 */
	  0xff,  	     /* [........] */
	  0x09,  	     /* [    .  .] */
	  0x09,  	     /* [    .  .] */
	  0x09,  	     /* [    .  .] */
	  0xf6,  	     /* [.... .. ] */
};

const uint8_t lapin_0x53[] __attribute__((__progmem__)) = {   /* 'S' width: 6 */
	  0x46,  	     /* [ .   .. ] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x89,  	     /* [.   .  .] */
	  0x72,  	     /* [ ...  . ] */
};

const uint8_t lapin_0x54[] __attribute__((__progmem__)) = {   /* 'T' width: 6 */
	  0x01,  	     /* [       .] */
	  0x01,  	     /* [       .] */
	  0xff,  	     /* [........] */
	  0x01,  	     /* [       .] */
	  0x01,  	     /* [       .] */
};

const uint8_t lapin_0x55[] __attribute__((__progmem__)) = {   /* 'U' width: 6 */
	  0x7f,  	     /* [ .......] */
	  0x80,  	     /* [.       ] */
	  0x80,  	     /* [.       ] */
	  0x80,  	     /* [.       ] */
	  0x7f,  	     /* [ .......] */
};

const uint8_t lapin_0x56[] __attribute__((__progmem__)) = {   /* 'V' width: 6 */
	  0x0f,  	     /* [    ....] */
	  0x30,  	     /* [  ..    ] */
	  0xc0,  	     /* [..      ] */
	  0x30,  	     /* [  ..    ] */
	  0x0f,  	     /* [    ....] */
};

const uint8_t lapin_0x57[] __attribute__((__progmem__)) = {   /* 'W' width: 6 */
	  0xff,  	     /* [........] */
	  0x40,  	     /* [ .      ] */
	  0x30,  	     /* [  ..    ] */
	  0x40,  	     /* [ .      ] */
	  0xff,  	     /* [........] */
};

const uint8_t lapin_0x58[] __attribute__((__progmem__)) = {   /* 'X' width: 6 */
	  0xc7,  	     /* [..   ...] */
	  0x28,  	     /* [  . .   ] */
	  0x10,  	     /* [   .    ] */
	  0x28,  	     /* [  . .   ] */
	  0xc7,  	     /* [..   ...] */
};

const uint8_t lapin_0x59[] __attribute__((__progmem__)) = {   /* 'Y' width: 6 */
	  0x0f,  	     /* [    ....] */
	  0x10,  	     /* [   .    ] */
	  0xe0,  	     /* [...     ] */
	  0x10,  	     /* [   .    ] */
	  0x0f,  	     /* [    ....] */
};

const uint8_t lapin_0x5a[] __attribute__((__progmem__)) = {   /* 'Z' width: 6 */
	  0xe1,  	     /* [...    .] */
	  0x91,  	     /* [.  .   .] */
	  0x89,  	     /* [.   .  .] */
	  0x85,  	     /* [.    . .] */
	  0x83,  	     /* [.     ..] */
};

const uint8_t lapin_0x5b[] __attribute__((__progmem__)) = {   /* '[' width: 6 */
	  0xff,  0xc0,  	     /* [..........] */
	  0x80,  0x40,  	     /* [.        .] */
};

const uint8_t lapin_0x5c[] __attribute__((__progmem__)) = {   /* '\' width: 6 */
	  0x00,  0xc0,  	     /* [        ..] */
	  0x03,  0x00,  	     /* [      ..  ] */
	  0x0c,  0x00,  	     /* [    ..    ] */
	  0x30,  0x00,  	     /* [  ..      ] */
	  0xc0,  0x00,  	     /* [..        ] */
};

const uint8_t lapin_0x5d[] __attribute__((__progmem__)) = {   /* ']' width: 6 */
	  0x80,  0x40,  	     /* [.        .] */
	  0xff,  0xc0,  	     /* [..........] */
};

const uint8_t lapin_0x5e[] __attribute__((__progmem__)) = {   /* '^' width: 6 */
	  0x80,  	    	     /* [.  ] */
	  0x40,  	    	     /* [ . ] */
	  0x20,  	    	     /* [  .] */
	  0x40,  	    	     /* [ . ] */
	  0x80,  	    	     /* [.  ] */
};

const uint8_t lapin_0x5f[] __attribute__((__progmem__)) = {   /* '_' width: 6 */
	  0x80,  	    	     /* [.] */
	  0x80,  	    	     /* [.] */
	  0x80,  	    	     /* [.] */
	  0x80,  	    	     /* [.] */
	  0x80,  	    	     /* [.] */
	  0x80,  	    	     /* [.] */
};

const uint8_t lapin_0x60[] __attribute__((__progmem__)) = {   /* '`' width: 6 */
	  0x20,  	    	     /* [  .] */
	  0x40,  	    	     /* [ . ] */
	  0x80,  	    	     /* [.  ] */
};

const uint8_t lapin_0x61[] __attribute__((__progmem__)) = {   /* 'a' width: 6 */
	  0x70,  	    	     /* [ ... ] */
	  0x88,  	    	     /* [.   .] */
	  0x88,  	    	     /* [.   .] */
	  0x48,  	    	     /* [ .  .] */
	  0xf8,  	    	     /* [.....] */
};

const uint8_t lapin_0x62[] __attribute__((__progmem__)) = {   /* 'b' width: 6 */
	  0xff,  	     /* [........] */
	  0x88,  	     /* [.   .   ] */
	  0x88,  	     /* [.   .   ] */
	  0x88,  	     /* [.   .   ] */
	  0x70,  	     /* [ ...    ] */
};

const uint8_t lapin_0x63[] __attribute__((__progmem__)) = {   /* 'c' width: 6 */
	  0x70,  	    	     /* [ ... ] */
	  0x88,  	    	     /* [.   .] */
	  0x88,  	    	     /* [.   .] */
	  0x88,  	    	     /* [.   .] */
	  0x90,  	    	     /* [.  . ] */
};

const uint8_t lapin_0x64[] __attribute__((__progmem__)) = {   /* 'd' width: 6 */
	  0x70,  	     /* [ ...    ] */
	  0x88,  	     /* [.   .   ] */
	  0x88,  	     /* [.   .   ] */
	  0x88,  	     /* [.   .   ] */
	  0xff,  	     /* [........] */
};

const uint8_t lapin_0x65[] __attribute__((__progmem__)) = {   /* 'e' width: 6 */
	  0x70,  	    	     /* [ ... ] */
	  0xa8,  	    	     /* [. . .] */
	  0xa8,  	    	     /* [. . .] */
	  0xa8,  	    	     /* [. . .] */
	  0xb0,  	    	     /* [. .. ] */
};

const uint8_t lapin_0x66[] __attribute__((__progmem__)) = {   /* 'f' width: 6 */
	  0x08,  	     /* [    .   ] */
	  0xfe,  	     /* [....... ] */
	  0x09,  	     /* [    .  .] */
	  0x01,  	     /* [       .] */
};

const uint8_t lapin_0x67[] __attribute__((__progmem__)) = {   /* 'g' width: 6 */
	  0x1c,  	    	     /* [   ... ] */
	  0xa2,  	    	     /* [. .   .] */
	  0xa2,  	    	     /* [. .   .] */
	  0xa2,  	    	     /* [. .   .] */
	  0x7e,  	    	     /* [ ......] */
};

const uint8_t lapin_0x68[] __attribute__((__progmem__)) = {   /* 'h' width: 6 */
	  0xff,  	     /* [........] */
	  0x08,  	     /* [    .   ] */
	  0x08,  	     /* [    .   ] */
	  0x08,  	     /* [    .   ] */
	  0xf0,  	     /* [....    ] */
};

const uint8_t lapin_0x69[] __attribute__((__progmem__)) = {   /* 'i' width: 6 */
	  0x88,  	     /* [.   .   ] */
	  0xf9,  	     /* [.....  .] */
	  0x80,  	     /* [.       ] */
};

const uint8_t lapin_0x6a[] __attribute__((__progmem__)) = {   /* 'j' width: 6 */
	  0x80,  0x00,  	     /* [.         ] */
	  0x82,  0x00,  	     /* [.     .   ] */
	  0x7e,  0x40,  	     /* [ ......  .] */
};

const uint8_t lapin_0x6b[] __attribute__((__progmem__)) = {   /* 'k' width: 6 */
	  0xff,  	     /* [........] */
	  0x20,  	     /* [  .     ] */
	  0x30,  	     /* [  ..    ] */
	  0x48,  	     /* [ .  .   ] */
	  0x80,  	     /* [.       ] */
};

const uint8_t lapin_0x6c[] __attribute__((__progmem__)) = {   /* 'l' width: 6 */
	  0x81,  	     /* [.      .] */
	  0xff,  	     /* [........] */
	  0x80,  	     /* [.       ] */
};

const uint8_t lapin_0x6d[] __attribute__((__progmem__)) = {   /* 'm' width: 6 */
	  0xf8,  	    	     /* [.....] */
	  0x08,  	    	     /* [    .] */
	  0xf8,  	    	     /* [.....] */
	  0x08,  	    	     /* [    .] */
	  0xf0,  	    	     /* [.... ] */
};

const uint8_t lapin_0x6e[] __attribute__((__progmem__)) = {   /* 'n' width: 6 */
	  0xf8,  	    	     /* [.....] */
	  0x10,  	    	     /* [   . ] */
	  0x08,  	    	     /* [    .] */
	  0x08,  	    	     /* [    .] */
	  0xf0,  	    	     /* [.... ] */
};

const uint8_t lapin_0x6f[] __attribute__((__progmem__)) = {   /* 'o' width: 6 */
	  0x70,  	    	     /* [ ... ] */
	  0x88,  	    	     /* [.   .] */
	  0x88,  	    	     /* [.   .] */
	  0x88,  	    	     /* [.   .] */
	  0x70,  	    	     /* [ ... ] */
};

const uint8_t lapin_0x70[] __attribute__((__progmem__)) = {   /* 'p' width: 6 */
	  0xfe,  	    	     /* [.......] */
	  0x22,  	    	     /* [  .   .] */
	  0x22,  	    	     /* [  .   .] */
	  0x22,  	    	     /* [  .   .] */
	  0x1c,  	    	     /* [   ... ] */
};

const uint8_t lapin_0x71[] __attribute__((__progmem__)) = {   /* 'q' width: 6 */
	  0x1c,  	    	     /* [   ... ] */
	  0x22,  	    	     /* [  .   .] */
	  0x22,  	    	     /* [  .   .] */
	  0x22,  	    	     /* [  .   .] */
	  0xfe,  	    	     /* [.......] */
};

const uint8_t lapin_0x72[] __attribute__((__progmem__)) = {   /* 'r' width: 6 */
	  0xf8,  	    	     /* [.....] */
	  0x10,  	    	     /* [   . ] */
	  0x08,  	    	     /* [    .] */
	  0x08,  	    	     /* [    .] */
	  0x10,  	    	     /* [   . ] */
};

const uint8_t lapin_0x73[] __attribute__((__progmem__)) = {   /* 's' width: 6 */
	  0x90,  	    	     /* [.  . ] */
	  0xa8,  	    	     /* [. . .] */
	  0xa8,  	    	     /* [. . .] */
	  0xa8,  	    	     /* [. . .] */
	  0x48,  	    	     /* [ .  .] */
};

const uint8_t lapin_0x74[] __attribute__((__progmem__)) = {   /* 't' width: 6 */
	  0x08,  	     /* [    .   ] */
	  0x7f,  	     /* [ .......] */
	  0x88,  	     /* [.   .   ] */
	  0x80,  	     /* [.       ] */
};

const uint8_t lapin_0x75[] __attribute__((__progmem__)) = {   /* 'u' width: 6 */
	  0x78,  	    	     /* [ ....] */
	  0x80,  	    	     /* [.    ] */
	  0x80,  	    	     /* [.    ] */
	  0x40,  	    	     /* [ .   ] */
	  0xf8,  	    	     /* [.....] */
};

const uint8_t lapin_0x76[] __attribute__((__progmem__)) = {   /* 'v' width: 6 */
	  0x18,  	    	     /* [   ..] */
	  0x60,  	    	     /* [ ..  ] */
	  0x80,  	    	     /* [.    ] */
	  0x60,  	    	     /* [ ..  ] */
	  0x18,  	    	     /* [   ..] */
};

const uint8_t lapin_0x77[] __attribute__((__progmem__)) = {   /* 'w' width: 6 */
	  0x78,  	    	     /* [ ....] */
	  0x80,  	    	     /* [.    ] */
	  0x78,  	    	     /* [ ....] */
	  0x80,  	    	     /* [.    ] */
	  0x78,  	    	     /* [ ....] */
};

const uint8_t lapin_0x78[] __attribute__((__progmem__)) = {   /* 'x' width: 6 */
	  0x88,  	    	     /* [.   .] */
	  0x50,  	    	     /* [ . . ] */
	  0x20,  	    	     /* [  .  ] */
	  0x50,  	    	     /* [ . . ] */
	  0x88,  	    	     /* [.   .] */
};

const uint8_t lapin_0x79[] __attribute__((__progmem__)) = {   /* 'y' width: 6 */
	  0x1e,  	    	     /* [   ....] */
	  0xa0,  	    	     /* [. .    ] */
	  0xa0,  	    	     /* [. .    ] */
	  0xa0,  	    	     /* [. .    ] */
	  0x7e,  	    	     /* [ ......] */
};

const uint8_t lapin_0x7a[] __attribute__((__progmem__)) = {   /* 'z' width: 6 */
	  0x88,  	    	     /* [.   .] */
	  0xc8,  	    	     /* [..  .] */
	  0xa8,  	    	     /* [. . .] */
	  0x98,  	    	     /* [.  ..] */
	  0x88,  	    	     /* [.   .] */
};

const uint8_t lapin_0x7b[] __attribute__((__progmem__)) = {   /* '{' width: 6 */
	  0x04,  0x00,  	     /* [     .     ] */
	  0x7b,  0xc0,  	     /* [ .... .... ] */
	  0x80,  0x20,  	     /* [.         .] */
};

const uint8_t lapin_0x7c[] __attribute__((__progmem__)) = {   /* '|' width: 6 */
	  0xff,  0xc0,  	     /* [..........] */
};

const uint8_t lapin_0x7d[] __attribute__((__progmem__)) = {   /* '}' width: 6 */
	  0x80,  0x20,  	     /* [.         .] */
	  0x7b,  0xc0,  	     /* [ .... .... ] */
	  0x04,  0x00,  	     /* [     .     ] */
};

const uint8_t lapin_0x7e[] __attribute__((__progmem__)) = {   /* '~' width: 6 */
	  0x80,  	    	     /* [. ] */
	  0x40,  	    	     /* [ .] */
	  0xc0,  	    	     /* [..] */
	  0x80,  	    	     /* [. ] */
	  0x40,  	    	     /* [ .] */
};


/* Mapping from ASCII codes to font characters, from space (0x20) to del (0x7f) */
const uint8_t lapin_asciimap[223] __attribute__((__progmem__)) = {  
	      0,    1,    2,    3,    4,    5,    6,    7,    8,    9,   10,   11,   12,   13,   14,   15,  
	     16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,  
	     32,   33,   34,   35,   36,   37,   38,   39,   40,   41,   42,   43,   44,   45,   46,   47,  
	     48,   49,   50,   51,   52,   53,   54,   55,   56,   57,   58,   59,   60,   61,   62,   63,  
	     64,   65,   66,   67,   68,   69,   70,   71,   72,   73,   74,   75,   76,   77,   78,   79,  
	     80,   81,   82,   83,   84,   85,   86,   87,   88,   89,   90,   91,   92,   93,   94,  255,  
	    255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  
	    255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  
	    255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  
	    255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  
	    255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  
	    255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  
	    255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  
	    255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  255,  };

const glyph_t lapin[] __attribute__((__progmem__)) = {
	   {  7,  1,  1,  0, 10, -1 }, /* ' ' */
	   {  6,  1,  8,  2,  3, lapin_0x21 }, /* '!' */
	   {  6,  3,  3,  1,  2, lapin_0x22 }, /* '"' */
	   {  6,  5,  5,  0,  3, lapin_0x23 }, /* '#' */
	   {  6,  5, 10,  0,  2, lapin_0x24 }, /* '$' */
	   {  6,  5,  8,  0,  3, lapin_0x25 }, /* '%' */
	   {  6,  5,  8,  0,  3, lapin_0x26 }, /* '&' */
	   {  6,  1,  3,  2,  2, lapin_0x27 }, /* ''' */
	   {  6,  3, 10,  1,  2, lapin_0x28 }, /* '(' */
	   {  6,  3, 10,  1,  2, lapin_0x29 }, /* ')' */
	   {  6,  5,  5,  0,  3, lapin_0x2a }, /* '*' */
	   {  6,  5,  5,  0,  5, lapin_0x2b }, /* '+' */
	   {  6,  2,  4,  1,  9, lapin_0x2c }, /* ',' */
	   {  6,  5,  1,  0,  7, lapin_0x2d }, /* '-' */
	   {  6,  2,  2,  2,  9, lapin_0x2e }, /* '.' */
	   {  6,  5, 10,  0,  3, lapin_0x2f }, /* '/' */
	   {  6,  5,  8,  0,  3, lapin_0x30 }, /* '0' */
	   {  6,  5,  8,  0,  3, lapin_0x31 }, /* '1' */
	   {  6,  5,  8,  0,  3, lapin_0x32 }, /* '2' */
	   {  6,  5,  8,  0,  3, lapin_0x33 }, /* '3' */
	   {  6,  5,  8,  0,  3, lapin_0x34 }, /* '4' */
	   {  6,  5,  8,  0,  3, lapin_0x35 }, /* '5' */
	   {  6,  5,  8,  0,  3, lapin_0x36 }, /* '6' */
	   {  6,  5,  8,  0,  3, lapin_0x37 }, /* '7' */
	   {  6,  5,  8,  0,  3, lapin_0x38 }, /* '8' */
	   {  6,  5,  8,  0,  3, lapin_0x39 }, /* '9' */
	   {  6,  2,  6,  2,  5, lapin_0x3a }, /* ':' */
	   {  6,  2,  8,  1,  5, lapin_0x3b }, /* ';' */
	   {  6,  4,  7,  1,  3, lapin_0x3c }, /* '<' */
	   {  6,  5,  3,  0,  5, lapin_0x3d }, /* '=' */
	   {  6,  4,  7,  1,  3, lapin_0x3e }, /* '>' */
	   {  6,  5,  8,  0,  3, lapin_0x3f }, /* '?' */
	   {  6,  5,  8,  0,  3, lapin_0x40 }, /* '@' */
	   {  6,  5,  8,  0,  3, lapin_0x41 }, /* 'A' */
	   {  6,  5,  8,  0,  3, lapin_0x42 }, /* 'B' */
	   {  6,  5,  8,  0,  3, lapin_0x43 }, /* 'C' */
	   {  6,  5,  8,  0,  3, lapin_0x44 }, /* 'D' */
	   {  6,  5,  8,  0,  3, lapin_0x45 }, /* 'E' */
	   {  6,  5,  8,  0,  3, lapin_0x46 }, /* 'F' */
	   {  6,  5,  8,  0,  3, lapin_0x47 }, /* 'G' */
	   {  6,  5,  8,  0,  3, lapin_0x48 }, /* 'H' */
	   {  6,  5,  8,  0,  3, lapin_0x49 }, /* 'I' */
	   {  6,  5,  8,  0,  3, lapin_0x4a }, /* 'J' */
	   {  6,  5,  8,  0,  3, lapin_0x4b }, /* 'K' */
	   {  6,  5,  8,  0,  3, lapin_0x4c }, /* 'L' */
	   {  6,  5,  8,  0,  3, lapin_0x4d }, /* 'M' */
	   {  6,  5,  8,  0,  3, lapin_0x4e }, /* 'N' */
	   {  6,  5,  8,  0,  3, lapin_0x4f }, /* 'O' */
	   {  6,  5,  8,  0,  3, lapin_0x50 }, /* 'P' */
	   {  6,  5,  9,  0,  3, lapin_0x51 }, /* 'Q' */
	   {  6,  5,  8,  0,  3, lapin_0x52 }, /* 'R' */
	   {  6,  5,  8,  0,  3, lapin_0x53 }, /* 'S' */
	   {  6,  5,  8,  0,  3, lapin_0x54 }, /* 'T' */
	   {  6,  5,  8,  0,  3, lapin_0x55 }, /* 'U' */
	   {  6,  5,  8,  0,  3, lapin_0x56 }, /* 'V' */
	   {  6,  5,  8,  0,  3, lapin_0x57 }, /* 'W' */
	   {  6,  5,  8,  0,  3, lapin_0x58 }, /* 'X' */
	   {  6,  5,  8,  0,  3, lapin_0x59 }, /* 'Y' */
	   {  6,  5,  8,  0,  3, lapin_0x5a }, /* 'Z' */
	   {  6,  2, 10,  2,  2, lapin_0x5b }, /* '[' */
	   {  6,  5, 10,  1,  3, lapin_0x5c }, /* '\' */
	   {  6,  2, 10,  1,  2, lapin_0x5d }, /* ']' */
	   {  6,  5,  3,  0,  3, lapin_0x5e }, /* '^' */
	   {  6,  6,  1,  0, 12, lapin_0x5f }, /* '_' */
	   {  6,  3,  3,  1,  2, lapin_0x60 }, /* '`' */
	   {  6,  5,  5,  0,  6, lapin_0x61 }, /* 'a' */
	   {  6,  5,  8,  0,  3, lapin_0x62 }, /* 'b' */
	   {  6,  5,  5,  0,  6, lapin_0x63 }, /* 'c' */
	   {  6,  5,  8,  0,  3, lapin_0x64 }, /* 'd' */
	   {  6,  5,  5,  0,  6, lapin_0x65 }, /* 'e' */
	   {  6,  4,  8,  1,  3, lapin_0x66 }, /* 'f' */
	   {  6,  5,  7,  0,  6, lapin_0x67 }, /* 'g' */
	   {  6,  5,  8,  0,  3, lapin_0x68 }, /* 'h' */
	   {  6,  3,  8,  1,  3, lapin_0x69 }, /* 'i' */
	   {  6,  3, 10,  0,  3, lapin_0x6a }, /* 'j' */
	   {  6,  5,  8,  0,  3, lapin_0x6b }, /* 'k' */
	   {  6,  3,  8,  1,  3, lapin_0x6c }, /* 'l' */
	   {  6,  5,  5,  0,  6, lapin_0x6d }, /* 'm' */
	   {  6,  5,  5,  0,  6, lapin_0x6e }, /* 'n' */
	   {  6,  5,  5,  0,  6, lapin_0x6f }, /* 'o' */
	   {  6,  5,  7,  0,  6, lapin_0x70 }, /* 'p' */
	   {  6,  5,  7,  0,  6, lapin_0x71 }, /* 'q' */
	   {  6,  5,  5,  0,  6, lapin_0x72 }, /* 'r' */
	   {  6,  5,  5,  0,  6, lapin_0x73 }, /* 's' */
	   {  6,  4,  8,  1,  3, lapin_0x74 }, /* 't' */
	   {  6,  5,  5,  0,  6, lapin_0x75 }, /* 'u' */
	   {  6,  5,  5,  0,  6, lapin_0x76 }, /* 'v' */
	   {  6,  5,  5,  0,  6, lapin_0x77 }, /* 'w' */
	   {  6,  5,  5,  0,  6, lapin_0x78 }, /* 'x' */
	   {  6,  5,  7,  0,  6, lapin_0x79 }, /* 'y' */
	   {  6,  5,  5,  0,  6, lapin_0x7a }, /* 'z' */
	   {  6,  3, 11,  1,  2, lapin_0x7b }, /* '{' */
	   {  6,  1, 10,  2,  2, lapin_0x7c }, /* '|' */
	   {  6,  3, 11,  1,  2, lapin_0x7d }, /* '}' */
	   {  6,  5,  2,  0,  5, lapin_0x7e }, /* '~' */
};

