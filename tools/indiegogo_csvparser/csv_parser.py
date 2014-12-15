from unidecode import unidecode
import hashlib
import smtplib
import math
import time
import csv
import re

# todo: SUPPRIMER dans liste IGG
# - 8708609 (refund of 140)
# - 9083031 (refund of 25)

#indexes for orders list
INDEX_EMAIL_ADDR	= 0
INDEX_NAME			= 1
INDEX_ADDR			= 2
INDEX_ADDR2			= 3
INDEX_CITY			= 4
INDEX_ZIP			= 5
INDEX_COUNTY		= 6
INDEX_COUNTRY		= 7
INDEX_ABS			= 8
INDEX_AL			= 9
INDEX_CARD			= 10
INDEX_CUSTOM_CARD	= 11
INDEX_COMMMORATIVE	= 12
INDEX_CONT_AMOUNT	= 13
INDEX_GROSS			= 14
INDEX_NET			= 15
INDEX_TRANS_IDS		= 16

#indexes for paypal list
INDEX_PAYPAL_ORDERID	= 0
INDEX_PAYPAL_GROSS		= 1
INDEX_PAYPAL_NET		= 2

THEMOOLTIPASS_PASSWORD = "dsqdqsd"
SECRET_SALT = "dqdqsdqs"

def findEmailIn7cardsList(list, email):
	i = 0
	s = len(list)
	while i < s:
		if list[i][0] == email:
			return i;
		i += 1
	return -1
	
def findEmailInOrdersList(list, email, name, city):
	i = 0
	s = len(list)
	while i < s:
		if list[i][INDEX_EMAIL_ADDR] == email and list[i][INDEX_NAME] == name and list[i][INDEX_CITY] == city:
			return i;
		i += 1
	return -1

