/**************************************************************************/
/*! 
    @file     Adafruit_NFCShield_I2C.cpp
    @author   Adafruit Industries
	@license  BSD (see license.txt)
	
	I2C Driver for NXP's PN532 NFC/13.56MHz RFID Transceiver

	This is a library for the Adafruit PN532 NFC/RFID shields
	This library works with the Adafruit NFC breakout 
	----> https://www.adafruit.com/products/364
	
	Check out the links above for our tutorials and wiring diagrams 
	These chips use I2C to communicate
	
	Adafruit invests time and resources providing this open source code, 
	please support Adafruit and open-source hardware by purchasing 
	products from Adafruit!

	@section  HISTORY

    v1.4 - Added setPassiveActivationRetries()
	
    v1.3 - Modified to work with I2C
	
    v1.2 - Added writeGPIO()
         - Added readGPIO()

    v1.1 - Changed readPassiveTargetID() to handle multiple UID sizes
         - Added the following helper functions for text display
             static void PrintHex(const byte * data, const uint32_t numBytes)
             static void PrintHexChar(const byte * pbtData, const uint32_t numBytes)
         - Added the following Mifare Classic functions:
             bool mifareclassic_IsFirstBlock (uint32_t uiBlock)
             bool mifareclassic_IsTrailerBlock (uint32_t uiBlock)
             uint8_t mifareclassic_AuthenticateBlock (uint8_t * uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t * keyData)
             uint8_t mifareclassic_ReadDataBlock (uint8_t blockNumber, uint8_t * data)
             uint8_t mifareclassic_WriteDataBlock (uint8_t blockNumber, uint8_t * data)
         - Added the following Mifare Ultalight functions:
             uint8_t mifareultralight_ReadPage (uint8_t page, uint8_t * buffer)	
*/
/**************************************************************************/
#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

#include <Wire.h>
#if defined(__AVR__) || defined(__i386__) //compatibility with Intel Galileo
 #define WIRE Wire
#else // Arduino Due
 #define WIRE Wire1
#endif

#include "Adafruit_NFCShield_I2C.h"

byte pn532ack[] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
byte pn532response_firmwarevers[] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};

// Uncomment these lines to enable debug output for PN532(I2C) and/or MIFARE related code
// #define PN532DEBUG
// #define MIFAREDEBUG

#define PN532_PACKBUFFSIZ 64
byte pn532_packetbuffer[PN532_PACKBUFFSIZ];

/**************************************************************************/
/*! 
    @brief  Sends a single byte via I2C

    @param  x    The byte to send
*/
/**************************************************************************/
static inline void wiresend(uint8_t x) 
{
  #if ARDUINO >= 100
    WIRE.write((uint8_t)x);
  #else
    WIRE.send(x);
  #endif
}

/**************************************************************************/
/*! 
    @brief  Reads a single byte via I2C
*/
/**************************************************************************/
static inline uint8_t wirerecv(void) 
{
  #if ARDUINO >= 100
    return WIRE.read();
  #else
    return WIRE.receive();
  #endif
}

/**************************************************************************/
/*! 
    @brief  Instantiates a new PN532 class

    @param  irq       Location of the IRQ pin
    @param  reset     Location of the RSTPD_N pin
*/
/**************************************************************************/
Adafruit_NFCShield_I2C::Adafruit_NFCShield_I2C(uint8_t irq, uint8_t reset) {
  _irq = irq;
  _reset = reset;

  pinMode(_irq, INPUT);
  pinMode(_reset, OUTPUT);
}

/**************************************************************************/
/*! 
    @brief  Setups the HW
*/
/**************************************************************************/
void Adafruit_NFCShield_I2C::begin() {
  WIRE.begin();

  // Reset the PN532  
  digitalWrite(_reset, HIGH);
  digitalWrite(_reset, LOW);
  delay(400);
  digitalWrite(_reset, HIGH);
}
 
/**************************************************************************/
/*! 
    @brief  Prints a hexadecimal value in plain characters

    @param  data      Pointer to the byte data
    @param  numBytes  Data length in bytes
*/
/**************************************************************************/
void Adafruit_NFCShield_I2C::PrintHex(const byte * data, const uint32_t numBytes)
{
  uint32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++) 
  {
    Serial.print(F("0x"));
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print(F("0"));
    Serial.print(data[szPos]&0xff, HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      Serial.print(F(" "));
    }
  }
  Serial.println();
}

/**************************************************************************/
/*! 
    @brief  Prints a hexadecimal value in plain characters, along with
            the char equivalents in the following format

            00 00 00 00 00 00  ......

    @param  data      Pointer to the byte data
    @param  numBytes  Data length in bytes
*/
/**************************************************************************/
void Adafruit_NFCShield_I2C::PrintHexChar(const byte * data, const uint32_t numBytes)
{
  uint32_t szPos;
  for (szPos=0; szPos < numBytes; szPos++) 
  {
    // Append leading 0 for small values
    if (data[szPos] <= 0xF)
      Serial.print(F("0"));
    Serial.print(data[szPos], HEX);
    if ((numBytes > 1) && (szPos != numBytes - 1))
    {
      Serial.print(F(" "));
    }
  }
  Serial.print(F("  "));
  for (szPos=0; szPos < numBytes; szPos++) 
  {
    if (data[szPos] <= 0x1F)
      Serial.print(F("."));
    else
      Serial.print((char)data[szPos]);
  }
  Serial.println();
}
 
