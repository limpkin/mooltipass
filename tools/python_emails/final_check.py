from unidecode import unidecode
import hashlib
import smtplib
import urllib
import math
import time
import sys
import csv
import re

#indexes for orders list
INDEX_ORDER_ID		= 0
INDEX_ORDER_SUMMARY	= 1
INDEX_EMAIL_ADDR	= 2
INDEX_NAME			= 3
INDEX_ADDR			= 4
INDEX_ADDR2			= 5
INDEX_CITY			= 6
INDEX_ZIP			= 7
INDEX_COUNTY		= 8
INDEX_COUNTRY		= 9
INDEX_ABS			= 10
INDEX_AL			= 11
INDEX_CARD			= 12
INDEX_CUSTOM_CARD	= 13
INDEX_COMMMORATIVE	= 14
INDEX_GROSS			= 15
INDEX_NET			= 16
INDEX_TRANS_IDS		= 17
#indexes for orders list
GOOGLE_INDEX_TIMESTAMP		= 0
GOOGLE_INDEX_NAME			= 1
GOOGLE_INDEX_EMAIL_ADDR		= 2
GOOGLE_INDEX_ADDR			= 3
GOOGLE_INDEX_ADDR2			= 4
GOOGLE_INDEX_CITY			= 5
GOOGLE_INDEX_COUNTY			= 6	
GOOGLE_INDEX_COUNTRY		= 7
GOOGLE_INDEX_ORDER_ID		= 8
GOOGLE_INDEX_PHONE			= 9
GOOGLE_INDEX_PREFILLED		= 10
GOOGLE_INDEX_ZIP			= 11


