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

mooltipass.device.status_parameters = {
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

// Version of the connected device
mooltipass.device.version = null;

// FlashChip ID of the connected device
mooltipass.device.flashChipId = null;

// save current status of connected device
mooltipass.device.status = null;

// Information about established connection
mooltipass.device.isConnected = false;

// Information about unlocked database
mooltipass.device.isUnlocked = false;

// Device has no card inserted
mooltipass.device.hasNoCard = true;

// Is communication with device is uniquely taken by a function?
// e.g. device is in MemoryManagementMode
mooltipass.device.singleCommunicationMode = false;

// singleCommunicationMode is activated and first results can now be shown
mooltipass.device.singleCommunicationModeEntered = false;

// Short slugified string why communication is blocked
// e.g. memorymanagement || synchronisation
mooltipass.device.singleCommunicationReason = null;

// Queue for executing commands
mooltipass.device.queue = [];

// Hash for command timeout to verify same command
mooltipass.device.queueHash = null;


/*********************************************************************************************************************/

/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */
mooltipass.device.init = function() {
    // Initial start processing queue
    mooltipass.device.restartProcessingQueue();

    setInterval(mooltipass.device.checkStatus, 1000);
};


/**
 * Reset information about device
 */
mooltipass.device.reset = function() {
    mooltipass.device.connectionId = null;
    mooltipass.device.version = null;
    mooltipass.device.flashChipId = null;
    mooltipass.device.isUnlocked = false;
    mooltipass.device.isConnected = false;
    mooltipass.device.hasNoCard = true;
    mooltipass.device.endSingleCommunicationMode();
};

/**
 * Connect to HID device if no connection is established
 */
mooltipass.device.connect = function() {
    //console.log('mooltipass.device.connect()');

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
        mooltipass.device.restartProcessingQueue();
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
        mooltipass.device.restartProcessingQueue();
        return;
    }

    mooltipass.device.connectionId = connectInfo.connectionId;
    mooltipass.device.isConnected = true;

    console.log('Connected to device');

    // Retrieve version and current date on each connect
    mooltipass.device.addToQueue('getVersion', [], null, null, null, null, true);
    mooltipass.device.addToQueue('setCurrentDate', [], null, null, null, null, true);

    mooltipass.app.updateOnConnect();

    // Trigger queue
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

/**
 * Start single communication mode
 * e.g. for MemoryManagementMode
 * @param reason for entering the mode (e.g. memorymanagementmode)
 */
mooltipass.device.startSingleCommunicationMode = function(reason) {
    mooltipass.device.singleCommunicationMode = true;
    mooltipass.device.singleCommunicationModeEntered = false;
    mooltipass.device.singleCommunicationReason = reason;
};

/**
 * End single communication mode
 */
mooltipass.device.endSingleCommunicationMode = function(skip) {
    // Skip ending single communication mode
    // e.g. because it was called by callbackAllQueuedCommandsInSingleCommunicationMode()
    if(skip) {
        return;
    }

    mooltipass.device.singleCommunicationMode = false;
    mooltipass.device.singleCommunicationModeEntered = false;
    mooltipass.device.singleCommunicationReason = null;
};


/**
 * Return next element in queue
 * @param command if given, return the next element with the specified command
 * @returns object
 */
mooltipass.device.getFromQueue = function(command, keepInQueue) {
    if(command) {
        //console.log('COMMAND', command, '(length queue: ', mooltipass.device.queue.length, ')');
        for(var i = 0; i < mooltipass.device.queue.length; i++) {
            if(mooltipass.device.queue[i].command == command) {
                if(keepInQueue) {
                    return mooltipass.device.queue[i];
                }
                var result = mooltipass.device.queue.splice(i, 1);
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
 * @param additionalArguments object with additional arguments like for singleCommunicationMode
 */
mooltipass.device.addToQueue = function(command, payload, responseParameters, callbackFunction, callbackParameters, timeoutObject, addToFirstPosition, additionalArguments) {
    var object = {
        'command': command,
        'payload': payload,
        'responseParameters': responseParameters,
        'callbackFunction': callbackFunction,
        'callbackParameters': callbackParameters,
        'timeout': timeoutObject,
        'additionalArguments': additionalArguments
    };

    if(addToFirstPosition) {
        mooltipass.device.queue.unshift(object);
    }
    else {
        mooltipass.device.queue.push(object);
    }

    return true;
};


mooltipass.device.setQueueHash = function() {
    mooltipass.device.queueHash = Math.random() + Math.random();
    var queuedItem = mooltipass.device.getFromQueue(null, true);
    if(queuedItem) {
        queuedItem.hash = mooltipass.device.queueHash;
        return mooltipass.device.queueHash;
    }

    return null;
};

mooltipass.device.restartProcessingQueue = function() {
    //console.log('mooltipass.device.restartProcessingQueue()');
    setTimeout(mooltipass.device.processQueue, 150);
};

mooltipass.device.callbackAllQueuedCommandsInSingleCommunicationMode = function() {
    var queuedItem;
    var responseObject = {
        'success': false,
        'code': 90,
        'msg': 'device blocks new communication',
        'skipEndingSingleCommunicationMode': true,
    };
    while(queuedItem = mooltipass.device.getFromQueue(null, false)) {
        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    }
};

/**
 * Process queued requests
 * Generates a valid packet from given command and payload and sends it to the device
 */
mooltipass.device.processQueue = function() {
    //console.log('mooltipass.device.processQueue()');

    if(mooltipass.device.connect()) {
        return;
    }

    if(mooltipass.device.queue.length == 0) {
        // If queue is processed, the device cannot be in a single mode
        mooltipass.device.endSingleCommunicationMode();
        mooltipass.device.restartProcessingQueue();
        return;
    }

    var queuedItem = mooltipass.device.getFromQueue(null, true);

    if(queuedItem.command == 'startSingleCommunicationMode') {
        // Enter singleCommunicationMode
        mooltipass.device.startSingleCommunicationMode(queuedItem.additionalArguments.reason);
        // Remove own request object from queue
        queuedItem = mooltipass.device.getFromQueue(null, false);
        // Remove all remaining queued requests and send callback with error to them
        mooltipass.device.callbackAllQueuedCommandsInSingleCommunicationMode();
        // Call callback to start single communication
        mooltipass.device.applyCallback(queuedItem.additionalArguments.callbackFunctionStart, queuedItem.callbackParameters, null);
        return;
    }

    // If queue is processed, the device cannot be in single communication mode
    mooltipass.device.endSingleCommunicationMode();

    if(queuedItem.command == 'setCurrentDate') {
        queuedItem.payload = mooltipass.device.getSettingCurrentDatePayload();
    }
    queuedItem.packet = mooltipass.device.createPacket(mooltipass.device.commands[queuedItem.command], queuedItem.payload);

    /*
    if (mooltipass.device.debug) {
        var msgUint8 = new Uint8Array(queuedItem.packet);
        // don't output the PING command since this is the keep alive
        if (msgUint8[1] != mooltipass.device.commands.ping) {
            console.log('sendMsg(', JSON.stringify(new Uint8Array(queuedItem.packet)), ')');
        }
    }
    */

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

    var queuedItem = mooltipass.device.getFromQueue(null, true);

    // Successfully processed command, no retries needed
    if(queuedItem.hash != mooltipass.device.queueHash) {
        return;
    }

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
    if(!queuedItem) {
        // TODO: Workaround, normally this should not happen
        console.warn('queuedItem should not be undefined!');
        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }

    var handlerName = 'response' + capitalizeFirstLetter(command);

    console.log('mooltipass.device.onDataReceived(', command, ')');
    /*
    console.log('reportId', reportId);
    console.log('queuedItem', queuedItem);
    console.log('data', data);
    console.log('bytes', bytes);
    console.log('msg', msg);
    console.log('len', len);
    console.log('cmd', cmd);
    console.log('command', command);
    */

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

    mooltipass.device.version = version;
    mooltipass.device.flashChipId = flashChipId;

    var responseObject = {
        'success': true,
        'value': version
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetMooltipassStatus = function(queuedItem, msg) {
    var status = mooltipass.device.convertMessageArrayToString(msg);

    var responseObject = {
        'success': true,
        'value': status
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetRandomNumber = function(queuedItem, msg) {
    // Use first 10 numbers of bytes to create a random number
    var value = "";
    for(var i = 0; i < 10; i++) {
        value += String(msg[i]);
    }

    var responseObject = {
        'success': true,
        'value': value
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responsePing = function(queuedItem, msg) {
    var responseObject = {
        'success': true
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetMooltipassStatus = function(queuedItem, msg) {
    var _status = mooltipass.device.status_parameters[msg[0]];
    _status = _status ? _status : 'error';

    mooltipass.device.status = _status;

    var responseObject = {
        'success': true,
        'value': _status
    };

    if(queuedItem && queuedItem.callbackFunction) {
        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    }
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetMooltipassParameter = function(queuedItem, msg) {
    var responseObject = {
        'success': true,
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
        'success': success,
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

mooltipass.device.getSettingCurrentDatePayload = function() {
    var date = new Date();
    var array = new Uint8Array(2);
    array[0] = ((date.getFullYear() - 2010) << 1) & 0xFE;
    if(date.getMonth() >= 8)
    {
        array[0] |= 0x01;
    }
    array[1] = ((date.getMonth()%8) << 5) & 0xE0;
    array[1] |= date.getDate();

    return array;
};

/**
 * Request status from device and set status in app.
 * Trigger actions for specific status
 */
mooltipass.device.checkStatus = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassStatus',
        'callbackFunction': function(_responseObject, _credentials) {
            if(_responseObject.success) {
                var unlocked = _responseObject.value == 'unlocked';
                var locked = _responseObject.value == 'locked';
                var noCard = _responseObject.value == 'no-card';

                if(!mooltipass.device.isUnlocked && unlocked) {
                    mooltipass.app.updateOnUnlock();
                }
                if(mooltipass.device.isUnlocked && locked) {
                    mooltipass.app.updateOnLock();
                }
                //console.log('mooltipass.device.isUnlocked =', unlocked);
                mooltipass.device.isUnlocked = unlocked;

                mooltipass.device.hasNoCard = noCard;
            }
            // Set to locked only if not in MemoryManagementMode
            else if(_responseObject.code != 90) {
                //console.log('mooltipass.device.isUnlocked = false bcs code ', _responseObject.code);
                mooltipass.device.isUnlocked = false;
                mooltipass.app.updateOnLock();
            }

            // TODO: inform connected clients
        }
    });
};


mooltipass.device.commandGetCredentials = function() {

};



/*********************************************************************************************************************/


var keys = Object.keys(mooltipass.device.commands);
mooltipass.device.commandsReverse = {};
for(var i = 0; i < keys.length; i++) {
    mooltipass.device.commandsReverse[mooltipass.device.commands[keys[i]]] = keys[i];
}