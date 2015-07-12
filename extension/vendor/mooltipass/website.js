if (typeof mooltipass === 'undefined') mooltipass = {};
mooltipass.website = mooltipass.website || {};


/* library functions for mooltipass.website ********************** */

// [MOCKUP]
mooltipass.website.generatePassword = function(length, callback) {
  // Return a random password with given length

  callback(Math.random().toString(36).substring(7));
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