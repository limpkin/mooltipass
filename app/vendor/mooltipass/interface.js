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
        mooltipass.device.interface._returnError(inputObject, 'error', 100, 'unknown command: ' + inputObject.command);
        return;
    }

    mooltipass.device.interface['_'+inputObject.command](inputObject);
};


mooltipass.device.interface._returnError = function(inputObject, status, code, msg) {
    var responseObject = {
        'status': status,
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


mooltipass.device.interface._getMooltipassParameter = function(inputObject) {
    var _param = mooltipass.device.parameters[inputObject.parameter];
    if(!_param) {
        mooltipass.device.interface._returnError(inputObject, 'error', 101, 'unknown parameter: ' + inputObject.parameter);
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
        mooltipass.device.interface._returnError(inputObject, 'error', 101, 'unknown parameter' + inputObject.parameter);
        return;
    }
    if(!inputObject.value) {
        mooltipass.device.interface._returnError(inputObject, 'error', 102, 'no parameter value');
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