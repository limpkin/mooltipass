var mp = {};

// Mooltipass device info
mp.deviceInfo = { 'vendorId': 0x16d0, 'productId': 0x09a0 };

mp.debug = false;

// Timeout for requests sent to the device
mp.authRequestTimeout = 15000;

// Number of bytes in an HID packet
mp.packetSize = 64;
mp.payLoadSize = mp.packetSize - 2;

// Commands that can be send to the device
mp.commands = {
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

// Generic responses from the device
mp.responses = {
    'error'                         : 0x00,
    'success'                       : 0x01,
    'noCard'                        : 0x03,
    'pleaseRetry'                   : 0xC4,
}



//###################################################################################################################
//###################################################################################################################
//###################################################################################################################


function log(msg) {
    $('#log').html($('#log').html() + '\n' + msg);
}

mp.connection = null;
mp.connected = false;
mp.connectMsg = null;

mp.reset = function() {
    mp.connected = false;
    mp.connection = null;
    mp.connectMsg = null;
}

/**
 * Connect to the mooltipass
 */
mp.connect = function(msg)
{
    mp.reset();
    if (msg)
    {
        mp.connectMsg = msg;
    }
    chrome.hid.getDevices(mp.deviceInfo, mp.onDeviceFound);
}

mp.checkConnection = function() {
    if (!mp.connected) {
        mp.connect();
    } else {
        mp.sendPing();
    }
}

mp.sendPing = function()  {
    console.log('sendPing()');
    var data = new Uint8Array(mp.payLoadSize);
    data.set([1,2,3,4], 0);
    mp.sendMsg(mp.commands.version, data);
}

mp.onDeviceFound = function(devices)
{
    if (!devices || !devices.length) {
        console.log('No compatible devices found.');
        return;
    }

    var device = devices[0];
    console.log('Found ' + devices.length + ' devices.');
    console.log('Device ' + device.deviceId + ' vendor' + device.vendorId + ' product ' + device.productId);

    console.log('Connecting to device '+device.deviceId);
    log('Connecting to device...');

    // Get a handler function that can decode the device signals
    var handler = getDeviceHandler(device);

    chrome.hid.connect(device.deviceId, function(connectInfo)
    {
        if (!chrome.runtime.lastError)
        {
            mp.connection = connectInfo.connectionId;
            mp.connected = true;

            console.log('Connected to device', device.deviceId);
            log('Connected');

            /*
            if (mp.connectMsg)
            {
                mp.sendMsg(mp.connectMsg);
            }
            else
            {
                mp.sendPing();
            }
            */
            mp.sendPing();
            // Poll the USB HID Interrupt pipe
            startPoller(connectInfo.connectionId, handler);
        }
        else
        {
            console.log('Failed to connect to device: ' + chrome.runtime.lastError.message);
            mp.reset();
        }
    });
}

function getDeviceHandler(device) {

    return function(data) {
        console.log( 'Uint8Array:', new Uint8Array(data) );
        console.log( 'Uint16Array:', new Uint16Array(data) );
        console.log( 'Uint32Array:', new Uint32Array(data) );
    };

}

function startPoller(connectionId, handler) {

    // The anonymous function poller() will keep the port, connectionID
    // and handler in its scope.
    var poller = function () {

        chrome.hid.receive(connectionId, function (reportID, data) {

            var event = handler(data);
            if (event) sendEvent('redspeak.' + event, port);
            console.log('poller');

            setTimeout(poller, 10);
        });

    };

    poller();
}

mp.sendMsg = function(command, data) {
    var buffer = new ArrayBuffer(mp.packetSize);
    buffer[0] = 0;
    buffer[1] = command;
    for(var i = 0; i < data.length; i++) {
        buffer[i+2] = data[i];
        buffer[0] += 1;
    }

    if (mp.debug) {
        msgUint8 = new Uint8Array(buffer);
        // don't output the CMD_VERSION command since this is the keep alive
        if (msgUint8[1] != mp.commands.version) {
            console.log('sending '+JSON.stringify(new Uint8Array(buffer)));
        }
    }

    console.log('command', command);
    console.log('buffer', buffer);

    chrome.hid.send(mp.connection, 0, buffer, function() {
        if (!chrome.runtime.lastError)
        {
            console.log('set receiver trigger');
            chrome.hid.receive(mp.connection, mp.onDataReceived);
        }
        else
        {
            if (mp.connected)
            {
                if (mp.debug) {
                    console.log('Failed to send to device: '+chrome.runtime.lastError.message);
                }
                log('Disconnected from mooltipass\n');
                if (mp.clientId) {
                    chrome.runtime.sendMessage(mp.clientId, {type: 'disconnected'});
                }
                mp.reset();
            }
        }
    });
}

/**
 * Handler for receiving new data from the mooltipass.
 * Decodes the HID message and updates the HTML message divider with
 * to report the received message.
 * @param data the received data
 */
mp.onDataReceived = function(reportId, data) {
    console.log('onDataReceived()')
    if (typeof reportId === "undefined" || typeof data === "undefined")
    {
        console.log('undefined response');
        if (chrome.runtime.lastError)
        {
            var err = chrome.runtime.lastError;
            if (err.message != "Transfer failed.")
            {
                console.log("Error in onDataReceived: " + err.message);
            }
        }
        return;
    }

    console.log('reportId', reportId);
    console.log('data', data);
}