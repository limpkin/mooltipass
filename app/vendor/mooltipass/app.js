/* Contains all methods which are accessed by the html app interface */
var mooltipass = mooltipass || {};
mooltipass.app = mooltipass.app || {};

// Is app already initialized
mooltipass.app._isInitializedLock = false;


chrome.runtime.onMessage.addListener(
    function(data, sender, callbackFunction) {
        mooltipass.app.onMessage(data.id, data.message, callbackFunction);
    });

/**
 * Initialize all app related functions on startup
 */
mooltipass.app.init = function() {
    if(mooltipass.app._isInitializedLock) {
        return false;
    }

    mooltipass.app._isInitializedLock = true;

    mooltipass.device.init();
    mooltipass.ui._.init();
    mooltipass.ui.settings.init();
    mooltipass.ui.credentials.init();
    mooltipass.ui.sync.init();
    mooltipass.ui.easteregg.init();
    mooltipass.ui.contributors.init();

    mooltipass.prefstorage.getStoredPreferences(mooltipass.memmgmt.preferencesCallback);
    mooltipass.filehandler.getSyncableFileSystemStatus(mooltipass.memmgmt.syncableFSStateCallback);
    mooltipass.filehandler.setSyncFSStateChangeCallback(mooltipass.memmgmt.syncableFSStateChangeCallback);

    return true;
};

mooltipass.app.onMessage = function(senderId, data, callbackFunction) {
    var inputObject = data;
    inputObject.callbackFunction = callbackFunction;

    // Backwards compatibility:
    // No callbackFunction was given -> send JSON object to sender
    if(!callbackFunction) {
        inputObject = mooltipass.app.translateRequestForBackwardsCompatibility(data);
        inputObject.callbackFunction = function(_responseObject) {
            var data = mooltipass.app.translateResponseForBackwardsCompatibility(_responseObject);
            chrome.runtime.sendMessage(senderId, data);
        };
    }

    mooltipass.device.addNewClient(senderId);
    mooltipass.device.interface.send(inputObject);
};

mooltipass.app.updateOnUnlock = function() {
    mooltipass.ui.settings.getSettings();
};

mooltipass.app.updateOnConnect = function() {
    mooltipass.ui._.reset();
};

mooltipass.app.updateOnLock = function() {
    mooltipass.device.endSingleCommunicationMode();
};

mooltipass.app.get_password = function(_context, _username, _callback) {
    mooltipass.memmgmt.getPasswordForCredential(_context, _username, function(_status, _password) {
        _callback(_context, _username, _status, _password);
    });
};

mooltipass.app.translateRequestForBackwardsCompatibility = function(_request) {
    var output = {};

    // Get random number
    if('getRandom' in _request) {
        output.command = 'getRandomNumber';
    }
    else if('ping' in _request) {
        output.command = 'ping';
    }
    else if('type' in _request && _request.type == 'update') {
        output.command = 'updateCredentials';
        output.payload = {
            'context': _request.context,
            'username': _request.login,
            'password': _request.password,
        };
    }
    else if('getInputs' in _request) {
        output.command = 'getCredentials';
        output.payload = [_request.getInputs.subdomain, _request.getInputs.domain];
    }

    return output;
};

mooltipass.app.translateResponseForBackwardsCompatibility = function(_response) {
    var output = {};

    // If request was not successful, return an empty object
    if(!_response.success) {
        return output;
    }

    var command = _response.command;

    if(command == 'getRandomNumber') {
        output.random = _response.value;
    }
    else if(command == 'getMooltipassStatus') {
        output.deviceStatus = {};
        output.deviceStatus.version = mooltipass.device.version;
        output.deviceStatus.connected = mooltipass.device.isUnlocked;
        if(mooltipass.device.status == 'no-card') {
            output.deviceStatus.state = 'NoCard';
        }
        else if(mooltipass.device.status == 'locked') {
            output.deviceStatus.state = 'Locked';
        }
        else if(mooltipass.device.status == 'unlocked') {
            output.deviceStatus.state = 'Unlocked';
        }
        else if(mooltipass.device.singleCommunicationMode) {
            output.deviceStatus.state = 'ManageMode';
        }
        else {
            output.deviceStatus.state = 'Error';
        }
    }
    else if(command == 'getCredentials') {
        if(_response.username && _response.password) {
            output.credentials = {
                'login': _response.username,
                'password': _response.password
            }
        }
        else {
            output.noCredentials = true;
        }
    }
    else if(command == 'addCredentials' || command == 'updateCredentials') {
        output.updateComplete = true;
    }

    return output;
};

$(function() {
    mooltipass.app.init();
});