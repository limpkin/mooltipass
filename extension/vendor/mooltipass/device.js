/* Initialize mooltipass lib */
if (typeof mooltipass == 'undefined') {
    mooltipass = {};
}

mooltipass.device = mooltipass.device || {};


/**
 * Information about connected Mooltipass app
 * Set on mooltipass.device.onSearchForApp()
 */
mooltipass.device._app = null;

/**
 * Contains status information about the device
 * Properties: connected, unlocked, version, state
 */
mooltipass.device._status = {};

/**
 * Boolean information whether the Mooltipass app was found and is connected
 * Used to speedup periodical requests
 */
mooltipass.device.connectedToApp = false;

/**
 * On initial load, the extension looks for the app to communicate with based on the exact app name
 * WARNING: If you change the app name, you have to also modify this parameter!
 */
mooltipass.device._appName = 'Mooltipass App';

/**
 * Interval of milliseconds to check connection to app and device
 */
mooltipass.device._intervalCheckConnection = 500;

/**
 * Parameters manually set for ansynchronous requests
 */
mooltipass.device._asynchronous = {
    // Callback function for returned random string
    'randomCallback': null,
    // Additional parameters for callback function, null or {}
    'randomParameters': null,
    // Callback function for received credentials from device
    'inputCallback': null,
    // Callback function for updated credentials
    'updateCallback': null,
};

/**
 * Requesting a new random string from device only once a minute
 * Minute of latest request stored in this parameter
 * Values: null or number: hour * 60 * minute
 */
mooltipass.device._latestRandomStringRequest = null;


/**
 * Checks for connected app and triggers search for app otherwise
 * Periodically sends PING to device which returns current status of device
 */
mooltipass.device.checkConnection = function() {
    if(!mooltipass.device.connectedToApp) {
        // Search for Mooltipass App
        chrome.management.getAll(mooltipass.device.onSearchForApp);
        return;
    }

    chrome.runtime.sendMessage(mooltipass.device._app.id, { ping: [] });
    setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
};

/**
 * Searches for mooltipass app in all available chrome apps
 * Triggers ping to device if app is found and sets mooltipass.device._app
 */
mooltipass.device.onSearchForApp = function(ext) {
    var foundApp = false;
    for (var i = 0; i < ext.length; i++) {
        if (ext[i].shortName == mooltipass.device._appName) {
            mooltipass.device._app = ext[i];
            foundApp = true;
            break;
        }
    }

    if(!foundApp) {
        mooltipass.device._app = null;
    }

    if (mooltipass.device._app != null) {
        mooltipass.device.connectedToApp = true;
        // Send ping which triggers status response from device
        chrome.runtime.sendMessage(mooltipass.device._app.id, { ping: [] });

        console.log('found mooltipass app "' + mooltipass.device._app.shortName + '" id=' + mooltipass.device._app.id,' app: ', mooltipass.device._app);
    }
    else {
        mooltipass.device.connectedToApp = false;
        mooltipass.device._status = {};
        console.log('No mooltipass app found');
    }

    setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
};

/**
 * Returns the current status of the connection to the device
 * @access backend
 * @returns {{connectedToApp: boolean, connectedToDevice: boolean, deviceUnlocked: boolean}}
 */
mooltipass.device.getStatus = function() {
    return {
        'connectedToApp': mooltipass.device._app ? true : false,
        'connectedToDevice': mooltipass.device._status.connected,
        'deviceUnlocked': mooltipass.device._status.unlocked
    };
};

/**
 * Checks if the device is unlocked
 * @access backend
 * @returns boolean
 */
mooltipass.device.isUnlocked = function() {
    return mooltipass.device.getStatus()['deviceUnlocked'];
};


/**
 * Generate a random password based on a random string returned from device
 * @access backend
 * @param callback to send the generated password to
 * @param tab current tab object with tab.id
 * @param length of the password
 */
mooltipass.device.generatePassword = function(callback, tab, length) {
    console.log('mooltipass.generatePassword()', 'length =', length);

    // unset error message
    page.tabs[tab.id].errorMessage = null;

    // Only request new random string from device once a minute
    // The requested random string is used to salt Math.random() again
    var currentDate = new Date();
    var currentDayMinute = currentDate.getUTCHours() * 60 + currentDate.getUTCMinutes();
    if(!mooltipass.device._latestRandomStringRequest || mooltipass.device._latestRandomStringRequest != currentDayMinute) {
        mooltipass.device._asynchronous.randomCallback = callback;
        mooltipass.device._asynchronous.randomParameters = {'length': length};
        mooltipass.device._latestRandomStringRequest = currentDayMinute;

        console.log('mooltipass.generatePassword()', 'request random string from app');
        var request = { getRandom : [] };
        chrome.runtime.sendMessage(mooltipass.device._app.id, request);
        return;
    }

    console.log('mooltipass.generatePassword()', 'use current seed for another password');
    callback({'seeds': mooltipass.device.generateRandomNumbers(length), 'settings': page.settings});
};

/**
 * Based on a salted Math.random() generate random numbers
 * @access backend
 * @param length number of random numbers to generate
 * @returns {Array} array of Numbers
 */
