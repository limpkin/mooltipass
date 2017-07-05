Mooltipass Extension Automated Testing
======================================

Setting up Travis, encrypting gloval vars
-----------------------------------------
- Download & install ruby from https://rubyinstaller.org/downloads/  
- go to https://github.com/settings/tokens/new and generate a token with scopes user:email read:org repo_deployment repo:status write:repo_hook
- (git terminal) gem install travis
- (git terminal) travis login --github-token <generated token>
- (git terminal) travis encrypt SAUCE_ACCESS_KEY="saucelabsaccesskey"
- add the output to .travis.yml, under SAUCE_USERNAME

Setting up passwords for each website
-------------------------------------
- in src\test\java\mooltipass\automatedTest\features open the .java of your choice
- set the name of the global var: System.getenv().get("[travis global var name]");
- encrypt the real password: travis encrypt [travis global var name]="[real password here]"
- add the output to .travis.yml

Adding support for a new website
--------------------------------
- start by creating credentials for this website (we use citesting[at]themooltipass[dot]com)
- in src\test\resources\mooltipass\automatedTest\features open the file of your choice and add a cucumber scenario (get inspired from the other scenarios)
- in src\test\java\mooltipass\automatedTest\pageObjects create your own .java with a class that should contain the different buttons and elements that need to be clicked/interacted with. There are different ways of identifying the elements such as using the element id or xpath.
- in src\test\java\mooltipass\automatedTest\features create a java file defining a method for each step of your scenario and using the page objects to interact with the website

Pull Requests
-------------
Encrypted vars can't be shared when a PR occurs. If you'd like to test the websites implemented in this folder, please get in touch with us.