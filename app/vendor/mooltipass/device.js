/* global chrome */
/* global chrome.hid */
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
    'endMemoryManagementMode'       : 0xD3
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
    'pleaseRetry'                   : 0xC4
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
    'tutorialEnabled': 16
};

mooltipass.device.status = {
    0: 'no-card',
    1: 'locked',
    2: 'error',
    3: 'locked',
    4: 'error',
    5: 'unlocked',
    6: 'error',
    7: 'error'
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
};

// Connection ID for communication with HID device
mooltipass.device.connectionId = null;

// Information about established connection
mooltipass.device.isConnected = false;

// External client to communicate with (e.g. an app)
mooltipass.device.clientId = null;

// Queue for executing commands
mooltipass.device.queue = [];

// Hash for command timeout to verify same command
mooltipass.device.queueHash = null;


/*********************************************************************************************************************/

/**
 * Return next element in queue
 * @param command if given, return the next element with the specified command
 * @returns object
 */
mooltipass.device.getFromQueue = function(command, keepInQueue) {
    if(command) {
        console.log('COMMAND', command, '(length queue: ', mooltipass.device.queue.length, ')');
        for(var i = 0; i < mooltipass.device.queue.length; i++) {
            if(mooltipass.device.queue[i].command == command) {
                if(keepInQueue) {
                    return mooltipass.device.queue[i];
                }
                var result = mooltipass.device.queue.splice(i, 1);
                console.log('.... returned.    (length queue: ', mooltipass.device.queue.length, ')');
                return result[0];
            }
        }
    }

    if(keepInQueue) {
        return  mooltipass.device.queue[0];
    }

    return mooltipass.device.queue.shift();
};

/**
 * Add new request to queue
 * @param command string
 * @param payload array
 * @param responseParameters array of parameters which are needed to process the response
 * @param callbackFunction function to send the response to
 * @param callbackParameters additional parameters with which the callback function has to be called
 * @param timeoutObject containing information for retries
 * @param addToFirstPosition if set, the request is added to first position
 */
mooltipass.device.addToQueue = function(command, payload, responseParameters, callbackFunction, callbackParameters, timeoutObject, addToFirstPosition) {
    var object = {
        'command': command,
        'payload': payload,
        'responseParameters': responseParameters,
        'callbackFunction': callbackFunction,
        'callbackParameters': callbackParameters,
        'timeout': timeoutObject
    };

    if(addToFirstPosition) {
        mooltipass.device.queue.unshift(object);
    }
    else {
        mooltipass.device.queue.push(object);
    }
};


mooltipass.device.setQueueHash = function() {
    mooltipass.device.queueHash = Math.random() + Math.random();
    mooltipass.device.queue[0]['hash'] = mooltipass.device.queueHash;

    return mooltipass.device.queueHash;
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
        return false;
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
    console.log('Found', devices.length, 'devices.');
    console.log('Device', device.deviceId,', vendor', device.vendorId, ', product', device.productId);

    console.log('Connecting to device', device.deviceId);

    chrome.hid.connect(device.deviceId, mooltipass.device.onConnectFinished);
};

/**
 * Callback of chrome.hid.connect()
 * Set connection ID for communication and execute original command
 * @param connectInfo
 */
mooltipass.device.onConnectFinished = function(connectInfo) {
    if (chrome.runtime.lastError) {
        console.log('Failed to connect to device: ', chrome.runtime.lastError.message);
        mooltipass.device.reset();
        return;
    }

    mooltipass.device.connectionId = connectInfo.connectionId;
    mooltipass.device.isConnected = true;

    console.log('Connected to device');

    mooltipass.device.processQueue();
};

/**
 * Helper function to create a valid packet for communication with the device.
 * @param command string
 * @param payload array
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
 * @return string representation of the array
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
};

mooltipass.device.restartProcessingQueue = function() {
    setTimeout(mooltipass.device.processQueue, 500);
};

/**
 * Process queued requests
 * Generates a valid packet from given command and payload and sends it to the device
 */
mooltipass.device.processQueue = function() {
    if(mooltipass.device.queue.length == 0) {
        mooltipass.device.restartProcessingQueue();
        return;
    }

    if(mooltipass.device.connect()) {
        return;
    }

    var queuedItem = mooltipass.device.getFromQueue(null, true);

    queuedItem.packet = mooltipass.device.createPacket(mooltipass.device.commands[queuedItem.command], queuedItem.payload);

    if (mooltipass.device.debug) {
        var msgUint8 = new Uint8Array(queuedItem.packet);
        // don't output the PING command since this is the keep alive
        if (msgUint8[1] != mooltipass.device.commands.ping) {
            console.log('sendMsg(', JSON.stringify(new Uint8Array(queuedItem.packet)), ')');
        }
    }

    mooltipass.device._sendMsg(queuedItem);
};

