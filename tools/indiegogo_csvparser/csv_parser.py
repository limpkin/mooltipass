from unidecode import unidecode
import math
import time
import csv
import re

def findEmailInList(list, email):
	for item in list:
		# Do something with item
		if item == email:
			return True
	return False

def unicode_csv_reader(utf8_data, dialect=csv.excel, **kwargs):
	csv_reader = csv.reader(utf8_data, dialect=dialect, **kwargs)
	for row in csv_reader:
		yield [unidecode(unicode(cell, 'utf-8')) for cell in row]
		
if __name__ == '__main__':
	# Main function
	print ""
	print "Mooltipass backers csv parser"
	print ""
	
	# Totals
	number_normal_cards = 0
	number_custom_card = 0
	number_com_card = 0
	number_abs = 0
	number_al = 0
	
	# Prints defs
	raw_printout = True
	perk_printout = True
	addr_printout = True
	anomaly_printout = False
	
	# 7 more cards list
	morecards_list = []
	
	# Csv reader, 7 more cards loop
	reader = unicode_csv_reader(open('mooltipass.txt'))
	for contributionID, giftID, shippingstatus, backingdate, payingmean, displaystatus, name, email, amount, chosenperk, backername, backeraddress, backeraddress2, backercity, backercounty, backercitycode, backercountry in reader:
		if chosenperk == "7 more cards":
			morecards_list.append(email)

	reader = unicode_csv_reader(open('mooltipass.txt'))
	for contributionID, giftID, shippingstatus, backingdate, payingmean, displaystatus, name, email, amount, chosenperk, backername, backeraddress, backeraddress2, backercity, backercounty, backercitycode, backercountry in reader:
		# Strip the ="" in the postal code
		backercitycode = re.sub('[="]', '', backercitycode)
		numamount = re.sub('[$]', '', amount)
		# Printing the fields as is		
		if raw_printout :
			print "-------------------------------------------------"
			if chosenperk: 
				print "Chosen perk:     ", chosenperk, "-", amount
			else:
				print amount, "donation"
			print "Backer email:    ", email
			if backername:
				print "Backer name:     ", backername
			if backeraddress:
				print "Backer address:  ", backeraddress
			if backeraddress2:
				print "Backer address2: ", backeraddress2
			if backercity:
				print "Backer city:     ", backercity
			if backercounty:
				print "Backer county:   ", backercounty
			if backercitycode:
				print "Backer city code:", backercitycode
			if backercountry:
				print "Backer country:  ", backercountry
			print ""
		#  Do some processing and suggest the address
		if chosenperk != "" and chosenperk != "Thank you!" and chosenperk != "7 more cards":
			# suggested perk
			user_cards = 0
			if chosenperk == "ABS Mooltipass":
				user_cards = int(math.floor((float(numamount)-100)/1.5))+2
				printout_perk_text = "ABS Mooltipass and"
				number_abs += 1
			elif chosenperk == "Aluminum Mooltipass":
				user_cards = int(math.floor((float(numamount)-140)/1.5))+2
				printout_perk_text = "Aluminum Mooltipass and"
				number_al += 1
			elif chosenperk == "EARLY ADOPTERS #1":
				user_cards = int(math.floor((float(numamount)-80)/1.5))+2
				printout_perk_text = "ABS Mooltipass and"
				number_abs += 1
			elif chosenperk == "EARLY ADOPTERS #2":
				user_cards = int(math.floor((float(numamount)-90)/1.5))+2
				printout_perk_text = "ABS Mooltipass and"
				number_abs += 1
			elif chosenperk == "Two ABS Mooltipass sets":
				user_cards = int(math.floor((float(numamount)-190)/1.5))+5
				printout_perk_text = "2x ABS Mooltipass and"
				number_abs += 2
			elif chosenperk == "Two Aluminum Mooltipass sets":
				user_cards = int(math.floor((float(numamount)-270)/1.5))+5
				printout_perk_text = "2x Aluminum Mooltipass and"
				number_al += 2
			elif chosenperk == "ABS  + Aluminum Mooltipass":
				user_cards = int(math.floor((float(numamount)-230)/1.5))+5
				printout_perk_text = "1x ABS Mooltipass + 1x Aluminum Mooltipass and"
				number_abs += 1
				number_al += 1
			elif chosenperk == "Your very own smartcard - Al":
				printout_perk_text = "1x Aluminum Mooltipass and a CUSTOM SMARTCARD"
				number_custom_card += 1
				number_al += 1
			elif chosenperk == "Your very own smartcard - ABS":
				printout_perk_text = "1x ABS Mooltipass and a CUSTOM SMARTCARD"
				number_custom_card += 1
				number_abs += 1
			elif chosenperk == "Commemorative smartcard":
				printout_perk_text = "Commemorative smartcard"
				number_com_card += 1
			else:
				print "ERROR"
				time.sleep(100)
				
			# see if the user opted for the 7 more cards perk
			if findEmailInList(morecards_list, email):
				print "user took the 7 more cards perk"
				user_cards += 7
			
			# add the cards
			number_normal_cards += user_cards
			
			# print the perk
			if perk_printout:
				# if the user's perk includes cards
				if user_cards > 0:
					print printout_perk_text + " " + repr(user_cards) + " smartcards"
				else:
					print printout_perk_text
					
			# suggested address...
			if addr_printout:
				print "To:"
				print backername
				# if the user just provided a street number
				if backeraddress.isdigit() or backeraddress2.isdigit():
					print backeraddress, backeraddress2
				else:
					print backeraddress
					if backeraddress2 != "" and backeraddress != backeraddress2: 
						print backeraddress2
				print backercity, backercounty, backercitycode
				print backercountry
				print ""
		else:
			# Thank you, no chosen perk...
			if int(numamount) > 10 and anomaly_printout:
				print "Atypical amount:", amount, "from", email, "-", chosenperk
				time.sleep(1)
		#time.sleep(2)
	
	print "-------------------------------------------------"
	print "Total number of ABS:", number_abs
	print "Total number of Al:", number_al
	print "Total number of custom cards:", number_custom_card
	print "Total number of normal cards:", number_normal_cards
	print "Total number of commemorative cards:", number_com_card