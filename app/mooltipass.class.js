var _mp =  {};

// Debug mode
_mp.debug = true;

// Mooltipass device info
_mp.deviceInfo = { 'vendorId': 0x16d0, 'productId': 0x09a0 };

// Number of bytes of a packet transferred over USB is fixed to 64
_mp.packetSize = 64;

// First 2 bytes contain packet length and command
_mp.payloadSize = _mp.packetSize - 2;

// Available command codes for Mooltipass
_mp.commands = {
    'debug'                         : 0xA0,
    'ping'                          : 0xA1,
    'getVersion'                    : 0xA2,
    'setContext'                    : 0xA3,
    'getLogin'                      : 0xA4,
    'getPassword'                   : 0xA5,
    'setLogin'                      : 0xA6,
    'setPassword'                   : 0xA7,
    'checkPassword'                 : 0xA8,
    'addContext'                    : 0xA9,
    'getRandomNumber'               : 0xAC,
    'startMemoryManagementMode'     : 0xAD,
    'startMediaImport'              : 0xAE,
    'mediaImport'                   : 0xAF,
    'endMediaImport'                : 0xB0,
    'setMooltipassParameter'        : 0xB1,
    'getMooltipassParameter'        : 0xB2,
    'resetCard'                     : 0xB3,
    'getCardLogin'                  : 0xB4,
    'getCardPassword'               : 0xB5,
    'setCardLogin'                  : 0xB6,
    'setCardPassword'               : 0xB7,
    'addUnknownCard'                : 0xB8,
    'getMooltipassStatus'           : 0xB9,
    'setCurrentDate'                : 0xBB,
    'setMooltipassUID'              : 0xBC,
    'getMooltipassUID'              : 0xBD,
    'setDataContext'                : 0xBE,
    'addDataContext'                : 0xBF,
    'write32BytesInCurrentContext'  : 0xC0,
    'read32BytesInCurrentContext'   : 0xC1,
    'getCurrentCardCPZ'             : 0xC2,
    'cancelUserRequest'             : 0xC3,
    'readNodeInFlash'               : 0xC5,
    'writeNodeInFlash'              : 0xC6,
    'getFavorite'                   : 0xC7,
    'setFavorite'                   : 0xC8,
    'getStartingParentAddress'      : 0xC9,
    'setStartingParentAddress'      : 0xCA,
    'getCTR'                        : 0xCB,
    'setCTR'                        : 0xCC,
    'addCPZandCTR'                  : 0xCD,
    'getCPZandCTR'                  : 0xCE,
    'exportCPZandCTR'               : 0xCF,
    'getFreeSlotAddresses'          : 0xD0,
    'getStartingDataParentAddress'  : 0xD1,
    'setStartingDataParentAddress'  : 0xD2,
    'endMemoryManagementMode'       : 0xD3,
};

/**
 * Switches keys and values of _mp.commands.
 * Used to generate the function name for individual response handling
 * This object is automatically filled on startup
 */
_mp.commandsReverse = {};

// Generic responses from the device
_mp.responses = {
    'error'                         : 0x00,
    'success'                       : 0x01,
    'noCard'                        : 0x03,
    'pleaseRetry'                   : 0xC4,
};

// Available Mooltipass parameters
_mp.parameters = {
    'keyboardLayout': 1,
    'userInteractionTimeout': 2,
    'lockTimeoutEnabled': 3,
    'lockTimeout': 4,
    'offlineMode': 8,
    'screensaver': 9,
    'flashScreen': 14,
    'userRequestCancel': 15,
    'tutorialEnabled': 16,
};

_mp.status = {
    /*
    0b000: 'noCard',
    0b001 -> Locked
    0b010 -> Error (shouldn't happen)
    0b011 -> Locked (unlocking screen)
    0b100 -> Error (shouldn't happen)
    0b101 -> Unlocked
    0b110 -> Error (shouldn't happen)
    0b111 -> Error (shouldn't happen)
    */
}

// Connection ID for communication with HID device
_mp.connectionId = null;

// Information about established connection
_mp.isConnected = false;

// External client to communicate with (e.g. an app)
_mp.clientId = null;

// Queue for executing commands
_mp.queue = []


/*********************************************************************************************************************/

