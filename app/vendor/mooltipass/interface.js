var mooltipass = mooltipass || {};
mooltipass.device = mooltipass.device || {};

mooltipass.device.interface = mooltipass.device.interface || {};

/*
An inputObject consists AT LEAST of the following attributes
inputObject = {
    'command': '',                          // Command to execute 'ping', 'getStatus' -> mooltipass.device.commands
                                               Required
    'responseParameters': ['', ...],        // Parameters which shall be used while processing the response in mooltipass.device.responseCOMMAND()
                                               Optional, can be []
    'callbackFunction': function() {},      // Callback function which is called after processing the response
                                               Optional, can be null
    'callbackParameters': ['', ...],        // Callback parameters for callback function
                                               Optional, can be []
    'timeout': {                            // Timeout object used to trigger retries
                                               Optional, can be null
        'milliseconds': 1000,               // Milliseconds after which the retry should be started, 1000 = 1 second
                                               Required, if timeout is set
        'retries': 3,                       // Number of retries
                                               Required, if timeout is set
        'callbackFunction': function() {}   // Callback function which is called in case of failed communication
                                               Optional, can be null
    }
}

For example, for getMooltipassParameter and setMooltipassParameter additional attributes like 'parameter' and 'value' exists.
Please check the mooltipass.device.interface._COMMAND function for additional attributes

*/


mooltipass.device.interface.send = function(inputObject) {
    var command = mooltipass.device.interface['_'+inputObject.command];
    if(!command) {
        mooltipass.device.interface._returnError(inputObject, 80, 'unknown command: ' + inputObject.command);
        return;
    }

    if(!mooltipass.device.isConnected) {
        mooltipass.device.interface._returnError(inputObject, 70, 'device not connected');
        return;
    }

    if(mooltipass.device.singleCommunicationMode) {
        mooltipass.device.interface._returnError(inputObject, 90, 'device blocks new communication');
        return;
    }

    mooltipass.device.interface['_'+inputObject.command](inputObject);
};


mooltipass.device.interface._returnError = function(inputObject, code, msg) {
    var responseObject = {
        'success': false,
        'code': code,
        'msg': msg
    };
    mooltipass.device.applyCallback(inputObject.callbackFunction, inputObject.callbackParameters, [responseObject]);
};


mooltipass.device.interface._sendFromListener = function(message, sender, callbackFunction) {
    var inputObject = message;
    inputObject.callbackFunction = callbackFunction;

    mooltipass.device.interface.send(inputObject);
};


mooltipass.device.interface._ping = function(inputObject) {
    mooltipass.device.addToQueue(
        inputObject.command,
        [],
        inputObject.responseParameters,
        inputObject.callbackFunction,
        inputObject.callbackParameters,
        inputObject.timeout
    );
};


mooltipass.device.interface._startMemoryManagementMode = function(inputObject) {
    mooltipass.device.addToQueue(
        inputObject.command,
        [],
        null,
        inputObject.callbackFunction,
        null,
        null
    );
};


mooltipass.device.interface._startSingleCommunicationMode = function(inputObject) {
    mooltipass.device.addToQueue(
        inputObject.command,
        [],
        null,
        inputObject.callbackFunction,
        inputObject.callbackParameters,
        null,
        false,
        {
            'reason': inputObject.reason,
            'callbackFunctionStart': inputObject.callbackFunctionStart
        }
    );
};


mooltipass.device.interface._getMooltipassStatus = function(inputObject) {
    mooltipass.device.addToQueue(
        inputObject.command,
        [],
        inputObject.responseParameters,
        inputObject.callbackFunction,
        inputObject.callbackParameters,
        inputObject.timeout
    );
};


mooltipass.device.interface._getRandomString = function(inputObject) {
    mooltipass.device.addToQueue(
        inputObject.command,
        [],
        inputObject.responseParameters,
        inputObject.callbackFunction,
        inputObject.callbackParameters,
        inputObject.timeout
    );
};


mooltipass.device.interface._getMooltipassParameter = function(inputObject) {
    var _param = mooltipass.device.parameters[inputObject.parameter];
    if(!_param) {
        mooltipass.device.interface._returnError(inputObject, 101, 'unknown parameter: ' + inputObject.parameter);
        return;
    }

    var payload = [_param];
    mooltipass.device.addToQueue(
        inputObject.command,
        payload,
        inputObject.responseParameters,
        inputObject.callbackFunction,
        inputObject.callbackParameters,
        inputObject.timeout
    );
};


mooltipass.device.interface._setMooltipassParameter = function(inputObject) {
    var _param = mooltipass.device.parameters[inputObject.parameter];
    if(!_param) {
        mooltipass.device.interface._returnError(inputObject, 101, 'unknown parameter' + inputObject.parameter);
        return;
    }
    if(!inputObject.value && inputObject.value != 0) {
        mooltipass.device.interface._returnError(inputObject, 102, 'no parameter value');
        return;
    }

    var payload = [_param, inputObject.value];
    mooltipass.device.addToQueue(
        inputObject.command,
        payload,
        inputObject.responseParameters,
        inputObject.callbackFunction,
        inputObject.callbackParameters,
        inputObject.timeout
    );
};