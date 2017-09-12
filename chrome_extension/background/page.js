// Detect if we're dealing with Firefox, Safari, or Chrome
var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
var isSafari = typeof(safari) == 'object'?true:false;

var page = {
	pageLoaded: true
};

// special information for every tab
page.tabs = {};

// Check for complete load and rendering before trying to act
page.allLoaded = false;

page.currentTabId = -1;
page.settings = (typeof(localStorage.settings) == 'undefined') ? {} : JSON.parse(localStorage.settings);
if (isFirefox || isSafari) page.settings.useMoolticute = true;

page.blockedTabs = {};

page.initSettings = function() {
	event.onLoadSettings();
    var changed = false;
	
	if(!("autoCompleteUsernames" in page.settings)) {
		page.settings.autoCompleteUsernames = 1;
        changed = true;
	}
	if(!("autoFillAndSend" in page.settings)) {
		page.settings.autoFillAndSend = 1;
        changed = true;
	}
	if(!("autoFillSingleEntry" in page.settings)) {
		page.settings.autoFillSingleEntry = 1;
        changed = true;
	}
	if(!("useUpdatePopup" in page.settings)) {
		page.settings.useUpdatePopup = true;
        changed = true;
	}
	// "popup" seems to be a legacy setting which may still be stored
	if(!("updateMethod" in page.settings) || page.settings.updateMethod == "popup") {
		page.settings.updateMethod = "notification";
        changed = true;
	}

    if(!("usePasswordGenerator" in page.settings)) {
        page.settings.usePasswordGenerator = true;
        changed = true;
    }
    if(!("usePasswordGeneratorLength" in page.settings) || parseInt(page.settings.usePasswordGeneratorLength) < 6) {
        page.settings.usePasswordGeneratorLength = 12;
        changed = true;
    }
    if(!("usePasswordGeneratorLowercase" in page.settings)) {
        page.settings.usePasswordGeneratorLowercase = true;
        changed = true;
    }
    if(!("usePasswordGeneratorUppercase" in page.settings)) {
        page.settings.usePasswordGeneratorUppercase = true;
        changed = true;
    }
    if(!("usePasswordGeneratorNumbers" in page.settings)) {
        page.settings.usePasswordGeneratorNumbers = true;
        changed = true;
    }
    if(!("usePasswordGeneratorSpecial" in page.settings)) {
        page.settings.usePasswordGeneratorSpecial = true;
        changed = true;
    }
	if(!("doNotSubmitAfterFill" in page.settings)) {
		page.settings.doNotSubmitAfterFill = false;
        changed = true;
	}

    if(changed) {
        localStorage.settings = JSON.stringify(page.settings);
        event.onLoadSettings();
    }
}

/*
* Store previously used logins
*/
page.cacheLogin = function( callback, tab, arguments ) {
	if (background_debug_msg > 4) mpDebug.log('%c page: %c cacheLogin ','background-color: #ffeef9','color: #246', arguments);
	page.tabs[ tab.id ].loginList = {'Login': arguments };
}

/*
* Retrieve cached login information
*/
page.cacheRetrieve = function( callback, tab, arguments ) {
	callback( page.tabs[ tab.id ].loginList, tab );
}

page.initOpenedTabs = function() {
	if (isSafari) {
		for (var i = 0; i < safari.application.activeBrowserWindow.tabs.length; i++) {
			page.createTabEntry(i)
		}
	} else {
		chrome.tabs.query({}, function(tabs) {
			for(var i = 0; i < tabs.length; i++) {
				page.createTabEntry(tabs[i].id);
			}
		});	
	}
}

page.isValidProtocol = function(url) {
	var protocol = url.substring(0, url.indexOf(":"));
	protocol = protocol.toLowerCase();
	return !(url.indexOf(".") == -1 || (protocol != "http" && protocol != "https" && protocol != "ftp" && protocol != "sftp"));
}

page.switchTab = function(callback, tab) {
	if ( typeof(tab) == 'number' ) tab = { id: tab };
	if (background_debug_msg > 4) mpDebug.log('%c page: %c switchTab ', mpDebug.css('ffeef9'), tab );
	browserAction.showDefault(null, tab);

	messaging({ action: 'activated_tab' }, tab)
}

page.setAllLoaded = function( callback, tab ) {
	if (background_debug_msg > 4) mpDebug.log('%c page: setAllLoaded ', mpDebug.css('ffeef9'));
	page.allLoaded = true;
	callback({}, tab );
}

page.setCurrentTab = function(callback, tab) {
    if(page.currentTabId != tab.id) {
        page.currentTabId = tab.id;
        page.switchTab(callback, tab);
    }
}

page.clearCredentials = function(tabId, complete) {
	if (background_debug_msg > 4) mpDebug.log('%c page: clearCredentials ', mpDebug.css('ffeef9'));
	if(!page.tabs[tabId]) {
		return;
	}

	if ( page.tabs[tabId].credentials ) {
		page.tabs[tabId].credentials = {};
		delete page.tabs[tabId].credentials;

	    if(complete) {
	        page.tabs[tabId].loginList = [];
	        chrome.tabs.sendMessage(tabId, {
	            action: "clear_credentials"
	        });
	    }
	}
}

page.createTabEntry = function(tabId) {
	if (background_debug_msg > 4) mpDebug.log('%c page: createTabEntry ', mpDebug.css('ffeef9'), tabId );
	page.tabs[tabId] = {
		"stack": [],
		"errorMessage": null,
		"loginList": {}
	};
}

page.removePageInformationFromNotExistingTabs = function() {
	var rand = Math.floor(Math.random()*1001);
	if(rand == 28) {
		chrome.tabs.query({}, function(tabs) {
			var $tabIds = {};
			var $infoIds = Object.keys(page.tabs);

			for(var i = 0; i < tabs.length; i++) {
				$tabIds[tabs[i].id] = true;
			}

			for(var i = 0; i < $infoIds.length; i++) {
				if(!($infoIds[i] in $tabIds)) {
					delete page.tabs[$infoIds[i]];
				}
			}
		});
	}
};

page.debugConsole = function() {
	if(arguments.length > 1) {
		console.log(page.sprintf(arguments[0], arguments));
	}
	else {
		console.log(arguments[0]);
	}
};

page.sprintf = function(input, args) {
	return input.replace(/{(\d+)}/g, function(match, number) {
      return typeof args[number] != 'undefined'
        ? (typeof args[number] == 'object' ? JSON.stringify(args[number]) : args[number])
        : match
      ;
    });
}

page.debugDummy = function() {};

//page.debug = page.debugDummy;
page.debug = page.debugConsole;

page.setDebug = function(bool) {
	if(bool) {
		page.debug = page.debugConsole;
		return "Debug mode enabled";
	}
	else {
		page.debug = page.debugDummy;
		return "Debug mode disabled";
	}
};
