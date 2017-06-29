Feature: Supported Websites

@supported
Scenario: Testing stackoverflow
Given I navigate to 'https://stackoverflow.com'
When I go to StackOverFlow login page
When I log in StackOverFlow with 'mooltipass@discard.email' and '!Pass12345'
Then I should be logged in StackOverFlow
When I logout StackOverFlow
And I go to StackOverFlow login page
Then I should be logged in StackOverFlow

@supported
Scenario: Testing dropbox.com
Given I navigate to 'https://www.dropbox.com/'
When I login DropBox with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in DropBox
When I logout DropBox
Then I should be logged in DropBox

@subdomain
@supported 
Scenario: Testing farnell.com
Given I navigate to 'https://ch.farnell.com/'
When I login Farnel with 'Mooltipass' and 'Testpass123'
Then I should be logged in Farnel
When I logout Farnel
And I go to Farnel login page
Then I should be logged in Farnel

@subdomain
@supported 
Scenario: Testing ebay.com
Given I navigate to 'http://www.ebay.com'
When I login EBay with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in EBay
When I logout EBay
And I go to EBay login page
Then I should be logged in EBay

@supported
Scenario: Testing esp8266.com
Given I navigate to 'http://www.esp8266.com'
When I login esp8266 with 'mooltipass' and 'testpass123'
Then I should be logged in esp8266
When I logout esp8266
And I go to esp8266 login page
Then I should be logged in esp8266

@supported
Scenario: Testing etsy.com
Given I navigate to 'https://www.etsy.com/'
When I login Etsy with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in Etsy
When I logout Etsy
And I go to Etsy login page
Then I should be logged in Etsy

@supported
Scenario: Testing github.com
Given I navigate to 'https://github.com/'
When I login GitHub with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in GitHub
When I logout GitHub
And I go to GitHub login page
Then I should be logged in GitHub

@supported  
Scenario: Testing hackaday.io
Given I navigate to 'https://hackaday.io/'
When I login HackADay with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in HackADay
When I logout HackADay
And I go to HackADay login page
Then I should be logged in HackADay


@subdomain
@supported
Scenario: Testing forum.hardware.fr
Given I navigate to 'http://forum.hardware.fr/'
When I login HardwareFR with 'mooltipass' and '0b18d348fd'
Then I should be logged in HardwareFR
When I logout HardwareFR
And I go to HardwareFR login page
Then I should be logged in HardwareFR


@supported
Scenario: Testing ldlc.ch
Given I navigate to 'http://www.ldlc.ch/'
When I login IDLC with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in IDLC
When I logout IDLC
And I go to IDLC login page
Then I should be logged in IDLC

@supported
Scenario: Testing linkedin.com
Given I navigate to 'https://www.linkedin.com/'
When I login LinkedIn with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in LinkedIn
When I logout LinkedIn
Then I should be logged in LinkedIn

@supported
Scenario: Testing patreon.com
Given I navigate to 'https://www.patreon.com'
When I login Patreon with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in Patreon
When I logout Patreon
And I go to Patreon login page
Then I should be logged in Patreon

@supported
Scenario: Testing paypal.com
Given I navigate to 'https://www.paypal.com/at/home'
When I login PayPal with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in PayPal
When I logout PayPal
And I go to PayPal login page
Then I should be logged in PayPal

@supported
Scenario: Testing pcbway.com
Given I navigate to 'https://www.pcbway.com'
When I login PcbWay with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in PcbWay
When I logout PcbWay
And I go to PcbWay login page
Then I should be logged in PcbWay

@supported
Scenario: Testing qoqa.ch
Given I navigate to 'https://www.qoqa.ch'
When I login qoqa with 'citesting@themooltipass.com' and 'Testpass123'
Then I should be logged in qoqa
When I logout qoqa
And I go to qoqa login page
Then I should be logged in qoqa

@supported
Scenario: Testing reddit.com
Given I navigate to 'https://www.reddit.com'
When I login reddit with 'mooltipas' and 'testpass123'
Then I should be logged in reddit
When I logout reddit
Then I should be logged in reddit

@supported
Scenario: Testing tindie.com
Given I navigate to 'https://www.tindie.com'
When I login tindie with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in tindie
When I logout tindie
And I go to tindie login page
Then I should be logged in tindie

@supported
Scenario: Testing https://www.komoot.de/
Given I navigate to 'http://www.komoot.de/'
When I login komoot with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in komoot
When I logout komoot
Then I should be logged in komoot

@subdomain @supported
Scenario: Testing alibaba.com
Given I navigate to 'http://www.alibaba.com/'
When I login AliBaba with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in AliBaba
When I confirm login to Alibaba
When I logout AliBaba
And I go to AliBaba login page
Then I should be logged in AliBaba
