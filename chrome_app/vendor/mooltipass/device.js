/* global chrome */
/* global chrome.hid */
var mooltipass = mooltipass || {};
mooltipass.device = mooltipass.device || {};

// Debug mode
mooltipass.device.debug = false;
mooltipass.device.packet_debug = false;

// Mooltipass device info
mooltipass.device.deviceInfo = { 'vendorId': 0x16d0, 'productId': 0x09a0 };

// Number of bytes of a packet transferred over USB is fixed to 64
mooltipass.device.packetSize = 64;

// First 2 bytes contain packet length and command
mooltipass.device.payloadSize = mooltipass.device.packetSize - 2;

// Available command codes for Mooltipass
mooltipass.device.commands = {
    'getStackFree'                  : 0x9C,
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
    'pleaseRetry'                   : 0xC4,  
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
    'jumpToBootloader'              : 0xAB
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
    'noCard'                        : 0x03
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
    'keypressLoginEnabled': 19,
    'keypressLogin': 20,
    'keypressPasswordEnabled': 21,
    'keypressPassword': 22,
    'keybOutputDelayEnabled': 23,
    'keybOutputDelay': 24,
    'wheelReverse': 25,
    'screenBrightness': 26,
    'ledAnimMask': 27,
    'knockDetectEnable': 28,
    'knockDetectThres': 29,
    'lockUnlock': 30,
	'hashDisplayEnable': 31,
    'randomPin':32
};

mooltipass.device.status_parameters = {
    0: 'no-card',
    1: 'locked',
    2: 'error',
    3: 'locked',
    4: 'error',
    5: 'unlocked',
    6: 'error',
    7: 'error',
    8: 'error',
    9: 'unknown-card'
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
mooltipass.device.version = 'v1';

// FlashChip ID of the connected device
mooltipass.device.flashChipId = null;

// save current status of connected device
mooltipass.device.status = null;

// Information about established connection
mooltipass.device.isConnected = false;

// Information about unlocked database
mooltipass.device.isUnlocked = false;

// Information whether the card is unknown for the device
mooltipass.device.isUnknownCard = false;

// Device has no card inserted
mooltipass.device.hasNoCard = true;

// Current get credential request id 
mooltipass.device.currentReqid = null;

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
    // only init if moolticute isn't running.
    var moolticuteSocket = new WebSocket('ws://127.0.0.1:30035');
    moolticuteSocket.onerror = function() {
        mooltipass.device._forceEndMemoryManagementModeLock = false;
        
        // Initial start processing queue
        mooltipass.device.restartProcessingQueue();

        setInterval(mooltipass.device.checkStatus, 1000);
    };
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
    delete mooltipass.device._setCurrentDateLock;
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
    if (!devices || !devices.length) 
    {
        if(mooltipass.device.debug)
        {
            console.log('No compatible devices found.');
        }
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
        console.log('Failed to connect to device:', chrome.runtime.lastError.message);
        mooltipass.device.reset();
        mooltipass.device.restartProcessingQueue();
        return;
    }

    mooltipass.device.connectionId = connectInfo.connectionId;

    // Retrieve version and current date on each connect
    //mooltipass.device.addToQueue('getVersion', [], null, null, null, null, true);
    //mooltipass.device.addToQueue('setCurrentDate', [], null, null, null, null, true);

    mooltipass.device.isConnected = true;
    console.log('Connected to device');

    mooltipass.app.updateOnConnect();

    mooltipass.device.processQueue();
};

/**
 * Helper function to create a valid packet for communication with the device.
 * @param _command string
 * @param _payload array
 * @returns {ArrayBuffer}
 */
