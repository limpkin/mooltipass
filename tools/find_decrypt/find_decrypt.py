from Crypto.PublicKey import RSA
from os.path import isfile, join
from array import array
from os import listdir
import pyqrcode
import pickle
import os

def pickle_read(filename):
	f = open(filename)
	data = pickle.load(f)
	f.close()
	return data

if __name__ == '__main__':
	# Main function
	print ""
	print "Mooltipass Decryption Tool"

	# Check for key file
	if not os.path.isfile("key.bin"):
		print "Decryption key file key.bin isn't found"
		sys.exit()
	else:
		print "Decryption key file key.bin found"
		
	# Import key
	key = RSA.importKey(pickle_read("key.bin"))
	
	# List files in the export folder
	export_file_names = [f for f in listdir("export/") if isfile(join("export/", f))]
		
	# Ask for Mooltipass ID
	mooltipass_id = raw_input("Enter Mooltipass ID: ")
	
	# Find Mooltipass ID in files
	for file_name in export_file_names:
		if mooltipass_id in file_name:
			print "Found export file:", file_name
			data = pickle_read(join("export/",file_name))
			decrypted_data = key.decrypt(data)
			items = decrypted_data.split('|')
			#print decrypted_data
			
			# Password
			password = items[1]
			password_qr = pyqrcode.create(password)
			print ""
			print "Password:"
			print(password_qr.terminal(quiet_zone=1))
			
			# Request UID
			request = items[2][:32]
			request_qr = pyqrcode.create(request)
			print "Request UID key:"
			print(request_qr.terminal(quiet_zone=1))
			
			# Request UID
			request = items[2][32:32+12]
			request_qr = pyqrcode.create(request)
			print "UID :"
			print(request_qr.terminal(quiet_zone=1))
			
			