_mp.getFromQueue = function(command) {
    if(command) {
        console.log('COMMAND', command, '(length queue: ', _mp.queue.length, ')')
        for(var i = 0; i < _mp.queue.length; i++) {
            if(_mp.queue[i].command == command) {
                var result = _mp.queue.splice(i, 1);
                console.log('.... returned.    (length queue: ', _mp.queue.length, ')');
                return result[0];
            }
        }
    }
    return _mp.queue.shift();
}

_mp.addToQueue = function(command, payload, responseParameters, callbackFunction, callbackParameters) {
    // Add only one ping to the queue
    if(command == 'ping') {
        for (var i = 0; i < _mp.queue.length; i++) {
            if(_mp.queue[i].command == 'ping') {
                return;
            }
        }
    }

    _mp.queue.push({'command': command, 'payload': payload, 'responseParameters': responseParameters, 'callbackFunction': callbackFunction, 'callbackParameters': callbackParameters});
    return true;
}


_mp.reset = function() {
    _mp.connectionId = null;
    _mp.isConnected = false;
};

/**
 * Connect to HID device if no connection is established
 */
_mp.connect = function() {
    if (_mp.isConnected) {
        return;
    }

    _mp.reset();
    chrome.hid.getDevices(_mp.deviceInfo, _mp.onDeviceFound);
    return true;
};

/**
 * Callback of chrome.hid.getDevices()
 * Gets a list of all matched devices and tries to connect to first device in list
 * @param devices
 */
_mp.onDeviceFound = function(devices) {
    if (!devices || !devices.length) {
        console.log('No compatible devices found.');
        return;
    }

    var device = devices[0];
    log('Found', devices.length, 'devices.');
    log('Device', device.deviceId,', vendor', device.vendorId, ', product', device.productId);

    log('Connecting to device', device.deviceId);

    chrome.hid.connect(device.deviceId, _mp.onConnectFinished);
};

/**
 * Callback of chrome.hid.connect()
 * Set connection ID for communication and execute original command
 * @param connectInfo
 */
_mp.onConnectFinished = function(connectInfo) {
    if (chrome.runtime.lastError) {
        log('Failed to connect to device: ', chrome.runtime.lastError.message);
        _mp.reset();
        return;
    }

    _mp.connectionId = connectInfo.connectionId;
    _mp.isConnected = true;

    log('Connected to device');

    var pipelinedData = _mp.getFromQueue();
    if (pipelinedData) {
        _mp.sendMsg(pipelinedData.command, pipelinedData.payload, pipelinedData.responseParameters, pipelinedData.callbackFunction, pipelinedData.callbackParameters);
    }
};

/**
 * Helper function to create a valid packet for communication with the device.
 * @param command
 * @param payload
 * @returns {ArrayBuffer}
 */
_mp.createPacket = function(command, payload) {
    var buffer = new ArrayBuffer(_mp.packetSize);
    var data = new Uint8Array(buffer);

    if (payload) {
        data.set([payload.length, command], 0);
        data.set(payload, 2);
    }
    else {
        data.set([0, command], 0);
    }

    for (var i = 0; i < data.byteLength; i++) {
        buffer[i] = data[i];
    }

    return buffer;
};

/**
 * Convert a uint8 array to string
 * @param uint8Array the array to convert
 * @return the string representation of the array
 * @note does not support unicode yet
 */
_mp.convertMessageArrayToString = function(uint8Array) {
    var output = '';
    for (var i=0; i < uint8Array.length; i++) {
        if (uint8Array[i] == 0) {
            return output;
        }
        else {
            output += String.fromCharCode(uint8Array[i]);
        }
    }
    return output;
}

/**
 * Send message to device.
 * Generates a valid packet from given command and payload and sends it to the device
 * @param command
 * @param payload
 * @param callbackFunction
 * @param callbackParameters
 */
