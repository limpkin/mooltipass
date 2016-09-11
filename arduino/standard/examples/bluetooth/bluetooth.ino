/*
BluetoothShield Demo Code Slave.pde. This sketch could be used with
Master.pde to establish connection between two Arduino. It can also
be used for one slave bluetooth connected by the device(PC/Smart Phone)
with bluetooth function.
2011 Copyright (c) Seeed Technology Inc.  All right reserved.

Author: Steve Chang

This demo code is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

For more details about the product please check http://www.seeedstudio.com/depot/

*/
#include <SoftwareSerial.h>
#include <usart_spi.h>
#include <oledmp.h>
#define RxD 9
#define TxD 6
#define DEBUG_ENABLED  1

SoftwareSerial blueToothSerial(RxD,TxD);
USARTSPI spi(SPI_BAUD_8_MHZ);
OledMP oled(spi);

// /!\ SOFTWARE SERIAL LIBRARY: Not all pins on the Leonardo support change interrupts, so only the following can be used for RX: 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI). /!\


void setup()
{
  spi.begin();
  oled.begin();
  oled.printf(F("BT sketch\n"));
  delay(4000);    
  Serial.begin(9600);  
  pinMode(RxD, INPUT);
  pinMode(TxD, OUTPUT);
  blueToothSerial.begin(38400);                           // Set BluetoothBee BaudRate to default baud rate 38400
  setupBlueToothConnection();
}

void loop()
{
    char temp_string[2] = {0, 0};
    char recvChar;
    while(1)
    {
        if(blueToothSerial.available())
        {//check if there's any data sent from the remote bluetooth shield
            recvChar = blueToothSerial.read();
            temp_string[0] = recvChar;
            oled.printf(temp_string);
            Serial.print(recvChar);
        }
        if(Serial.available())
        {//check if there's any data sent from the local serial terminal, you can add the other applications here
            recvChar  = Serial.read();
            blueToothSerial.print(recvChar);
        }
    }
}

void setupBlueToothConnection()
{
    blueToothSerial.print("\r\n+STWMOD=0\r\n");             // set the bluetooth work in slave mode
    blueToothSerial.print("\r\n+STNA=SeeedBTSlave\r\n");    // set the bluetooth name as "SeeedBTSlave"
    blueToothSerial.print("\r\n+STOAUT=1\r\n");             // Permit Paired device to connect me
    blueToothSerial.print("\r\n+STAUTO=0\r\n");             // Auto-connection should be forbidden here
    delay(2000);                                            // This delay is required.
    blueToothSerial.print("\r\n+INQ=1\r\n");                // make the slave bluetooth inquirable
    Serial.println("The slave bluetooth is inquirable!");
    delay(2000);                                            // This delay is required.
    blueToothSerial.flush();
}
