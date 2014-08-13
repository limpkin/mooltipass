- MP requires using the Chrome Developer Channel version of Chrome.
- Tests with BETATESTERS_AUTOACCEPT_SETUP hex
- Download and unzip chromedriver - http://chromedriver.storage.googleapis.com/index.html
- File where each line is: url,login_link_text,logout_link_text
- When you see the message "Waiting 2 minutes for you to install Extensions manually." install chrome.hid-app and chrome.ext manually (Click the Developement
- Mode checkbox in the upper right of chromes extensions page, then click Load Unpacked Extension..., and select from mooltipass/authentication_clients )


./aft.sh -Dwebdriver.chrome.driver=full_path_to_chromedriver -Dmooltipass.auto.login.file=full_path_to_file

