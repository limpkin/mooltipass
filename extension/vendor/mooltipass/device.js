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