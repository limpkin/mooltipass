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

def findHIDDevice(vendor_id, product_id, print_debug):
	# Find our device
	hid_device = usb.core.find(idVendor=vendor_id, idProduct=product_id)

	# Was it found?
	if hid_device is None:
		if print_debug:
			print "Device not found"
		return None, None, None, None

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
			return None, None, None, None

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
		return None, None, None, None
	#print "Selected OUT endpoint:", epout.bEndpointAddress

	# Match the first IN endpoint
	epin = usb.util.find_descriptor(intf, custom_match = lambda e: usb.util.endpoint_direction(e.bEndpointAddress) == usb.util.ENDPOINT_IN)
	if epin is None:
		hid_device.reset()
		return None, None, None, None
	#print "Selected IN endpoint:", epin.bEndpointAddress

	# Return device & endpoints
	return hid_device, intf, epin, epout
	
def collect():
	global xcurve, ycurve, zcurve, xdata, ydata, zdata, last_second, sample_counter

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
	xvalue = data[DATA_INDEX] + data[DATA_INDEX+1]*256
	yvalue = data[DATA_INDEX+2] + data[DATA_INDEX+3]*256
	zvalue = data[DATA_INDEX+4] + data[DATA_INDEX+5]*256
	
	if xvalue > 32768:
		xvalue = -((~xvalue & 0x7FFF) + 1)
	if yvalue > 32768:
		yvalue = -((~yvalue & 0x7FFF) + 1)
	if zvalue > 32768:
		zvalue = -((~zvalue & 0x7FFF) + 1)

	# append to data list
	xdata.append(float(xvalue))
	ydata.append(float(yvalue))
	zdata.append(float(zvalue))

	# plot 
	xdata = xdata[-1000:]
	ydata = ydata[-1000:]
	zdata = zdata[-1000:]

def update():
	global xcurve, ycurve, zcurve, xdata, ydata, zdata, last_second, sample_counter
	
	xcurve.setData(xdata)
	ycurve.setData(ydata)
	zcurve.setData(zdata)
	app.processEvents()	 

app = QtGui.QApplication([])
xdata = [0]
ydata = [0]
zdata = [0]
last_second = 0
sample_counter = 0

# set up a plot window
graph = pg.plot()
graph.setWindowTitle("Mooltipass realtime accelerometer data")
graph.setInteractive(True)

xcurve = graph.plot(pen=(255,0,0), name="X axis")
ycurve = graph.plot(pen=(0,255,0), name="Y axis")
zcurve = graph.plot(pen=(0,0,255), name="Z axis")

# Search for the mooltipass and read hid data
hid_device, intf, epin, epout = findHIDDevice(USB_VID, USB_PID, True)
if hid_device is None:
	sys.exit(0)

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
		