mooltipass.device._sendMsg = function(queuedItem) {
    mooltipass.device.setQueueHash();
    if(queuedItem.timeout) {
        setTimeout(function() {
            mooltipass.device._retrySendMsg();
        }, queuedItem.timeout.milliseconds)
    }

    chrome.hid.send(mooltipass.device.connectionId, 0, queuedItem.packet, mooltipass.device.onSendMsg);
};

mooltipass.device._retrySendMsg = function() {
    console.log('mooltipass.device._retrySendMsg()');

    // No requests in queue -> nothing to retry
    if(mooltipass.device.queue.length < 1) {
        return;
    }

    console.log('    queue not empty');

    var queuedItem = mooltipass.device.getFromQueue(null, true);

    // Successfully processed command, no retries needed
    if(queuedItem.hash != mooltipass.device.queueHash) {
        return;
    }

    console.log('    same hash');

    // No timeout object found (shouldn't happen)
    if(!queuedItem.timeout) {
        console.error('queuedItem does not contain a valid timeoutObject:', queuedItem);
        return;
    }

    console.log('   ', queuedItem.timeout.retries, 'retries');

    // Retry request until retries is 0
    if(queuedItem.timeout.retries > 0) {
        queuedItem.timeout.retries -= 1;
        mooltipass.device._sendMsg(queuedItem);
        return;
    }

    console.log('    call callback function');

    // If callbackFunction is set, call it in case of retries is reached
    if(queuedItem.timeout.callbackFunction) {
        queuedItem.timeout.callbackFunction.apply(this, [queuedItem]);
    }
};

/**
 * Callback of chrome.hid.send()
 * It sets a callback function for receiving data
 */
mooltipass.device.onSendMsg = function() {
    // TODO: refactor chrome.runtime.lastError code to own method #1
    if (chrome.runtime.lastError) {
        if (mooltipass.device.isConnected) {
            if (mooltipass.device.debug) {
                console.log('Failed to send to device: '+chrome.runtime.lastError.message);
            }
            // TODO: Send disconnect information to all known clients
            // TODO: refactor code
            if (mooltipass.device.clientId) {
                chrome.runtime.sendMessage(mooltipass.device.clientId, {type: 'disconnected'});
            }
            mooltipass.device.reset();
        }

        // TODO: trigger request callback
        mooltipass.device.restartProcessingQueue();
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
        // TODO: refactor chrome.runtime.lastError code to own method #2
        if (chrome.runtime.lastError) {
            var error = chrome.runtime.lastError;
            if (error.message != 'Transfer failed.') {
                console.log('Error in onDataReceived:', error.message);
            }
        }

        // TODO: trigger request callback
        mooltipass.device.restartProcessingQueue();
        return;
    }

    // Change queueHash to avoid retry after timeout
    mooltipass.device.setQueueHash();

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
        // Process next queued request
        mooltipass.device.processQueue();
    }
};

mooltipass.device.applyCallback = function(callbackFunction, callbackParameters, ownParameters) {
    if(callbackFunction) {
        var args = ownParameters || [];
        args = args.concat(callbackParameters || []);
        callbackFunction.apply(this, args);
    }
};

mooltipass.device.responseGetVersion = function(queuedItem, msg) {
    var flashChipId = msg[0];
    var version = mooltipass.device.convertMessageArrayToString(new Uint8Array(msg, 1));

    var responseObject = {
        'status': 'success',
        'value': version
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetMooltipassStatus = function(queuedItem, msg) {
    var status = mooltipass.device.convertMessageArrayToString(msg);

    var responseObject = {
        'status': 'success',
        'value': status
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responsePing = function(queuedItem, msg) {
    var responseObject = {
        'status': 'success'
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetMooltipassStatus = function(queuedItem, msg) {
    var _status = mooltipass.device.status[msg[0]];
    _status = _status ? _status : 'error';

    var responseObject = {
        'status': 'success',
        'value': _status
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetMooltipassParameter = function(queuedItem, msg) {
    var responseObject = {
        'status': 'success',
        'payload': queuedItem.payload,
        'value': msg[0]
    };

    console.log('getMooltipassParameter(', queuedItem.payload, ') =', msg[0]);

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseSetMooltipassParameter = function(queuedItem, msg) {
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
    // Process next queued request
    mooltipass.device.processQueue();
};



/*********************************************************************************************************************/


var keys = Object.keys(mooltipass.device.commands);
mooltipass.device.commandsReverse = {};
for(var i = 0; i < keys.length; i++) {
    mooltipass.device.commandsReverse[mooltipass.device.commands[keys[i]]] = keys[i];
}

// Initial start processing queue
mooltipass.device.restartProcessingQueue();