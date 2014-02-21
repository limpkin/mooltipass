
/*
 * Mooltipass Oled Demo
 */

#include <spi.h>
#include <oledmp.h>
#include <util/delay.h>

#include "interrupts.h"
#include "CARD/smartcard.h"

#include "had_mooltipass.h"
#include "had_mooltipass_2.h"
#if 0
#include "aqua.h"
#include "sphere.h"
#endif
#include "hackaday.h"
#include "gear.h"
//#include "qrcode.h"

#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

uint32_t count=0;

int main()
{
    CPU_PRESCALE(0);				// Set for 16MHz clock
    _delay_ms(500);				// Let the power settle
    initPortSMC();				// Init smart card Port
    initIRQ();					// Init interrupts	
    usb_init();					// Init USB controller
    //while(!usb_configured());			// Wait for host to set configuration	

    spi_begin(SPI_BAUD_8_MHZ);
    oled_begin(FONT_DEFAULT);	

    oled_setColour(2);
    oled_setBackground(0);
    oled_setContrast(80);
    //oled_setContrast(255);
    
 
#if 0
    oled_bitmapDraw(0,0, &image_qrcode);
    oled_setXY(72,10);
    oled_printf_P(PSTR("MOOLTIPASS"));
    oled_setXY(72,40);
    oled_printf_P(PSTR("Developed on Hackaday"));

#else

    while (1) {
#if 1
	oled_bitmapDraw(0,0, &image_HaD_Mooltipass);
	_delay_ms(5000);
	oled_clear();
#endif


#if 2
	oled_bitmapDraw(0,0, &image_HaD_Mooltipass_2);
	_delay_ms(5000);
	oled_clear();
#endif

#if 1
	oled_bitmapDraw(0,0, &image_hackaday);
	oled_setXY(72,37);
	oled_printf_P(PSTR("HACK A DAY"));
	oled_setXY(108,4);
	oled_printf_P(PSTR("Mooltipass"));
	oled_bitmapDraw(192,0, &image_gear);
#endif

	_delay_ms(5000);
	oled_clear();

#if 0
	oled_bitmapDraw(0,0, &image_hackaday);
	oled_bitmapDraw(60,0, &image_sphere);
	oled_bitmapDraw(116,0, &image_aqua);
	oled_bitmapDraw(192,0, &image_gear);

	_delay_ms(5000);
	oled_clear();
#endif

#if 1
	oled_printf_P(PSTR("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));
	oled_printf_P(PSTR("abcdefghijklmnopqrstuvwxyz\n"));
	oled_printf_P(PSTR("01234567890"));
	oled_printf_P(PSTR("!$%%&'()*,./:;?"));
#endif
	_delay_ms(5000);
	oled_clear();
    }
#endif

    while (1);
}