/**************************************************************************/
/*! 
    @brief  Checks the firmware version of the PN5xx chip

    @returns  The chip's firmware version and ID
*/
/**************************************************************************/
uint32_t Adafruit_NFCShield_I2C::getFirmwareVersion(void) {
  uint32_t response;

  pn532_packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;

  if (! sendCommandCheckAck(pn532_packetbuffer, 1))
    return 0;
	
  // read data packet
  wirereaddata(pn532_packetbuffer, 12);
  
  // check some basic stuff
  if (0 != strncmp((char *)pn532_packetbuffer, (char *)pn532response_firmwarevers, 6)) {
    #ifdef PN532DEBUG
    Serial.println(F("Firmware doesn't match!"));
	#endif
    return 0;
  }
  
  response = pn532_packetbuffer[7];
  response <<= 8;
  response |= pn532_packetbuffer[8];
  response <<= 8;
  response |= pn532_packetbuffer[9];
  response <<= 8;
  response |= pn532_packetbuffer[10];

  return response;
}


/**************************************************************************/
/*! 
    @brief  Sends a command and waits a specified period for the ACK

    @param  cmd       Pointer to the command buffer
    @param  cmdlen    The size of the command in bytes 
    @param  timeout   timeout before giving up
    
    @returns  1 if everything is OK, 0 if timeout occured before an
              ACK was recieved
*/
/**************************************************************************/
// default timeout of one second
boolean Adafruit_NFCShield_I2C::sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout) {
  uint16_t timer = 0;
  
  // write the command
  wiresendcommand(cmd, cmdlen);
  
  // Wait for chip to say its ready!
  while (wirereadstatus() != PN532_I2C_READY) {
    if (timeout != 0) {
      timer+=10;
      if (timer > timeout)  
        return false;
    }
    delay(10);
  }
  
  #ifdef PN532DEBUG
  Serial.println(F("IRQ received"));
  #endif
  
  // read acknowledgement
  if (!readackframe()) {
    #ifdef PN532DEBUG
    Serial.println(F("No ACK frame received!"));
    #endif
    return false;
  }

  return true; // ack'd command
}

/**************************************************************************/
/*! 
    Writes an 8-bit value that sets the state of the PN532's GPIO pins
    
    @warning This function is provided exclusively for board testing and
             is dangerous since it will throw an error if any pin other
             than the ones marked "Can be used as GPIO" are modified!  All
             pins that can not be used as GPIO should ALWAYS be left high
             (value = 1) or the system will become unstable and a HW reset
             will be required to recover the PN532.
    
             pinState[0]  = P30     Can be used as GPIO
             pinState[1]  = P31     Can be used as GPIO
             pinState[2]  = P32     *** RESERVED (Must be 1!) ***
             pinState[3]  = P33     Can be used as GPIO
             pinState[4]  = P34     *** RESERVED (Must be 1!) ***
             pinState[5]  = P35     Can be used as GPIO
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
boolean Adafruit_NFCShield_I2C::writeGPIO(uint8_t pinstate) {
  uint8_t errorbit;

  // Make sure pinstate does not try to toggle P32 or P34
  pinstate |= (1 << PN532_GPIO_P32) | (1 << PN532_GPIO_P34);
  
  // Fill command buffer
  pn532_packetbuffer[0] = PN532_COMMAND_WRITEGPIO;
  pn532_packetbuffer[1] = PN532_GPIO_VALIDATIONBIT | pinstate;  // P3 Pins
  pn532_packetbuffer[2] = 0x00;    // P7 GPIO Pins (not used ... taken by I2C)

  #ifdef PN532DEBUG
    Serial.print(F("Writing P3 GPIO: ")); Serial.println(pn532_packetbuffer[1], HEX);
  #endif

  // Send the WRITEGPIO command (0x0E)  
  if (! sendCommandCheckAck(pn532_packetbuffer, 3))
    return 0x0;
  
  // Read response packet (00 00 FF PLEN PLENCHECKSUM D5 CMD+1(0x0F) DATACHECKSUM)
  wirereaddata(pn532_packetbuffer, 8);

  #ifdef PN532DEBUG
    Serial.print(F("Received: "));
    PrintHex(pn532_packetbuffer, 8);
    Serial.println();
  #endif  
  
  return  (pn532_packetbuffer[6] == 0x0F);
}

/**************************************************************************/
/*! 
    Reads the state of the PN532's GPIO pins
    
    @returns An 8-bit value containing the pin state where:
    
             pinState[0]  = P30     
             pinState[1]  = P31     
             pinState[2]  = P32     
             pinState[3]  = P33     
             pinState[4]  = P34     
             pinState[5]  = P35     
*/
/**************************************************************************/
uint8_t Adafruit_NFCShield_I2C::readGPIO(void) {
  pn532_packetbuffer[0] = PN532_COMMAND_READGPIO;

  // Send the READGPIO command (0x0C)  
  if (! sendCommandCheckAck(pn532_packetbuffer, 1))
    return 0x0;
  
  // Read response packet (00 00 FF PLEN PLENCHECKSUM D5 CMD+1(0x0D) P3 P7 IO1 DATACHECKSUM)
  wirereaddata(pn532_packetbuffer, 11);

  /* READGPIO response should be in the following format:
  
    byte            Description
    -------------   ------------------------------------------
    b0..6           Frame header and preamble
    b7              P3 GPIO Pins
    b8              P7 GPIO Pins (not used ... taken by I2C)
    b9              Interface Mode Pins (not used ... bus select pins) 
    b10             checksum */
  
  #ifdef PN532DEBUG
    Serial.print(F("Received: "));
    PrintHex(pn532_packetbuffer, 11);
    Serial.println();
    Serial.print(F("P3 GPIO: 0x")); Serial.println(pn532_packetbuffer[7], HEX);
    Serial.print(F("P7 GPIO: 0x")); Serial.println(pn532_packetbuffer[8], HEX);
    Serial.print(F("IO GPIO: 0x")); Serial.println(pn532_packetbuffer[9], HEX);
    // Note: You can use the IO GPIO value to detect the serial bus being used
    switch(pn532_packetbuffer[9])
    {
      case 0x00:    // Using UART
        Serial.println(F("Using UART (IO = 0x00)"));
        break;
      case 0x01:    // Using I2C 
        Serial.println(F("Using I2C (IO = 0x01)"));
        break;
      case 0x02:    // Using I2C
        Serial.println(F("Using I2C (IO = 0x02)"));
        break;
    }
  #endif

  return pn532_packetbuffer[6];
}

