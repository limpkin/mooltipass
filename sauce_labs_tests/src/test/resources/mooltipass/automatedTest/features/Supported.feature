Feature: Supported Websites

@supported 
Scenario: Testing dropbox.com
Given I navigate to 'https://www.dropbox.com/'
When I login DropBox with 'citesting@themooltipass.com'
Then I should be logged in DropBox
When I logout DropBox
Then I should be logged in DropBox

@subdomain @supported 
Scenario: Testing farnell.com
Given I navigate to 'https://ch.farnell.com/'
When I login Farnel with 'Mooltipass'
Then I should be logged in Farnel
When I logout Farnel
And I go to Farnel login page
Then I should be logged in Farnel

@subdomain @supported 
Scenario: Testing ebay.com
Given I navigate to 'http://www.ebay.com'
When I login EBay with 'citesting@themooltipass.com'
Then I should be logged in EBay
When I logout EBay
And I go to EBay login page
Then I should be logged in EBay

@supported
Scenario: Testing esp8266.com
Given I navigate to 'http://www.esp8266.com'
When I login esp8266 with 'mooltipass'
Then I should be logged in esp8266
When I logout esp8266
And I go to esp8266 login page
Then I should be logged in esp8266

@supported
Scenario: Testing etsy.com
Given I navigate to 'https://www.etsy.com/'
When I login Etsy with 'citesting@themooltipass.com'
Then I should be logged in Etsy
When I logout Etsy
And I go to Etsy login page
Then I should be logged in Etsy

@subdomain @supported
Scenario: Testing firefox.com
Given I navigate to 'https://addons.mozilla.org'
When I login firefox with 'citesting@themooltipass.com'
Then I should be logged in firefox
When I logout firefox
And I go to firefox login page
And I click the sign in button
Then I should be logged in firefox

@supported
Scenario: Testing github.com
Given I navigate to 'https://github.com/'
When I login GitHub with 'citesting@themooltipass.com'
Then I should be logged in GitHub
When I logout GitHub
And I go to GitHub login page
Then I should be logged in GitHub

@supported
Scenario: Testing gmail
Given I navigate to 'https://gmail.com/'
When I login Gmail with 'Mooltipasstest@gmail.com'
Then I should be logged in Gmail
When I logout Gmail
Then I should be logged in Gmail

@supported  
Scenario: Testing hackaday.io
Given I navigate to 'https://hackaday.io/'
When I login HackADay with 'citesting@themooltipass.com'
Then I should be logged in HackADay
When I logout HackADay
#And I go to HackADay login page
Then I should be logged in HackADay

@subdomain @supported
Scenario: Testing forum.hardware.fr
Given I navigate to 'http://forum.hardware.fr/'
When I login HardwareFR with 'mooltipass'
Then I should be logged in HardwareFR
When I logout HardwareFR
And I go to HardwareFR login page
Then I should be logged in HardwareFR

@supported
Scenario: Testing linkedin.com
Given I navigate to 'https://www.linkedin.com/'
When I login LinkedIn with 'citesting@themooltipass.com'
Then I should be logged in LinkedIn
When I logout LinkedIn
Then I should be logged in LinkedIn

@supported
Scenario: Testing pcbway.com
Given I navigate to 'https://www.pcbway.com'
When I login PcbWay with 'citesting@themooltipass.com'
Then I should be logged in PcbWay
When I logout PcbWay
And I go to PcbWay login page
Then I should be logged in PcbWay

@supported
Scenario: Testing qoqa.ch
Given I navigate to 'https://www.qoqa.ch'
When I login qoqa with 'citesting@themooltipass.com'
Then I should be logged in qoqa
When I logout qoqa
And I go to qoqa login page
Then I should be logged in qoqa

@supported
Scenario: Testing reddit.com
Given I navigate to 'https://www.reddit.com'
When I login reddit with 'mooltipas'
Then I should be logged in reddit
When I logout reddit
Then I should be logged in reddit

@supported
Scenario: Testing tindie.com
Given I navigate to 'https://www.tindie.com'
When I login tindie with 'citesting@themooltipass.com'
Then I should be logged in tindie
When I logout tindie
And I go to tindie login page
Then I should be logged in tindie

@supported
Scenario: Testing https://www.komoot.de/
Given I navigate to 'http://www.komoot.de/'
When I login komoot with 'citesting@themooltipass.com'
Then I should be logged in komoot
When I logout komoot
Then I should be logged in komoot

@subdomain @supported
Scenario: Testing alibaba.com
Given I navigate to 'http://www.alibaba.com/'
When I login AliBaba with 'citesting@themooltipass.com'
Then I should be logged in AliBaba
#When I confirm login to Alibaba
When I logout AliBaba
And I go to AliBaba login page
Then I should be logged in AliBaba

@supported
Scenario: Testing Microspot
Given I navigate to 'https://www.microspot.ch/'
When I go to Microspot login page
When I login Microspot with 'mooltipasstest@gmail.com'
Then I should be logged in Microspot
When I logout Microspot
When I go to Microspot login page
Then I should be logged in Microspot

@supported 
Scenario: Testing ebookers.fr
Given I navigate to 'https://www.ebookers.fr/'
When I login EBookers with 'citesting@themooltipass.com'
Then I should be logged in EBookers
When I logout EBookers
And I go to EBookers login page
Then I should be logged in EBookers

@supported
Scenario: Testing ricardo.ch
Given I navigate to 'https://www.ricardo.ch'
When I login ricardo with 'citesting@themooltipass.com'
Then I should be logged in ricardo
When I logout ricardo
And I go to ricardo login page
Then I should be logged in ricardo

@supported 
Scenario: Testing http://www.metacritic.com/
Given I navigate to 'http://www.metacritic.com/'
When I login metacritic with 'citesting@themooltipass.com'
Then I should be logged in metacritic
When I logout metacritic
And I go to metacritic login page
Then I should be logged in metacritic

@supported
Scenario: Testing anibis.ch
Given I navigate to 'https://anibis.ch'
When I login Anibis with 'citesting@themooltipass.com'
Then I should be logged in Anibis
When I logout Anibis
And I go to Anibis login page
Then I should be logged in Anibis

@supported
Scenario: Testing Steam
Given I navigate to 'https://store.steampowered.com/'
When I login Steam with 'citesting'
Then I should be logged in Steam
When I logout Steam
When I go to Steam login page
Then I should be logged in Steam

@supported
Scenario: Testing protonmail.com
Given I navigate to 'https://www.protonmail.com'
When I login ProtonMail with 'mooltipas'
Then I should be logged in ProtonMail
When I logout ProtonMail
And I go to ProtonMail login page
Then I should be logged in ProtonMail