def sendOrderSummaryEmail(session, log_f, order_item):
	# Recreate perk text
	printout_perk_text = ""
	if int(order_item[INDEX_ABS]) > 0:
		printout_perk_text += order_item[INDEX_ABS] + "x ABS Mooltipass, "
	if int(order_item[INDEX_AL]) > 0:
		printout_perk_text += order_item[INDEX_AL] + "x Aluminum Mooltipass, "
	if int(order_item[INDEX_CARD]) > 0:
		printout_perk_text += order_item[INDEX_CARD] + " smartcards, "
	if int(order_item[INDEX_CUSTOM_CARD]) > 0:
		printout_perk_text += order_item[INDEX_CUSTOM_CARD] + " custom cards, "
	if int(order_item[INDEX_COMMMORATIVE]) > 0:
		printout_perk_text += order_item[INDEX_COMMMORATIVE] + " commemorative cards, "
	# remove last 2 chars
	printout_perk_text = printout_perk_text[:-2]
	
	# Remove bad chars
	if order_item[INDEX_ADDR] == "-" or order_item[INDEX_ADDR] == "/" or order_item[INDEX_ADDR] == "NA":
		order_item[INDEX_ADDR] = ""
	if order_item[INDEX_ADDR2] == "-" or order_item[INDEX_ADDR2] == "/" or order_item[INDEX_ADDR2] == "NA":
		order_item[INDEX_ADDR2] = ""	
	
	# Check that the text correctly describes the ordering fields
	if order_item[INDEX_ORDER_SUMMARY] == printout_perk_text:
		email_recipient = order_item[INDEX_EMAIL_ADDR]
		email_subject = "[Mooltipass] [/!\ACTION REQUIRED/!\] Order #" + order_item[INDEX_ORDER_ID] + " - Delivery Address and Phone Number"
		body_of_email = "Dear " + order_item[INDEX_NAME] + ",<br><br>"
		body_of_email += "The Mooltipass team would like to thank you again for backing its campaign and making the Mooltipass a reality.<br>"
		body_of_email += "The mass production is about to start and we are hoping to ship the devices by the <b>end of May</b>.<br>"
		body_of_email += "We therefore need you to click "
		body_of_email += "<a href=\"https://docs.google.com/forms/d/1ExZ5qCCUrqEShic6TmJQwKD_svs3akhtoGADBpJWSCE/viewform?entry.2010172533="+urllib.quote_plus(order_item[INDEX_NAME])+"&entry.18105043="+urllib.quote_plus(order_item[INDEX_EMAIL_ADDR])+"&entry.587847468="+urllib.quote_plus(order_item[INDEX_ORDER_ID])+"&entry.707680627="+urllib.quote_plus(order_item[INDEX_ADDR])+"&entry.1539828860="+urllib.quote_plus(order_item[INDEX_ADDR2])+"&entry.1766202789="+urllib.quote_plus(order_item[INDEX_ZIP])+"&entry.1048851778="+urllib.quote_plus(order_item[INDEX_CITY])+"&entry.183849517="+urllib.quote_plus(order_item[INDEX_COUNTY])+"&entry.364197520="+urllib.quote_plus(order_item[INDEX_COUNTRY])+"&entry.1664263874=" + hashlib.sha512(SECRET_SALT+order_item[INDEX_EMAIL_ADDR]+order_item[INDEX_ORDER_ID]).hexdigest() + "\"><b>this link</b></a>"
		body_of_email += " to check your delivery address but also provide us with your <b>phone number</b> (required for delivery).<br><br>"
		body_of_email += "As a reminder, we registered this perk from you: <u>" + order_item[INDEX_ORDER_SUMMARY] + "</u>.<br>"
		body_of_email += "If you want to add anything to your order (another Mooltipass, smartcard, etc...), please <b>reply to this email</b> to let us know.<br><br>"
		body_of_email += "Thanks again for your support,<br>"
		body_of_email += "The Mooltipass Development Team<br>"
		headers = "\r\n".join(["from: " + EMAIL_SENDER,
				   "subject: " + email_subject,
				   "to: " + email_recipient,
				   "mime-version: 1.0",
				   "content-type: text/html"])

		# body_of_email can be plaintext or html!
		content = headers + "\r\n\r\n" + body_of_email
		try:
			if False:
				session.sendmail(EMAIL_SENDER, email_recipient, content)
			log_f.write("Email sent to " + email_recipient + " for order #" + order_item[INDEX_ORDER_ID] + " and items " + order_item[INDEX_ORDER_SUMMARY] + "\n")
			print "Email sent to", order_item[INDEX_EMAIL_ADDR]
		except Exception as e:
			log_f.write("Couldn't send email to " + email_recipient + "\n")
			print "Error: unable to send email to", email_recipient
			print e
			raw_input("Press enter to acknowledge")
	else:
		print "Error with order #" + order_item[INDEX_ORDER_ID]
		print "Printout: " + printout_perk_text
		print "From file: " + order_item[INDEX_ORDER_SUMMARY]
		raw_input("Press enter to acknowledge")

def unicode_csv_reader(utf8_data, dialect=csv.excel_tab, **kwargs):
	csv_reader = csv.reader(utf8_data, dialect=dialect, **kwargs)
	for row in csv_reader:
		yield [unidecode(unicode(cell, 'utf-8')) for cell in row]