/**************************************************************************/
/*! 
    @brief  Configures the SAM (Secure Access Module)
*/
/**************************************************************************/
boolean Adafruit_NFCShield_I2C::SAMConfig(void) {
  pn532_packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
  pn532_packetbuffer[1] = 0x01; // normal mode;
  pn532_packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
  pn532_packetbuffer[3] = 0x01; // use IRQ pin!
  
  if (! sendCommandCheckAck(pn532_packetbuffer, 4))
     return false;

  // read data packet
  wirereaddata(pn532_packetbuffer, 8);
  
  return  (pn532_packetbuffer[6] == 0x15);
}

/**************************************************************************/
/*! 
    Sets the MxRtyPassiveActivation byte of the RFConfiguration register
    
    @param  maxRetries    0xFF to wait forever, 0x00..0xFE to timeout
                          after mxRetries
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
boolean Adafruit_NFCShield_I2C::setPassiveActivationRetries(uint8_t maxRetries) {
  pn532_packetbuffer[0] = PN532_COMMAND_RFCONFIGURATION;
  pn532_packetbuffer[1] = 5;    // Config item 5 (MaxRetries)
  pn532_packetbuffer[2] = 0xFF; // MxRtyATR (default = 0xFF)
  pn532_packetbuffer[3] = 0x01; // MxRtyPSL (default = 0x01)
  pn532_packetbuffer[4] = maxRetries;

#ifdef MIFAREDEBUG
  Serial.print(F("Setting MxRtyPassiveActivation to ")); Serial.print(maxRetries, DEC); Serial.println(F(" "));
#endif
  
  if (! sendCommandCheckAck(pn532_packetbuffer, 5))
    return 0x0;  // no ACK
  
  return 1;
}

/***** ISO14443A Commands ******/

/**************************************************************************/
/*! 
    Waits for an ISO14443A target to enter the field
    
    @param  cardBaudRate  Baud rate of the card
    @param  uid           Pointer to the array that will be populated
                          with the card's UID (up to 7 bytes)
    @param  uidLength     Pointer to the variable that will hold the
                          length of the card's UID.
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
boolean Adafruit_NFCShield_I2C::readPassiveTargetID(uint8_t cardbaudrate, uint8_t * uid, uint8_t * uidLength) {
  pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
  pn532_packetbuffer[1] = 1;  // max 1 cards at once (we can set this to 2 later)
  pn532_packetbuffer[2] = cardbaudrate;
  
  if (! sendCommandCheckAck(pn532_packetbuffer, 3))
  {
    #ifdef PN532DEBUG
	Serial.println(F("No card(s) read"));
	#endif
    return 0x0;  // no cards read
  }
  
  // Wait for a card to enter the field
  uint8_t status = PN532_I2C_BUSY;
  #ifdef PN532DEBUG
  Serial.println(F("Waiting for IRQ (indicates card presence)"));
  #endif
  while (wirereadstatus() != PN532_I2C_READY)
  {
	delay(10);
  }

  #ifdef PN532DEBUG
  Serial.println(F("Found a card"));
  #endif
 
  // read data packet
  wirereaddata(pn532_packetbuffer, 20);
  
  // check some basic stuff
  /* ISO14443A card response should be in the following format:
  
    byte            Description
    -------------   ------------------------------------------
    b0..6           Frame header and preamble
    b7              Tags Found
    b8              Tag Number (only one used in this example)
    b9..10          SENS_RES
    b11             SEL_RES
    b12             NFCID Length
    b13..NFCIDLen   NFCID                                      */
  
