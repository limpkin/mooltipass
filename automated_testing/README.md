These tests open an instance of chrome to run in.  They can be brittle and are most useful in manually verifing things are working as they should initially designed to be run with a BETATESTERS_AUTOACCEPT_SETUP defined mooltipass.hex.  A list of comma delimited login urls, login link text, and logout link text should be defined in a file.  See AutoLoginTestData.txt as an example.  I don't recommended running in BETATESTERS_AUTOACCEPT_SETUP when entering credentials on sites you don't know work with Mooltipass

- MP requires using the Chrome Developer Channel version of Chrome.
- Download and unzip chromedriver - http://chromedriver.storage.googleapis.com/index.html
- Use -Dmooltipass.auto.login.file=full_path_to_file where each line is: url,login_link_text,logout_link_text
- Use -Dmooltipass.timeout.seconds=20 to set the number of seconds to wait before failing.
- Use -Dmooltipass.extension.timeout.seconds=45 to set the number of seconds for the test to wait while you manually install the extensions.
- When you see the message "Waiting for you to install Extensions manually." install chrome.hid-app and chrome.ext manually (Click the Developement
- Mode checkbox in the upper right of chromes extensions page, then click Load Unpacked Extension..., and select from mooltipass/authentication_clients )


./aft.sh -Dwebdriver.chrome.driver=full_path_to_chromedriver -Dmooltipass.auto.login.file=full_path_to_file

