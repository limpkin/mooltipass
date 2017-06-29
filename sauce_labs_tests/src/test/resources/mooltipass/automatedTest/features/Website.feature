

Feature: Testing mooltipass on different websites


#Not Working-> site sometimes say invalid email pass
Scenario: Testing indiegogo.com
Given I navigate to 'https://www.indiegogo.com'
When I login Indiegogo with 'citesting@themooltipass.com'
Then I should be logged in Indiegogo
When I logout Indiegogo
And I go to Indiegogo login page
Then I should be logged in Indiegogo


@ignore 
Scenario: Testing facebook.com
Given I navigate to 'https://www.facebook.com/'
When I login Facebook with 'citesting@themooltipass.com'
Then I should be logged in Facebook
When I logout Facebook
Then I should be logged in Facebook

