

Feature: Testing mooltipass on different websites





#slide to verify and error in website
@subdomain @run
Scenario: Testing alibaba.com
Given I navigate to 'http://www.alibaba.com/'
When I login AliBaba with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in AliBaba
When I logout AliBaba
And I go to AliBaba login page
Then I should be logged in AliBaba

#Not Working-> site sometimes say invalid email pass
Scenario: Testing indiegogo.com
Given I navigate to 'https://www.indiegogo.com'
When I login Indiegogo with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in Indiegogo
When I logout Indiegogo
And I go to Indiegogo login page
Then I should be logged in Indiegogo


@ignore
Scenario: Testing facebook.com
Given I navigate to 'https://www.facebook.com/'
When I login Facebook with 'citesting@themooltipass.com' and 'testpass123'
Then I should be logged in Facebook
When I logout Facebook
Then I should be logged in Facebook