mooltipass.device.createPacket = function(_command, _payload) {
    if(_command == mooltipass.device.commands['getMooltipassUID']) {
        var tempPayload = new Uint8Array(16);
        for(var i = 0; i < _payload[0].length; i+= 2)
        {
            tempPayload[i/2] = parseInt(_payload[0].substr(i, 2), 16);
        }

        _payload = tempPayload;
    }

    var buffer = new ArrayBuffer(mooltipass.device.packetSize);
    var bufferView = new Uint8Array(buffer);

    var length = 0;
    if(_payload) {
        var containStrings = false;
        for(var i = 0; i < _payload.length; i++) {
            if(typeof _payload[i] === 'string' || _payload[i] instanceof String) {
                length += _payload[i].length;
                containStrings = true;
            }
            else {
                // Numbers need only 1 place
                length += 1;
            }
        }
        if(containStrings) {
            // Add \0 null character to end of data
            length += 1;
        }
    }

    bufferView[0] = length;
    bufferView[1] = _command;

    if(_payload) {
        var index = 2;
        for (var i = 0; i < _payload.length; i++) {
            if (index >= bufferView.byteLength) {
                // If packet size is reached, stop it
                console.error('Packet size exceeded! Cannot insert complete data into one packet:', _payload);
                break;
            }

            if (typeof _payload[i] === 'string' || _payload[i] instanceof String) {
                for (var z = 0; z < _payload[i].length; z++) {
                    if (index >= bufferView.byteLength) {
                        // If packet size is reached, stop it
                        console.error('Packet size exceeded! Cannot insert complete data into one packet:', _payload);
                        break;
                    }
                    
                    // Don't allow unicode, replace with '?'
                    if (_payload[i].charCodeAt(z) > 254)
                    {
                        bufferView[index] = 63;
                    }
                    else
                    {
                        bufferView[index] = _payload[i].charCodeAt(z);
                    }
                    
                    index += 1;
                }
            }
            else {
                bufferView[index] = _payload[i];
                index += 1;
            }
        }
    }

    //console.log(bufferView);
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
 * Convert a uint8 array to HEX-String
 * @param uint8Array the array to convert
 * @return hex string representation of the array
 * @note does not support unicode yet
 */
mooltipass.device.convertMessageArrayToHexString = function(uint8Array) {
    var output = '';
    for (var i=0; i < uint8Array.length; i++) {
        if (uint8Array[i] == 0) {
            return output;
        }
        else {
            output += parseInt(uint8Array[i]).toString(16) + ' ';
        }
    }
    return output.trim();
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
        // TODO: check whether return NULL here or return any element from queue
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
    
    //console.log(object);

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
        responseObject.command = queuedItem.command;
        responseObject.senderId = queuedItem.responseParameters ? queuedItem.responseParameters.senderId : null,
            mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    }
};

mooltipass.device.callbackAllQueuedCommandsOnDisconnect = function() {
    var queuedItem;
    var responseObject = {
        'success': false,
        'code': 69,
        'msg': 'device not connected',
    };
    while(queuedItem = mooltipass.device.getFromQueue(null, false)) {
        responseObject.command = queuedItem.command;
        responseObject.senderId = queuedItem.responseParameters ? queuedItem.responseParameters.senderId : null,
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

    if (mooltipass.device.debug) 
    {
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
    
    if(mooltipass.device.packet_debug)
    {
        console.log('send queuedItem', queuedItem);
        console.log('send packet', new Uint8Array(queuedItem.packet));      
    }

    chrome.hid.send(mooltipass.device.connectionId, 0, queuedItem.packet, mooltipass.device.onSendMsg);

    if(queuedItem.command == "jumpToBootloader") {
        mooltipass.device.getFromQueue("jumpToBooloader");
    }
};

mooltipass.device._retrySendMsg = function() {
    //console.log('mooltipass.device._retrySendMsg()');

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

    //console.log('    call callback function');

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
        console.warn('Failed to send to device:', chrome.runtime.lastError.message);

        if (mooltipass.device.isConnected) {
            // TODO: Send disconnect information to all known clients
            mooltipass.device.reset();
            mooltipass.device.callbackAllQueuedCommandsOnDisconnect();
        }

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
            var message = chrome.runtime.lastError.message;
            if (message != 'Transfer failed.') {
                console.log('Error in onDataReceived:', message);
            }
        }

        console.error('undefined data', reportId, data);

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

    if(mooltipass.device.debug)
    {
        console.log('mooltipass.device.onDataReceived(', command, ')');
    }   

    var queuedItem = mooltipass.device.getFromQueue(command);
    if(!queuedItem) {
        // TODO: Workaround, normally this should not happen
        console.warn('queuedItem should not be undefined!');
        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }

    var handlerName = 'response' + capitalizeFirstLetter(command);
    
    if(mooltipass.device.packet_debug)
    {
        console.log('reportId', reportId);
        console.log('queuedItem', queuedItem);
        console.log('data', data);
        console.log('bytes', bytes);
        console.log('msg', msg);
        console.log('len', len);
        console.log('cmd', cmd);
        console.log('command', command);
    }
    

    // Invoke function to process message
    if(handlerName in mooltipass.device) {
        mooltipass.device[handlerName].apply(this, [queuedItem, msg]);
    }
    else {
        if(command == 'debug')
        {
            console.log("Debug message received: " + mooltipass.util.arrayToStr(msg));
            mooltipass.device.processQueue();   
        }
        else
        {
            mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, []);
            // Process next queued request
            mooltipass.device.processQueue();           
        }
    }
};

