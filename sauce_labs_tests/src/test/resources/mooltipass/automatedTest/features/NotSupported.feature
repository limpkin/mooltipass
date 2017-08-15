Feature: Not supported Websites


@notsupported
Scenario: Testing https://www.trillian.im/web/4.0/
Given I navigate to 'https://www.trillian.im/web/4.0/'
When I login trillian with 'mooltipass'
Then I should be logged in trillian
When I logout trillian
Then I should be logged in trillian

@notsupported
Scenario: Testing http://minfin.com.ua
Given I navigate to 'http://minfin.com.ua'
When I login minfin with 'citesting@themooltipass.com'
Then I should be logged in minfin
When I logout minfin
And I go to minfin login page
Then I should be logged in minfin