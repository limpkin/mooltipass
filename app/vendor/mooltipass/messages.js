/* Contains all methods which are accessed by the html app interface */
var mooltipass = mooltipass || {};
mooltipass.messages = mooltipass.messages || {};

/**
 * Receives incoming requests from external applications (e.g. extension)
 * @param _senderId
 * @param _message
 * @param _callbackFunction
 */
mooltipass.messages.onExternalMessage = function(_senderId, _message, _callbackFunction) {
    var inputObject = _message;

    // Backwards compatibility:
    // No attribute command was given
    if(!_message.command) {
        inputObject = mooltipass.messages.translateRequestForBackwardsCompatibility(_message);
    }

    if(inputObject.command == 'getMooltipassStatus') {
        var responseObject = {
            'command': inputObject.command,
            'success': true,
            'value': status,
            'connected': mooltipass.device.isConnected,
            'unlocked': mooltipass.device.isUnlocked,
            'locked': !mooltipass.device.isUnlocked,
            'noCard': mooltipass.device.hasNoCard,
            'version': mooltipass.device.version,
        };

        // Add backwards-compatible data information
        var backwards = mooltipass.messages.translateResponseForBackwardsCompatibility(responseObject);
        // Merge backwards-compatible information into data object
        mergeObjects(backwards, responseObject);

        console.log('Response Status:', responseObject);

        chrome.runtime.sendMessage(_senderId, responseObject, function() {
            if(chrome.runtime.lastError) {
                console.warn('Could not send response to client <', _senderId, '>');
                console.warn('Error:', chrome.runtime.lastError.message);
            }
        });
        return;
    }


    inputObject.callbackFunction = function(_responseObject) {
        _responseObject.command = inputObject.command;
        // Add backwards-compatible data information
        var backwards = mooltipass.messages.translateResponseForBackwardsCompatibility(_responseObject);
        // Merge backwards-compatible information into data object
        mergeObjects(backwards, _responseObject);
        console.log('Response Status:', responseObject);
        chrome.runtime.sendMessage(_senderId, _responseObject, function() {
            if(chrome.runtime.lastError) {
                console.warn('Could not send response to client <', _senderId, '>');
                console.warn('Error:', chrome.runtime.lastError.message);
            }
        });
    };

    //console.warn('mooltipass.messages.onMessage()', 'inputObject:', inputObject);
    if(!inputObject.responseParameters) {
        inputObject.responseParameters = {};
    }
    inputObject.responseParameters.senderId = _senderId;

    mooltipass.device.clients.add(_senderId);
    mooltipass.device.interface.send(inputObject);
};


mooltipass.messages.onInternalMessage = function(_message, _callbackFunction) {
    var responseObject = {};

    if(!('_from' in _message)) {
        responseObject = {
            'success': false,
            'code': 1101,
            'msg': 'No FROM attribute in message'
        };
        applyCallback(_callbackFunction, null, [responseObject]);
        return;
    }

    if(_message._from == 'backend') {
        return;
    }

    if(!('_action' in _message)) {
        responseObject = {
            'success': false,
            'code': 1100,
            'msg': 'No action for message processing provided'
        };
        applyCallback(_callbackFunction, null, [responseObject]);
        return;
    }


    var callback = function(_responseObject) {
        _responseObject._from = 'backend';
        _responseObject._action = 'response-' + (_responseObject.command || 'undefined');
        _responseObject._parameters = [];

        for(var i = 1; i < arguments.length; i++) {
            _responseObject._parameters.push(arguments[i]);
        }

        chrome.runtime.sendMessage(_responseObject, function() {
            if(chrome.runtime.lastError) {
                console.error('Could not send response from backend to frontend:', chrome.runtime.lastError.message);
            }
        });
    };

    responseObject.success = true;
    responseObject._from = 'backend';

    switch(_message._action) {
        case 'add-to-queue':
            mooltipass.device.addToQueue(_message.command, _message.payload, _message.responseParameters,
                callback, _message.callbackParameters, _message.timeout,
                _message.addToFirstPosition, _message.additionalArguments);
            applyCallback(_callbackFunction, null, [{'success': true}]);
            return;
        case 'status':
            mooltipass.device.sendStatusToFrontend();
            return;
        case 'get-password-for-credential':
            var context = _message.context;
            var username = _message.username;
            mooltipass.memmgmt.getPasswordForCredential(context, username, function(_status, _password) {
                applyCallback(_callbackFunction, [], [_context, _username, _status, _password]);
            });
            return;
        case 'start-singleCommunicationMode':
            mooltipass.device.startSingleCommunicationMode(_message.reason);
            mooltipass.device.sendStatusToFrontend();
            return;
        case 'end-singleCommunicationMode':
            mooltipass.device.endSingleCommunicationMode(_message.skip || false);
            mooltipass.device.sendStatusToFrontend();
            return;
        case 'entered-singleCommunicationMode':
            mooltipass.device.singleCommunicationModeEntered = true;
            mooltipass.device.sendStatusToFrontend();
            return;
    }

    console.warn('Unknown internal backend message received:', _message);
};



/**
 * Translate an incoming request into the new data format
 * @param _request object containing the old request format
 * @returns object with the new request commands
 */
mooltipass.messages.translateRequestForBackwardsCompatibility = function(_request) {
    //console.log('mooltipass.messages.translateRequestForBackwardsCompatibility()', _request);
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
        if(_request.getInputs.subdomain) {
            output.contexts.push(_request.getInputs.subdomain + '.' + _request.getInputs.domain);
        }
        if(_request.getInputs.domain) {
            output.contexts.push(_request.getInputs.domain);
        }
    }

    return output;
};


/**
 * Converts the current response format into the old response format
 * @param _response object containing the current response format
 * @returns object containing the old and current response format
 */
mooltipass.messages.translateResponseForBackwardsCompatibility = function(_response) {
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
            output.deviceStatus.state = 'NotConnected';
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