#ifdef MIFAREDEBUG
    Serial.print(F("Found ")); Serial.print(pn532_packetbuffer[7], DEC); Serial.println(F(" tags"));
#endif
  if (pn532_packetbuffer[7] != 1) 
    return 0;
    
  uint16_t sens_res = pn532_packetbuffer[9];
  sens_res <<= 8;
  sens_res |= pn532_packetbuffer[10];
#ifdef MIFAREDEBUG
    Serial.print(F("ATQA: 0x"));  Serial.println(sens_res, HEX);
    Serial.print(F("SAK: 0x"));  Serial.println(pn532_packetbuffer[11], HEX);
#endif
  
  /* Card appears to be Mifare Classic */
  *uidLength = pn532_packetbuffer[12];
#ifdef MIFAREDEBUG
    Serial.print(F("UID:"));
#endif
  for (uint8_t i=0; i < pn532_packetbuffer[12]; i++) 
  {
    uid[i] = pn532_packetbuffer[13+i];
#ifdef MIFAREDEBUG
      Serial.print(F(" 0x"));Serial.print(uid[i], HEX);
#endif
  }
#ifdef MIFAREDEBUG
    Serial.println();
#endif

  return 1;
}


/***** Mifare Classic Functions ******/

/**************************************************************************/
/*! 
      Indicates whether the specified block number is the first block
      in the sector (block 0 relative to the current sector)
*/
/**************************************************************************/
bool Adafruit_NFCShield_I2C::mifareclassic_IsFirstBlock (uint32_t uiBlock)
{
  // Test if we are in the small or big sectors
  if (uiBlock < 128)
    return ((uiBlock) % 4 == 0);
  else
    return ((uiBlock) % 16 == 0);
}

/**************************************************************************/
/*! 
      Indicates whether the specified block number is the sector trailer
*/
/**************************************************************************/
bool Adafruit_NFCShield_I2C::mifareclassic_IsTrailerBlock (uint32_t uiBlock)
{
  // Test if we are in the small or big sectors
  if (uiBlock < 128)
    return ((uiBlock + 1) % 4 == 0);
  else
    return ((uiBlock + 1) % 16 == 0);
}

/**************************************************************************/
/*! 
    Tries to authenticate a block of memory on a MIFARE card using the
    INDATAEXCHANGE command.  See section 7.3.8 of the PN532 User Manual
    for more information on sending MIFARE and other commands.

    @param  uid           Pointer to a byte array containing the card UID
    @param  uidLen        The length (in bytes) of the card's UID (Should
                          be 4 for MIFARE Classic)
    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  keyNumber     Which key type to use during authentication
                          (0 = MIFARE_CMD_AUTH_A, 1 = MIFARE_CMD_AUTH_B)
    @param  keyData       Pointer to a byte array containing the 6 byte
                          key value
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t Adafruit_NFCShield_I2C::mifareclassic_AuthenticateBlock (uint8_t * uid, uint8_t uidLen, uint32_t blockNumber, uint8_t keyNumber, uint8_t * keyData)
{
  uint8_t len;
  uint8_t i;
  
  // Hang on to the key and uid data
  memcpy (_key, keyData, 6); 
  memcpy (_uid, uid, uidLen); 
  _uidLen = uidLen;  

  #ifdef MIFAREDEBUG
  Serial.print(F("Trying to authenticate card "));
  Adafruit_NFCShield_I2C::PrintHex(_uid, _uidLen);
  Serial.print(F("Using authentication KEY "));Serial.print(keyNumber ? 'B' : 'A');Serial.print(F(": "));
  Adafruit_NFCShield_I2C::PrintHex(_key, 6);
  #endif
  
  // Prepare the authentication command //
  pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;   /* Data Exchange Header */
  pn532_packetbuffer[1] = 1;                              /* Max card numbers */
  pn532_packetbuffer[2] = (keyNumber) ? MIFARE_CMD_AUTH_B : MIFARE_CMD_AUTH_A;
  pn532_packetbuffer[3] = blockNumber;                    /* Block Number (1K = 0..63, 4K = 0..255 */
  memcpy (pn532_packetbuffer+4, _key, 6);
  for (i = 0; i < _uidLen; i++)
  {
    pn532_packetbuffer[10+i] = _uid[i];                /* 4 byte card ID */
  }

  if (! sendCommandCheckAck(pn532_packetbuffer, 10+_uidLen))
    return 0;

  // Read the response packet
  wirereaddata(pn532_packetbuffer, 12);
  
  // Check if the response is valid and we are authenticated???
  // for an auth success it should be bytes 5-7: 0xD5 0x41 0x00
  // Mifare auth error is technically byte 7: 0x14 but anything other and 0x00 is not good
  if (pn532_packetbuffer[7] != 0x00)
  {
    #ifdef PN532DEBUG
    Serial.print(F("Authentification failed: "));
    Adafruit_NFCShield_I2C::PrintHexChar(pn532_packetbuffer, 12);
    #endif
    return 0;
  }  
  
  return 1;
}

