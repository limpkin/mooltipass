 /* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * smartcard.c
 *
 * Created: 08/12/2013 16:50:05
 *  Author: Mathieu Stephan
 */
/*!	\file 	smartcard.c
*	\brief	Smart Card low level functions
*/
#include <avr/interrupt.h>
#include "../mooltipass.h"
#include <util/delay.h>
#include <avr/io.h>
#include <stdlib.h>

/** Counter for successive card detects **/
volatile uint8_t card_detect_counter;
/** Current detection state */
volatile uint8_t button_return;


/*!	\fn		clock_pulse(void)
*	\brief	Send a 4us H->L clock pulse (datasheet: min 3.3us)
*/
void clock_pulse(void)
{
	#if SPI_SMARTCARD == SPI_NATIVE
		PORTB |= (1 << 1);
		_delay_us(2);
		PORTB &= ~(1 << 1);
		_delay_us(2);
	#else
		#error "SPI not supported"
	#endif	
}

/*!	\fn		inverted_clock_pulse(void)
*	\brief	Send a 4us L->H clock pulse (datasheet: min 3.3us)
*/
void inverted_clock_pulse(void)
{
	#if SPI_SMARTCARD == SPI_NATIVE
		PORTB &= ~(1 << 1);
		_delay_us(2);
		PORTB |= (1 << 1);
		_delay_us(2);
	#else
		#error "SPI not supported"
	#endif	
}

/*!	\fn		clear_pgm_rst_signals(void)
*	\brief	Clear PGM / RST signal for normal operation mode
*/
void clear_pgm_rst_signals(void)
{
	PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
	PORT_SC_RST &= ~(1 << PORTID_SC_RST);
	_delay_us(3);
}

/*!	\fn		set_pgm_rst_signals(void)
*	\brief	Set PGM / RST signal for standby mode
*/
void set_pgm_rst_signals(void)
{
	PORT_SC_RST |= (1 << PORTID_SC_RST);
	PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
	_delay_us(3);
}

/*!	\fn		perform_low_level_write_nerase(uint8_t is_write)
*	\brief	Perform a write or erase operation
*	\param	is_write	Boolean to indicate if it is a write
*/
void perform_low_level_write_nerase(uint8_t is_write)
{
	#if SPI_SMARTCARD == SPI_NATIVE
		/* Set programming control signal */
		PORT_SC_PGM |= (1 << PORTID_SC_PGM);
		_delay_us(2);

		/* Set data according to write / erase */
		if(is_write != FALSE)
			PORTB |= (1 << 2);
		else
			PORTB &= ~(1 << 2);
		_delay_us(2);

		/* Set clock */
		PORTB |= (1 << 1);
		_delay_us(2);

		/* Release program signal and data, wait for tchp */
		PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
		_delay_ms(4);

		/* Release clock */
		PORTB &= ~(1 << 1);
		_delay_us(2);

		/* Release data */
		PORTB &= ~(1 << 2);
		_delay_us(2);
	#else
		#error "SPI not supported"
	#endif	
}

/*!	\fn		setSPIModeSMC(void)
*	\brief	Activate SPI controller
*/
void setSPIModeSMC(void)
{
	#if SPI_SMARTCARD == SPI_NATIVE	
		/* Enable SPI in master mode at 125kbits/s */
		SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR1) | (1 << SPR0);
	#else
		#error "SPI not supported"
	#endif	
}

/*!	\fn		setBBModeAndPgmRstSMC(void)
*	\brief	Switch to big banging, and clear pgm/rst signal for normal operation
*/
void setBBModeAndPgmRstSMC(void)
{
	#if SPI_SMARTCARD == SPI_NATIVE
		/* Deactivate SPI port */
		SPCR = 0;
	
		/* Clock & data low */
		PORTB &= ~(1 << 1);
		PORTB &= ~(1 << 2);
		_delay_us(1);
	
		/* Clear PGM and RST signals */
		clear_pgm_rst_signals();
	#else
		#error "SPI not supported"
	#endif	
}

