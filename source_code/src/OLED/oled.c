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
 * oled.c
 *
 * Created: 08/12/2013 20:46:51
 *  Author: Mathieu Stephan
 */
#include "../mooltipass.h"
#include <util/delay.h>
#include <avr/io.h>


static inline void enable_vcc_oled(void)
{
	PORT_OLED_POW &= ~(1 << PORTID_OLED_POW);
}

static inline void disable_vcc_oled(void)
{
	PORT_OLED_POW |= (1 << PORTID_OLED_POW);
}

void oled_write_data(uint8_t byte)
{
	/* Clear frame transmitted flag */
	UCSR1A |= (1 << TXC1);
	/* Enable slave select */
	PORT_OLED_SS &= ~(1 << PORTID_OLED_SS);	
	/* Put data into buffer, sends the data */
	UDR1 = byte;
	/* Wait for frame transmitted flag */
	while(!(UCSR1A & (1<<TXC1)));
	/* Disable slave select */
	PORT_OLED_SS |= (1 << PORTID_OLED_SS);
}

void oled_write_cont_data(uint8_t byte)
{
	/* Wait for frame transmitted flag */
	while(!(UCSR1A & (1<<TXC1)));
	/* Clear frame transmitted flag */
	UCSR1A |= (1 << TXC1);
	/* Slave select cycle */
	PORT_OLED_SS |= (1 << PORTID_OLED_SS);
	PORT_OLED_SS &= ~(1 << PORTID_OLED_SS);
	/* Put data into buffer, sends the data */
	UDR1 = byte;
}

void oled_relieve_slave_select()
{
	/* Wait for frame transmitted flag */
	while(!(UCSR1A & (1<<TXC1)));
	PORT_OLED_SS |= (1 << PORTID_OLED_SS);
}

