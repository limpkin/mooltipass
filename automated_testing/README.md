These tests open an instance of chrome to run in.  They attempt to login and logout from a list of urls defined in a file.  They can be brittle and are most useful in manually verifying behavior.  I don't recommended running in BETATESTERS_AUTOACCEPT_SETUP when entering credentials on sites you don't know work with Mooltipass.  See AutoLoginTestData.txt as an example of how to setup your auto login file.

- Watch how this test works - https://plus.google.com/u/0/117594012155500951563/videos/p/pub?pid=6047655449680232530&oid=117594012155500951563
- MP requires using the Chrome Developer Channel version of Chrome.
- Download and unzip chromedriver - http://chromedriver.storage.googleapis.com/index.html
- Using a file -Dmooltipass.auto.login.file=full_path_to_file where each line is: url,login_link_text,logout_link_text
- Using command line (replacing spaces with underscores) -Dmooltipass.auto.login.line=URL,Login_Text,Logout_Text 
- Use -Dmooltipass.timeout.seconds=20 to set the number of seconds to wait before failing.
- Use -Dmooltipass.extension.timeout.seconds=45 to set the number of seconds for the test to wait while you manually install the extensions.
- When you see the message "Waiting for you to install Extensions manually." install chrome.hid-app and chrome.ext manually (Click the Developer mode checkbox in the upper right of the extensions page, then click Load Unpacked Extension..., and select from mooltipass/authentication_clients )

File example:
./aft.sh -Dwebdriver.chrome.driver=full_path_to_chromedriver -Dmooltipass.auto.login.file=full_path_to_file

Command line with more options example:
./aft.sh -Dwebdriver.chrome.driver=/java/tools/chromedriver -Dmooltipass.extension.timeout.seconds=25 -Dmooltipass.timeout.seconds=8 -Dmooltipass.auto.login.line=https://github.com/login,,Pull_Requests