/*!	\fn		blow_man_nissuer_fuse(uint8_t bool_man_nissuer)
*	\brief	Blow the manufacturer or issuer fuse
*	\param	bool_man_nissuer	Boolean to indicate if we blow the manufacturer fuse
*/
void blow_man_nissuer_fuse(uint8_t bool_man_nissuer)
{
	uint16_t i;
	
	/* Set the index to write */
	if(bool_man_nissuer != FALSE)
		i = 1460;
	else
		i = 1560;
		
	/* Switch to bit banging */
	setBBModeAndPgmRstSMC();
	
	/* Get to the good index */
	while(i--)clock_pulse();
	
	/* Set RST signal */
	PORT_SC_RST |= (1 << PORTID_SC_RST);
	
	/* Perform a write */
	perform_low_level_write_nerase(TRUE);
	
	/* Set PGM / RST signals to standby mode */
	set_pgm_rst_signals();
	
	/* Switch to SPI mode */
	setSPIModeSMC();
}

/*!	\fn		isCardPlugged(void)
*	\brief	Know if a card is plugged
*	\return	just released/pressed, (non)detected
*/
RET_TYPE isCardPlugged(void)
{
	volatile RET_TYPE return_val = button_return;
	
	if((return_val != RETURN_DET) && (return_val != RETURN_REL))
	{
		cli();
		if(button_return == RETURN_JDETECT)
			button_return = RETURN_DET;
		else if(button_return == RETURN_JRELEASED)
			button_return = RETURN_REL;
		sei();
	}
	
	return return_val;
}

/*!	\fn		scanSMCDectect(void)
*	\brief	card detect debounce called by 1ms interrupt
*/
void scanSMCDectect(void)
{
	if(PIN_SC_DET & (1 << PORTID_SC_DET))
	{
		if(card_detect_counter != 0xFF)
			card_detect_counter++;
		if(card_detect_counter == 150)
			button_return = RETURN_JDETECT;		
	}
	else
	{
		if(button_return == RETURN_DET)
			button_return = RETURN_JRELEASED;
		else if(button_return != RETURN_JRELEASED)
			button_return = RETURN_REL;
		card_detect_counter = 0;
	}	
}

/*!	\fn		eraseApplicationZone1NZone2SMC(uint8_t zone1_nzone2)
*	\brief	Set E1 or E2 flag by presenting the correct erase key (always FFFF...) and erase the AZ1 or AZ2
*	\param	zone1_nzone2	Zone 1 or Not Zone 2
*/
void eraseApplicationZone1NZone2SMC(uint8_t zone1_nzone2)
{
	uint16_t i;
	
	/* Which index to go to */
	if(zone1_nzone2 == FALSE)
		i = 1248;
	else
		i = 688;
	
	#if SPI_SMARTCARD == SPI_NATIVE
		/* Switch to bit banging */
		setBBModeAndPgmRstSMC();
	
		/* Get to the good EZx */
		while(i--) inverted_clock_pulse();
		
		/* How many bits to compare */
		if(zone1_nzone2 == FALSE)
			i = 32;
		else
			i = 48;	
		
		/* Clock is at high level now, as input must be switched during this time */
		/* Enter the erase key */
		_delay_us(2);
		while(i--)
		{
			// The code is always FFFF...
			_delay_us(2);
			
			/* Inverted clock pulse */
			inverted_clock_pulse();
		}
		
		/* Bring clock and data low */
		PORTB &= ~(1 << 1);
		_delay_us(3);
		PORTB &= ~(1 << 2);
		_delay_us(3);
		
		/* Erase AZ1/AZ2 */
		perform_low_level_write_nerase(FALSE);
		
		/* Set PGM / RST signals to standby mode */
		set_pgm_rst_signals();
		
		/* Switch to SPI mode */
		setSPIModeSMC();
	#else
		#error "SPI not supported"
	#endif		
}