_mp.sendMsg = function(command, payload, responseParameters, callbackFunction, callbackParameters) {
    if(!_mp.addToQueue(command, payload, responseParameters, callbackFunction, callbackParameters)) {
        return;
    }
    if(!_mp.isConnected) {
        if(_mp.connect()) {
            return;
        }
    }

    var packet = _mp.createPacket(_mp.commands[command], payload);

    if (_mp.debug) {
        msgUint8 = new Uint8Array(packet);
        // don't output the CMD_VERSION command since this is the keep alive
        if (msgUint8[1] != _mp.commands.version) {
            console.log('sendMsg(', JSON.stringify(new Uint8Array(packet)), ')');
        }
    }

    chrome.hid.send(_mp.connectionId, 0, packet, _mp.onSendMsg);
};

/**
 * Callback of chrome.hid.send()
 * It sets a callback function for receiving data
 */
_mp.onSendMsg = function() {
    if (chrome.runtime.lastError) {
        if (_mp.isConnected) {
            if (_mp.debug) {
                console.log('Failed to send to device: '+chrome.runtime.lastError.message);
            }
            if (_mp.clientId) {
                chrome.runtime.sendMessage(_mp.clientId, {type: 'disconnected'});
            }
            _mp.reset();
        }
        return;
    }

    chrome.hid.receive(_mp.connectionId, _mp.onDataReceived);
};

/**
 * Callback of chrome.hid.receive()
 * Handler for receiving new data from the device.
 * Decodes the HID message and updates the HTML message divider with
 * to report the received message.
 * @param reportId
 * @param data the received data
 */
_mp.onDataReceived = function(reportId, data) {
    if (typeof reportId === 'undefined' || typeof data === 'undefined') {
        console.log('undefined response');
        if (chrome.runtime.lastError) {
            var error = chrome.runtime.lastError;
            if (error.message != 'Transfer failed.') {
                console.log('Error in onDataReceived:', error.message);
            }
        }
        return;
    }

    var bytes = new Uint8Array(data);
    var msg = new Uint8Array(data, 2);
    var len = bytes[0];
    var cmd = bytes[1];

    var command = _mp.commandsReverse[cmd];

    var queuedItem = _mp.getFromQueue(command);
    var handlerName = 'response' + capitalizeFirstLetter(command);

    console.log(handlerName);

    console.log('reportId', reportId);
    console.log('queuedItem', queuedItem);
    console.log('data', data);
    console.log('bytes', bytes);
    console.log('msg', msg);
    console.log('len', len);
    console.log('cmd', cmd);
    console.log('command', command);

    // Invoke function to process message
    if(handlerName in _mp) {
        _mp[handlerName].apply(this, [queuedItem, msg]);
    }
    else {
        _mp.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, []);
    }
};

_mp.applyCallback = function(callbackFunction, callbackParameters, ownParameters) {
    if(callbackFunction) {
        var args = callbackParameters || [];
        args = args.concat(ownParameters || []);
        callbackFunction.apply(this, args);
    }
}

_mp.responseGetVersion = function(queuedItem, msg) {
    var version = _mp.convertMessageArrayToString(msg);
    var flashChipId = msg[0];

    log('Connected to Mooltipass', version, ', flashId', flashChipId);

    return;

    if (!connected) {
        connected = true;
        if (clientId) {
            chrome.runtime.sendMessage(clientId, {type: 'connected', version: version});
        }
    }
}

_mp.responsePing = function(queuedItem, msg) {
    console.log('Process PING command');
};

_mp.responseGetMooltipassParameter = function(queuedItem, msg) {
    console.log('Process getMooltipassParameter command');

    var responseObject = {
        'status': 'success',
        'payload': queuedItem.payload,
        'value': msg[0]
    };

    log('getMooltipassParameter(', queuedItem.payload, ') =', msg[0]);

    console.log('queuedItem', queuedItem);

    _mp.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
};

_mp.responseSetMooltipassParameter = function(queuedItem, msg) {
    console.log('Process setMooltipassParameter command');

    var success = msg[0] == 1;

    var responseObject = {
        'status': (success) ? 'success' : 'error',
        'payload': queuedItem.payload
    };

    if(!success) {
        responseObject['code'] = 601;
        responseObject['msg'] = 'request was not performed';
    }
    _mp.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
};



/*********************************************************************************************************************/


var keys = Object.keys(_mp.commands);
_mp.commandsReverse = {};
for(var i = 0; i < keys.length; i++) {
    _mp.commandsReverse[_mp.commands[keys[i]]] = keys[i];
}