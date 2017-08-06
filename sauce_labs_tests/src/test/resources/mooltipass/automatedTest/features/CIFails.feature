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

#Not Working-> sauce labs can't find login fields
Scenario: Testing tripadvisor.com
Given I navigate to 'https://www.tripadvisor.com'
When I login tripadvisor with 'citesting@themooltipass.com'
Then I should be logged in tripadvisor
When I logout tripadvisor
And I go to tripadvisor login page
Then I should be logged in tripadvisor