/*!	\fn		securityValidationSMC(uint16_t code)
*	\brief	Check security code
*	\param	code	The code
*	\return	success_status (see enum)
*/
RET_TYPE securityValidationSMC(uint16_t code)
{
	RET_TYPE return_val = RETURN_PIN_NOK_0;
	uint8_t temp_bool;
	uint16_t i;
	
	#if SPI_SMARTCARD == SPI_NATIVE
		/* Switch to bit banging */
		setBBModeAndPgmRstSMC();
	
		/* Get to the SC */
		for(i = 0; i < 80; i++)
			inverted_clock_pulse();
		
		/* Clock is at high level now, as input must be switched during this time */		
		/* Enter the SC */
		_delay_us(2);
		for(i = 0; i < 16; i++)
		{
			if(((code >> (15-i)) & 0x0001) != 0x0000)
				PORTB &= ~(1 << 2);
			else
				PORTB |= (1 << 2);
			_delay_us(2);
			
			/* Inverted clock pulse */
			inverted_clock_pulse();
		}
		
		/* Bring clock and data low */
		PORTB &= ~(1 << 1);
		_delay_us(3);
		PORTB &= ~(1 << 2);
		_delay_us(3);
		
		i = 0;
		temp_bool = TRUE;
		/* Write one of the four SCAC bits to 0 and check if successful */
		while((temp_bool == TRUE) && (i < 4))
		{
			/* If one of the four bits is at 1, write a 0 */
			if(PINB & (1 << 3))
			{
				/* Set write command */
				perform_low_level_write_nerase(TRUE);
				
				/* Wait for the smart card to output a 0 */
				while(PINB & (1 << 3));
				
				/* Now, erase SCAC */
				perform_low_level_write_nerase(FALSE);
				
				/* Were we successful? */
				if(PINB & (1 << 3))
				{
					// Success !
					return_val = RETURN_PIN_OK;
				}
				else
				{
					// Wrong pin, return number of tries left
					if(i == 0)
						return_val = RETURN_PIN_NOK_3;
					else if(i == 1)
						return_val = RETURN_PIN_NOK_2;
					else if(i == 2)
						return_val = RETURN_PIN_NOK_1;
					else if(i == 3)
						return_val = RETURN_PIN_NOK_0;
				}
				
				/* Clock pulse */
				clock_pulse();
				
				/* Exit loop */
				temp_bool = FALSE;
			}
			else
			{				
				/* Clock pulse */
				clock_pulse();
				i++;
			}
		}
		
		/* If we couldn't find a spot to write, no tries left */
		if(i == 4)
			return_val = RETURN_PIN_NOK_0;
		
		/* Set PGM / RST signals to standby mode */
		set_pgm_rst_signals();
		
		/* Switch to SPI mode */
		setSPIModeSMC();
	#else
		#error "SPI not supported"
	#endif
	
	return return_val;
}

/*!	\fn		writeSMC(uint16_t start_index_bit, uint16_t nb_bits, uint8_t* data_to_write)
*	\brief	Write bits to the smart card
*	\param	start_index_bit			Where to start writing bits
*	\param	nb_bits					Number of bits to write
*	\param	data_to_write			Pointer to the buffer
*/
void writeSMC(uint16_t start_index_bit, uint16_t nb_bits, uint8_t* data_to_write)
{
	uint16_t current_written_bit = 0;
	uint8_t masked_bit_to_write = 0;
	uint16_t i;	
	
	#if SPI_SMARTCARD == SPI_NATIVE	
		/* Switch to bit banging */
		setBBModeAndPgmRstSMC();
		
		/* Get to the good index, clock pulses */
		for(i = 0; i < start_index_bit; i++)
			clock_pulse();
		
		/* Start writing */
		for(current_written_bit = 0; current_written_bit < nb_bits; current_written_bit++)
		{
			/* If we are at the start of a 16bits word or writing our first bit, erase the word */
			if((((start_index_bit+current_written_bit) & 0x000F) == 0) || (current_written_bit == 0))
				perform_low_level_write_nerase(FALSE);
			
			/* Get good bit to write */
			masked_bit_to_write = (data_to_write[(current_written_bit>>3)] >> (7 - (current_written_bit & 0x0007))) & 0x01;
			
			/* Write only if the data is a 0 */
			if(masked_bit_to_write == 0x00)
				perform_low_level_write_nerase(TRUE);
			
			/* Go to next address */
			clock_pulse();	
		}
		
		/* Set PGM / RST signals to standby mode */
		set_pgm_rst_signals();
		
		/* Switch to SPI mode */
		setSPIModeSMC();
	#else
		#error "SPI not supported"
	#endif	
}

/*!	\fn		readSMC(uint8_t nb_bytes_total_read, uint8_t start_record_index, uint8_t* data_to_send_receive)
*	\brief	Read bytes from the smart card
*	\param	nb_bytes_total_read		The number of bytes to be read
*	\param	start_record_index		The index at which we start recording the answer
*	\param	data_to_receive	Pointer to the buffer
*	\return	The buffer
*/
uint8_t* readSMC(uint8_t nb_bytes_total_read, uint8_t start_record_index, uint8_t* data_to_receive)
{
	uint8_t* return_val = data_to_receive;
	uint8_t i;
	
	/* Set PGM / RST signals for operation */
	clear_pgm_rst_signals();
	
	for(i = 0; i < nb_bytes_total_read; i++)
	{
		/* Start transmission */
		SPDR = 0x00;
		/* Wait for transmission complete */
		while(!(SPSR & (1<<SPIF)));
		/* Store data in buffer or discard it*/
		if(i >= start_record_index)
			*(data_to_receive++) = SPDR;		
		else
			SPDR;
	}
	
	/* Set PGM / RST signals to standby mode */
	set_pgm_rst_signals();
	
	return return_val;
}

