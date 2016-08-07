from pyqtgraph.Qt import QtGui, QtCore
from array import array
import pyqtgraph as pg
import numpy as np
import datetime
import platform
import usb.core
import usb.util
import random
import serial
import time
import sys
import os

USB_VID					= 0x16D0
USB_PID					= 0x09A0
LEN_INDEX               = 0x00
CMD_INDEX               = 0x01
DATA_INDEX              = 0x02
CMD_PING				= 0xA1

def receiveHidPacket(epin):
	try :
		data = epin.read(epin.wMaxPacketSize, timeout=15000)
		return data
	except usb.core.USBError as e:
		#print e
		sys.exit("Mooltipass didn't send a packet")

def receiveHidPacketWithTimeout(epin):
	try :
		data = epin.read(epin.wMaxPacketSize, timeout=15000)
		return data
	except usb.core.USBError as e:
		return None
		
def sendHidPacket(epout, cmd, len, data):
	# data to send
	arraytosend = array('B')

	# if command copy it otherwise copy the data
	if cmd != 0:
		arraytosend.append(len)
		arraytosend.append(cmd)

	# add the data
	if data is not None:
		arraytosend.extend(data)

	#print arraytosend
	#print arraytosend

	# send data
	epout.write(arraytosend)


def findHIDDevice(vendor_id, product_id, print_debug):
	# Find our device
	hid_device = usb.core.find(idVendor=vendor_id, idProduct=product_id)

	# Was it found?
	if hid_device is None:
		if print_debug:
			print "Device not found"
		return None, None, None, None, None

	# Device found
	if print_debug:
		print "Mooltipass found"

	# Different init codes depending on the platform
	if platform.system() == "Linux":
		# Need to do things differently
		try:
			hid_device.detach_kernel_driver(0)
			hid_device.reset()
		except Exception, e:
			pass # Probably already detached
	else:
		# Set the active configuration. With no arguments, the first configuration will be the active one
		try:
			hid_device.set_configuration()
		except Exception, e:
			if print_debug:
				print "Cannot set configuration the device:" , str(e)
			return None, None, None, None, None

	#for cfg in hid_device:
	#	print "configuration val:", str(cfg.bConfigurationValue)
	#	for intf in cfg:
	#		print "int num:", str(intf.bInterfaceNumber), ", int alt:", str(intf.bAlternateSetting)
	#		for ep in intf:
	#			print "endpoint addr:", str(ep.bEndpointAddress)

	# Get an endpoint instance
	cfg = hid_device.get_active_configuration()
	intf = cfg[(0,0)]

	# Match the first OUT endpoint
	epout = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_OUT)
	if epout is None:
		hid_device.reset()
		return None, None, None, None, None
	#print "Selected OUT endpoint:", epout.bEndpointAddress

	# Match the first IN endpoint
	epin = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN)
	if epin is None:
		hid_device.reset()
		return None, None, None, None, None
	#print "Selected IN endpoint:", epin.bEndpointAddress
	
	# If platform already streaming values
	try :
		data = epin.read(epin.wMaxPacketSize, timeout=200)
		if data and data[CMD_INDEX] == 0x9F:
			return hid_device, intf, epin, epout, True
	except usb.core.USBError as e:
		print ""

	# prepare ping packet
	byte1 = random.randint(0, 255)
	byte2 = random.randint(0, 255)
	ping_packet = array('B')
	ping_packet.append(2)
	ping_packet.append(CMD_PING)
	ping_packet.append(byte1)
	ping_packet.append(byte2)

	time.sleep(0.5)
	try:
		# try to send ping packet
		epout.write(ping_packet)
		# try to receive one answer
		temp_bool = 0
		while temp_bool == 0:
			try :
				# try to receive answer
				data = epin.read(epin.wMaxPacketSize, timeout=2000)
				if data[CMD_INDEX] == CMD_PING and data[DATA_INDEX] == byte1 and data[DATA_INDEX+1] == byte2 :
					temp_bool = 1
					if print_debug:
						print "Mooltipass replied to our ping message"
				else:
					if print_debug:
						print "Cleaning remaining input packets"
				time.sleep(.5)
			except usb.core.USBError as e:
				if print_debug:
					print e
				return None, None, None, None, None
	except usb.core.USBError as e:
		if print_debug:
			print e
		return None, None, None, None, None

	# Return device & endpoints
	return hid_device, intf, epin, epout, False
	
