
var mooltipass = {};
var mpClient = null;
var contentAddr = null;
var connected = null;
var mpInputCallback = null;
var mpUpdateCallback = null;

mooltipass.latestChromeipassVersionUrl = 'https://raw.githubusercontent.com/limpkin/mooltipass/master/authentication_clients/chromeipass.ext/manifest.json';
mooltipass.latestChromeipass = (typeof(localStorage.latestChromeipass) == 'undefined') ? {"version": 0, "versionParsed": 0, "lastChecked": null} : JSON.parse(localStorage.latestChromeipass);

var extVersion = chrome.app.getDetails().version;
mooltipass.currentChromeipass = { version: extVersion, versionParsed: parseInt(extVersion.replace(/\./g,'')) };
mooltipass.blacklist = typeof(localStorage.mpBlacklist)=='undefined' ? {} : JSON.parse(localStorage.mpBlacklist);

var maxServiceSize = 123;       // Maximum size of a site / service name, not including null terminator

function mpCheckConnection()
{
    if (!connected) {
        if (!mpClient) {
            // Search for the Mooltipass Client
            chrome.management.getAll(getAll);
        } else {
            chrome.runtime.sendMessage(mpClient.id, { type: 'ping' });
            setTimeout(mpCheckConnection,500);
        }
    }
}

function getAll(ext)
{
    for (var ind=0; ind<ext.length; ind++) {
        if (ext[ind].shortName == 'Mooltipass Client') {
            mpClient = ext[ind];
            break;
        }
    }

    if (mpClient != null) {
        chrome.runtime.sendMessage(mpClient.id, { type: 'ping' });
        console.log('found mooltipass client "'+ext[ind].shortName+'" id='+ext[ind].id);
    } else {
        console.log('No mooltipass client found');
    }

    setTimeout(mpCheckConnection,500);
}

// Search for the Mooltipass Client
chrome.management.getAll(getAll);

// Messages from the mooltipass client app
chrome.runtime.onMessageExternal.addListener(function(request, sender, sendResponse) 
{
    console.log('back: app req '+request.type);
    //console.log('back: app req '+JSON.stringify(request));
    switch (request.type) {
        case 'credentials':
            //chrome.tabs.sendMessage(contentAddr, request);
            if (mpInputCallback) {
                mpInputCallback([{Login: request.inputs.login.value, Name: '<name>', Uuid: '<Uuid>', Password: request.inputs.password.value, StringFields: []}]);
                mpInputCallback = null;
            }
            break;
        case 'updateComplete':
            if (mpUpdateCallback) {
                try {
                    mpUpdateCallback('success');
                } catch (e) {
                    console.log("Error: " + e);
                }
                mpUpdateCallback = null;
            }
            //chrome.tabs.sendMessage(contentAddr, request);
            break;
        case 'connected':
            connected = request;
            //if (contentAddr) {
                //chrome.tabs.sendMessage(contentAddr, request);
            //}
            break;
        case 'disconnected':
            connected = null;
            //if (contentAddr) {
                //chrome.tabs.sendMessage(contentAddr, request);
            //}
            break;
        case 'cardPresent':
            //if (contentAddr) {
                //chrome.tabs.sendMessage(contentAddr, request);
            //}
            //if (!request.state){
                //chrome.browserAction.setIcon({path: 'mooltipass-nocard.png'});
            //}
            break;
        case 'rescan':
            //if (contentAddr) {
                //chrome.tabs.sendMessage(contentAddr, request);
            //}
            break;
        default:
            break;
    }
});

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
	console.log("mp.updateCredentials(})", tab.id, entryId, username, url);

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

    request = { type: 'update',
                url: url, 
                inputs: {
                    login: {id: 'login.id', name: 'login.name', value: username},
                    password: {id: 'pass.id', name: 'pass.name', value: password} } };

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
        chrome.runtime.sendMessage(mpClient.id, { type: 'ping' });
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


mooltipass.retrieveCredentials = function(callback, tab, url, submiturl, forceCallback, triggerUnlock) 
{
    mooltipass.associate();
	page.debug("mp.retrieveCredentials(callback, {1}, {2}, {3}, {4})", tab.id, url, submiturl, forceCallback);
	page.tabs[tab.id].errorMessage = null;

	// unset error message
	page.tabs[tab.id].errorMessage = null;

	// is browser associated to keepass?
	if (!mooltipass.isConnected()) {
		browserAction.showDefault(null, tab);
		if(forceCallback) {
			callback([]);
		}
		return;
	}

    request = { type: 'inputs',
                url: submiturl, 
                inputs: {
                    login: {id: 'login.id', name: 'login.name'},
                    password: {id: 'pass.id', name: 'pass.name'} } };

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
        mooltipass.latestChromeipass.version = manifest.version;
        mooltipass.latestChromeipass.versionParsed = parseInt(manifest.version.replace(/\./g,''));
	} catch (e) {
		console.log("Error: " + e);
	}

	if (version != -1) {
		localStorage.latestChromeipass = JSON.stringify(mooltipass.latestChromeipass);
	}
	mooltipass.latestChromeipass.lastChecked = new Date();
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