/*!	\fn		detectFunctionSMC(void)
*	\brief	functions performed once the smart card is detected
*	\return	The detection result (see enum)
*/
RET_TYPE detectFunctionSMC(void)
{
	uint8_t data_buffer[2];
	uint16_t temp_uint;
	
	/* Enable power to the card */
	PORT_SC_POW &= ~(1 << PORTID_SC_POW);
	
	/* Default state: PGM to 0 and RST to 1 */
	PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
	DDR_SC_PGM |= (1 << PORTID_SC_PGM);
	PORT_SC_RST |= (1 << PORTID_SC_RST);
	DDR_SC_RST |= (1 << PORTID_SC_RST);
	
	/* Activate SPI port */
	#if SPI_SMARTCARD == SPI_NATIVE
		PORTB &= ~((1 << 1) | (1 << 2));
		DDRB |= (1 << 1) | (1 << 2);
		setSPIModeSMC();
	#else
		#error "SPI not supported"
	#endif
	
	/* Let the card come online */
	_delay_ms(10);
	
	/* Check smart card FZ */
	readFabricationZone(data_buffer);
	if((swap16(*(uint16_t*)data_buffer)) != SMARTCARD_FABRICATION_ZONE)
		return RETURN_CARD_NDET;
	
	/* Perform test write on MTZ... should we use our own rand()? */
	temp_uint = (uint16_t)rand();
	write_memory_test_zone((uint8_t*)&temp_uint);
	if(*(uint16_t*)readMemoryTestZone(data_buffer) != temp_uint)	
		return RETURN_CARD_TEST_PB;
		
	/* Read security code attempts counter */
	switch(get_number_of_security_code_tries_left())
	{
		case 4: return RETURN_CARD_4_TRIES_LEFT; break;
		case 3: return RETURN_CARD_3_TRIES_LEFT; break;
		case 2: return RETURN_CARD_2_TRIES_LEFT; break;
		case 1: return RETURN_CARD_1_TRIES_LEFT; break;
		case 0: return RETURN_CARD_0_TRIES_LEFT; break;
	}
	
	return RETURN_CARD_0_TRIES_LEFT;
}

/*!	\fn		removeFunctionSMC(void)
*	\brief	functions performed once the smart card is removed
*/
void removeFunctionSMC(void)
{
	/* Deactivate power to the smart card */
	PORT_SC_POW |= (1 << PORTID_SC_POW);
	
	/* Setup all output pins as tri-state */
	PORT_SC_PGM &= ~(1 << PORTID_SC_PGM);
	DDR_SC_PGM &= ~(1 << PORTID_SC_PGM);
	PORT_SC_RST &= ~(1 << PORTID_SC_RST);
	DDR_SC_RST &= ~(1 << PORTID_SC_RST);
	
	/* Deactivate SPI port */
	#if SPI_SMARTCARD == SPI_NATIVE
		SPCR = 0;
		DDRB &= ~(1 << 1) | (1 << 2);
		PORTB &= ~((1 << 1) | (1 << 2));
	#else
		#error "SPI not supported"
	#endif
}

/*!	\fn		initPortSMC(void)
*	\brief	Initialize smart card port
*/
void initPortSMC(void)
{
	/* Setup card detection input with pull-up */
	DDR_SC_DET &= ~(1 << PORTID_SC_DET);
	PORT_SC_DET |= (1 << PORTID_SC_DET);
	
	/* Setup power enable, disabled by default */
	PORT_SC_POW |= (1 << PORTID_SC_POW);
	DDR_SC_POW |= (1 << PORTID_SC_POW);
	
	/* Setup MISO as input, SS as input with pull-up */
	#if SPI_SMARTCARD == SPI_NATIVE
		DDRB &= ~((1 << 3) | (1 << 0));
		PORTB &= ~(1 << 3);
		PORTB |= (1 << 0);
	#else
		#error "SPI not supported"
	#endif	
	
	/* Set all output pins as tri-state */
	removeFunctionSMC();
}