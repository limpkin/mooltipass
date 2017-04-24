if (typeof mooltipass == 'undefined') {
	mooltipass = {};
}
mooltipass.website = mooltipass.website || {};

/* library functions for mooltipass.website ********************** */


mooltipass.website.generatePassword = function(length, callback) {
    // Return a random password with given length
    chrome.runtime.sendMessage({
        action: 'generate_password',
        args: [length]
    }, function(response) {
        var charactersLowercase = 'abcdefghijklmnopqrstuvwxyz';
        var charactersUppercase = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
        var charactersNumbers = '1234567890';
        var charactersSpecial = '!$%*()_+{}-[]:"|;\'?,./';

        var hash = "";
        var possible = "";

        if(response.settings["usePasswordGeneratorLowercase"]) {
            possible += charactersLowercase;
        }
        if(response.settings["usePasswordGeneratorUppercase"]) {
            possible += charactersUppercase;
        }
        if(response.settings["usePasswordGeneratorNumbers"]) {
            possible += charactersNumbers;
        }
        if(response.settings["usePasswordGeneratorSpecial"]) {
            possible += charactersSpecial;
        }

        for( var i=0; i < length; i++ ) {
            hash += possible.charAt(Math.floor(response.seeds[i] * possible.length));
        }

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
    if ( isSafari ) {
        messaging({ action: "choose_credential_fields" }, safari.application.activeBrowserWindow.activeTab );
    } else {
        var global = chrome.extension.getBackgroundPage();

        chrome.tabs.sendMessage(global.page.currentTabId, {
            action: "choose_credential_fields"
        });     
    }
}

mooltipass.website.reportError = function(callback) {
    var global = chrome.extension.getBackgroundPage();
    chrome.tabs.sendMessage(global.page.currentTabId, {
        action: "get_website_info"
    },
    function(response){
        var $ = jQuery.noConflict(true);
        if ("url" in response) {
            var url = response.url;
        } else {
            var url = "not-set";
        }
        
        callback("https://docs.google.com/forms/d/1lFKaTR3LQxySyGsZwtHudVE6aGErGU2DHfn-YpuW8aE/viewform?entry.449375470=" + url);
        return;
        /* commenting this code to avoid warnings at Firefox
        // not currently required, but loads html source and sends it to a custom error reporting tool
        if ("html" in response) {
            var html = response.html;
        } else {
            var html = "not-set";
        }        

        $.post("http://api.alex.zone/mooltipass/add-log", {
            "version" : 1,
            "url" : url,
            "html" : html
        }, function(response) {
            callback(response.url);
        });
		*/
    });     
}

if ( typeof($) == 'function') window.mpJQ = $;