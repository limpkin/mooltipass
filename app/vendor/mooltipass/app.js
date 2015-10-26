/* Contains all methods which are accessed by the html app interface */
var mooltipass = mooltipass || {};
mooltipass.app = mooltipass.app || {};

// Is app already initialized
mooltipass.app._isInitializedLock = false;


chrome.runtime.onMessage.addListener(
    function(data, sender, callbackFunction) {
        console.warn('chrome.runtime.onMessage(', data.id, ')');
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
    mooltipass.prefstorage.getStoredPreferences(mooltipass.datamemmgmt.preferencesCallback);
    mooltipass.filehandler.getSyncableFileSystemStatus(mooltipass.memmgmt.syncableFSStateCallback);
    mooltipass.filehandler.setSyncFSStateChangeCallback(mooltipass.memmgmt.syncableFSStateChangeCallback);

    return true;
};

mooltipass.app.onMessage = function(senderId, data, callbackFunction) {
    var inputObject = data;
    inputObject.callbackFunction = callbackFunction;

    // Backwards compatibility:
    // No attribute command was given
    if(!data.command) {
        inputObject = mooltipass.app.translateRequestForBackwardsCompatibility(data);
        inputObject.callbackFunction = function(_responseObject) {
            var data = mooltipass.app.translateResponseForBackwardsCompatibility(_responseObject);
            console.warn(_responseObject);
            console.warn(data);
            chrome.runtime.sendMessage(senderId, data, function() {
                if(chrome.runtime.lastError) {
                    console.warn('Could not send response to client <', senderId, '>');
                    console.warn('Error:', chrome.runtime.lastError.message);
                }
            });
        };
    }

    //console.warn('mooltipass.app.onMessage()', 'inputObject:', inputObject);
    if(!inputObject.responseParameters) {
        inputObject.responseParameters = {};
    }
    inputObject.responseParameters.senderId = senderId;

    mooltipass.device.clients.add(senderId);
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

mooltipass.app.getPassword = function(_context, _username, _callback) {
    mooltipass.memmgmt.getPasswordForCredential(_context, _username, function(_status, _password) {
        _callback(_context, _username, _status, _password);
    });
};

mooltipass.app.translateRequestForBackwardsCompatibility = function(_request) {
    //console.log('mooltipass.app.translateRequestForBackwardsCompatibility()', _request);
    var output = {};

    // Get random number
    if('getRandom' in _request) {
        // { getRandom : [] }
        output.command = 'getRandomNumber';
    }
    else if('ping' in _request) {
        // { ping: [] }
        output.command = 'getMooltipassStatus';
    }
    else if('update' in _request) {
        // {update: {context: url, login: username, password: password}}
        output.command = 'updateCredentials';
        output.data = {
            'context': _request.context,
            'username': _request.login,
            'password': _request.password,
        };
    }
    else if('getInputs' in _request) {
        // { getInputs : {context: parsed_url.domain, domain: parsed_url.domain, subdomain: parsed_url.subdomain} }
        output.command = 'getCredentials';
        output.contexts = [_request.getInputs.subdomain, _request.getInputs.domain];
    }

    return output;
};

mooltipass.app.translateResponseForBackwardsCompatibility = function(_response) {
    var output = {};

    var command = _response.command;

    if(_response.success && command == 'getRandomNumber') {
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
        else if(!mooltipass.device.isConnected) {
            output.deviceStatus.state = 'Disconnected';
        }
        else {
            output.deviceStatus.state = 'Error';
        }
    }
    else if(_response.success && command == 'getCredentials') {
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
    else if(_response.success && (command == 'addCredentials' || command == 'updateCredentials')) {
        output.updateComplete = true;
    }

    return output;
};

$(function() {
    mooltipass.app.init();
});