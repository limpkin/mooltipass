import os
import sys
 
import usb.core
import usb.util
 
from time import sleep
import random
 
# handler called when a report is received
def rx_handler(data):
	len = data[0];
	cmd = data[1];

	if cmd == 0x01:
		print 'dbg:', "".join(map(chr,data[2:len+2]))

 
def findHIDDevice(vendor_id, product_id):
    # Find device
    hid_device = usb.core.find(idVendor=vendor_id,idProduct=product_id)
    
    if not hid_device:
        print "No device connected"
    else:

        if hid_device.is_kernel_driver_active(0):
            try:
		print "detach device from kernel"
                hid_device.detach_kernel_driver(0)
            except usb.core.USBError as e:
                sys.exit("Could not detatch kernel driver: %s" % str(e))

        try:
		#hid_device.set_configuration()
		hid_device.reset()
        except usb.core.USBError as e:
		sys.exit("Could not set configuration: %s" % str(e))

        
        endpoint = hid_device[0][(0,0)][0]      
        
        while True:
            data = [0x0] * 32
                        
            #read the data
            bytes = hid_device.read(endpoint.bEndpointAddress, 32)
            rx_handler(bytes);
            
 
if __name__ == '__main__':
    vendor_id = 0x16C0
    product_id = 0x047C

    print "USB Debug Client"
 
    # Search for the mooltipass and read hid data
    findHIDDevice(vendor_id, product_id)
s
