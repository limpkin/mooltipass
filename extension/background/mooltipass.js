
var mooltipass = mooltipass || {};

mooltipass.deviceStatus = {};
mooltipass.app = null;

mooltipass.connectedToApp = false;
mooltipass.locked = true;

var contentAddr = null;
var mpInputCallback = null;
var mpUpdateCallback = null;

mooltipass.latestApp = (typeof(localStorage.latestApp) == 'undefined') ?
                                {"version": 0, "versionParsed": 0, "lastChecked": null} :
                                JSON.parse(localStorage.latestApp);

var extVersion = chrome.app.getDetails().version;
mooltipass.currentExtension = { version: extVersion, versionParsed: parseInt(extVersion.replace(/\./g,'')) };
mooltipass.currentApp = { version: 0, versionParsed: 0 };

mooltipass.blacklist = typeof(localStorage.mpBlacklist)=='undefined' ? {} : JSON.parse(localStorage.mpBlacklist);

var maxServiceSize = 123;       // Maximum size of a site / service name, not including null terminator



mooltipass.checkConnection = function() {
    if(!mooltipass.connectedToApp) {
        // Search for Mooltipass App
        chrome.management.getAll(mooltipass.onSearchForApp);
        return;
    }

    chrome.runtime.sendMessage(mooltipass.app.id, { ping: [] });
    setTimeout(mooltipass.checkConnection, 500);
};

mooltipass.onSearchForApp = function(ext) {
    for (var i = 0; i < ext.length; i++) {
        if (ext[i].shortName == 'Mooltipass App') {
            mooltipass.app = ext[i];
            break;
        }
    }

    if (mooltipass.app != null) {
        mooltipass.connectedToApp = true;
        chrome.runtime.sendMessage(mooltipass.app.id, { ping: [] });

        console.log('found mooltipass app "' + mooltipass.app.shortName + '" id=' + mooltipass.app.id,' app: ', mooltipass.app);
    }
    else {
        mooltipass.connectedToApp = false;
        mooltipass.deviceStatus = {};
        console.log('No mooltipass app found');
    }

    setTimeout(mooltipass.checkConnection, 500);
}


// Search for the Mooltipass App
chrome.management.getAll(mooltipass.onSearchForApp);

// Messages from the mooltipass client app
chrome.runtime.onMessageExternal.addListener(function(message, sender, sendResponse) {
    if (message.deviceStatus !== null) {
        mooltipass.deviceStatus = {
            'connected': message.deviceStatus.version != 'unknown',
            'unlocked': message.deviceStatus.connected,
            'version': message.deviceStatus.version
        };

        mooltipass.device.currentFirmware.version = message.deviceStatus.version;
        mooltipass.device.currentFirmware.versionParsed = parseInt(message.deviceStatus.version.replace(/[\.a-zA-Z]/g,''));

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

mooltipass.associate = function(callback, tab)
{
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
}

mooltipass.addCredentials = function(callback, tab, username, password, url)
{
	page.tabs[tab.id].errorMessage = null;
    mooltipass.associate();
    mooltipass.updateCredentials(callback, tab, null, username, password, url);
}


// needs to block until a response is received.
mooltipass.updateCredentials = function(callback, tab, entryId, username, password, url)
{
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

    request = {update: {context: url, login: username, password: password}}
    console.log('sending update to '+mooltipass.app.id);
    contentAddr = tab.id;
    mpUpdateCallback = callback;
    chrome.runtime.sendMessage(mooltipass.app.id, request);
}


mooltipass.generatePassword = function(callback, tab)
{
	page.tabs[tab.id].errorMessage = null;
    console.log('mp.generatePassword()');
}

mooltipass.copyPassword = function(callback, tab)
{
	page.tabs[tab.id].errorMessage = null;
    console.log('mp.copyPassword()');
}

toContext = function (url) {
    // URL regex to extract base domain for context
    var reContext = /^\https?\:\/\/([\w\-\+]+\.)*([\w\-\_]+\.[\w\-\_]+)/;
    return reContext.exec(url)[2];
}

mooltipass.extractDomainAndSubdomain = function (url) {
	var url_valid;
	var domain = null;
	var subdomain = null;
	console.log("Parsing ", url);
	
	// URL trimming
	// Remove possible www.
	url = url.replace('www.', '');
	// Remove everything before //
	url = url.replace(/.*?:\/\//g, "");
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
		var ipPattern = /^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3})$/;
		var ipArray = url.match(ipPattern);
		if(ipArray != null)
		{
			url_valid = true;
			domain = url;
			subdomain = null;
			console.log("ip address detected")		
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
		return;
	}

	// todo: two requests for domain and subdomain!
    request = { getInputs : {context: parsed_url.domain} };

    console.log('sending to '+mooltipass.app.id);
    contentAddr = tab.id;
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