/**************************************************************************/
/*! 
    Tries to read an entire 16-byte data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          Pointer to the byte array that will hold the
                          retrieved data (if any)
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t Adafruit_NFCShield_I2C::mifareclassic_ReadDataBlock (uint8_t blockNumber, uint8_t * data)
{
  #ifdef MIFAREDEBUG
  Serial.print(F("Trying to read 16 bytes from block "));Serial.println(blockNumber);
  #endif
  
  /* Prepare the command */
  pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = 1;                      /* Card number */
  pn532_packetbuffer[2] = MIFARE_CMD_READ;        /* Mifare Read command = 0x30 */
  pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */

  /* Send the command */
  if (! sendCommandCheckAck(pn532_packetbuffer, 4))
  {
    #ifdef MIFAREDEBUG
    Serial.println(F("Failed to receive ACK for read command"));
    #endif
    return 0;
  }

  /* Read the response packet */
  wirereaddata(pn532_packetbuffer, 26);

  /* If byte 8 isn't 0x00 we probably have an error */
  if (pn532_packetbuffer[7] != 0x00)
  {
    #ifdef MIFAREDEBUG
    Serial.println(F("Unexpected response"));
    Adafruit_NFCShield_I2C::PrintHexChar(pn532_packetbuffer, 26);
    #endif
    return 0;
  }
    
  /* Copy the 16 data bytes to the output buffer        */
  /* Block content starts at byte 9 of a valid response */
  memcpy (data, pn532_packetbuffer+8, 16);

  /* Display data for debug if requested */
  #ifdef MIFAREDEBUG
    Serial.print(F("Block "));
    Serial.println(blockNumber);
    Adafruit_NFCShield_I2C::PrintHexChar(data, 16);
  #endif

  return 1;  
}

/**************************************************************************/
/*! 
    Tries to write an entire 16-byte data block at the specified block
    address.

    @param  blockNumber   The block number to authenticate.  (0..63 for
                          1KB cards, and 0..255 for 4KB cards).
    @param  data          The byte array that contains the data to write.
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t Adafruit_NFCShield_I2C::mifareclassic_WriteDataBlock (uint8_t blockNumber, uint8_t * data)
{
  #ifdef MIFAREDEBUG
  Serial.print(F("Trying to write 16 bytes to block "));Serial.println(blockNumber);
  #endif
  
  /* Prepare the first command */
  pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = 1;                      /* Card number */
  pn532_packetbuffer[2] = MIFARE_CMD_WRITE;       /* Mifare Write command = 0xA0 */
  pn532_packetbuffer[3] = blockNumber;            /* Block Number (0..63 for 1K, 0..255 for 4K) */
  memcpy (pn532_packetbuffer+4, data, 16);          /* Data Payload */

  /* Send the command */
  if (! sendCommandCheckAck(pn532_packetbuffer, 20))
  {
    #ifdef MIFAREDEBUG
    Serial.println(F("Failed to receive ACK for write command"));
    #endif
    return 0;
  }  
  delay(10);
  
  /* Read the response packet */
  wirereaddata(pn532_packetbuffer, 26);

  return 1;  
}

