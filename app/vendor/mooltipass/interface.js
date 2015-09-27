var mooltipass = mooltipass || {};
mooltipass.device = mooltipass.device || {};

mooltipass.device.interface = mooltipass.device.interface || {};

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


mooltipass.device.interface._getMooltipassParameter = function(inputObject) {
    var _param = mooltipass.device.parameters[inputObject.parameter];
    if(!_param) {
        mooltipass.device.interface._returnError(inputObject, 'error', 101, 'unknown parameter: ' + inputObject.parameter);
        return;
    }

    var payload = [_param];
    mooltipass.device.sendMsg(inputObject.command, payload, inputObject.responseParameters, inputObject.callbackFunction, inputObject.callbackParameters);
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
    mooltipass.device.sendMsg(inputObject.command, payload, inputObject.responseParameters, inputObject.callbackFunction, inputObject.callbackParameters);
};