void oled_write_command(uint8_t byte)
{
	/* Clear frame transmitted flag */
	UCSR1A |= (1 << TXC1);
	/* Enable slave select */
	PORT_OLED_SS &= ~(1 << PORTID_OLED_SS);
	PORT_OLED_DnC &= ~(1 << PORTID_OLED_DnC);
	/* Put data into buffer, sends the data */
	UDR1 = byte;
	/* Wait for frame transmitted flag */
	while(!(UCSR1A & (1<<TXC1)));
	/* Disable slave select */
	PORT_OLED_DnC |= (1 << PORTID_OLED_DnC);
	PORT_OLED_SS |= (1 << PORTID_OLED_SS);
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Instruction Setting
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Column_Address(uint8_t start_addr, uint8_t end_addr)
{
	oled_write_command(0x15);						// Set Column Address
	oled_write_data(start_addr);					//   Default => 0x00
	oled_write_data(end_addr);						//   Default => 0x77
}

void Set_Row_Address(uint8_t start_addr, uint8_t end_addr)
{
	oled_write_command(0x75);						// Set Row Address
	oled_write_data(start_addr);					//   Default => 0x00
	oled_write_data(end_addr);						//   Default => 0x7F
}

void Set_Write_RAM()
{
	oled_write_command(0x5C);						// Enable MCU to Write into RAM
}

void Set_Read_RAM()
{
	oled_write_command(0x5D);						// Enable MCU to Read from RAM
}

void Set_Remap_Format(uint8_t format)
{
	oled_write_command(0xA0);						// Set Re-Map / Dual COM Line Mode
	oled_write_data(format);						// Default => 0x40, Horizontal Address Increment, Column Address 0 Mapped to SEG0, Disable Nibble Remap
													// Scan from COM0 to COM[N-1], Disable COM Split Odd Even
	oled_write_data(0x11);							// Default => 0x01 (Disable Dual COM Mode)
}

void Set_Start_Line(uint8_t line)
{
	oled_write_command(0xA1);						// Set Vertical Scroll by RAM
	oled_write_data(line);							// Default => 0x00
}

void Set_Display_Offset(uint8_t offset)
{
	oled_write_command(0xA2);						// Set Vertical Scroll by Row
	oled_write_data(offset);						// Default => 0x00
}

void Set_Display_Mode(uint8_t mode)
{
	oled_write_command(0xA4|mode);					// Set Display Mode
													//   Default => 0xA4
													//     0xA4 (0x00) => Entire Display Off, All Pixels Turn Off
													//     0xA5 (0x01) => Entire Display On, All Pixels Turn On at GS Level 15
													//     0xA6 (0x02) => Normal Display
													//     0xA7 (0x03) => Inverse Display
}

void Set_Partial_Display_On(uint8_t start_row, uint8_t end_row)
{
	oled_write_command(0xA8);
	oled_write_data(start_row);
	oled_write_data(end_row);
}

void Set_Partial_Display_Off()
{
	oled_write_command(0xA9);
}

void Set_Function_Selection(uint8_t function)
{
	oled_write_command(0xAB);						// Function Selection
	oled_write_data(function);						//   Default => 0x01, Enable Internal VDD Regulator
}

void Set_Display_On()
{
	oled_write_command(0xAF);
}

void Set_Display_Off()
{
	oled_write_command(0xAE);
}

void Set_Phase_Length(uint8_t length)
{
	oled_write_command(0xB1);						// Phase 1 (Reset) & Phase 2 (Pre-Charge) Period Adjustment
	oled_write_data(length);						//   Default => 0x74 (7 Display Clocks [Phase 2] / 9 Display Clocks [Phase 1])
													//     D[3:0] => Phase 1 Period in 5~31 Display Clocks
													//     D[7:4] => Phase 2 Period in 3~15 Display Clocks
}

void Set_Display_Clock(uint8_t divider)
{
	oled_write_command(0xB3);						// Set Display Clock Divider / Oscillator Frequency
	oled_write_data(divider);						//   Default => 0xD0, A[3:0] => Display Clock Divider, A[7:4] => Oscillator Frequency
}

void Set_Display_Enhancement_A(uint8_t vsl, uint8_t quality)
{
	oled_write_command(0xB4);						// Display Enhancement
	oled_write_data(0xA0|vsl);						//   Default => 0xA2, 0xA0 (0x00) => Enable External VSL, 0xA2 (0x02) => Enable Internal VSL (Kept VSL Pin N.C.)
	oled_write_data(0x05|quality);					//   Default => 0xB5, 0xB5 (0xB0) => Normal, 0xFD (0xF8) => Enhance Low Gray Scale Display Quality
}

void Set_GPIO(uint8_t gpio)
{
	oled_write_command(0xB5);						// General Purpose IO
	oled_write_data(gpio);							//   Default => 0x0A (GPIO Pins output Low Level.)
}

void Set_Precharge_Period(uint8_t period)
{
	oled_write_command(0xB6);						// Set Second Pre-Charge Period
	oled_write_data(period);						//   Default => 0x08 (8 Display Clocks)
}

void Set_Precharge_Voltage(uint8_t voltage)
{
	oled_write_command(0xBB);						// Set Pre-Charge Voltage Level
	oled_write_data(voltage);						//   Default => 0x17 (0.50*VCC)
}

void Set_VCOMH(uint8_t voltage_level)
{
	oled_write_command(0xBE);						// Set COM Deselect Voltage Level
	oled_write_data(voltage_level);					//   Default => 0x04 (0.80*VCC)
}

void Set_Contrast_Current(uint8_t contrast)
{
	oled_write_command(0xC1);						// Set Contrast Current
	oled_write_data(contrast);						//   Default => 0x7F
}

void Set_Master_Current(uint8_t master_current)
{
	oled_write_command(0xC7);						// Master Contrast Current Control
	oled_write_data(master_current);				//   Default => 0x0f (Maximum)
}

void Set_Multiplex_Ratio(uint8_t ratio)
{
	oled_write_command(0xCA);						// Set Multiplex Ratio
	oled_write_data(ratio);							//   Default => 0x7F (1/128 Duty)
}

void Set_Display_Enhancement_B(uint8_t enhancement)
{
	oled_write_command(0xD1);						// Display Enhancement
	oled_write_data(0x82|enhancement);				//   Default => 0xA2, 0x82 (0x00) => Reserved, 0xA2 (0x20) => Normal
	oled_write_data(0x20);
}

void Set_Command_Lock(uint8_t lock)
{
	oled_write_command(0xFD);						// Set Command Lock
	oled_write_data(0x12|lock);						//   Default => 0x12
													//     0x12 => Driver IC interface is unlocked from entering command.
													//     0x16 => All Commands are locked except 0xFD.
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Gray Scale Table Setting (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Gray_Scale_Table()
{
	oled_write_command(0xB8);						// Set Gray Scale Table
	#ifdef NEW_VERSION
		oled_write_data(0x00);						//   Gray Scale Level 1
		oled_write_data(0x28);						//   Gray Scale Level 2
		oled_write_data(0x37);						//   Gray Scale Level 3
		oled_write_data(0x43);						//   Gray Scale Level 4
		oled_write_data(0x4D);						//   Gray Scale Level 5
		oled_write_data(0x56);						//   Gray Scale Level 6
		oled_write_data(0x60);						//   Gray Scale Level 7
		oled_write_data(0x68);						//   Gray Scale Level 8
		oled_write_data(0x72);						//   Gray Scale Level 9
		oled_write_data(0x7C);						//   Gray Scale Level 10
		oled_write_data(0x86);						//   Gray Scale Level 11
		oled_write_data(0x91);						//   Gray Scale Level 12
		oled_write_data(0x9B);						//   Gray Scale Level 13
		oled_write_data(0xA6);						//   Gray Scale Level 14
		oled_write_data(0xB4);						//   Gray Scale Level 15
	#else
		oled_write_data(0x0C);						//   Gray Scale Level 1
		oled_write_data(0x18);						//   Gray Scale Level 2
		oled_write_data(0x24);						//   Gray Scale Level 3
		oled_write_data(0x30);						//   Gray Scale Level 4
		oled_write_data(0x3C);						//   Gray Scale Level 5
		oled_write_data(0x48);						//   Gray Scale Level 6
		oled_write_data(0x54);						//   Gray Scale Level 7
		oled_write_data(0x60);						//   Gray Scale Level 8
		oled_write_data(0x6C);						//   Gray Scale Level 9
		oled_write_data(0x78);						//   Gray Scale Level 10
		oled_write_data(0x84);						//   Gray Scale Level 11
		oled_write_data(0x90);						//   Gray Scale Level 12
		oled_write_data(0x9C);						//   Gray Scale Level 13
		oled_write_data(0xA8);						//   Gray Scale Level 14
		oled_write_data(0xB4);						//   Gray Scale Level 15
	#endif
	oled_write_command(0x00);						// Enable Gray Scale Table
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void lcd_fill_ram(uint8_t data)
{
	uint8_t i,j;

	Set_Column_Address(0x00,0x77);
	Set_Row_Address(0x00,0x7F);
	Set_Write_RAM();

	for(i=0;i<0x80;i++)
	{
		for(j=0;j<0x78;j++)
		{
			oled_write_data(data);
			oled_write_data(data);
		}
	}
}

void init_oled_screen(void)
{	
	/* Setup output signals, all at one by default */
	PORT_OLED_SS |= (1 << PORTID_OLED_SS);
	DDR_OLED_SS |= (1 << PORTID_OLED_SS);
	PORT_OLED_nR &= ~(1 << PORTID_OLED_nR);
	DDR_OLED_nR |= (1 << PORTID_OLED_nR);
	PORT_OLED_DnC |= (1 << PORTID_OLED_DnC);
	DDR_OLED_DnC |= (1 << PORTID_OLED_DnC);
	PORT_OLED_POW |= (1 << PORTID_OLED_POW);
	DDR_OLED_POW |= (1 << PORTID_OLED_POW);
	
	/* Setup SPI interface */
	#if SPI_OLED == SPI_USART
		if(!(UCSR1B & (1 << TXEN1)))
		{
			DDRD |= (1 << 3) | (1 << 5);												// MOSI & SCK as ouputs
			DDRD &= ~(1 << 2);															// MISO as input
			PORTD &= ~(1 << 2);															// Disable pull-up
			UBRR1 = 0x00;																// Set USART baud divider to 0
			UCSR1C = (1 << UMSEL11) | (1 << UMSEL10) | (1 << UCPOL1) | (1 << UCSZ10);	// Enable USART1 as Master SPI mode 3
			UCSR1B = (1 << TXEN1) | (1 << RXEN1);										// Enable receiver and transmitter
			UBRR1 = 0x00;																// Set USART baud divider to 0, final baud rate 8Mbit/s			
		}
	#else
		#error "SPI not implemented"
	#endif
	
	/* Reset OLED screen */
	_delay_ms(100);
	PORT_OLED_nR |= (1 << PORTID_OLED_nR);
	_delay_ms(10);
	
	/* Initialization code */	
	Set_Command_Lock(0x12);							// Unlock Basic Commands (0x12/0x16)
	Set_Display_Off();								// Display Off (0x00/0x01)
	Set_Display_Clock(0x91);						// Set Clock as 80 Frames/Sec
	Set_Multiplex_Ratio(0x3F);						// 1/64 Duty (0x0F~0x3F)
	Set_Display_Offset(0x00);						// Shift Mapping RAM Counter (0x00~0x3F)
	Set_Start_Line(0x00);							// Set Mapping RAM Display Start Line (0x00~0x7F)
	Set_Remap_Format(0x14);							// Set Horizontal Address Increment
													//     Column Address 0 Mapped to SEG0
													//     Disable Nibble Remap
													//     Scan from COM[N-1] to COM0
													//     Disable COM Split Odd Even
													//     Enable Dual COM Line Mode
	Set_GPIO(0x00);									// Disable GPIO Pins Input
	Set_Function_Selection(0x01);					// Enable Internal VDD Regulator
	Set_Display_Enhancement_A(0xA0,0xFD);			// Enable External VSL, Low Gray Scale Enhancement
	Set_Contrast_Current(OLED_Contrast);			// Set Segment Output Current
	Set_Master_Current(OLED_Brightness);			// Set Scale Factor of Segment Output Current Control
	Set_Gray_Scale_Table();							// Set Pulse Width for Gray Scale Table
	Set_Phase_Length(0xE2);							// Set Phase 1 as 5 Clocks & Phase 2 as 14 Clocks
	Set_Display_Enhancement_B(0x20);				// Enhance Driving Scheme Capability (0x00/0x20)
	#define ALTERNATE_OLED_VERSION
	#ifdef ALTERNATE_OLED_VERSION
		Set_Precharge_Voltage(0x08);				// Set Pre-Charge Voltage Level as 0.30*VCC
	#else
		Set_Precharge_Voltage(0x1F);				// Set Pre-Charge Voltage Level as 0.60*VCC
	#endif
	#ifdef ALTERNATE_OLED_VERSION
		Set_Precharge_Period(0x0F);					// Set Second Pre-Charge Period as 15 Clocks
	#else
		Set_Precharge_Period(0x08);					// Set Second Pre-Charge Period as 8 Clocks
	#endif
	Set_VCOMH(0x07);								// Set Common Pins Deselect Voltage Level as 0.86*VCC
	Set_Display_Mode(0x02);							// Normal Display Mode (0x00/0x01/0x02/0x03)
	Set_Partial_Display_Off();						// Disable Partial Display
	lcd_fill_ram(0x00);								// Clear Screen
	enable_vcc_oled();								// Enable 12V
	_delay_ms(2000);								// Wait for Vcc to be stable
	Set_Display_On();								// Display On (0x00/0x01)
}