mooltipass.device.generateRandomNumbers = function(length) {
    var seeds = [];
    for(var i = 0; i < length; i++) {
        seeds.push(Math.random());
    }

    return seeds;
};


/**
 * Add credentials to device
 * @access backend
 * @param callback function to be triggered on response from device
 * @param tab which triggered the storing request
 * @param username
 * @param password
 * @param url
 */
mooltipass.device.addCredentials = function(callback, tab, username, password, url) {
    mooltipass.device.updateCredentials(callback, tab, null, username, password, url);
};


/**
 * Update or add credentials to device
 * IMPORTANT: needs to block until a response is received.
 *
 * @access backend
 * @param callback function to be triggered on response from device
 * @param tab which triggered the storing request
 * @param entryId not used
 * @param username
 * @param password
 * @param url
 */
mooltipass.device.updateCredentials = function(callback, tab, entryId, username, password, url) {
    //TODO: Trigger unlock if device is connected but locked
    // Check that the Mooltipass is unlocked
    if(!event.isMooltipassUnlocked()) {
        return;
    }

    if (mooltipass.backend.isBlacklisted(url)) {
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
    mooltipass.device._asynchronous.updateCallback = callback;
    chrome.runtime.sendMessage(mooltipass.device._app.id, request);
};

/**
 * Request credentials for given URL
 * @access backend
 * @param callback function to be triggered on response from device
 * @param tab which triggered the request
 * @param url
 * @param submiturl
 * @param forceCallback
 * @param triggerUnlock
 */
mooltipass.device.retrieveCredentials = function(callback, tab, url, submiturl, forceCallback, triggerUnlock) {
    page.debug("mp.retrieveCredentials(callback, {1}, {2}, {3}, {4})", tab.id, url, submiturl, forceCallback);

    // unset error message
    page.tabs[tab.id].errorMessage = null;

    //TODO: Trigger unlock if device is connected but locked
    // Check that the Mooltipass is unlocked
    if(!event.isMooltipassUnlocked()) {
        if(forceCallback) {
            callback([]);
        }
        return;
    }

    // parse url and check if it is valid
    var parsed_url = mooltipass.backend.extractDomainAndSubdomain(submiturl);
    if(!parsed_url.valid) {
        if(forceCallback) {
            callback([]);
        }
        return;
    }

    //TODO: remove obsolete parameter "context" which is replaced by "domain" (2015-07-27, wait a month)
    request = { getInputs : {context: parsed_url.domain, domain: parsed_url.domain, subdomain: parsed_url.subdomain} };

    console.log('sending to '+mooltipass.device._app.id);
    mooltipass.device._asynchronous.inputCallback = callback;
    chrome.runtime.sendMessage(mooltipass.device._app.id, request);
};

/****************************************************************************************************************/

/* Initialize device specific settings */

/**
 * Initially start searching for the Mooltipass app
 * This also triggers the status request to the device
 */
chrome.management.getAll(mooltipass.device.onSearchForApp);

/**
 * Process messages from content scripts
 */
chrome.extension.onMessage.addListener(function(req, sender, callback) {
    if ('action' in req) {
        if(req.action == "mooltipass.device.addCredentials") {
            mooltipass.device.addCredentials(callback, sender.tab, req.username, req.password, req.url);
        }
    }
});


/**
 * Process messages from the Mooltipass app
 */
chrome.runtime.onMessageExternal.addListener(function(message, sender, sendResponse) {
    // Returned on a PING, contains the status of the device
    if (message.deviceStatus !== null) {
        mooltipass.device._status = {
            'connected': message.deviceStatus.version != 'unknown',
            'unlocked': message.deviceStatus.connected,
            'version': message.deviceStatus.version,
            'state' : message.deviceStatus.state
        };
    }
    // Returned on request for a random number
    else if (message.random !== null) {
        Math.seedrandom(message.random);
        if(mooltipass.device._asynchronous.randomCallback) {
            mooltipass.device._asynchronous.randomCallback({
                'seeds': mooltipass.device.generateRandomNumbers(mooltipass.device._asynchronous.randomParameters.length),
                'settings': page.settings,
            });
        }
    }
    // Returned on successfully requesting credentials for a specific URL
    else if (message.credentials !== null) {
        if (mooltipass.device._asynchronous.inputCallback) {
            mooltipass.device._asynchronous.inputCallback([
                {
                    Login: message.credentials.login,
                    Name: '<name>',
                    Uuid: '<Uuid>',
                    Password: message.credentials.password,
                    StringFields: []
                }
            ]);
            mooltipass.device._asynchronous.inputCallback = null;
        }
    }
    // Returned on requesting credentials for a specific URL, but no credentials were found
    else if (message.noCredentials !== null) {
        if (mooltipass.device._asynchronous.inputCallback) {
            mooltipass.device._asynchronous.inputCallback([]);
            mooltipass.device._asynchronous.inputCallback = null;
        }
    }
    // Returned on a completed update of credentials on the device
    else if (message.updateComplete !== null) {
        if (mooltipass.device._asynchronous.updateCallback) {
            try {
                mooltipass.device._asynchronous.updateCallback('success');
            } catch (e) {
                console.log("Error: " + e);
            }
            mooltipass.device._asynchronous.updateCallback = null;
        }
    }
});

