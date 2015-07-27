
var mooltipass = mooltipass || {};

mooltipass.deviceStatus = {};
mooltipass.app = null;

mooltipass.disableNonUnlockedNotifications = false;
mooltipass.connectedToApp = false;
mooltipass.locked = true;

var mpInputCallback = null;
var mpUpdateCallback = null;
var mpRandomCallback = null;
var mpRandomCallbackParams = null;
var mpSeedDate = null;

mooltipass.latestApp = (typeof(localStorage.latestApp) == 'undefined') ?
                                {"version": 0, "versionParsed": 0, "lastChecked": null} :
                                JSON.parse(localStorage.latestApp);

var extVersion = chrome.app.getDetails().version;
mooltipass.currentExtension = { version: extVersion, versionParsed: parseInt(extVersion.replace(/\./g,'')) };
mooltipass.currentApp = { version: 0, versionParsed: 0 };

mooltipass.blacklist = typeof(localStorage.mpBlacklist)=='undefined' ? {} : JSON.parse(localStorage.mpBlacklist);

var maxServiceSize = 123;       // Maximum size of a site / service name, not including null terminator



// Messages from the mooltipass client app
chrome.runtime.onMessageExternal.addListener(function(message, sender, sendResponse) {
    if (message.deviceStatus !== null) {
		//console.log(message.deviceStatus);
        mooltipass.deviceStatus = {
            'connected': message.deviceStatus.version != 'unknown',
            'unlocked': message.deviceStatus.connected,
            'version': message.deviceStatus.version,
			'state' : message.deviceStatus.state
        };

        mooltipass.device.currentFirmware.version = message.deviceStatus.version;
        mooltipass.device.currentFirmware.versionParsed = parseInt(message.deviceStatus.version.replace(/[\.a-zA-Z]/g,''));

    }
    else if (message.random !== null) {
        console.log('fromApp.randomString', message.random);
        Math.seedrandom(message.random);
        if(mooltipass.device._asynchronous.randomCallback) {
            mooltipass.device._asynchronous.randomCallback({
                'seeds': mooltipass.device.generateRandomNumbers(mooltipass.device._asynchronous.randomParameters.length)
            });
        }
    }
    else if (message.credentials !== null) {
        if (mpInputCallback) {
            mpInputCallback([
                {
                    Login: message.credentials.login,
                    Name: '<name>',
                    Uuid: '<Uuid>',
                    Password: message.credentials.password,
                    StringFields: []
                }
            ]);
            mpInputCallback = null;
        }
   }
    else if (message.noCredentials !== null) {
        if (mpInputCallback) {
            mpInputCallback([]);
            mpInputCallback = null;
        }
   }
    else if (message.updateComplete !== null) {
        if (mpUpdateCallback) {
            try {
                mpUpdateCallback('success');
            } catch (e) {
                console.log("Error: " + e);
            }
            mpUpdateCallback = null;
        }
   }
});

mooltipass.getClientVersion = function() {
    if (mooltipass.app) {
        mooltipass.currentApp = { version: mooltipass.app.version, versionParsed: parseInt(mooltipass.app.version.replace(/\./g,'')) };
        return mooltipass.app.version;
    } else {
        return 'not connected';
    }
};

mooltipass.associate = function(callback, tab) {
    if (!mooltipass.app) {
        console.log('mp.associate()');
        chrome.management.getAll(mooltipass.onSearchForApp);
    }
    else if (!mooltipass.deviceStatus.connected) {
        // try pinging the app
        chrome.runtime.sendMessage(mooltipass.app.id, { ping: [] });
        console.log('mp.associate() already have client connection, sending ping');
    } else {
        console.log('mp.associate() already connected');
    }
};

mooltipass.addCredentials = function(callback, tab, username, password, url) {
	page.tabs[tab.id].errorMessage = null;
    mooltipass.associate();
    mooltipass.updateCredentials(callback, tab, null, username, password, url);
};


