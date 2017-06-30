Mooltipass Extension Automated Testing
======================================

Setting up Travis, encrypting gloval vars
-----------------------------------------
- Download & install ruby from https://rubyinstaller.org/downloads/  
- go to https://github.com/settings/tokens/new and generate a token with scopes user:email read:org repo_deployment repo:status write:repo_hook
- (git terminal) gem install travis
- (git terminal) travis login --github-token <generated token>
- (git terminal) travis encrypt SAUCE_ACCESS_KEY="saucelabspassword" (do not forget escaping special chars)
- 