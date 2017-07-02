Feature: Not supported Websites

@notsupported
Scenario: Testing ldlc.ch
Given I navigate to 'http://www.ldlc.ch/'
When I login IDLC with 'citesting@themooltipass.com'
Then I should be logged in IDLC
When I logout IDLC
And I go to IDLC login page
Then I should be logged in IDLC

@notsupported
Scenario: Testing Techmania
Given I navigate to 'https://techmania.ch/'
When I log in Techmania with 'mooltipass'
Then I should be logged in Techmania
When I logout Techmania
When I go to Techmania login page
Then I should be logged in Techmania

@notsupported
Scenario: Testing Microspot
Given I navigate to 'https://www.microspot.ch/'
When I go to Microspot login page
When I login Microspot with 'Mooltipasstest@gmail.com'
Then I should be logged in Microspot
When I logout Microspot
When I go to Microspot login page
Then I should be logged in Microspot

#password is wrong from mooltipass
@notsupported 
Scenario: Testing ebookers.fr
Given I navigate to 'https://www.ebookers.fr/'
When I login EBookers with 'citesting@themooltipass.com'
Then I should be logged in EBookers
When I logout EBookers
And I go to EBookers login page
Then I should be logged in EBookers

@notsupported
Scenario: Testing evernote.com
Given I navigate to 'https://www.evernote.com/Login.action'
When I login Evernote with 'citesting@themooltipass.com'
Then I should be logged in Evernote
When I logout Evernote
And I go to Evernote login page
Then I should be logged in Evernote

@notsupported
Scenario: Testing protonmail.com
Given I navigate to 'https://www.protonmail.com'
When I login ProtonMail with 'mooltipas'
Then I should be logged in ProtonMail
When I logout ProtonMail
And I go to ProtonMail login page
Then I should be logged in ProtonMail

@notsupported
Scenario: Testing ricardo.ch
Given I navigate to 'https://www.ricardo.ch'
When I login ricardo with 'citesting@themooltipass.com'
Then I should be logged in ricardo
When I logout ricardo
And I go to ricardo login page
Then I should be logged in ricardo

@notsupported
Scenario: Testing tripadvisor.com
Given I navigate to 'https://www.tripadvisor.com'
When I login tripadvisor with 'citesting@themooltipass.com'
Then I should be logged in tripadvisor
When I logout tripadvisor
And I go to tripadvisor login page
Then I should be logged in tripadvisor

@notsupported
Scenario: Testing pc-ostschweiz.ch
Given I navigate to 'https://www.pc-ostschweiz.ch'
When I login pc-ostschweiz with 'mooltipass'
Then I should be logged in pc-ostschweiz
When I logout pc-ostschweiz
Then I should be logged in pc-ostschweiz

@notsupported
Scenario: Testing https://www.kenwoodworld.com/uk/account/sign-in#
Given I navigate to 'https://www.kenwoodworld.com/uk/account/sign-in'
When I login kenwoodworld with 'citesting@themooltipass.com'
Then I should be logged in kenwoodworld
When I logout kenwoodworld
And I go to kenwoodworld login page
Then I should be logged in kenwoodworld

@subdomain
@notsupported
Scenario: Testing https://workbench.cisecurity.org/
Given I navigate to 'https://workbench.cisecurity.org/'
When I login cisecurity with 'citesting@themooltipass.com'
Then I should be logged in cisecurity
When I logout cisecurity
And I go to cisecurity login page
Then I should be logged in cisecurity

@notsupported
Scenario: Testing https://www.trillian.im/web/4.0/
Given I navigate to 'https://www.trillian.im/web/4.0/'
When I login trillian with 'mooltipass'
Then I should be logged in trillian
When I logout trillian
Then I should be logged in trillian

@notsupported
Scenario: Testing http://minfin.com.ua
Given I navigate to 'http://minfin.com.ua'
When I login minfin with 'citesting@themooltipass.com'
Then I should be logged in minfin
When I logout minfin
And I go to minfin login page
Then I should be logged in minfin

@notsupported
Scenario: Testing https://www.microchipdirect.com/
Given I navigate to 'https://www.microchipdirect.com/'
When I login microchipdirect with 'citesting@themooltipass.com'
Then I should be logged in microchipdirect
When I logout microchipdirect
And I go to microchipdirect login page
Then I should be logged in microchipdirect

@notsupported 
Scenario: Testing http://www.metacritic.com/
Given I navigate to 'http://www.metacritic.com/'
When I login metacritic with 'citesting@themooltipass.com'
Then I should be logged in metacritic
When I logout metacritic
And I go to metacritic login page
Then I should be logged in metacritic

@notsupported
Scenario: Testing anibis.ch
Given I navigate to 'https://anibis.ch'
When I login Anibis with 'citesting@themooltipass.com'
Then I should be logged in Anibis
When I logout Anibis
And I go to Anibis login page
Then I should be logged in Anibis

@notsupported 
Scenario: Testing airbnb
Given I navigate to 'https://www.airbnb.com/'
When I login airBnB with 'citesting@themooltipass.com'
Then I should be logged in airBnB
When I logout airBnB
And I go to airBnB login page
Then I should be logged in airBnB