if (typeof mooltipass == 'undefined') {
	mooltipass = {};
}
mooltipass.website = mooltipass.website || {};


/* library functions for mooltipass.website ********************** */

// [MOCKUP]
mooltipass.website.generatePassword = function(length, callback) {
    // Return a random password with given length
    chrome.extension.sendMessage({
        action: 'generate_password',
        args: [length]
    }, function(response) {
        console.log('seed:', response.seeds)

        var hash = "";
        var possible = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789:.,;-_'#*+!\"()$=?{[]}%&/";

        for( var i=0; i < length; i++ )
            hash += possible.charAt(Math.floor(response.seeds[i] * possible.length));

        callback(hash);
    });
}

// [MOCKUP]
mooltipass.website.hasCredentialFields = function(callback) {
  // Return true if current website has credential fields which can be
  // selected by the 'choose_credential_fields' message

  callback(true);
}

mooltipass.website.chooseCredentialFields = function() {
	var global = chrome.extension.getBackgroundPage();

	chrome.tabs.sendMessage(global.page.currentTabId, {
		action: "choose_credential_fields"
	});	
}