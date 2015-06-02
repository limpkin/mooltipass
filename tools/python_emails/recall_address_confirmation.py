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
		email_subject = "[Mooltipass Campaign] [/!\ACTION REQUIRED/!\] Order #" + order_item[INDEX_ORDER_ID] + " - Delivery Address and Phone Number"
		body_of_email = "Dear " + order_item[INDEX_NAME] + ",<br><br>"
		body_of_email += "The Mooltipass team would like to thank you again for backing its campaign and making the Mooltipass a reality.<br>"
		body_of_email += "The mass production is about to start and we are hoping to ship the devices by the <b>middle of June</b>.<br>"
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
	print "Mooltipass Order Summary Email Generator"
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
		
	# Parse google export, store the emails
	google_email_list = list();
	google_reader = unicode_csv_reader(open('google_form.csv'))
	for google_order_item in google_reader:
		google_email_list.append(google_order_item[2].strip())
	
	# Check if we just want to send a particular email
	if len(sys.argv) > 1:
		reader = unicode_csv_reader(open('4-indiegogo_treated_order_export.txt'))
		for order_item in reader:
			# If we find the email address, send the email
			if order_item[INDEX_EMAIL_ADDR] == sys.argv[1]:
				sendOrderSummaryEmail(session, log_f, order_item)
	else:
		# Open our orders list
		reader = unicode_csv_reader(open('4-indiegogo_treated_order_export.txt'))
		for order_item in reader:
			# Send our email if we don't find it in the google export
			if not order_item[INDEX_EMAIL_ADDR].strip() in google_email_list:
				sendOrderSummaryEmail(session, log_f, order_item)
				time.sleep(0.5)

	# Close log fine
	log_f.close()