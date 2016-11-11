from intelhex import IntelHex
import hashlib
import os


def generateFlashAndEepromHex(originalFlashHexName, bootloaderHexName, serialNumber, AESKey1, AESKey2, UIDKey, UID, newFlashHexName, newBootloaderHex, verbose):
	FW_MAX_LENGTH = 28672
	BL_MAX_LENGTH = 4096
	
	# Check for original firmware file presence
	if not os.path.isfile(originalFlashHexName):
		print "Couldn't find firmware hex file", originalFlashHexName
		return
				
	# Check for bootloader file presence
	if not os.path.isfile(bootloaderHexName):
		print "Couldn't find bootloader hex file", bootloaderHexName
		return
		
	# Read firmware Hex
	flashHex = IntelHex(originalFlashHexName)
	if len(flashHex) > FW_MAX_LENGTH:
		print "Firmware file too long:", len(flashHex), "bytes long"
		return False
	else:	
		if verbose == True:
			print "Firmware file is", len(flashHex), "bytes long"
	
	# Read bootloader hex
	bootloaderHex = IntelHex(bootloaderHexName)
	if len(bootloaderHex) > BL_MAX_LENGTH:
		print "Bootloader file too long:", len(bootloaderHex), "bytes long"
		return False
	else:	
		if verbose == True:
			print "Bootloader file is", len(bootloaderHex), "bytes long"
	
	# Merge firmware with bootloader	
	flashHex.merge(bootloaderHex)
	
	# Print hash if need
	if verbose == True:
		print "Original Firmware/Bootloader Hash:", hashlib.sha1(flashHex.tobinarray()).hexdigest()

	# Include serial number in the hex to be flashed
	flashHex[0x7F7C] = (serialNumber >> 24) & 0x000000FF
	flashHex[0x7F7D] = (serialNumber >> 16) & 0x000000FF
	flashHex[0x7F7E] = (serialNumber >> 8) & 0x000000FF
	flashHex[0x7F7F] = (serialNumber >> 0) & 0x000000FF
	
	# Write production firmware file
	flashHex.tofile(newFlashHexName, format="hex")

		
generateFlashAndEepromHex("Mooltipass.hex", "bootloader_mini.hex", 323232, [2,2,2], [2,2,2], [2,2,2], [2,2,2], "newflash.hex", "newbooltoaderhex.hex", True)