if __name__ == '__main__':
	# Main function
	print ""
	print "Mooltipass Order Summary Final Check"
	print ""
	
	# Write log file
	log_f = open(time.strftime("%Y-%m-%d-%H-%M-%S-log.txt"), 'w')
	
	# Login to mooltipass gunmail account
	if True:
		session = smtplib.SMTP('smtp.mailgun.org', 587)
		session.ehlo()
		session.starttls()
		session.login(MAILGUN_LOGIN, MAILGUN_PASSWORD)
	else:
		session = smtplib.SMTP('smtp.gmail.com', 587)
		session.ehlo()
		session.starttls()
		session.login(GMAIL_LOGIN, GMAIL_PASSWORD)	
		
	# Open our orders list
	order_list = list();
	order_email_list = list();
	reader = unicode_csv_reader(open('6-export_for_script.txt'))
	for order_item in reader:
		order_item[INDEX_EMAIL_ADDR] = order_item[INDEX_EMAIL_ADDR].strip()
		order_list.append(order_item)
		order_email_list.append(order_item[INDEX_EMAIL_ADDR])
		# check perk text
		printout_perk_text = ""
		if int(order_item[INDEX_ABS]) > 0:
			printout_perk_text += order_item[INDEX_ABS] + "x ABS Mooltipass, "
		if int(order_item[INDEX_AL]) > 0:
			printout_perk_text += order_item[INDEX_AL] + "x Aluminum Mooltipass, "
		if int(order_item[INDEX_CARD]) > 0:
			printout_perk_text += order_item[INDEX_CARD] + " smartcards, "
		if int(order_item[INDEX_CUSTOM_CARD]) > 0:
			printout_perk_text += order_item[INDEX_CUSTOM_CARD] + " custom cards, "
		if int(order_item[INDEX_COMMMORATIVE]) > 0:
			printout_perk_text += order_item[INDEX_COMMMORATIVE] + " commemorative cards, "
		# remove last 2 chars
		printout_perk_text = printout_perk_text[:-2]		
		# Remove bad chars
		if order_item[INDEX_ADDR] == "-" or order_item[INDEX_ADDR] == "/" or order_item[INDEX_ADDR] == "NA":
			order_item[INDEX_ADDR] = ""
		if order_item[INDEX_ADDR2] == "-" or order_item[INDEX_ADDR2] == "/" or order_item[INDEX_ADDR2] == "NA":
			order_item[INDEX_ADDR2] = ""		
		# Check that the text correctly describes the ordering fields
		if order_item[INDEX_ORDER_SUMMARY] != printout_perk_text:
			print "Error with order #" + order_item[INDEX_ORDER_ID]
			print "Printout: " + printout_perk_text
			print "From file: " + order_item[INDEX_ORDER_SUMMARY]
			raw_input("Press enter to acknowledge")
			
	# check for orders from the same guys
	# this works because the first duplicate element will contain the complete aggregation... our final script takes the first index!
	treated_duplicate_emails = list()
	for order_item in order_list:
		duplicates = [i for i,x in enumerate(order_list) if str(x[INDEX_EMAIL_ADDR]).lower() == str(order_item[INDEX_EMAIL_ADDR]).lower()]
		if len(duplicates) > 1 and order_item[INDEX_EMAIL_ADDR] not in treated_duplicate_emails:
			print "Found duplicate item for", order_item[INDEX_EMAIL_ADDR]
			treated_duplicate_emails.append(order_item[INDEX_EMAIL_ADDR])
			# check if the email address cases is the same
			for i in range(len(duplicates) - 1):
				if order_list[duplicates[i + 1]][INDEX_EMAIL_ADDR] != order_list[duplicates[0]][INDEX_EMAIL_ADDR]:
					print "Email address case problem!"
					raw_input("Press enter to acknowledge")
			# check if the address is the same
			for i in range(len(duplicates) - 1):
				if order_list[duplicates[i + 1]][INDEX_ADDR] != order_list[duplicates[0]][INDEX_ADDR] or order_list[duplicates[i + 1]][INDEX_ADDR2] != order_list[duplicates[0]][INDEX_ADDR2]:
					print "Different addresses:"
					print order_list[duplicates[i + 1]][INDEX_ADDR], "&", order_list[duplicates[0]][INDEX_ADDR]
					print order_list[duplicates[i + 1]][INDEX_ADDR2], "&", order_list[duplicates[0]][INDEX_ADDR2]
					raw_input("Press enter to acknowledge")
				else:
					# add the orders together
					order_list[duplicates[0]][INDEX_ORDER_ID] = order_list[duplicates[0]][INDEX_ORDER_ID] + " " + order_list[duplicates[i + 1]][INDEX_ORDER_ID]
					print order_list[duplicates[0]][INDEX_ORDER_ID] 
					order_list[duplicates[0]][INDEX_ABS] = int(order_list[duplicates[0]][INDEX_ABS]) + int(order_list[duplicates[i + 1]][INDEX_ABS])
					order_list[duplicates[0]][INDEX_AL] = int(order_list[duplicates[0]][INDEX_AL]) + int(order_list[duplicates[i + 1]][INDEX_AL])
					order_list[duplicates[0]][INDEX_CARD] = int(order_list[duplicates[0]][INDEX_CARD]) + int(order_list[duplicates[i + 1]][INDEX_CARD])
					order_list[duplicates[0]][INDEX_CUSTOM_CARD] = int(order_list[duplicates[0]][INDEX_CUSTOM_CARD]) + int(order_list[duplicates[i + 1]][INDEX_CUSTOM_CARD])
					order_list[duplicates[0]][INDEX_COMMMORATIVE] = int(order_list[duplicates[0]][INDEX_COMMMORATIVE]) + int(order_list[duplicates[i + 1]][INDEX_COMMMORATIVE])
					# generate new order text
					printout_perk_text = ""
					if int(order_item[INDEX_ABS]) > 0:
						printout_perk_text += repr(order_list[duplicates[0]][INDEX_ABS]) + "x ABS Mooltipass, "
					if int(order_item[INDEX_AL]) > 0:
						printout_perk_text += repr(order_list[duplicates[0]][INDEX_AL]) + "x Aluminum Mooltipass, "
					if int(order_item[INDEX_CARD]) > 0:
						printout_perk_text += repr(order_list[duplicates[0]][INDEX_CARD]) + " smartcards, "
					if int(order_item[INDEX_CUSTOM_CARD]) > 0:
						printout_perk_text += repr(order_list[duplicates[0]][INDEX_CUSTOM_CARD]) + " custom cards, "
					if int(order_item[INDEX_COMMMORATIVE]) > 0:
						printout_perk_text += repr(order_list[duplicates[0]][INDEX_COMMMORATIVE]) + " commemorative cards, "
					# remove last 2 chars
					order_list[duplicates[0]][INDEX_ORDER_SUMMARY] = printout_perk_text[:-2]
		
	# Open our google orders list
	google_order_list = list()
	google_order_email_list = list()
	google_reader = unicode_csv_reader(open('google_form.txt'))
	for google_order_item in google_reader:
		google_order_item[GOOGLE_INDEX_EMAIL_ADDR] = google_order_item[GOOGLE_INDEX_EMAIL_ADDR].strip()
		# Check if we don't already know this user
		if google_order_item[GOOGLE_INDEX_EMAIL_ADDR] in google_order_email_list:
			# If the user already entered, remove the previous item as we only keep the newer address
			print "Another entry for", google_order_item[GOOGLE_INDEX_EMAIL_ADDR]
			position = google_order_email_list.index(google_order_item[GOOGLE_INDEX_EMAIL_ADDR])
			google_order_list.pop(position)
			google_order_email_list.pop(position)
		# append email & order item
		google_order_list.append(google_order_item)
		google_order_email_list.append(google_order_item[GOOGLE_INDEX_EMAIL_ADDR])
		# Check hash
		if hashlib.sha512(SECRET_SALT+google_order_item[GOOGLE_INDEX_EMAIL_ADDR]+google_order_item[GOOGLE_INDEX_ORDER_ID]).hexdigest() != google_order_item[GOOGLE_INDEX_PREFILLED]:
			print "Wrong hash for email", google_order_item[GOOGLE_INDEX_EMAIL_ADDR]
			raw_input("confirm")
			
	# Open our holders orders list
	holders_emails = list()
	holders_quantity = list()
	holders_order_list = list()
	holder_reader = unicode_csv_reader(open('holders_export.txt'))
	for holders_order_item in holder_reader:
		holders_order_list.append(holders_order_item)
	for holders_order_item in holders_order_list:
		# count number of duplicates to know number of purchased holders
		duplicates = [i for i,x in enumerate(holders_order_list) if str(x[0]).lower() == str(holders_order_item[0]).lower()]
		if len(duplicates) > 0 and holders_order_item[0] not in holders_emails:
			#print "Added", len(duplicates), "holders to", holders_order_item[0]
			holders_emails.append(holders_order_item[0])
			holders_quantity.append(len(duplicates))
			
	# loop through our generated array to generate our final export file
	total_abs = 0
	total_al = 0
	total_cards = 0
	total_abs_holders = 0
	total_al_holders = 0
	csvexport = csv.writer(open("final_export.txt", "wb"), quoting=csv.QUOTE_NONNUMERIC)
	csvexport.writerow(["order id", "perk text", "remarks", "email", "name", "address 1", "address 2", "city", "zip code", "state", "country", "phone", "tracking number", "mooltipass IDs"])
	for google_order_item in google_order_list:
		# find order summary
		if google_order_item[GOOGLE_INDEX_EMAIL_ADDR] in order_email_list:
			# get user position in our order list
			position = order_email_list.index(google_order_item[GOOGLE_INDEX_EMAIL_ADDR])
			
			# check if the user has holders
			if google_order_item[GOOGLE_INDEX_EMAIL_ADDR] in holders_emails:
				holder_item_position = holders_emails.index(google_order_item[GOOGLE_INDEX_EMAIL_ADDR])
				#check if the number of holders is the number of mooltipass and if not that there are different types of mooltipass (that's a problem!)
				if holders_quantity[holder_item_position] != int(order_list[position][INDEX_ABS]) + int(order_list[position][INDEX_AL]):
					if int(order_list[position][INDEX_ABS]) != 0 and int(order_list[position][INDEX_AL]) != 0:
						print "User", google_order_item[GOOGLE_INDEX_EMAIL_ADDR], "only took", holders_quantity[holder_item_position], "holders for his", int(order_list[position][INDEX_ABS]), "ABS MP and", int(order_list[position][INDEX_AL]), "AL MP"
						# too lazy to deal with that.... give him all holders he wants						
						if int(order_list[position][INDEX_ABS]) != 0:
							order_list[position][INDEX_ORDER_SUMMARY] = order_list[position][INDEX_ORDER_SUMMARY] + ", " + repr(int(order_list[position][INDEX_ABS])) + "x ABS holders"
							total_abs_holders = total_abs_holders + int(order_list[position][INDEX_ABS])
						if int(order_list[position][INDEX_AL]) != 0:
							order_list[position][INDEX_ORDER_SUMMARY] = order_list[position][INDEX_ORDER_SUMMARY] + ", " + repr(int(order_list[position][INDEX_AL])) + "x Aluminum holders"		
							total_al_holders = total_al_holders + int(order_list[position][INDEX_AL])
					elif int(order_list[position][INDEX_ABS]) != 0:
						order_list[position][INDEX_ORDER_SUMMARY] = order_list[position][INDEX_ORDER_SUMMARY] + ", " + repr(holders_quantity[holder_item_position]) + "x ABS holders"
						total_abs_holders = total_abs_holders + holders_quantity[holder_item_position]
					elif int(order_list[position][INDEX_AL]) != 0:
						order_list[position][INDEX_ORDER_SUMMARY] = order_list[position][INDEX_ORDER_SUMMARY] + ", " + repr(holders_quantity[holder_item_position]) + "x Aluminum holders"
						total_al_holders = total_al_holders + holders_quantity[holder_item_position]
				else:
					# same number of holders as mooltipass
					if int(order_list[position][INDEX_ABS]) != 0:
						order_list[position][INDEX_ORDER_SUMMARY] = order_list[position][INDEX_ORDER_SUMMARY] + ", " + repr(int(order_list[position][INDEX_ABS])) + "x ABS holders"
						total_abs_holders = total_abs_holders + int(order_list[position][INDEX_ABS])
					if int(order_list[position][INDEX_AL]) != 0:
						order_list[position][INDEX_ORDER_SUMMARY] = order_list[position][INDEX_ORDER_SUMMARY] + ", " + repr(int(order_list[position][INDEX_AL])) + "x Aluminum holders"		
						total_al_holders = total_al_holders + int(order_list[position][INDEX_AL])
				
			csvexport.writerow([order_list[position][INDEX_ORDER_ID], order_list[position][INDEX_ORDER_SUMMARY], "", google_order_item[GOOGLE_INDEX_EMAIL_ADDR], google_order_item[GOOGLE_INDEX_NAME], google_order_item[GOOGLE_INDEX_ADDR], google_order_item[GOOGLE_INDEX_ADDR2], google_order_item[GOOGLE_INDEX_CITY], google_order_item[GOOGLE_INDEX_ZIP], google_order_item[GOOGLE_INDEX_COUNTY], google_order_item[GOOGLE_INDEX_COUNTRY], google_order_item[GOOGLE_INDEX_PHONE], "please complete", "please complete"])	
			total_abs = total_abs + int(order_list[position][INDEX_ABS])
			total_al = total_al + int(order_list[position][INDEX_AL])
			total_cards = total_cards + int(order_list[position][INDEX_CARD])
			
			#send email
			email_recipient = google_order_item[GOOGLE_INDEX_EMAIL_ADDR]
			email_subject = "[Mooltipass Campaign] Order #" + order_list[position][INDEX_ORDER_ID] + " - Your Order Summary"
			body_of_email = "Dear " + google_order_item[GOOGLE_INDEX_NAME] + ",<br><br>"
			body_of_email += "The Mooltipass team would like to thank you again for backing its campaign and making the Mooltipass a reality.<br>"
			body_of_email += "The mass production should be over in around two or three weeks and we are sending you this email to make sure that we correctly registered your order and contact details.<br>"
			body_of_email += "We registered this perk from you: <u>" + order_list[position][INDEX_ORDER_SUMMARY] + "</u>.<br>"
			body_of_email += "Your delivery address is: " + google_order_item[GOOGLE_INDEX_ADDR] + " " + google_order_item[GOOGLE_INDEX_ADDR2] + ", " + google_order_item[GOOGLE_INDEX_CITY] + " " + google_order_item[GOOGLE_INDEX_ZIP] + ", " + google_order_item[GOOGLE_INDEX_COUNTRY] + ".<br><br>"
			body_of_email += "If any of the above informations is wrong or if you want to add anything to your order (another Mooltipass, smartcard, etc...), please <b>reply to this email</b> to let us know.<br>"
			body_of_email += "Thanks again for your support,<br>"
			body_of_email += "The Mooltipass Development Team<br>"
			headers = "\r\n".join(["from: " + EMAIL_SENDER,
					   "subject: " + email_subject,
					   "to: " + email_recipient,
					   "mime-version: 1.0",
					   "content-type: text/html"])

			# body_of_email can be plaintext or html!
			content = headers + "\r\n\r\n" + body_of_email
			try:
				if False:
					session.sendmail(EMAIL_SENDER, email_recipient, content)
				log_f.write("Email sent to " + email_recipient + " for order #" + order_list[position][INDEX_ORDER_ID] + " and items " + order_list[position][INDEX_ORDER_SUMMARY] + "\n")
				print "Email sent to", google_order_item[GOOGLE_INDEX_EMAIL_ADDR]
				#time.sleep(0.1)
			except Exception as e:
				log_f.write("Couldn't send email to " + email_recipient + "\n")
				print "Error: unable to send email to", email_recipient
				print e
				raw_input("Press enter to acknowledge")
		else:
			print "Couldn't find email in our order list:", google_order_item[GOOGLE_INDEX_EMAIL_ADDR]
			raw_input("Press enter to acknowledge")
			
	print "Total cards: ", total_cards
	print "Total abs: ", total_abs
	print "Total al: ", total_al
	print "Total abs holders: ", total_abs_holders
	print "Total al holders: ", total_al_holders

	# Close log fine
	log_f.close()