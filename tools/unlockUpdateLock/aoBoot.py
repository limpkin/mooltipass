# aoBoot v0.1
# Andrew Olson 2012
# Adapted to ATMega32u4 by M.Stephan
# PC side python program
from intelhex import IntelHex
import serial, sys
import commands
import time

class loader:
	def __init__(self):
		name = 'aoBoot'
	def writePage(self):
		self.ser.write('m')
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'Page write failed, return char:', ret_val

	def getId(self):
		self.ser.write('S')
		ret_val = self.ser.read(7)
		print "Bootloader ID:", ret_val

	def getType(self):
		self.ser.write('p')
		ret_val = self.ser.read()
		print "Bootloader Type:", ret_val

	def getVersion(self):
		self.ser.write('V')
		ret_val = self.ser.read(2)
		print "BL Version:", ret_val[0] + "." + ret_val[1]		
			
	def setAddress(self,addr):
		self.ser.write('A')
		self.ser.write(chr((addr>>9) & 0xff))
		self.ser.write(chr((addr>>1) & 0xff))
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'Set address failed, return char:', ret_val
		
	def sendLowByte(self,data):
		#print hex(data)
		self.ser.write('c')
		self.ser.write(chr(data))
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'Low byte write failed, return char:', ret_val

	def sendHighByte(self,data):
		#print hex(data)
		self.ser.write('C')
		self.ser.write(chr(data))
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'High byte write failed, return char:', ret_val

	def enableRwwSection(self):
		self.ser.write('E')
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'enableRwwSection write failed, return char:', ret_val

	def eraseFlash(self):
		self.ser.write('e')
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'Flash erase failed, return char:', ret_val

	def resetTimeOut(self):
		# Here is actually a read mem command with erroneous parameters
		self.ser.write("gggg")
		ret_val = self.ser.read()
		if ret_val != '?':
			print "Couldn't reset timeout timer, return char:", ret_val
			
	def writeFlashPage(self,data):
		self.ser.write("B" + chr(0) + chr(128) + "F")
		for i in range(0, 128):
			self.ser.write(chr(data[i]))
		ret_val = self.ser.read()
		if ret_val != '\r':
			print 'Write flash page failed, return char:', ret_val

	def openSerial(self,port,baud):
		try:
			self.ser = serial.Serial(port,baud)
		except:
			print 'Failed to open COM port'
			print 'Check stuff'		

def main():
	addr = 0
	page = 0
	port = sys.argv[1]
	firmware = IntelHex(sys.argv[2])
	
	# Wait for device to appear
	print "Waiting for device to appear"
	dev_found = False
	while dev_found == False:
		cmd_output = commands.getstatusoutput("ls " + port)
		if "such" not in cmd_output[1]:
			dev_found = True
	print "Device Found!"

	prog = loader()
	# try:
		
	# except:
		# print "Usage: "+sys.argv[0]+" name of hex file"
		# raise SystemExit
		
	prog.openSerial(port,57600)
	prog.getId()
	prog.getType()
	prog.getVersion()
	for i in range(0,len(firmware),128):
		prog.setAddress(addr)
		#print "Writing page", hex(addr)
		prog.writeFlashPage(firmware.tobinarray(start=addr, size=128))
		addr += 128
  
	prog.enableRwwSection()
	prog.ser.close()

if __name__ == '__main__':
	main()
