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

@supported
Scenario: Testing patreon.com
Possible problem: captcha on login page
Given I navigate to 'https://www.patreon.com'
When I login Patreon with 'citesting@themooltipass.com'
Then I should be logged in Patreon
When I logout Patreon
And I go to Patreon login page
Then I should be logged in Patreon