mooltipass.device.applyCallback = function(callbackFunction, callbackParameters, ownParameters) {
    applyCallback(callbackFunction, callbackParameters, ownParameters);
};

mooltipass.device.responseResetCard = function(queuedItem, msg) {
    var status = msg[0];
    status = status ? status : 'error';
    var responseObject = {
        'command': queuedItem.command,
        'success': true,
        'status': status
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseEndMemoryManagementMode = function(queuedItem, msg) {
    var responseObject = {
        'command': queuedItem.command,
        'success': true
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.stackFree = function() {
    mooltipass.device.addToQueue('getStackFree', null, null, function(){}, null, null, false, null);
}

mooltipass.device.responseGetStackFree = function(queuedItem, msg) {
    console.log("Stack free: ", (msg[0] + msg[1]*256), " bytes");
    console.log("Stack at boot: ", (msg[2] + msg[3]*256), " bytes");
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetVersion = function(queuedItem, msg) {
    var flashChipId = msg[0];
    var version = mooltipass.device.convertMessageArrayToString(msg.subarray(1));

    mooltipass.device.version = version;
    mooltipass.device.flashChipId = flashChipId;
    
    console.log("Firmware Version:", version);

    var responseObject = {
        'command': queuedItem.command,
        'success': true,
        'value': version
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseSetCurrentDate = function(queuedItem, msg) {
    var responseObject = {
        'command': queuedItem.command,
        'success': true
    };

    mooltipass.device._setCurrentDateLock = true;

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetMooltipassStatus = function(queuedItem, msg) {
    if(queuedItem.command != 'getMooltipassStatus') {
        // Device answers with status response if user is asked for PIN -> resend request
        mooltipass.device.queue.push(queuedItem);

        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }

    var status = mooltipass.device.status_parameters[msg[0]];
    status = status ? status : 'error';

    mooltipass.device.status = status;

    var unlocked = status == 'unlocked';
    var locked = status == 'locked';
    var noCard = status == 'no-card';
    var unknownCard = status == 'unknown-card';

    var responseObject = {
        'command': queuedItem.command,
        'success': true,
        'value': status,
        'senderId': queuedItem.responseParameters ? queuedItem.responseParameters.senderId : null,
        'connected': mooltipass.device.isConnected,
        'unlocked': unlocked,
        'locked': locked,
        'noCard': noCard,
        'unknownCard': unknownCard,
        'version': mooltipass.device.version,
    };

    if(queuedItem && queuedItem.callbackFunction) {
        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    }

    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responsePleaseRetry = function(queuedItem, msg)
{
    mooltipass.device.queue.unshift(queuedItem);
    setTimeout(mooltipass.device.processQueue, 500);
};

mooltipass.device.responseGetRandomNumber = function(queuedItem, msg) {
    // Use first 10 numbers of bytes to create a random number
    var value = "";
    for(var i = 0; i < 10; i++) {
        value += String(msg[i]);
    }

    var responseObject = {
        'command': queuedItem.command,
        'success': true,
        'value': value,
        'rawdata': msg
    };
    
    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responsePing = function(queuedItem, msg) {
    var responseObject = {
        'command': queuedItem.command,
        'success': true
    };

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);

    // Send ping information to all connected clients
    mooltipass.device.clients.send(responseObject, queuedItem.responseParameters);

    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetMooltipassParameter = function(queuedItem, msg) {
    var responseObject = {
        'command': queuedItem.command,
        'payload': queuedItem.payload,
        'success': true,
        'value': msg[0]
    };

    //console.log('getMooltipassParameter(', queuedItem.payload, ') =', msg[0]);

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseSetMooltipassParameter = function(queuedItem, msg) {
    var success = msg[0] == 1;

    var responseObject = {
        'command': queuedItem.command,
        'payload': queuedItem.payload,
        'success': success
    };

    if(!success) {
        responseObject['code'] = 601;
        responseObject['msg'] = 'request was not performed';
    }

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseGetMooltipassUID = function(queuedItem, msg) {
    var success = msg[0] != 0;

    var responseObject = {
        'command': queuedItem.command,
        'payload': queuedItem.payload,
        'success': success
    };

    if(success) {
        responseObject['value'] = mooltipass.device.convertMessageArrayToHexString(msg);
    }
    else {
        responseObject['code'] = 602;
        responseObject['msg'] = 'could not fetch UID with given password';
    }

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};

mooltipass.device.responseSetContext = function(queuedItem, msg) {
    var params = queuedItem.responseParameters;

    var responseObject = {
        'command': queuedItem.command,
        'payload': queuedItem.payload,
    };

    if(!params) {
        // TODO: can this happen? are there any use-cases for setContext without add/update/get credentials?
        responseObject.success = false;
        responseObject.code = 201;
        responseObject.msg = "Nothing to continue after setting context!";

        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }

    var success = msg[0] == 1;
    responseObject.success = success;

    if(success) {
        responseObject.context = queuedItem.payload[0];
    }

    var requestType = params.requestType;
    if(requestType) {
        // Either add, update or get credentials
        switch(requestType) {
            case 'addCredentials':
            case 'updateCredentials':
                // Block app usage because of user interaction
                mooltipass.ui._.blockInput();

                // Add and update is currently the same
                if(success) {
                    // Update context
                    mooltipass.device.addToQueue('setLogin', [params.username], params, queuedItem.callbackFunction, queuedItem.callbackParameters, queuedItem.timeout, true, queuedItem.additionalArguments);
                    mooltipass.device.processQueue();
                    return;
                }
                else {
                    // Add context
                    if(mooltipass.device.debug)
                    {
                        console.log('add context:', params.context);
                    }
                    mooltipass.device.addToQueue('addContext', [params.context], params, queuedItem.callbackFunction, queuedItem.callbackParameters, queuedItem.timeout, true, queuedItem.additionalArguments);
                    mooltipass.device.processQueue();
                    return;
                }
                break;
            case 'getCredentials':
                if(success) {
                    // Contexts no longer needed
                    delete params.contexts;
                    params.context = queuedItem.payload[0];
                    // Get login
                    mooltipass.ui._.blockInput();
                    mooltipass.device.addToQueue('getLogin', [], params, queuedItem.callbackFunction, queuedItem.callbackParameters, queuedItem.timeout, true, queuedItem.additionalArguments);
                    mooltipass.device.currentReqid = params.reqid;
                    mooltipass.device.processQueue();
                    return;
                }
                else {
                    // Context not found, try next in list
                    if(params.contexts.length > 0) {
                        // More contexts available
                        var nextContext = params.contexts.splice(0, 1);
                        queuedItem.payload = [nextContext[0]];

                        // Add next context to first element in queue
                        mooltipass.device.addToQueue(queuedItem.command, queuedItem.payload, params, queuedItem.callbackFunction, queuedItem.callbackParameters, queuedItem.timeout, true, queuedItem.additionalArguments);

                        mooltipass.device.processQueue();
                        return;
                    }
                    else {
                        // No more contexts
                        responseObject.code = 204;
                        responseObject.msg = "No valid context found";

                        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
                        // Process next queued request
                        mooltipass.device.processQueue();
                        return;
                    }
                }
                break;
            default:
                // Unknown requestType
                responseObject.code = 205;
                responseObject.msg = "Given request type for continuing setContext is invalid";

                mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
                // Process next queued request
                mooltipass.device.processQueue();
                return;
        }
    }

    // In case no valid context was found, apply callback and continue with queue

    responseObject.code = 202;
    responseObject.msg = "Could not set context";
    console.warn('Could not set context:', queuedItem.payload[0]);

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};


mooltipass.device.responseGetLogin = function(queuedItem, msg) {
    var params = queuedItem.responseParameters;

    var responseObject = {
        'command': queuedItem.command,
        'payload': queuedItem.payload,
    };
    
    mooltipass.device.currentReqid = null;

    if(!params) {
        responseObject.success = false;
        responseObject.code = 301;
        responseObject.msg = "Did not receive context information";
        console.error('Context information not provided');

        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
        // Unblock app usage
        mooltipass.ui._.unblockInput();
        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }

    /*

    2015-12-09 Login can be empty
    var success = msg[0] == 1;
    if(!success) {
        responseObject.success = false;
        responseObject.code = 302;
        responseObject.msg = "Mooltipass device did not return a login";
        console.error('Login not found');

        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
        // Unblock app usage
        mooltipass.ui._.unblockInput();
        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }
    */

    var username = mooltipass.device.convertMessageArrayToString(msg);

    params.username = username;
    mooltipass.device.addToQueue('getPassword', [], params, queuedItem.callbackFunction, queuedItem.callbackParameters, queuedItem.timeout, true, queuedItem.additionalArguments);
    // Process next queued request
    mooltipass.device.processQueue();
};


mooltipass.device.responseGetPassword = function(queuedItem, msg) {
    var params = queuedItem.responseParameters;

    var responseObject = {
        'command': queuedItem.command,
        'payload': queuedItem.payload,
    };

    if(!params) {
        responseObject.success = false;
        responseObject.code = 401;
        responseObject.msg = "Context and username information not provided";
        console.error('Context and username information not provided');

        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
        // Unblock app usage
        mooltipass.ui._.unblockInput();
        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }

    var success = msg[0] != 0;
    if(!success) {
        responseObject.success = false;
        responseObject.code = 302;
        responseObject.msg = "Mooltipass device did not return a password";
        console.info('Password access denied');

        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
        // Unblock app usage
        mooltipass.ui._.unblockInput();
        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }

    var password = mooltipass.device.convertMessageArrayToString(msg);

    responseObject.success = true;
    responseObject.context = params.context;
    responseObject.username = params.username;
    responseObject.password = password;

    // Unblock app usage
    mooltipass.ui._.unblockInput();

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};


mooltipass.device.responseAddContext = function(queuedItem, msg) {
    var params = queuedItem.responseParameters;

    if(!params) {
        responseObject.success = false;
        responseObject.code = 451;
        responseObject.msg = "Did not receive context information";
        console.error('Context information not provided');

        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
        // Unblock app usage
        mooltipass.ui._.unblockInput();
        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }

    var success = msg[0] == 1;

    var responseObject = {
        'command': queuedItem.command,
        'payload': queuedItem.payload,
        'success': success,
    };

    if(success) {
        mooltipass.device.addToQueue('setContext', [params.context], params, queuedItem.callbackFunction, queuedItem.callbackParameters, queuedItem.timeout, true, queuedItem.additionalArguments);
        mooltipass.device.processQueue();
        return;
    }

    responseObject.code = 452;
    responseObject.msg = "User denied saving new context";

    // Unblock app usage
    mooltipass.ui._.unblockInput();

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};


mooltipass.device.responseSetLogin = function(queuedItem, msg) {
    var params = queuedItem.responseParameters;

    if(!params) {
        responseObject.success = false;
        responseObject.code = 501;
        responseObject.msg = "Did not receive credential information";
        console.error('Credential information not provided');

        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
        // Unblock app usage
        mooltipass.ui._.unblockInput();
        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }

    var success = msg[0] == 1;

    var responseObject = {
        'command': queuedItem.command,
        'payload': queuedItem.payload,
        'success': success,
    };

    if(success) {
        mooltipass.device.addToQueue('checkPassword', [params.password], params, queuedItem.callbackFunction, queuedItem.callbackParameters, queuedItem.timeout, true, queuedItem.additionalArguments);
        mooltipass.device.processQueue();
        return;
    }

    responseObject.code = 502;
    responseObject.msg = "User denied saving credentials";

    // Unblock app usage
    mooltipass.ui._.unblockInput();

    mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
    // Process next queued request
    mooltipass.device.processQueue();
};


mooltipass.device.responseCheckPassword = function(queuedItem, msg) {
    var params = queuedItem.responseParameters;

    if(!params) {
        responseObject.success = false;
        responseObject.code = 521;
        responseObject.msg = "Did not receive credential information";
        console.error('Credential information not provided');

        mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
        // Unblock app usage
        mooltipass.ui._.unblockInput();
        // Process next queued request
        mooltipass.device.processQueue();
        return;
    }

    switch(msg[0]) {
        case 1:
            var responseObject = {
                'command': queuedItem.command,
                'payload': queuedItem.payload,
                'success': true,
            };

            // Unblock app usage
            mooltipass.ui._.unblockInput();

            mooltipass.device.applyCallback(queuedItem.callbackFunction, queuedItem.callbackParameters, [responseObject]);
            mooltipass.device.processQueue();
            return;
        case 0: // Missmatch of password
        case 2: // Timer blocks request (avoid brute-force)
        default:
            mooltipass.device.addToQueue('setPassword', [params.password], params, queuedItem.callbackFunction, queuedItem.callbackParameters, queuedItem.timeout, true, queuedItem.additionalArguments);
            mooltipass.device.processQueue();
            return;
    }
};


mooltipass.device.responseSetPassword = function(queuedItem, msg) {
    var success = msg[0] == 1;

    var responseObject = {
        'command': queuedItem.command,
        'payload': queuedItem.payload,
        'success': success,
    };

    if(!success) {
        responseObject.code = 551;
        responseObject.msg = "User denied saving password";
    }

    // Unblock app usage
    mooltipass.ui._.unblockInput();

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
    if(mooltipass.device.isConnected && !mooltipass.device._setCurrentDateLock) {
        mooltipass.device.interface.send({
            'command': 'setCurrentDate',
            'payload': []
        });
    }

    if(mooltipass.device.isConnected && !mooltipass.device._forceEndMemoryManagementModeLock) {
        mooltipass.device.interface.send({
            'command': 'endMemoryManagementMode',
            'payload': [],
            'callbackFunction': function(_statusObject) {
                if(_statusObject.success) {
                    mooltipass.device._forceEndMemoryManagementModeLock = true;
                }
            }
        });
    }

    if(mooltipass.device.isConnected && !mooltipass.device.version) {
        mooltipass.device.interface.send({
            'command': 'getVersion',
            'payload': []
        });
    }

    mooltipass.device.interface.send({
        'command': 'getMooltipassStatus',
        'payload': [],
        'callbackFunction': mooltipass.device.checkStatusCallback
    });
};

mooltipass.device.checkStatusCallback = function(_responseObject, _credentials) {
    if(_responseObject && _responseObject.success) {
        if(!mooltipass.device.isUnlocked && _responseObject.unlocked) {
            mooltipass.device.isUnlocked = _responseObject.unlocked;
            mooltipass.app.updateOnUnlock();
        }
        if(mooltipass.device.isUnlocked && _responseObject.locked) {
            mooltipass.app.updateOnLock();
        }
        //console.log('mooltipass.device.isUnlocked =', unlocked);
        mooltipass.device.isUnlocked = _responseObject.unlocked;
        mooltipass.device.isUnknownCard = _responseObject.unknownCard;
        mooltipass.device.hasNoCard = _responseObject.noCard;
    }
    // Set to locked only if not in MemoryManagementMode
    else if(_responseObject.code != 90) {
        //console.log('mooltipass.device.isUnlocked = false bcs code ', _responseObject.code);
        mooltipass.device.isUnlocked = false;
        mooltipass.app.updateOnLock();
    }

    // Inform connected clients
    mooltipass.device.clients.send(_responseObject, { 'senderId': _responseObject.senderId });
}



/*********************************************************************************************************************/


var keys = Object.keys(mooltipass.device.commands);
mooltipass.device.commandsReverse = {};
for(var i = 0; i < keys.length; i++) {
    mooltipass.device.commandsReverse[mooltipass.device.commands[keys[i]]] = keys[i];
}