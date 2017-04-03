/* Contains all methods which are accessed by the html app interface */
var mooltipass = mooltipass || {};
mooltipass.app = mooltipass.app || {};

// Is app already initialized
mooltipass.app._isInitializedLock = false;
// Platform running the app
mooltipass.app.os = "";


chrome.runtime.onMessage.addListener(
    function(data, sender, callbackFunction) {
        //console.warn('chrome.runtime.onMessage(', data.id, ')');
        mooltipass.app.onMessage(data.id, data.message, callbackFunction);
    });


/**
 * Initialize all app related functions on startup
 */
mooltipass.app.init = function() {
    if(mooltipass.app._isInitializedLock) {
        return false;
    }
    
    /* Find out the OS */
    chrome.runtime.getPlatformInfo(function(info) { mooltipass.app.os = info.os; });

    mooltipass.app._isInitializedLock = true;

    mooltipass.device.init();
    mooltipass.ui._.init();
    mooltipass.ui.settings.init();
    mooltipass.ui.credentials.init();
    mooltipass.ui.sync.init();
    mooltipass.ui.developers.init();
    mooltipass.ui.experts.init();
    mooltipass.ui.contributors.init();

    mooltipass.prefstorage.getStoredPreferences(mooltipass.memmgmt.preferencesCallback);
    mooltipass.prefstorage.getStoredPreferences(mooltipass.datamemmgmt.preferencesCallback);
    mooltipass.filehandler.getSyncableFileSystemStatus(mooltipass.memmgmt.syncableFSStateCallback);
    mooltipass.filehandler.setSyncFSStateChangeCallback(mooltipass.memmgmt.syncableFSStateChangeCallback);

    return true;
};

mooltipass.app.onMessage = function(senderId, data, callbackFunction) {
    var inputObject = data;

    // Backwards compatibility:
    // No attribute command was given
    if(!data.command) {
        inputObject = mooltipass.app.translateRequestForBackwardsCompatibility(data);
    }

    if(inputObject.command == 'getMooltipassStatus') {
        var responseObject = {};
        responseObject.deviceStatus = {};
        responseObject.deviceStatus.version = mooltipass.device.version;
        responseObject.deviceStatus.connected = mooltipass.device.isConnected;
        responseObject.deviceStatus.unlocked = mooltipass.device.isUnlocked;
        
        if ( true === mooltipass.emulator.active ) {
            responseObject.deviceStatus.version = '1.2_emul';
            responseObject.deviceStatus.connected = true;
            responseObject.deviceStatus.unlocked = true;
            responseObject.deviceStatus.state = 'Unlocked';
        } else if(mooltipass.device.status == 'no-card') {
            responseObject.deviceStatus.state = 'NoCard';
        }
        else if(mooltipass.device.status == 'locked') {
            responseObject.deviceStatus.state = 'Locked';
        }
        else if(mooltipass.device.status == 'unlocked') {
            responseObject.deviceStatus.state = 'Unlocked';
        }
        else if(mooltipass.device.singleCommunicationMode) {
            responseObject.deviceStatus.state = 'ManageMode';
        }
        else if(!mooltipass.device.isConnected) {
            responseObject.deviceStatus.state = 'NotConnected';
        }
        else {
            responseObject.deviceStatus.state = 'Error';
        }

        responseObject.deviceStatus.middleware = 'Chrome App';

        chrome.runtime.sendMessage(senderId, responseObject, function() {
            if(chrome.runtime.lastError) {
                // TODO: Chrome 49 returns this error which does not affect the functionality. No real solution found yet (2016-03-18)
                // TODO: Also contains a typo "reponse" instead of "response"
                if(chrome.runtime.lastError.message != "The message port closed before a reponse was received.") {
                    console.warn('Could not send response to client <', senderId, '> #12');
                    console.warn('Error:', chrome.runtime.lastError.message);
                }
            }
        });
        return;
    }
    else if(inputObject.command == 'cancelGetCredentials')
    {
        console.log("Cancel request for reqid " + inputObject.reqid);
        if ( 'undefined' !== typeof mooltipass.emulator.active && true === mooltipass.emulator.active ) 
        {
            mooltipass.emulator._cancelRequest( inputObject );
        } 
        else if (mooltipass.util.getFirmwareFunctionalityVersionFromVersionString(mooltipass.device.version) >= "v1.1" && mooltipass.device.currentReqid == inputObject.reqid)
        {
            // The cancel message doesn't generate any reply from the device, so we can just send it as is
            chrome.hid.send(mooltipass.device.connectionId, 0, mooltipass.device.createPacket(mooltipass.device.commands['cancelUserRequest'], null), function(){});
            console.log("Cancel packet sent");
        }
        //console.log(inputObject);
        return;
    }

    inputObject.callbackFunction = function(_responseObject) {
        _responseObject.command = inputObject.command;
        // Add backwards-compatible data information
        var backwards = mooltipass.app.translateResponseForBackwardsCompatibility(_responseObject);
        // Merge backwards-compatible information into data object
        mergeObjects(backwards, _responseObject);
        //console.log('Response Status:', _responseObject);
        chrome.runtime.sendMessage(senderId, _responseObject, function() {
            // TODO: Chrome 49 returns this error which does not affect the functionality. No real solution found yet (2016-03-18)
            // TODO: Also contains a typo "reponse" instead of "response"
            if(chrome.runtime.lastError.message != "The message port closed before a reponse was received.") {
                console.warn('Could not send response to client <', senderId, '> #37');
                console.warn('Error:', chrome.runtime.lastError.message);
            }
        });
    };

    //console.warn('mooltipass.app.onMessage()', 'inputObject:', inputObject);
    if(!inputObject.responseParameters) {
        inputObject.responseParameters = {};
    }
    inputObject.responseParameters.senderId = senderId;

    mooltipass.device.clients.add(senderId);
    mooltipass.device.interface.send(inputObject);
};