def findTransactionIDInList(list, transactionid):
	i = 0
	s = len(list)
	while i < s:
		if list[i][INDEX_PAYPAL_ORDERID] == transactionid:
			return float(list[i][INDEX_PAYPAL_GROSS]), float(list[i][INDEX_PAYPAL_NET]);
		i += 1
	return -1, -1

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
	debug_number_normal_cards = 0
	debug_number_custom_card = 0
	debug_number_com_card = 0
	debug_number_abs = 0
	debug_number_al = 0
	number_normal_cards = 0
	number_custom_card = 0
	number_com_card = 0
	number_abs = 0
	number_al = 0
	seed_money = 0
	seed_money_net = 0
	campaign_money = 0
	received_money = 0
	unfindable_money = 0
	raw_indiegogo_raised = 0
	
	# Email defs	
	perk_email_sending = False
	
	# Prints defs
	raw_printout = False
	perk_printout = True
	addr_printout = True
	anomaly_printout = False
	paypal_raw_printout = False
	crosschecking_prinout = True
	
	# Paypal crosschecking
	paypal_crosschecking = True
	
	# 7 more cards list
	morecards_list = list()
	
	# final order list
	orders_list = list()
	
	# paypal orders
	paypal_list = list()
	
	# login mooltipass gmail account
	if perk_email_sending:
		session = smtplib.SMTP('smtp.gmail.com', 587)
		session.ehlo()
		session.starttls()
		session.login("themooltipass@gmail.com", THEMOOLTIPASS_PASSWORD)
		
	# Csv reader, paypal export, store transaction ids together with gross and net
	reader = unicode_csv_reader(open('paypal.txt'))
	for paypal_date, paypal_time, paypal_timez, paypal_name, paypal_type, paypal_status, paypal_currency, paypal_gross, paypal_fee, paypal_net, paypal_email, paypal_account, paypal_transactionID, paypal_counterpart, paypal_addr, paypal_title, paypal_itemid, paypal_shipping, paypal_insur, paypal_saletax, paypal_option1n, paypal_option1v, paypal_option2n, paypal_option2v, paypal_auctionsite, paypal_buyerid, paypal_itemurl, paypal_closingdate, paypal_escrowid, paypal_invoiceid, paypal_reference, paypal_invoicenumber, paypal_customnum, paypal_receiptID, paypal_balance, paypal_address_line1, paypal_address_line2, paypal_city, paypal_state, paypal_zip, paypal_country, paypal_phone, paypal_blank in reader:
		if paypal_name == "Indiegogo.com" and paypal_type == "Payment Received" and paypal_status == "Completed":
			# Completed transaction from paypal
			if paypal_raw_printout:
				print "Completed transaction number #" + paypal_invoicenumber + " gross: " + paypal_gross + " net: " + paypal_net
			paypal_list.append([paypal_invoicenumber, paypal_gross, paypal_net])
	
	# Csv reader, 7 more cards detection loop and gross total
	reader = unicode_csv_reader(open('mooltipass.txt'))
	for contributionID, giftID, shippingstatus, backingdate, payingmean, displaystatus, name, email, amount, chosenperk, backername, backeraddress, backeraddress2, backercity, backercounty, backercitycode, backercountry in reader:
		raw_indiegogo_raised += int(re.sub('[$]', '', amount))
		if chosenperk == "7 more cards":
			morecards_list.append([email, giftID, int(re.sub('[$]', '', amount))])

	# Csv reader, traverse indiegogo's orders list
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
		
		#  Do some processing on perks involving a delivery
		if chosenperk != "" and chosenperk != "Thank you!" and chosenperk != "7 more cards":
			# chosen perk
			user_transaction_ids = ""
			user_commemorative_card = 0
			user_custom_card = 0
			user_cards = 0
			user_abs = 0
			user_al = 0
			if chosenperk == "ABS Mooltipass":
				user_cards = int(math.floor((float(numamount)-100)/1.5))+2
				user_abs = 1
			elif chosenperk == "Aluminum Mooltipass":
				user_cards = int(math.floor((float(numamount)-140)/1.5))+2
				user_al = 1
			elif chosenperk == "EARLY ADOPTERS #1":
				user_cards = int(math.floor((float(numamount)-80)/1.5))+2
				user_abs = 1
			elif chosenperk == "EARLY ADOPTERS #2":
				user_cards = int(math.floor((float(numamount)-90)/1.5))+2
				user_abs = 1
			elif chosenperk == "Two ABS Mooltipass sets":
				user_cards = int(math.floor((float(numamount)-190)/1.5))+5
				user_abs = 2
			elif chosenperk == "Two Aluminum Mooltipass sets":
				user_cards = int(math.floor((float(numamount)-270)/1.5))+5
				user_al = 2
			elif chosenperk == "ABS  + Aluminum Mooltipass":
				user_cards = int(math.floor((float(numamount)-230)/1.5))+5
				user_abs = 1
				user_al = 1
			elif chosenperk == "Your very own smartcard - Al":
				user_custom_card = 1
				user_al = 1
			elif chosenperk == "Your very own smartcard - ABS":
				user_custom_card = 1
				user_abs = 1
			elif chosenperk == "Commemorative smartcard":
				user_commemorative_card = 1
			elif chosenperk == "custom7only":
				user_cards += 7
			else:
				print "-------------------------------------------------"
				print "Chosen perk parsing error:", chosenperk
				raw_input("Press enter to acknowledge")
				
			# Try to find the reference in our paypal export
			user_transaction_ids = giftID
			user_numamount = float(numamount)
			user_gross, user_net = findTransactionIDInList(paypal_list, giftID)
			if user_gross == -1 and paypal_crosschecking:
				unfindable_money += user_numamount
				print "-------------------------------------------------"
				print "Transaction missing!"
				print backername, "email:", email, "transaction id:", giftID, "("+numamount+")"
				raw_input("Press enter to acknowledge")
			else:
				# we found the transaction, add it to our orders' list
				# calculate fees
				user_fees = round((((float(numamount) - user_net) / float(numamount))*100) , 2)
				if crosschecking_prinout:
					print "Transaction found: " + numamount + " paypal received: " + repr(user_gross) + " net: " + repr(user_net) + " fees: " + repr(user_fees) + "%"
				# Add the fields in our orders list, check if we already added the user with his address
				user_index_in_order_list = findEmailInOrdersList(orders_list, email, backername, backercity)
				if user_index_in_order_list == -1:
					# we didn't find the user in our orders list, add it
					# first, see if the user opted for the 7 more cards perk
					for i in range(5):
						temp_index_id =  findEmailIn7cardsList(morecards_list, email)
						if temp_index_id != -1:
							user_transaction_ids += " & " + morecards_list[temp_index_id][1]
							if raw_printout:
								print "user took the 7 more cards perk"
								#time.sleep(1)
							# check paypal transaction for 7 more cards perk
							if paypal_crosschecking:
								morecards_user_gross, morecards_user_net = findTransactionIDInList(paypal_list, morecards_list[temp_index_id][1])
								# we didn't find the transaction
								if morecards_user_gross == -1:
									unfindable_money += morecards_list[temp_index_id][2]
									print "-------------------------------------------------"
									print "Transaction missing for 7 more cards perk!"
									print backername, "email:", email, "transaction id:", morecards_list[temp_index_id][1]
									raw_input("Press enter to acknowledge")
								else:
									user_cards += 7
									user_numamount += morecards_list[temp_index_id][2]
									user_gross += morecards_user_gross
									user_net += morecards_user_net
									if crosschecking_prinout:
										print i, "- 7 cards transaction found, paypal received: " + repr(morecards_user_gross) + " net: " + repr(morecards_user_net)
										#time.sleep(0.1)
							else:
								user_cards += 7							
							# finally, pop element
							morecards_list.pop(temp_index_id)
					orders_list.append([email, backername.strip(), backeraddress.strip(), backeraddress2.strip(), backercity, backercitycode, backercounty, backercountry, user_abs, user_al, user_cards, user_custom_card, user_commemorative_card, user_numamount, user_gross, user_net, user_transaction_ids])
				else:
					if raw_printout:
						print "user already found, adding to old order"
						#time.sleep(2)
					orders_list[user_index_in_order_list][INDEX_ABS] += user_abs
					orders_list[user_index_in_order_list][INDEX_AL] += user_al
					orders_list[user_index_in_order_list][INDEX_CARD] += user_cards
					orders_list[user_index_in_order_list][INDEX_CUSTOM_CARD] += user_custom_card
					orders_list[user_index_in_order_list][INDEX_COMMMORATIVE] += user_commemorative_card
					orders_list[user_index_in_order_list][INDEX_CONT_AMOUNT] += float(numamount)
					orders_list[user_index_in_order_list][INDEX_GROSS] += user_gross
					orders_list[user_index_in_order_list][INDEX_NET] += user_net
					orders_list[user_index_in_order_list][INDEX_TRANS_IDS] += " & " + user_transaction_ids
				
				# add the numbers
				number_com_card += user_commemorative_card
				number_custom_card += user_custom_card
				number_normal_cards += user_cards
				number_abs += user_abs
				number_al += user_al
				campaign_money += user_numamount
				received_money += user_net
			
		else:
			# Thank you, no chosen perk...
			if int(numamount) > 10 and anomaly_printout:
				print "-------------------------------------------------"
				print "Atypical amount:", amount, "from", email, "-", chosenperk
				raw_input("Press enter to acknowledge")
			# Seed money
			if chosenperk != "7 more cards":
				if paypal_crosschecking:
					seed_gross, seed_net = findTransactionIDInList(paypal_list, giftID)
					if seed_gross == -1:
						unfindable_money += int(numamount)
						print "-------------------------------------------------"
						print "Couldn't find seed money transaction!"
						print backername, "email:", email, "transaction id:", giftID, "("+numamount+")"
						raw_input("Press enter to acknowledge")
					else:
						seed_money += float(numamount)
						seed_money_net += seed_net
				else:
					seed_money += int(numamount)
		#time.sleep(5)
	
	# Users that only took a 7 more cards perk.... sigh
	# In that case, change csv file to "custom7only" for perk
	i = 0
	for remaining_7cards_item in morecards_list:
		print "7 more cards perk only:", remaining_7cards_item[0], remaining_7cards_item[1], remaining_7cards_item[2]
		campaign_money += remaining_7cards_item[2]
		i += 1
		
	if i > 0:
		print "-------------------------------------------------"
		raw_input("Press enter to acknowledge")
	
	# Traverse our orders list, export csv file
	order_id = 0
	csvexport = csv.writer(open("order_export.txt", "wb"), quoting=csv.QUOTE_NONNUMERIC)
	csvexport.writerow(["order id", "perk text", "email", "name", "address 1", "address 2", "city", "zip code", "county", "country", "#ABS", "#Al", "#cards", "#custom cards", "#commemorative", "contribution", "net", "trans IDs"])
	for order_item in orders_list:		
		# check totals
		debug_number_custom_card += order_item[INDEX_CUSTOM_CARD]
		debug_number_com_card += order_item[INDEX_COMMMORATIVE]
		debug_number_normal_cards += order_item[INDEX_CARD]
		debug_number_abs += order_item[INDEX_ABS]
		debug_number_al += order_item[INDEX_AL]
		
		# backer's perk
		printout_perk_text = ""
		if order_item[INDEX_ABS] > 0:
			printout_perk_text += repr(order_item[INDEX_ABS]) + "x ABS Mooltipass, "
		if order_item[INDEX_AL] > 0:
			printout_perk_text += repr(order_item[INDEX_AL]) + "x Aluminum Mooltipass, "
		if order_item[INDEX_CARD] > 0:
			printout_perk_text += repr(order_item[INDEX_CARD]) + " smartcards, "
		if order_item[INDEX_CUSTOM_CARD] > 0:
			printout_perk_text += repr(order_item[INDEX_CUSTOM_CARD]) + " custom cards, "
		if order_item[INDEX_COMMMORATIVE] > 0:
			printout_perk_text += repr(order_item[INDEX_COMMMORATIVE]) + " commemorative cards, "
		# remove last 2 chars
		printout_perk_text = printout_perk_text[:-2]	
		
		# export
		csvexport.writerow([repr(order_id), printout_perk_text, order_item[INDEX_EMAIL_ADDR], order_item[INDEX_NAME], order_item[INDEX_ADDR], order_item[INDEX_ADDR2], order_item[INDEX_CITY], order_item[INDEX_ZIP], order_item[INDEX_COUNTY], order_item[INDEX_COUNTRY], repr(order_item[INDEX_ABS]), repr(order_item[INDEX_AL]), repr(order_item[INDEX_CARD]), repr(order_item[INDEX_CUSTOM_CARD]), repr(order_item[INDEX_COMMMORATIVE]), repr(order_item[INDEX_CONT_AMOUNT]), repr(round(order_item[INDEX_NET], 2)), order_item[INDEX_TRANS_IDS]])
		order_id += 1	
		
		# chosen perk
		if perk_printout:
			print printout_perk_text
		
		# backer's address
		if addr_printout:
			temp_bool = False
			print "To:"
			print order_item[INDEX_NAME]
			# if the user just provided a street number
			if order_item[INDEX_ADDR].isdigit() or order_item[INDEX_ADDR2].isdigit():
				print order_item[INDEX_ADDR], order_item[INDEX_ADDR2]
				# address checking
				if len(order_item[INDEX_ADDR] + order_item[INDEX_ADDR2]) < 3:
					temp_bool = True
			elif order_item[INDEX_ADDR] == "-" or order_item[INDEX_NAME] == order_item[INDEX_ADDR]:
				print order_item[INDEX_ADDR2]
				# address checking
				if len(order_item[INDEX_ADDR2]) < 3:
					temp_bool = True
			else:
				print order_item[INDEX_ADDR]
				# address checking
				if len(order_item[INDEX_ADDR]) < 3:
					temp_bool = True
				if order_item[INDEX_ADDR2] != "" and order_item[INDEX_ADDR] != order_item[INDEX_ADDR2] and order_item[INDEX_ADDR2] != " ." and order_item[INDEX_ADDR2] != "-" and order_item[INDEX_ADDR2] != "/" and order_item[INDEX_ADDR2] != "_" and order_item[INDEX_ADDR2] != "." and order_item[INDEX_ADDR2] != "X": 
					print order_item[INDEX_ADDR2]
					# address checking
					if len(order_item[INDEX_ADDR2]) < 3:
						temp_bool = True
			print order_item[INDEX_CITY], order_item[INDEX_COUNTY], order_item[INDEX_ZIP]
			print order_item[INDEX_COUNTRY]
			#print order_item[INDEX_TRANS_IDS]
			#print order_item[INDEX_GROSS]
			#print order_item[INDEX_NET]
			#time.sleep(0.1)
			print ""
			if temp_bool and False:
				time.sleep(2)
			
		# email sending
		if perk_email_sending:
			email_recipient = order_item[INDEX_EMAIL_ADDR]
			email_recipient = "mdqsdqdqn@gmail.com"
			email_subject = "[Mooltipass Campaign] Your Selected Perk - Do You Want To Make Any Change?"
			body_of_email = "Dear " + order_item[INDEX_NAME] + ",<br><br><br>"
			body_of_email += "The Mooltipass team would like to <b>thank you</b> for backing its campaign and making the Mooltipass a reality.<br>"
			body_of_email += "We're sending you this email so you can check that we <b>correctly registered your pledge</b> and give you the opportunity to <b>make an addition to it</b>.<br><br>"
			body_of_email += "You have selected: <u>" + printout_perk_text + ".</u><br>"
			body_of_email += "For information, you sent us $" + repr(order_item[INDEX_CONT_AMOUNT]) + " and we received $" + repr(order_item[INDEX_NET]) + " after indiegogo and paypal fees<br>"
			body_of_email += "Your Indiegogo transaction ID(s): " + order_item[INDEX_TRANS_IDS] + "<br><br>"
			body_of_email += "If this isn't correct or if you want to add anything to your order (another Mooltipass, smartcard, etc...), please <b>reply to this email</b> to let us know.<br>"
			body_of_email += "If you are <b>planning to use the Mooltipass as an Arduino platform </b> or want to <b>provide some feedback on the campaign and future steps</b>, we would really appreciate if you could take a few minutes of your time to fill our end of campaign survey "
			body_of_email += "<a href=\"https:/dqdsdsqdqs="+order_item[INDEX_NAME]+"&entry.1932639819="+order_item[INDEX_EMAIL_ADDR]+"&entry.1617982862=" + hashlib.sha512(SECRET_SALT+order_item[INDEX_EMAIL_ADDR]).hexdigest() + "\">here</a>.<br>"
			body_of_email += "Thanks again for your support,<br>"
			body_of_email += "The Mooltipass development team"
			headers = "\r\n".join(["from: " + "themooltipass@gmail.com",
					   "subject: " + email_subject,
					   "to: " + email_recipient,
					   "mime-version: 1.0",
					   "content-type: text/html"])

			# body_of_email can be plaintext or html!                    
			content = headers + "\r\n\r\n" + body_of_email
			session.sendmail("themooltipass@gmail.com", email_recipient, content)
			print "Email sent to", email_recipient
			time.sleep(4)
	
	print "-------------------------------------------------"
	print "Total number of commemorative cards:", number_com_card, "- check:", debug_number_com_card
	print "Total number of normal cards:", number_normal_cards, "- check:", debug_number_normal_cards
	print "Total number of custom cards:", number_custom_card, "- check:", debug_number_custom_card
	print "Total number of ABS:", number_abs, "- check:", debug_number_abs
	print "Total number of Al:", number_al, "- check:", debug_number_al
	print "-------------------------------------------------"
	print "Indiegogo unverified total:", raw_indiegogo_raised, "dollars"
	print "Total money raised:", int(campaign_money), "dollars"
	print "Unfindable amount:", unfindable_money, "dollars"
	print "Total money received:", int(received_money), "dollars"
	print "Fees:",repr(round((((campaign_money - received_money) / campaign_money)*100), 2))+"%"
	print "Seed money:", seed_money, "dollars"
	print "Seed money (net):", seed_money_net, "dollars"
	print "-------------------------------------------------"