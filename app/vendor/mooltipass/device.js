var mooltipass = mooltipass || {};
mooltipass.device = mooltipass.device || {};

// Debug mode
mooltipass.device.debug = true;

// Mooltipass device info
mooltipass.device.deviceInfo = { 'vendorId': 0x16d0, 'productId': 0x09a0 };

// Number of bytes of a packet transferred over USB is fixed to 64
mooltipass.device.packetSize = 64;

// First 2 bytes contain packet length and command
mooltipass.device.payloadSize = mooltipass.device.packetSize - 2;

// Available command codes for Mooltipass
mooltipass.device.commands = {
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
 * Switches keys and values of mooltipass.device.commands.
 * Used to generate the function name for individual response handling
 * This object is automatically filled on startup
 */
mooltipass.device.commandsReverse = {};

// Generic responses from the device
mooltipass.device.responses = {
    'error'                         : 0x00,
    'success'                       : 0x01,
    'noCard'                        : 0x03,
    'pleaseRetry'                   : 0xC4,
};

// Available Mooltipass parameters
mooltipass.device.parameters = {
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

mooltipass.device.status = {
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
mooltipass.device.connectionId = null;

// Information about established connection
mooltipass.device.isConnected = false;

// External client to communicate with (e.g. an app)
mooltipass.device.clientId = null;

// Queue for executing commands
mooltipass.device.queue = []


/*********************************************************************************************************************/

mooltipass.device.getFromQueue = function(command) {
    if(command) {
        console.log('COMMAND', command, '(length queue: ', mooltipass.device.queue.length, ')')
        for(var i = 0; i < mooltipass.device.queue.length; i++) {
            if(mooltipass.device.queue[i].command == command) {
                var result = mooltipass.device.queue.splice(i, 1);
                console.log('.... returned.    (length queue: ', mooltipass.device.queue.length, ')');
                return result[0];
            }
        }
    }
    return mooltipass.device.queue.shift();
}

mooltipass.device.addToQueue = function(command, payload, responseParameters, callbackFunction, callbackParameters) {
    // Add only one ping to the queue
    if(command == 'ping') {
        for (var i = 0; i < mooltipass.device.queue.length; i++) {
            if(mooltipass.device.queue[i].command == 'ping') {
                return;
            }
        }
    }

    mooltipass.device.queue.push({'command': command, 'payload': payload, 'responseParameters': responseParameters, 'callbackFunction': callbackFunction, 'callbackParameters': callbackParameters});
    return true;
}


mooltipass.device.reset = function() {
    mooltipass.device.connectionId = null;
    mooltipass.device.isConnected = false;
};

/**
 * Connect to HID device if no connection is established
 */
mooltipass.device.connect = function() {
    if (mooltipass.device.isConnected) {
        return;
    }

    mooltipass.device.reset();
    chrome.hid.getDevices(mooltipass.device.deviceInfo, mooltipass.device.onDeviceFound);
    return true;
};

/**
 * Callback of chrome.hid.getDevices()
 * Gets a list of all matched devices and tries to connect to first device in list
 * @param devices
 */
mooltipass.device.onDeviceFound = function(devices) {
    if (!devices || !devices.length) {
        console.log('No compatible devices found.');
        return;
    }

    var device = devices[0];
    log('Found', devices.length, 'devices.');
    log('Device', device.deviceId,', vendor', device.vendorId, ', product', device.productId);

    log('Connecting to device', device.deviceId);

    chrome.hid.connect(device.deviceId, mooltipass.device.onConnectFinished);
};

/**
 * Callback of chrome.hid.connect()
 * Set connection ID for communication and execute original command
 * @param connectInfo
 */
mooltipass.device.onConnectFinished = function(connectInfo) {
    if (chrome.runtime.lastError) {
        log('Failed to connect to device: ', chrome.runtime.lastError.message);
        mooltipass.device.reset();
        return;
    }

    mooltipass.device.connectionId = connectInfo.connectionId;
    mooltipass.device.isConnected = true;

    log('Connected to device');

    var pipelinedData = mooltipass.device.getFromQueue();
    if (pipelinedData) {
        mooltipass.device.sendMsg(pipelinedData.command, pipelinedData.payload, pipelinedData.responseParameters, pipelinedData.callbackFunction, pipelinedData.callbackParameters);
    }
};

/**
 * Helper function to create a valid packet for communication with the device.
 * @param command
 * @param payload
 * @returns {ArrayBuffer}
 */
mooltipass.device.createPacket = function(command, payload) {
    var buffer = new ArrayBuffer(mooltipass.device.packetSize);
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
mooltipass.device.convertMessageArrayToString = function(uint8Array) {
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
mooltipass.device.sendMsg = function(command, payload, responseParameters, callbackFunction, callbackParameters) {
    if(!mooltipass.device.addToQueue(command, payload, responseParameters, callbackFunction, callbackParameters)) {
        return;
    }
    if(!mooltipass.device.isConnected) {
        if(mooltipass.device.connect()) {
            return;
        }
    }

    var packet = mooltipass.device.createPacket(mooltipass.device.commands[command], payload);

    if (mooltipass.device.debug) {
        msgUint8 = new Uint8Array(packet);
        // don't output the CMD_VERSION command since this is the keep alive
        if (msgUint8[1] != mooltipass.device.commands.version) {
            console.log('sendMsg(', JSON.stringify(new Uint8Array(packet)), ')');
        }
    }

    chrome.hid.send(mooltipass.device.connectionId, 0, packet, mooltipass.device.onSendMsg);
};

/**
 * Callback of chrome.hid.send()
 * It sets a callback function for receiving data
 */
mooltipass.device.onSendMsg = function() {
    if (chrome.runtime.lastError) {
        if (mooltipass.device.isConnected) {
            if (mooltipass.device.debug) {
                console.log('Failed to send to device: '+chrome.runtime.lastError.message);
            }
            if (mooltipass.device.clientId) {
                chrome.runtime.sendMessage(mooltipass.device.clientId, {type: 'disconnected'});
            }
            mooltipass.device.reset();
        }
        return;
    }

    chrome.hid.receive(mooltipass.device.connectionId, mooltipass.device.onDataReceived);
};

/**
 * Callback of chrome.hid.receive()
 * Handler for receiving new data from the device.
 * Decodes the HID message and updates the HTML message divider with
 * to report the received message.
 * @param reportId
 * @param data the received data
 */
mooltipass.device.onDataReceived = function(reportId, data) {
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

    var command = mooltipass.device.commandsReverse[cmd];

    var queuedItem = mooltipass.device.getFromQueue(command);
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
    if(handlerName in mooltipass.device) {
        mooltipass.device[handlerName].apply(this, [queuedItem, msg]);
    }
    else {
        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, []);
    }
};

mooltipass.device.applyCallback = function(callbackFunction, callbackParameters, ownParameters) {
    if(callbackFunction) {
        var args = callbackParameters || [];
        args = args.concat(ownParameters || []);
        callbackFunction.apply(this, args);
    }
}

mooltipass.device.responseGetVersion = function(queuedItem, msg) {
    var version = mooltipass.device.convertMessageArrayToString(msg);
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

mooltipass.device.responsePing = function(queuedItem, msg) {
    console.log('Process PING command');
};

mooltipass.device.responseGetMooltipassParameter = function(queuedItem, msg) {
    console.log('Process getMooltipassParameter command');

    var responseObject = {
        'status': 'success',
        'payload': queuedItem.payload,
        'value': msg[0]
    };

    log('getMooltipassParameter(', queuedItem.payload, ') =', msg[0]);

    console.log('queuedItem', queuedItem);

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
};

mooltipass.device.responseSetMooltipassParameter = function(queuedItem, msg) {
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
    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
};



/*********************************************************************************************************************/


var keys = Object.keys(mooltipass.device.commands);
mooltipass.device.commandsReverse = {};
for(var i = 0; i < keys.length; i++) {
    mooltipass.device.commandsReverse[mooltipass.device.commands[keys[i]]] = keys[i];
}