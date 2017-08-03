Feature: Supported Websites - Continued

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

@supported
Scenario: Testing ldlc.ch
Given I navigate to 'http://www.ldlc.ch/'
When I login IDLC with 'citesting@themooltipass.com'
Then I should be logged in IDLC
When I logout IDLC
And I go to IDLC login page
Then I should be logged in IDLC

@supported
Scenario: Testing evernote.com
Given I navigate to 'https://www.evernote.com/Login.action'
When I login Evernote with 'citesting@themooltipass.com'
Then I should be logged in Evernote
When I logout Evernote
And I go to Evernote login page
Then I should be logged in Evernote

@supported
Scenario: Testing pc-ostschweiz.ch
Given I navigate to 'https://www.pc-ostschweiz.ch'
When I login pc-ostschweiz with 'mooltipass'
Then I should be logged in pc-ostschweiz
When I logout pc-ostschweiz
Then I should be logged in pc-ostschweiz

@supported
Scenario: Testing https://www.kenwoodworld.com/uk/account/sign-in#
Given I navigate to 'https://www.kenwoodworld.com/uk/account/sign-in'
When I login kenwoodworld with 'citesting@themooltipass.com'
Then I should be logged in kenwoodworld
When I logout kenwoodworld
And I go to kenwoodworld login page
Then I should be logged in kenwoodworld

@subdomain
@supported
Scenario: Testing https://workbench.cisecurity.org/
Given I navigate to 'https://workbench.cisecurity.org/'
When I login cisecurity with 'citesting@themooltipass.com'
Then I should be logged in cisecurity
When I logout cisecurity
And I go to cisecurity login page
Then I should be logged in cisecurity

@supported
Scenario: Testing https://www.microchipdirect.com/
Given I navigate to 'https://www.microchipdirect.com/'
When I login microchipdirect with 'citesting@themooltipass.com'
Then I should be logged in microchipdirect
When I logout microchipdirect
And I go to microchipdirect login page
Then I should be logged in microchipdirect
