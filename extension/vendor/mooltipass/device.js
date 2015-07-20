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
 * Returns the current status of the connection to the device
 * @access backend
 * @returns {{connectedToApp: boolean, connectedToDevice: boolean, deviceUnlocked: boolean}}
 */
mooltipass.device.getStatus = function() {
    return {
        'connectedToApp': mooltipass.app ? true : false,
        'connectedToDevice': mooltipass.deviceStatus.connected,
        'deviceUnlocked': mooltipass.deviceStatus.unlocked,
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
 * Get current firmware version
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
 * Checks a remote source for the latest version number of the firmware
 * @access backend
 */
mooltipass.device.getLatestFirmwareVersion = function() {
    //TODO: mocked, bcs wrong URL
    mooltipass.device.latestFirmware = {'version': 'v1', 'versionParsed': 1, 'lastChecked': new Date()};
    return

    var xhr = new XMLHttpRequest();
    xhr.open("GET", mooltipass.device.latestFirmwareVersionUrl, false);
    xhr.setRequestHeader("Content-Type", "application/json");
    var version = -1;
    try {
        xhr.send();
        manifest = JSON.parse(xhr.responseText);
        version = manifest.version;
        mooltipass.device.latestFirmware.version = version;
        mooltipass.device.latestFirmware.versionParsed = parseInt(version.replace(/[\.a-zA-Z]/g,''));
    } catch (e) {
        console.log("Error: " + e);
    }

    mooltipass.device.latestFirmware.lastChecked = new Date();

    if (version != -1) {
        localStorage.latestFirmware = JSON.stringify(mooltipass.device.latestFirmware);
    }
}