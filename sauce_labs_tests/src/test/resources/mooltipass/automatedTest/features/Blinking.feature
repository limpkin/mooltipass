Feature: Supported Websites

@supported
Scenario: Testing stackoverflow
Possible problem: stackoverflow flags us as a bot
Given I navigate to 'https://stackoverflow.com'
When I go to StackOverFlow login page
When I log in StackOverFlow with 'mooltipass@discard.email'
Then I should be logged in StackOverFlow
When I logout StackOverFlow
And I go to StackOverFlow login page
Then I should be logged in StackOverFlow

@supported
Scenario: Testing paypal.com
Possible problem: Paypal asks for some additional information
Given I navigate to 'https://www.paypal.com/at/home'
When I login PayPal with 'citesting@themooltipass.com'
Then I should be logged in PayPal
When I logout PayPal
And I go to PayPal login page
Then I should be logged in PayPal
