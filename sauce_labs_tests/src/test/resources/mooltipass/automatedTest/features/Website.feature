Feature: Testing mooltipass on different websites

@ignore 
Scenario: Testing facebook.com
Given I navigate to 'https://www.facebook.com/'
When I login Facebook with 'citesting@themooltipass.com'
Then I should be logged in Facebook
When I logout Facebook
Then I should be logged in Facebook