// needs to block until a response is received.
mooltipass.updateCredentials = function(callback, tab, entryId, username, password, url) {
    mooltipass.associate();

    if (mooltipass.isBlacklisted(url)) {
        console.log('notify: ignoring blacklisted url',url);
        if (callback) {
            callback('failure');
        }
        return;
    }

	// unset error message
	page.tabs[tab.id].errorMessage = null;

    chrome.runtime.sendMessage({type: 'update', url: url, inputs: {login: {id: 0, name: 0, value: username}, password: { id: 1, name: 1, value: password }}});

    request = {update: {context: url, login: username, password: password}};
    mpUpdateCallback = callback;
    chrome.runtime.sendMessage(mooltipass.app.id, request);
};

mooltipass.copyPassword = function(callback, tab)
{
	page.tabs[tab.id].errorMessage = null;
    console.log('mp.copyPassword()');
};

mooltipass.extractDomainAndSubdomain = function (url) {
	var url_valid;
	var domain = null;
	var subdomain = null;
	console.log("Parsing ", url);
	
	// URL trimming
	// Remove possible www.
	url = url.replace('www.', '');
	// Remove everything before ://
    //    also ensure that only the first :// is used
    //    (negative example: https://id.atlassian.com/login?continue=https://my.atlassian.com&application=mac)
	url = url.replace(/^[^:]+:\/\//g, "");
	// Remove everything after first /
	var n = url.indexOf('/');
	url = url.substring(0, n != -1 ? n : url.length);
	// Remove everything after first :
	var n = url.indexOf(':');
	url = url.substring(0, n != -1 ? n : url.length);
	console.log("Trimmed URL: ", url)
	
	if(psl.isValid(url))
	{
		// Managed to extract a domain using the public suffix list
		console.log("valid URL detected")
		
		url_valid = true;
		var parsed = psl.parse(String(url))
		domain = parsed.domain;
		subdomain = parsed.subdomain;
		
		console.log("Extracted domain: ", domain);
		console.log("Extracted subdomain: ", subdomain);
	}
	else
	{
		// Check if it is an ip address
		var ipV4Pattern = /^\s*(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\s*$/;
        var ipV6Pattern = /^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$/;
        var ipV4Array = url.match(ipV4Pattern);
        var ipV6Array = url.match(ipV6Pattern);
        if(ipV4Array != null)
        {
            url_valid = true;
            domain = url;
            subdomain = null;
            console.log("ip v4 address detected")
        }
        else if(ipV6Array != null)
        {
            url_valid = true;
            domain = url;
            subdomain = null;
            console.log("ip v6 address detected")
        }
        else
		{
			url_valid = false;
			console.log("invalid URL detected")			
		}
	}	
	
	return {valid: url_valid, domain: domain, subdomain: subdomain}
}

mooltipass.retrieveCredentials = function(callback, tab, url, submiturl, forceCallback, triggerUnlock)
{
    mooltipass.associate();
	page.debug("mp.retrieveCredentials(callback, {1}, {2}, {3}, {4})", tab.id, url, submiturl, forceCallback);
	page.tabs[tab.id].errorMessage = null;

	// unset error message
	page.tabs[tab.id].errorMessage = null;
	
	// Check that the Mooltipass is unlocked
	event.mooltipassUnlockedCheck();

	// is browser associated to keepass?
	if (!mooltipass.device.isUnlocked()) {
		browserAction.showDefault(null, tab);
		if(forceCallback) {
			callback([]);
		}
		return;
	}
	
	// parse url and check it is valid
	var parsed_url = mooltipass.extractDomainAndSubdomain(submiturl);	
	if(!parsed_url.valid)
	{
        if(forceCallback) {
            callback([]);
        }
		return;
	}

	// todo: two requests for domain and subdomain!
    request = { getInputs : {context: parsed_url.domain, domain: parsed_url.domain, subdomain: parsed_url.subdomain} };

    console.log('sending to '+mooltipass.app.id);
    mpInputCallback = callback;
    chrome.runtime.sendMessage(mooltipass.app.id, request);
}


mooltipass.loadSettings = function() {
    mooltipass.blacklist = typeof(localStorage.mpBlacklist)=='undefined' ? {} : JSON.parse(localStorage.mpBlacklist);
}

mooltipass.isBlacklisted = function(url)
{
    return url in mooltipass.blacklist;
}

mooltipass.blacklistUrl = function(url)
{
    console.log('got blacklist req. for', url);
    mooltipass.blacklist[url] = true;
    localStorage.mpBlacklist = JSON.stringify(mooltipass.blacklist);
    console.log('updated blacklist store');
}

