Feature: Websites causing problems with Travis and Sauce Labs

@supported
Scenario: Testing icloud
Given I navigate to 'https://www.icloud.com'
When I login icloud with 'citesting@themooltipass.com'
Then I should be logged in icloud
When I logout icloud
When I go to login page
Then I should be logged in icloud

#Not Working-> site sometimes say invalid email pass
Scenario: Testing indiegogo.com
Given I navigate to 'https://www.indiegogo.com'
When I login Indiegogo with 'citesting@themooltipass.com'
Then I should be logged in Indiegogo
When I logout Indiegogo
And I go to Indiegogo login page
Then I should be logged in Indiegogo

@supported
Scenario: Testing ldlc.ch
Given I navigate to 'http://www.ldlc.ch/'
When I login IDLC with 'citesting@themooltipass.com'
Then I should be logged in IDLC
When I logout IDLC
And I go to IDLC login page
Then I should be logged in IDLC

@supported
Scenario: Testing Techmania
Given I navigate to 'https://techmania.ch/'
When I log in Techmania with 'mooltipass'
Then I should be logged in Techmania
When I logout Techmania
When I go to Techmania login page
Then I should be logged in Techmania

@supported
Scenario: Testing tripadvisor.com
Given I navigate to 'https://www.tripadvisor.com'
When I login tripadvisor with 'citesting@themooltipass.com'
Then I should be logged in tripadvisor
When I logout tripadvisor
And I go to tripadvisor login page
Then I should be logged in tripadvisor

@supported
Scenario: Testing pc-ostschweiz.ch
Given I navigate to 'https://www.pc-ostschweiz.ch'
When I login pc-ostschweiz with 'mooltipass'
Then I should be logged in pc-ostschweiz
When I logout pc-ostschweiz
Then I should be logged in pc-ostschweiz
