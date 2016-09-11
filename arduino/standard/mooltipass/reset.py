#!/usr/bin/env python

import serial, sys, time
serialPort = sys.argv[1]
ser = serial.Serial( port=serialPort, baudrate=1200,
    parity=serial.PARITY_NONE, stopbits=serial.STOPBITS_ONE,
    bytesize=serial.EIGHTBITS)
ser.isOpen()
time.sleep(0.1)
ser.close() # always close port
time.sleep(2)
