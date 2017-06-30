Mooltipass Extension Automated Testing
======================================

Setting up Travis, encrypting gloval vars
-----------------------------------------
- Download & install ruby from https://rubyinstaller.org/downloads/  
- go to https://github.com/settings/tokens/new and generate a token with scopes user:email read:org repo_deployment repo:status write:repo_hook
- (git terminal) gem install travis
- (git terminal) travis login --github-token <generated token>
- (git terminal) travis encrypt SAUCE_ACCESS_KEY="saucelabsaccesskey" (do not forget escaping special chars)
- add the output to .travis.yml, under SAUCE_USERNAME

Setting up passwords for each website
-------------------------------------
- in src\test\java\mooltipass\automatedTest\features open the .java of your choice
- set the name of the global var: System.getenv().get("<travis global var name>");
- encrypt the real password: travis encrypt <travis global var name>="<real password here>"
- add the output to .travis.yml