/* Initialize mooltipass lib */
if (typeof mooltipass == 'undefined') {
    mooltipass = {};
}

mooltipass.device = mooltipass.device || {};

mooltipass.device.latestFirmwareVersionUrl = 'https://raw.githubusercontent.com/limpkin/mooltipass/master/authentication_clients/chromeipass.ext/manifest.json';

mooltipass.device.latestFirmware = (typeof(localStorage.latestFirmware) == 'undefined')
    ? {"version": 0, "versionParsed": 0, "lastChecked": null}
    : JSON.parse(localStorage.latestFirmware);

mooltipass.device.currentFirmware = {'version': 0, 'versionParsed': 0};


/* library functions for mooltipass.device ********************** */

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
    if(!mooltipass.connectedToApp) {
        // Search for Mooltipass App
        chrome.management.getAll(mooltipass.device.onSearchForApp);
        return;
    }

    chrome.runtime.sendMessage(mooltipass.app.id, { ping: [] });
    setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
};

/**
 * Searches for mooltipass app in all available chrome apps
 * Triggers ping to device if app is found and sets mooltipass.app
 */
mooltipass.device.onSearchForApp = function(ext) {
    var foundApp = false;
    for (var i = 0; i < ext.length; i++) {
        if (ext[i].shortName == mooltipass.device._appName) {
            mooltipass.app = ext[i];
            foundApp = true;
            break;
        }
    }

    if(!foundApp) {
        mooltipass.app = null;
    }

    if (mooltipass.app != null) {
        mooltipass.connectedToApp = true;
        // Send ping which triggers status response from device
        chrome.runtime.sendMessage(mooltipass.app.id, { ping: [] });

        console.log('found mooltipass app "' + mooltipass.app.shortName + '" id=' + mooltipass.app.id,' app: ', mooltipass.app);
    }
    else {
        mooltipass.connectedToApp = false;
        mooltipass.deviceStatus = {};
        console.log('No mooltipass app found');
    }

    setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
};


/**
 * Initially start search for Mooltipass app
 * This also triggers the status request to the device
 */
chrome.management.getAll(mooltipass.device.onSearchForApp);





/**
 * Returns the current status of the connection to the device
 * @access backend
 * @returns {{connectedToApp: boolean, connectedToDevice: boolean, deviceUnlocked: boolean}}
 */
mooltipass.device.getStatus = function() {
    return {
        'connectedToApp': mooltipass.app ? true : false,
        'connectedToDevice': mooltipass.deviceStatus.connected,
        'deviceUnlocked': mooltipass.deviceStatus.unlocked
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
 * Return current firmware version
 * @access backend
 * @returns {string}
 */
mooltipass.device.getFirmwareVersion = function() {
    if (mooltipass.deviceStatus.version) {
        return mooltipass.deviceStatus.version;
    }
    else {
        return 'not connected';
    }
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
        chrome.runtime.sendMessage(mooltipass.app.id, request);
        return;
    }

    console.log('mooltipass.generatePassword()', 'use current seed for another password');
    callback({'seeds': mooltipass.device.generateRandomNumbers(length)});
};

/**
 * Based on a salted Math.random() generate random numbers
 * @param length number of random numbers to generate
 * @returns {Array} array of Numbers
 */
mooltipass.device.generateRandomNumbers = function(length) {
    var seeds = [];
    for(var i = 0; i < length; i++) {
        seeds.push(Math.random());
    }

    return seeds;
}