def collect():
	global xcurve, ycurve, zcurve, xdata, ydata, zdata, zcordata, detectiondata, last_second, sample_counter, mutex

	# data read
	data = receiveHidPacketWithTimeout(epin)
	sample_counter += 1
	
	# sample counter
	now = datetime.datetime.now()
	if now.second != last_second:
		print sample_counter , "samples per second (should be around 400)"
		last_second = now.second
		sample_counter = 0
	
	# cast data
	#xvalue = data[DATA_INDEX] + data[DATA_INDEX+1]*256
	#yvalue = data[DATA_INDEX+2] + data[DATA_INDEX+3]*256
	#zvalue = data[DATA_INDEX+4] + data[DATA_INDEX+5]*256
	#zcorvalue = data[DATA_INDEX+6] + data[DATA_INDEX+7]*256
	#detvalue = data[DATA_INDEX+8] + data[DATA_INDEX+9]*256
	#
	#if xvalue >= 32768:
	#	xvalue = -((~xvalue & 0x7FFF) + 1)
	#if yvalue >= 32768:
	#	yvalue = -((~yvalue & 0x7FFF) + 1)
	#if zvalue >= 32768:
	#	zvalue = -((~zvalue & 0x7FFF) + 1)
	#if zcorvalue >= 32768:
	#	zcorvalue = -((~zcorvalue & 0x7FFF) + 1)
	
	xvalue = data[DATA_INDEX+1]
	yvalue = data[DATA_INDEX+3]
	zvalue = data[DATA_INDEX+5]
	zcorvalue = data[DATA_INDEX+7]
	detvalue = data[DATA_INDEX+9]
	
	if detvalue == 20:
		print "algo armed"
	elif detvalue == 50:
		print "------ pulse too large ------"
	elif detvalue == 100:
		print "algo disarmed"
	elif detvalue == 255:
		print "------ detection ------"
	
	if xvalue >= 128:
		xvalue = -((~xvalue & 0x7F) + 1)
	if yvalue >= 128:
		yvalue = -((~yvalue & 0x7F) + 1)
	if zvalue >= 128:
		zvalue = -((~zvalue & 0x7F) + 1)
		
	#print yvalue
	

	mutex.lock()
	# append to data list
	xdata.append(float(xvalue))
	ydata.append(float(yvalue))
	zdata.append(float(zvalue))
	zcordata.append(float(zcorvalue))
	detectiondata.append(float(detvalue))

	# plot 
	xdata = xdata[-1000:]
	ydata = ydata[-1000:]
	zdata = zdata[-1000:]
	zcordata = zcordata[-1000:]
	detectiondata = detectiondata[-1000:]
	mutex.unlock()

def update():
	global xcurve, ycurve, zcurve, xdata, ydata, zdata, zcordata, detectiondata, last_second, sample_counter, mutex
	
	mutex.lock()
	xcurve.setData(xdata)
	ycurve.setData(ydata)
	zcurve.setData(zdata)
	zcorcurve.setData(zcordata)
	detectioncurve.setData(detectiondata)
	mutex.unlock()
	app.processEvents()	 

app = QtGui.QApplication([])
mutex = QtCore.QMutex()
xdata = [0]
ydata = [0]
zdata = [0]
zcordata = [0]
detectiondata = [0]
last_second = 0
sample_counter = 0

# set up a plot window
graph = pg.plot()
graph.setWindowTitle("Mooltipass realtime accelerometer data")
graph.setInteractive(True)

xcurve = graph.plot(pen=(255,0,0), name="X axis")
ycurve = graph.plot(pen=(0,255,0), name="Y axis")
zcurve = graph.plot(pen=(0,0,255), name="Z axis")
zcorcurve = graph.plot(pen=(255,255,255), name="Z axis corrected")
detectioncurve = graph.plot(pen=(255,0,255), name="Detections")

# Search for the mooltipass and read hid data
hid_device, intf, epin, epout, isStreaming = findHIDDevice(USB_VID, USB_PID, True)
if hid_device is None:
	sys.exit(0)
	
# Start stream mode
if isStreaming == False:
	sendHidPacket(epout, 0x9F, 0, None)

# Qt timers
displaytimer = QtCore.QTimer()
displaytimer.timeout.connect(update)
displaytimer.start(0)
# Data gathering timers
datatimer = QtCore.QTimer()
datatimer.timeout.connect(collect)
datatimer.start(0)


if __name__ == '__main__':	
	if (sys.flags.interactive != 1) or not hasattr(QtCore, 'PYQT_VERSION'):
		QtGui.QApplication.instance().exec_()		
		