mooltipass.app.updateOnUnlock = function() {

};

mooltipass.app.updateOnConnect = function() {
    mooltipass.ui._.reset();
    mooltipass.ui.settings.getSettings();
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
        output.context = _request.update.context;
        output.username = _request.update.login;
        output.password = _request.update.password;
    }
    else if('getInputs' in _request) {
        // { getInputs : {context: parsed_url.domain, domain: parsed_url.domain, subdomain: parsed_url.subdomain} }
        output.command = 'getCredentials';
        output.contexts = [];
        if(_request.getInputs.subdomain && _request.getInputs.domain) {
            output.contexts.push(_request.getInputs.subdomain + '.' + _request.getInputs.domain);
        }
        if(_request.getInputs.domain) {
            output.contexts.push(_request.getInputs.domain);
        }
        output.reqid = _request.getInputs.reqid;
    }
    else if('cancelGetInputs' in _request)
    {
        output.command = 'cancelGetCredentials';
        output.contexts = [];
        if(_request.cancelGetInputs.subdomain && _request.cancelGetInputs.domain) {
            output.contexts.push(_request.cancelGetInputs.subdomain + '.' + _request.cancelGetInputs.domain);
        }
        if(_request.cancelGetInputs.domain) {
            output.contexts.push(_request.cancelGetInputs.domain);
        }
        output.reqid = _request.cancelGetInputs.reqid;
    }

    return output;
};

mooltipass.app.translateResponseForBackwardsCompatibility = function(_response) {
    var output = {};

    var command = _response.command;

    output.random = null;
    output.deviceStatus = null;
    output.credentials = null;
    output.noCredentials = null;
    output.updateComplete = null;
    
    if(_response.success && command == 'getRandomNumber') {
        output.random = _response.value;
    }
    else if(command == 'getCredentials') {
        if(_response.success) {
            output.credentials = {
                'login': _response.username || "",
                'password': _response.password || ""
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