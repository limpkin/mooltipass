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

@notsupported 
Scenario: Testing airbnb
Given I navigate to 'https://www.airbnb.com/'
When I login airBnB with 'citesting@themooltipass.com'
Then I should be logged in airBnB
When I logout airBnB
And I go to airBnB login page
Then I should be logged in airBnB

@notsupported
Scenario: Testing Turkish airlines
Given I navigate to 'https://p.turkishairlines.com/'
When I login turkish airlines with '441928383'
Then I should be logged in turkish airlines
When I logout turkish airlines
And I go to turkish airlines login page
Then I should be logged in turkish airlines