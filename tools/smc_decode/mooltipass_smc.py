# Mooltipass Export File Decoding Utility
#
# What to install:
# - swig from http://www.swig.org/download.html (Windows: download the archive, copy the contained folder to C:\Program Files (x86), add the folder to PATH)
# - pyscard from http://sourceforge.net/projects/pyscard/files/pyscard/ or by "typing pip install pyscard" (official website: http://pyscard.sourceforge.net/)
# - The MSI installer for PC/SC driver: http://www.acs.com.hk/en/products/4/acr38-smart-card-reader/#tab_downloads
from smartcard.CardRequest import CardRequest
from smartcard.CardType import AnyCardType
from smartcard.CardService import *
from smartcard.Exceptions import *
from smartcard.System import *
from smartcard.scard import *
from smartcard.util import *
import sys

if __name__ == '__main__':
	# Main function	
	try:
		# Context establishment
		hresult, hcontext = SCardEstablishContext(SCARD_SCOPE_SYSTEM)
		if hresult != SCARD_S_SUCCESS:
			raise Exception('Failed to establish context : ' +	SCardGetErrorMessage(hresult))
		print 'Context established!'

		# Find the correct reader
		try:
			hresult, readers = SCardListReaders(hcontext, [])
			if hresult != SCARD_S_SUCCESS:
				raise Exception('Failed to list readers: ' + SCardGetErrorMessage(hresult))
			print 'PCSC Readers:', readers

			if len(readers) < 1:
				raise Exception('No smart card readers')

			# connect to smartcard
			hresult, hcard, dwActiveProtocol = SCardConnect(hcontext, readers[0], SCARD_SHARE_DIRECT, 0)
			if hresult != SCARD_S_SUCCESS:
				raise Exception('Unable to connect: ' +	SCardGetErrorMessage(hresult))
			print 'Connected with active protocol', dwActiveProtocol
			
			# check active protocol
			if dwActiveProtocol == SCARD_PROTOCOL_T0:
				pioSendPci = SCARD_PCI_T0
			elif dwActiveProtocol == SCARD_PROTOCOL_T1:
				pioSendPci = SCARD_PCI_T1
			else:
				pioSendPci = dwActiveProtocol

			try:	
				#TEST = []
				#hresult, response = SCardControl(hcard, SCARD_CTL_CODE(2067), TEST)
				#if hresult != SCARD_S_SUCCESS:
				#	raise Exception('Failed to transmit get version: ' + SCardGetErrorMessage(hresult))
				#print smartcard.util.toHexString(response, smartcard.util.HEX)
				#
				TEST = [0x09, 0x00, 0x00, 0x00]
				hresult, response = SCardControl(hcard, SCARD_CTL_CODE(2060), TEST)
				if hresult != SCARD_S_SUCCESS:
					raise Exception('Failed to transmit card type: ' + SCardGetErrorMessage(hresult))
				print smartcard.util.toHexString(response, smartcard.util.HEX)
				
				TEST = [0xFF, 0xA4, 0x00, 0x00, 0x01, 0x09]
				hresult, response = SCardTransmit(hcard, dwActiveProtocol, TEST)
				if hresult != SCARD_S_SUCCESS:
					raise error('Failed to transmit: ' + SCardGetErrorMessage(hresult))
				
				for i in range(0, 65536):
					if i != 2050:
						TEST = []	
						TEST = [9, 0x00, 0x00, 0x00]
						print "Trying with " + repr(i)
						hresult, response = SCardControl(hcard, SCARD_CTL_CODE(i), TEST)	
						if hresult == SCARD_S_SUCCESS:
							print "Valid command found: " + repr(i) + " -> "+ hex(i)
							print smartcard.util.toHexString(response, smartcard.util.HEX)
							raw_input("Press enter")
				
				TEST = []
				hresult, response = SCardControl(hcard, SCARD_CTL_CODE(0x0101), TEST)
				if hresult != SCARD_S_SUCCESS:
					raise Exception("Failed to transmit test: " + SCardGetErrorMessage(hresult))
				print smartcard.util.toHexString(response, smartcard.util.HEX)
			finally:
				hresult = SCardDisconnect(hcard, SCARD_UNPOWER_CARD)
				if hresult != SCARD_S_SUCCESS:
					raise Exception('Failed to disconnect: ' + SCardGetErrorMessage(hresult))
				print 'Disconnected'

			#except Exception, message:
			#	print "Exception:", message

		finally:
			hresult = SCardReleaseContext(hcontext)
			if hresult != SCARD_S_SUCCESS:
				raise Exception('Failed to release context: ' +
						SCardGetErrorMessage(hresult))
			print 'Released context.'

	except Exception, message:
		print "Exception:", message