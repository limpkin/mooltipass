var _if = {};

_if.send = function(inputObject) {
    var command = _if['_'+inputObject.command];
    if(!command) {
        _if._returnError(inputObject, 'error', 100, 'unknown command: ' + inputObject.command);
        return;
    }

    _if['_'+inputObject.command](inputObject);
};

_if._returnError = function(inputObject, status, code, msg) {
    var responseObject = {
        'status': status,
        'code': code,
        'msg': msg
    };
    _mp.applyCallback(inputObject.callbackFunction, inputObject.callbackParameters, [responseObject]);
};


_if._getMooltipassParameter = function(inputObject) {
    var _param = _mp.parameters[inputObject.parameter];
    if(!_param) {
        _if._returnError(inputObject, 'error', 101, 'unknown parameter: ' + inputObject.parameter);
        return;
    }

    var payload = [_param];
    _mp.sendMsg(inputObject.command, payload, inputObject.responseParameters, inputObject.callbackFunction, inputObject.callbackParameters);
};


_if._setMooltipassParameter = function(inputObject) {
    var _param = _mp.parameters[inputObject.parameter];
    if(!_param) {
        _if._returnError(inputObject, 'error', 101, 'unknown parameter' + inputObject.parameter);
        return;
    }
    if(!inputObject.value) {
        _if._returnError(inputObject, 'error', 102, 'no parameter value');
        return;
    }

    var payload = [_param, inputObject.value];
    _mp.sendMsg(inputObject.command, payload, inputObject.responseParameters, inputObject.callbackFunction, inputObject.callbackParameters);
};