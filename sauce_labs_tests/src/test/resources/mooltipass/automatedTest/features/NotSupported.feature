Feature: Not supported Websites

@notsupported
Scenario: Testing http://minfin.com.ua
Given I navigate to 'http://minfin.com.ua'
When I login minfin with 'citesting@themooltipass.com'
Then I should be logged in minfin
When I logout minfin
And I go to minfin login page
Then I should be logged in minfin

@notsupported
Scenario: Testing https://www.microchipdirect.com/
Given I navigate to 'https://www.microchipdirect.com/'
When I login microchipdirect with 'citesting@themooltipass.com'
Then I should be logged in microchipdirect
When I logout microchipdirect
And I go to microchipdirect login page
Then I should be logged in microchipdirect

@notsupported 
Scenario: Testing airbnb
Given I navigate to 'https://www.airbnb.com/'
When I login airBnB with 'citesting@themooltipass.com'
Then I should be logged in airBnB
When I logout airBnB
And I go to airBnB login page
Then I should be logged in airBnB