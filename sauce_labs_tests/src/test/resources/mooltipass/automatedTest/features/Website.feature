

Feature: Testing mooltipass on different websites

Scenario: Testing stackoverflow
Given I navigate to 'https://stackoverflow.com'
When I go to StackOverFlow login page
When I log in StackOverFlow with 'mooltipass@discard.email' and '!Pass12345'
Then I should be logged in StackOverFlow
When I logout StackOverFlow
And I go to StackOverFlow login page
Then I should be logged in StackOverFlow