/**************************************************************************/
/*! 
    Formats a Mifare Classic card to store NDEF Records 
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t Adafruit_NFCShield_I2C::mifareclassic_FormatNDEF (void)
{
  uint8_t sectorbuffer1[16] = {0x14, 0x01, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
  uint8_t sectorbuffer2[16] = {0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1, 0x03, 0xE1};
  uint8_t sectorbuffer3[16] = {0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0x78, 0x77, 0x88, 0xC1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

  // Note 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 must be used for key A
  // for the MAD sector in NDEF records (sector 0)
  
  // Write block 1 and 2 to the card
  if (!(mifareclassic_WriteDataBlock (1, sectorbuffer1)))
    return 0;
  if (!(mifareclassic_WriteDataBlock (2, sectorbuffer2)))
    return 0;
  // Write key A and access rights card
  if (!(mifareclassic_WriteDataBlock (3, sectorbuffer3)))
    return 0;

  // Seems that everything was OK (?!)
  return 1;
}

/**************************************************************************/
/*! 
    Writes an NDEF URI Record to the specified sector (1..15)
    
    Note that this function assumes that the Mifare Classic card is
    already formatted to work as an "NFC Forum Tag" and uses a MAD1
    file system.  You can use the NXP TagWriter app on Android to
    properly format cards for this.

    @param  sectorNumber  The sector that the URI record should be written
                          to (can be 1..15 for a 1K card)
    @param  uriIdentifier The uri identifier code (0 = none, 0x01 = 
                          "http://www.", etc.)
    @param  url           The uri text to write (max 38 characters).
    
    @returns 1 if everything executed properly, 0 for an error
*/
/**************************************************************************/
uint8_t Adafruit_NFCShield_I2C::mifareclassic_WriteNDEFURI (uint8_t sectorNumber, uint8_t uriIdentifier, const char * url)
{
  // Figure out how long the string is
  uint8_t len = strlen(url);
  
  // Make sure we're within a 1K limit for the sector number
  if ((sectorNumber < 1) || (sectorNumber > 15))
    return 0;
  
  // Make sure the URI payload is between 1 and 38 chars
  if ((len < 1) || (len > 38))
    return 0;
    
  // Note 0xD3 0xF7 0xD3 0xF7 0xD3 0xF7 must be used for key A
  // in NDEF records
	
  // Setup the sector buffer (w/pre-formatted TLV wrapper and NDEF message)
  uint8_t sectorbuffer1[16] = {0x00, 0x00, 0x03, len+5, 0xD1, 0x01, len+1, 0x55, uriIdentifier, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t sectorbuffer2[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t sectorbuffer3[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  uint8_t sectorbuffer4[16] = {0xD3, 0xF7, 0xD3, 0xF7, 0xD3, 0xF7, 0x7F, 0x07, 0x88, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
  if (len <= 6)
  {
    // Unlikely we'll get a url this short, but why not ...
    memcpy (sectorbuffer1+9, url, len);
    sectorbuffer1[len+9] = 0xFE;
  }
  else if (len == 7)
  {
    // 0xFE needs to be wrapped around to next block
    memcpy (sectorbuffer1+9, url, len);
    sectorbuffer2[0] = 0xFE;
  }
  else if ((len > 7) && (len <= 22))
  {
    // Url fits in two blocks
    memcpy (sectorbuffer1+9, url, 7);
    memcpy (sectorbuffer2, url+7, len-7);
    sectorbuffer2[len-7] = 0xFE;
  }
  else if (len == 23)
  {
    // 0xFE needs to be wrapped around to final block
    memcpy (sectorbuffer1+9, url, 7);
    memcpy (sectorbuffer2, url+7, len-7);
    sectorbuffer3[0] = 0xFE;
  }
  else
  {
    // Url fits in three blocks
    memcpy (sectorbuffer1+9, url, 7);
    memcpy (sectorbuffer2, url+7, 16);
    memcpy (sectorbuffer3, url+23, len-24);
    sectorbuffer3[len-22] = 0xFE;
  }
  
  // Now write all three blocks back to the card
  if (!(mifareclassic_WriteDataBlock (sectorNumber*4, sectorbuffer1)))
    return 0;
  if (!(mifareclassic_WriteDataBlock ((sectorNumber*4)+1, sectorbuffer2)))
    return 0;
  if (!(mifareclassic_WriteDataBlock ((sectorNumber*4)+2, sectorbuffer3)))
    return 0;
  if (!(mifareclassic_WriteDataBlock ((sectorNumber*4)+3, sectorbuffer4)))
    return 0;

  // Seems that everything was OK (?!)
  return 1;
}

/***** Mifare Ultralight Functions ******/

/**************************************************************************/
/*! 
    Tries to read an entire 4-byte page at the specified address.

    @param  page        The page number (0..63 in most cases)
    @param  buffer      Pointer to the byte array that will hold the
                        retrieved data (if any)
*/
/**************************************************************************/
uint8_t Adafruit_NFCShield_I2C::mifareultralight_ReadPage (uint8_t page, uint8_t * buffer)
{
  if (page >= 64)
  {
    #ifdef MIFAREDEBUG
    Serial.println(F("Page value out of range"));
    #endif
    return 0;
  }

  #ifdef MIFAREDEBUG
    Serial.print(F("Reading page "));Serial.println(page);
  #endif

  /* Prepare the command */
  pn532_packetbuffer[0] = PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = 1;                   /* Card number */
  pn532_packetbuffer[2] = MIFARE_CMD_READ;     /* Mifare Read command = 0x30 */
  pn532_packetbuffer[3] = page;                /* Page Number (0..63 in most cases) */

  /* Send the command */
  if (! sendCommandCheckAck(pn532_packetbuffer, 4))
  {
    #ifdef MIFAREDEBUG
    Serial.println(F("Failed to receive ACK for write command"));
    #endif
    return 0;
  }
  
  /* Read the response packet */
  wirereaddata(pn532_packetbuffer, 26);
  #ifdef MIFAREDEBUG
    Serial.println(F("Received: "));
    Adafruit_NFCShield_I2C::PrintHexChar(pn532_packetbuffer, 26);
  #endif

  /* If byte 8 isn't 0x00 we probably have an error */
  if (pn532_packetbuffer[7] == 0x00)
  {
    /* Copy the 4 data bytes to the output buffer         */
    /* Block content starts at byte 9 of a valid response */
    /* Note that the command actually reads 16 byte or 4  */
    /* pages at a time ... we simply discard the last 12  */
    /* bytes                                              */
    memcpy (buffer, pn532_packetbuffer+8, 4);
  }
  else
  {
    #ifdef MIFAREDEBUG
      Serial.println(F("Unexpected response reading block: "));
      Adafruit_NFCShield_I2C::PrintHexChar(pn532_packetbuffer, 26);
    #endif
    return 0;
  }

  /* Display data for debug if requested */
  #ifdef MIFAREDEBUG
    Serial.print(F("Page "));Serial.print(page);Serial.println(F(":"));
    Adafruit_NFCShield_I2C::PrintHexChar(buffer, 4);
  #endif

  // Return OK signal
  return 1;
}



/************** high level I2C */


/**************************************************************************/
/*! 
    @brief  Tries to read the PN532 ACK frame (not to be confused with 
	        the I2C ACK signal)
*/
/**************************************************************************/
boolean Adafruit_NFCShield_I2C::readackframe(void) {
  uint8_t ackbuff[6];
  
  wirereaddata(ackbuff, 6);
    
  return (0 == strncmp((char *)ackbuff, (char *)pn532ack, 6));
}

/************** mid level I2C */

/**************************************************************************/
/*! 
    @brief  Checks the IRQ pin to know if the PN532 is ready
	
	@returns 0 if the PN532 is busy, 1 if it is free
*/
/**************************************************************************/
uint8_t Adafruit_NFCShield_I2C::wirereadstatus(void) {
  uint8_t x = digitalRead(_irq);
  
  if (x == 1)
    return PN532_I2C_BUSY;
  else
    return PN532_I2C_READY;
}

/**************************************************************************/
/*! 
    @brief  Reads n bytes of data from the PN532 via I2C

    @param  buff      Pointer to the buffer where data will be written
    @param  n         Number of bytes to be read
*/
/**************************************************************************/
void Adafruit_NFCShield_I2C::wirereaddata(uint8_t* buff, uint8_t n) {
  uint16_t timer = 0;
  
  delay(2); 

#ifdef PN532DEBUG
  Serial.print(F("Reading: "));
#endif
  // Start read (n+1 to take into account leading 0x01 with I2C)
  WIRE.requestFrom((uint8_t)PN532_I2C_ADDRESS, (uint8_t)(n+2));
  // Discard the leading 0x01
  wirerecv();
  for (uint8_t i=0; i<n; i++) {
    delay(1);
    buff[i] = wirerecv();
#ifdef PN532DEBUG
    Serial.print(F(" 0x"));
    Serial.print(buff[i], HEX);
#endif
  }
  // Discard trailing 0x00 0x00
  // wirerecv();
    
#ifdef PN532DEBUG
  Serial.println();
#endif
}

/**************************************************************************/
/*! 
    @brief  Writes a command to the PN532, automatically inserting the
            preamble and required frame details (checksum, len, etc.)

    @param  cmd       Pointer to the command buffer
    @param  cmdlen    Command length in bytes 
*/
/**************************************************************************/
void Adafruit_NFCShield_I2C::wiresendcommand(uint8_t* cmd, uint8_t cmdlen) {
  uint8_t checksum;

  cmdlen++;
  
#ifdef PN532DEBUG
  Serial.print(F("\nSending: "));
#endif

  delay(2);     // or whatever the delay is for waking up the board

  // I2C START
  WIRE.beginTransmission(PN532_I2C_ADDRESS);
  checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
  wiresend(PN532_PREAMBLE);
  wiresend(PN532_PREAMBLE);
  wiresend(PN532_STARTCODE2);

  wiresend(cmdlen);
  wiresend(~cmdlen + 1);
 
  wiresend(PN532_HOSTTOPN532);
  checksum += PN532_HOSTTOPN532;

#ifdef PN532DEBUG
  Serial.print(F(" 0x")); Serial.print(PN532_PREAMBLE, HEX);
  Serial.print(F(" 0x")); Serial.print(PN532_PREAMBLE, HEX);
  Serial.print(F(" 0x")); Serial.print(PN532_STARTCODE2, HEX);
  Serial.print(F(" 0x")); Serial.print(cmdlen, HEX);
  Serial.print(F(" 0x")); Serial.print(~cmdlen + 1, HEX);
  Serial.print(F(" 0x")); Serial.print(PN532_HOSTTOPN532, HEX);
#endif

  for (uint8_t i=0; i<cmdlen-1; i++) {
   wiresend(cmd[i]);
   checksum += cmd[i];
#ifdef PN532DEBUG
   Serial.print(F(" 0x")); Serial.print(cmd[i], HEX);
#endif
  }
  
  wiresend(~checksum);
  wiresend(PN532_POSTAMBLE);
  
  // I2C STOP
  WIRE.endTransmission();

#ifdef PN532DEBUG
  Serial.print(F(" 0x")); Serial.print(~checksum, HEX);
  Serial.print(F(" 0x")); Serial.print(PN532_POSTAMBLE, HEX);
  Serial.println();
#endif
} 

/**************************************************************************/
/*! 
    @brief  Waits until the PN532 is ready.

    @param  timeout   Timeout before giving up
*/
/**************************************************************************/
boolean Adafruit_NFCShield_I2C::waitUntilReady(uint16_t timeout) {
  uint16_t timer = 0;
  while(wirereadstatus() != PN532_I2C_READY) {
    if (timeout != 0) {
      timer += 10;
      if (timer > timeout) {
        return false;
      }
    }
    delay(10);
  }
  return true;
}

/**************************************************************************/
/*! 
    @brief  Exchanges an APDU with the currently inlisted peer

    @param  send            Pointer to data to send
    @param  sendLength      Length of the data to send
    @param  response        Pointer to response data
    @param  responseLength  Pointer to the response data length
*/
/**************************************************************************/
boolean Adafruit_NFCShield_I2C::inDataExchange(uint8_t * send, uint8_t sendLength, uint8_t * response, uint8_t * responseLength) {
  if (sendLength > PN532_PACKBUFFSIZ -2) {
    #ifdef PN532DEBUG
      Serial.println(F("APDU length too long for packet buffer"));
    #endif
    return false;
  }
  uint8_t i;
  
  pn532_packetbuffer[0] = 0x40; // PN532_COMMAND_INDATAEXCHANGE;
  pn532_packetbuffer[1] = inListedTag;
  for (i=0; i<sendLength; ++i) {
    pn532_packetbuffer[i+2] = send[i];
  }
  
  if (!sendCommandCheckAck(pn532_packetbuffer,sendLength+2,1000)) {
    #ifdef PN532DEBUG
      Serial.println(F("Could not send ADPU"));
    #endif
    return false;
  }

  if (!waitUntilReady(1000)) {
    #ifdef PN532DEBUG
      Serial.println(F("Response never received for ADPU..."));
    #endif
    return false;
  }

  wirereaddata(pn532_packetbuffer,sizeof(pn532_packetbuffer));
  
  if (pn532_packetbuffer[0] == 0 && pn532_packetbuffer[1] == 0 && pn532_packetbuffer[2] == 0xff) {
    uint8_t length = pn532_packetbuffer[3];
    if (pn532_packetbuffer[4]!=(uint8_t)(~length+1)) {
      #ifdef PN532DEBUG
        Serial.println(F("Length check invalid"));
        Serial.println(length,HEX);
        Serial.println((~length)+1,HEX);
      #endif
      return false;
    }
    if (pn532_packetbuffer[5]==PN532_PN532TOHOST && pn532_packetbuffer[6]==PN532_RESPONSE_INDATAEXCHANGE) {
      if ((pn532_packetbuffer[7] & 0x3f)!=0) {
        #ifdef PN532DEBUG
          Serial.println(F("Status code indicates an error"));
        #endif
        return false;
      }
      
      length -= 3;
      
      if (length > *responseLength) {
        length = *responseLength; // silent truncation...
      }
      
      for (i=0; i<length; ++i) {
        response[i] = pn532_packetbuffer[8+i];
      }
      *responseLength = length;
      
      return true;
    } 
    else {
      Serial.print(F("Don't know how to handle this command: "));
      Serial.println(pn532_packetbuffer[6],HEX);
      return false;
    } 
  } 
  else {
    Serial.println(F("Preamble missing"));
    return false;
  }
}

/**************************************************************************/
/*! 
    @brief  'InLists' a passive target. PN532 acting as reader/initiator,
            peer acting as card/responder.
*/
/**************************************************************************/
boolean Adafruit_NFCShield_I2C::inListPassiveTarget() {
  pn532_packetbuffer[0] = PN532_COMMAND_INLISTPASSIVETARGET;
  pn532_packetbuffer[1] = 1;
  pn532_packetbuffer[2] = 0;
  
  #ifdef PN532DEBUG 
    Serial.print(F("About to inList passive target"));
  #endif

  if (!sendCommandCheckAck(pn532_packetbuffer,3,1000)) {
    #ifdef PN532DEBUG
      Serial.println(F("Could not send inlist message"));
    #endif
    return false;
  }

  if (!waitUntilReady(30000)) {
    return false;
  }

  wirereaddata(pn532_packetbuffer,sizeof(pn532_packetbuffer));
  
  if (pn532_packetbuffer[0] == 0 && pn532_packetbuffer[1] == 0 && pn532_packetbuffer[2] == 0xff) {
    uint8_t length = pn532_packetbuffer[3];
    if (pn532_packetbuffer[4]!=(uint8_t)(~length+1)) {
      #ifdef PN532DEBUG
        Serial.println(F("Length check invalid"));
        Serial.println(length,HEX);
        Serial.println((~length)+1,HEX);
      #endif
      return false;
    }
    if (pn532_packetbuffer[5]==PN532_PN532TOHOST && pn532_packetbuffer[6]==PN532_RESPONSE_INLISTPASSIVETARGET) {
      if (pn532_packetbuffer[7] != 1) {
        #ifdef PN532DEBUG
        Serial.println(F("Unhandled number of targets inlisted"));
        #endif
        Serial.println(F("Number of tags inlisted:"));
        Serial.println(pn532_packetbuffer[7]);
        return false;
      }
      
      inListedTag = pn532_packetbuffer[8];
      Serial.print(F("Tag number: "));
      Serial.println(inListedTag);
      
      return true;
    } else {
      #ifdef PN532DEBUG
        Serial.print(F("Unexpected response to inlist passive host"));
      #endif
      return false;
    } 
  } 
  else {
    #ifdef PN532DEBUG
      Serial.println(F("Preamble missing"));
    #endif
    return false;
  }

  return true;
}
