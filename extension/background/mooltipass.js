
var mooltipass = {};
var mpClient = null;
var contentAddr = null;
var connected = null;
var mpInputCallback = null;
var mpUpdateCallback = null;

mooltipass.latestChromeipassVersionUrl = 'https://raw.githubusercontent.com/limpkin/mooltipass/master/authentication_clients/chromeipass.ext/manifest.json';
mooltipass.latestClientVersionUrl = 'https://raw.githubusercontent.com/limpkin/mooltipass/master/authentication_clients/chrome.hid-app/manifest.json';
mooltipass.latestChromeipass = (typeof(localStorage.latestChromeipass) == 'undefined') ?
                                {"version": 0, "versionParsed": 0, "lastChecked": null} :
                                JSON.parse(localStorage.latestChromeipass);

mooltipass.latestClient = (typeof(localStorage.latestClient) == 'undefined') ?
                                {"version": 0, "versionParsed": 0, "lastChecked": null} :
                                JSON.parse(localStorage.latestClient);

mooltipass.latestFirmware = (typeof(localStorage.latestFirmware) == 'undefined') ?
                                {"version": 0, "versionParsed": 0, "lastChecked": null} :
                                JSON.parse(localStorage.latestFirmware);

var extVersion = chrome.app.getDetails().version;
mooltipass.currentChromeipass = { version: extVersion, versionParsed: parseInt(extVersion.replace(/\./g,'')) };
mooltipass.currentClient = { version: 0, versionParsed: 0 };

mooltipass.blacklist = typeof(localStorage.mpBlacklist)=='undefined' ? {} : JSON.parse(localStorage.mpBlacklist);

var maxServiceSize = 123;       // Maximum size of a site / service name, not including null terminator

function mpCheckConnection()
{
    if (!connected) {
        if (!mpClient) {
            // Search for the Mooltipass Client
            chrome.management.getAll(getAll);
        } else {
            chrome.runtime.sendMessage(mpClient.id, { ping: [] });
            setTimeout(mpCheckConnection,500);
        }
    }
}

function getAll(ext)
{
    for (var ind=0; ind<ext.length; ind++) {
        if (ext[ind].shortName == 'Mooltipass App') {
            mpClient = ext[ind];
            break;
        }
    }

    if (mpClient != null) {
        chrome.runtime.sendMessage(mpClient.id, { ping: [] });
        console.log('found mooltipass app "'+ext[ind].shortName+'" id='+ext[ind].id,' app: ',mpClient);
    } else {
        console.log('No mooltipass app found');
    }

    setTimeout(mpCheckConnection,500);
}

// Search for the Mooltipass Client
chrome.management.getAll(getAll);

// Messages from the mooltipass client app
chrome.runtime.onMessageExternal.addListener(function(message, sender, sendResponse)
{
    if (message.deviceStatus !== null) {
        if (message.deviceStatus.connected) {
            connected = { version : message.deviceStatus.version };
        }
        else {
            connected = null;
        }
    } else if (message.credentials !== null) {
        if (mpInputCallback) {
            mpInputCallback([
                    { Login: message.credentials.login
                    , Name: '<name>', Uuid: '<Uuid>'
                    , Password: message.credentials.password
                    , StringFields: []
                    }
            ]);
            mpInputCallback = null;
        }
   } else if (message.noCredentials !== null) {
        if (mpInputCallback) {
            mpInputCallback([]);
        }
   } else if (message.updateComplete !== null) {
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

mooltipass.getFirmwareVersion = function()
{
    if (connected) {
        return connected.version;
    } else {
        return 'not connected';
    }
}

mooltipass.getClientVersion = function()
{
    if (mpClient) {
        mooltipass.currentClient = { version: mpClient.version, versionParsed: parseInt(mpClient.version.replace(/\./g,'')) };
        return mpClient.version;
    } else {
        return 'not connected';
    }
}

mooltipass.addCredentials = function(callback, tab, username, password, url)
{
	page.tabs[tab.id].errorMessage = null;
    mooltipass.associate();
    mooltipass.updateCredentials(callback, tab, null, username, password, url);
}

mooltipass.isConnected = function()
{
    return connected != null;
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
    console.log('sending update to '+mpClient.id);
    contentAddr = tab.id;
    mpUpdateCallback = callback;
    chrome.runtime.sendMessage(mpClient.id, request);
}


mooltipass.associate = function(callback, tab)
{
    if (!mpClient) {
        console.log('mp.associate()');
        chrome.management.getAll(getAll);
    } else if (!connected) {
        // try pinging the app
        chrome.runtime.sendMessage(mpClient.id, { ping: [] });
        console.log('mp.associate() already have client connection, sending ping');
    } else {
        console.log('mp.associate() already connected');
    }
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

mooltipass.extractDomainAndSubdomain = function (url)
{
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
	if (!mooltipass.isConnected()) 
	{
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

    console.log('sending to '+mpClient.id);
    contentAddr = tab.id;
    mpInputCallback = callback;
    chrome.runtime.sendMessage(mpClient.id, request);
}

mooltipass.getLatestChromeipassVersion = function()
{
	var xhr = new XMLHttpRequest();
	xhr.open("GET", mooltipass.latestChromeipassVersionUrl, false);
	xhr.setRequestHeader("Content-Type", "application/json");
    var version = -1;
	try {
		xhr.send();
		manifest = JSON.parse(xhr.responseText);
        version = manifest.version;
        mooltipass.latestChromeipass.version = version;
        mooltipass.latestChromeipass.versionParsed = parseInt(version.replace(/\./g,''));
	} catch (e) {
		console.log("Error: " + e);
	}

	if (version != -1) {
		localStorage.latestChromeipass = JSON.stringify(mooltipass.latestChromeipass);
	}
	mooltipass.latestChromeipass.lastChecked = new Date();
}

mooltipass.getLatestClientVersion = function()
{
	var xhr = new XMLHttpRequest();
	xhr.open("GET", mooltipass.latestClientVersionUrl, false);
	xhr.setRequestHeader("Content-Type", "application/json");
    var version = -1;
	try {
		xhr.send();
		manifest = JSON.parse(xhr.responseText);
        version = manifest.version;
        mooltipass.latestClient.version = version;
        mooltipass.latestClient.versionParsed = parseInt(version.replace(/\./g,''));
	} catch (e) {
		console.log("Error: " + e);
	}

	if (version != -1) {
		localStorage.latestClient = JSON.stringify(mooltipass.latestClient);
	}
	mooltipass.latestClient.lastChecked = new Date();
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
    console.log('got blacklist req. for',url);
    mooltipass.blacklist[url] = true;
    localStorage.mpBlacklist = JSON.stringify(mooltipass.blacklist);
    console.log('updated blacklist store');
}

