/* Contains all methods which are accessed by the html app interface */
var mooltipass = mooltipass || {};
mooltipass.app = mooltipass.app || {};

// Is app already initialized
mooltipass.app._isInitializedLock = false;

mooltipass.app._deviceStatus = {
    'status': null,
    'connected': false,
    'unlocked': false,
    'locked': true,
    'noCard': true,
    'unknownCard': true,
    'version': null,
    'singleCommunicationMode': false,
    'singleCommunicationModeEntered': false,
    'singleCommunicationReason': null
};

// Available Mooltipass parameters
mooltipass.app._deviceParameters = {
    'keyboardLayout': 1,
    'userInteractionTimeout': 2,
    'lockTimeoutEnabled': 3,
    'lockTimeout': 4,
    'offlineMode': 8,
    'screensaver': 9,
    'flashScreen': 14,
    'userRequestCancel': 15,
    'tutorialEnabled': 16
};


chrome.runtime.onMessage.addListener(
    function(_message, _sender, _callbackFunction) {
        //console.warn('chrome.runtime.onMessage(', data.id, ')');
        mooltipass.app.onInternalMessage(_message, _callbackFunction);
    });


/**
 * Initialize all app related functions on startup
 */
mooltipass.app.init = function() {
    if(mooltipass.app._isInitializedLock) {
        return false;
    }

    mooltipass.app._isInitializedLock = true;

    mooltipass.ui._.init();
    mooltipass.ui.settings.init();
    mooltipass.ui.credentials.init();
    mooltipass.ui.sync.init();
    mooltipass.ui.easteregg.init();
    mooltipass.ui.contributors.init();

    return true;
};

mooltipass.app.onInternalMessage = function(_message, _callbackFunction) {
    if(!('_from' in _message)) {
        console.error('No FROM attribute in message:', _message);
        return;
    }

    if(_message._from == 'frontend') {
        return;
    }

    if(!('_action' in _message)) {
        console.error('No action for message processing provided:', _message);
        return;
    }

    switch(_message._action) {
        case 'status':
            delete _message.success;
            delete _message._action;
            delete _message._from;
            mooltipass.app._deviceStatus = _message;
            update_device_status_classes();
            return;
        case 'update-on-connect':
            mooltipass.app.updateOnConnect();
            return;
        case 'update-on-unlock':
            mooltipass.app.updateOnUnlock();
            return;
        case 'update-on-lock':
            mooltipass.app.updateOnLock();
            return;
        case 'response-getMooltipassParameter':
            var param = ('_parameters' in _message && _message._parameters.length > 0) ? _message._parameters[0] : 'undefined';
            mooltipass.ui.settings.getParameter(_message.success, _message.msg, param, _message.value);
            return;
        case 'response-setMooltipassParameter':
            console.log(_message);
            var param = ('_parameters' in _message && _message._parameters.length > 0) ? _message._parameters[0] : 'undefined';
            mooltipass.ui.settings.setParameter(_message.success, _message.msg, param);
            return;
        case 'response-undefined':
            console.warn('Could not identify command for original request:', _message);
            return;
    }

    console.warn('Unknown internal frontend message received:', _message);
};

mooltipass.app.callbackRequest = function(_responseObject) {
    if(chrome.runtime.lastError) {
        console.error('Error sending request to background:', chrome.runtime.lastError.message);
        return;
    }
    if(!_responseObject.success) {
        console.error('Error #' + String(_responseObject.code), _responseObject.msg);
        return;
    }
}

mooltipass.app.updateOnUnlock = function() {
    mooltipass.ui.settings.getSettings();
};

mooltipass.app.updateOnConnect = function() {
    mooltipass.ui._.reset();
};

mooltipass.app.updateOnLock = function() {
    // currently nothing to do
};

mooltipass.app.getPassword = function(_context, _username, _callback) {
    var inputObject = {
        'action': 'get-password-for-credential',
        'context': _context,
        'username': _username
    }
    chrome.runtime.sendMessage(inputObject, function(_context, _username, _status, _password) {
        if(chrome.runtime.lastError) {
            responseObject = {
                'success': false,
                'code': 802,
                'msg': 'Could not send request to background process.'
            };
        }

        applyCallback(_callback, [], [_context, _username, _status, _password]);
    });
};

$(function() {
    mooltipass.app.init();
});