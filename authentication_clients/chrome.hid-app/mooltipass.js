/* CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at src/license_cddl-1.0.txt
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at src/license_cddl-1.0.txt
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */

/* Copyright (c) 2014 Darran Hunt. All rights reserved. */

/*!      \file mooltipass.js
*        \brief        Mooltipass Chrome HID App plugin
*        Created: 30/5/2014
*        Author: Darran Hunt
*/


/*
 * The Basic sequence is:
 * 1. extension detects credential input fields in a web page.
 * 2. extension sends request for credential values to this app
 * 3. app connects to mooltipass
 * 4. app sets context based on the URL of the web page
 * 5. app requests each of the credentials from the mooltipass
 * 6. app sends all of the credentials to the extension
 * 7. extension fills in the input fields in the web page.
 */

var device_info = { "vendorId": 0x16d0, "productId": 0x09a0 };      // Mooltipass
//var device_info = { "vendorId": 0x16c0, "productId": 0x0486 };    // Teensy 3.1

var packetSize = 64;    // number of bytes in an HID packet

var reContext = /^\https?\:\/\/([\w.]+)/;

// Commands that the MP device can send.
var CMD_DEBUG        = 0x01;
var CMD_PING         = 0x02;
var CMD_VERSION      = 0x03;
var CMD_CONTEXT      = 0x04;
var CMD_GET_LOGIN    = 0x05;
var CMD_GET_PASSWORD = 0x06;
var CMD_SET_LOGIN    = 0x07;
var CMD_SET_PASSWORD = 0x08;
var CMD_ADD_CONTEXT  = 0x0A;
var CMD_EXPORT_FLASH        = 0x30;    // resp: 0x30 packets until 0x31
var CMD_EXPORT_FLASH_END    = 0x31;
var CMD_IMPORT_FLASH_BEGIN  = 0x32;    // confirmed by 0x32,0x01
var CMD_IMPORT_FLASH        = 0x33;    // send 4x60 byte + 1x24 byte packets, acked with 0x33,0x01
var CMD_IMPORT_FLASH_END    = 0x34;
var CMD_EXPORT_EEPROM       = 0x35;    // resp: 0x35 packets until 0x36
var CMD_EXPORT_EEPROM_END   = 0x36;
var CMD_IMPORT_EEPROM_BEGIN = 0x37;    // confirmed by 0x37,0x01
var CMD_IMPORT_EEPROM       = 0x38;    // send packet, acked with 0x38,0x01
var CMD_IMPORT_EEPROM_END   = 0x39; 

var message = null;     // reference to the message div in the app HTML for logging
var debug = null;       // reference to the hidDebug div in the app HTML for hid debug logging
var exportLog = null;   // reference to the management tab log

var connection = null;  // connection to the mooltipass
var authReq = null;     // current authentication request
var context = null;
var contextGood = false;
var createContext = false;
var loginSet = false;
var loginValue = null;

var FLASH_PAGE_COUNT = 512;
var FLASH_PAGE_SIZE = 264;
var EEPROM_SIZE = 1024;

var exportData = null;        // arraybuffer for receiving exported data
var exportDataUint8 = null;   // uint8 view of exportData 
var exportDataEntry = null;   // File entry for flash export
var exportDataOffset = 0;     // current data offset in arraybuffer
var exportPacketCount = 0;    // number of packets received from mooltipass

var importProrgessBar = null;
var exportProgressBar = null;

// map between input field types and mooltipass credential types
var getFieldMap = {
    password:   CMD_GET_PASSWORD,
    email:      CMD_GET_LOGIN,
    username:   CMD_GET_LOGIN
};

var setFieldMap = {
    password:   CMD_SET_PASSWORD,
    email:      CMD_SET_LOGIN,
    username:   CMD_SET_LOGIN
};


/**
 * convert a string to a uint8 array
 * @param str the string to convert
 * @returns the uint8 array representing the string
 * @note does not support unicode yet
 */
function strToArray(str) 
{
    var buf = new Uint8Array(str.length);
    for (var ind=0; ind<str.length; ind++) 
    {
        buf[ind] = str.charCodeAt(ind);
    }
    return buf;
}
 

/**
 * convert a uint8 array to a string
 * @param buf the array to convert
 * @returns the string representation of the array
 * @note does not support unicode yet
 */
function arrayToStr(buf)
{
    res = '';
    for (var ind=0; ind<buf.length; ind++) 
    {
        if (buf[ind] == 0) 
        {
            return res;
        } else {
            res += String.fromCharCode(buf[ind]);
        }
    }
    return res;
}


/**
 * Send a command and string to the mooltipass
 * @param type the command type (e.g. CMD_SET_PASSWORD)
 * @param str the string to send with the command
 */
function sendString(type, str)
{
    var len = str.length + 1;
    msg = new ArrayBuffer(packetSize);
    header = new Uint8Array(msg, 0);
    body = new Uint8Array(msg, 2);

    header.set([len, type], 0);
    body.set(strToArray(str), 0);
    body.set[str.length] = 0;  // null terminator

    console.log('body '+JSON.stringify(body));

    chrome.hid.send(connection, 0, msg, function() 
    {
        if (!chrome.runtime.lastError) 
        {
            console.log('Send complete');
            //chrome.hid.receive(connection, packetSize, onContextAck);
        }
        else
        {
          console.log('Failed to send to device: '+chrome.runtime.lastError.message);
          throw chrome.runtime.lastError.message;  
        }					
    });
}


/**
 * Send a single byte request to the mooltipass
 * @param type the request type to send (e.g. CMD_VERSION)
 */
function sendRequest(type)
{
    msg = new ArrayBuffer(packetSize);
    header = new Uint8Array(msg, 0);
    header.set([0,type], 0);

    chrome.hid.send(connection, 0, msg, function() 
    {
        if (!chrome.runtime.lastError) 
        {
            console.log('Send complete');
        }
        else
        {
          console.log('Failed to send to device: '+chrome.runtime.lastError.message);
          throw chrome.runtime.lastError.message;  
        }					
    });
}


/**
 * Push the current pending credential onto the
 * credentials list with the specified value.
 * @param value the credential value
 */
function storeField(value)
{
    if (authReq.pending) {
        authReq.pending.value = value;
        authReq.credentials.push(authReq.pending);
        authReq.pending = null;
    }
    else
    {
        console.log('err: storeField('+value+') no field pending');
    }
}


/**
 * Get the next credential field value from the mooltipass
 * The pending credential is set to the next one, and
 * a request is sent to the mooltipass to retreive its value.
 */
function getNextField()
{
    if (authReq && authReq.type == 'inputs')
    {
        if (authReq.inputs.length > 0) 
        {
            authReq.pending = authReq.inputs.pop();
            var type = authReq.pending.type;

            if (type in getFieldMap)
            {
                if (type == 'password' && !loginSet && loginValue) 
                {
                    // need to set the login first
                    sendString(CMD_SET_LOGIN, loginValue);
                    return;
                }
                if (type == 'login')
                {
                    loginSet = false;
                    loginValue = null;
                }
                console.log('get '+type+' for '+authReq.context+' '+authReq.pending.type);
                message.innerHTML += 'get '+type+'<br />';
                sendRequest(getFieldMap[type]);
            }
            else
            {
                console.log('getNextField: type "'+authReq.pending.type+'" not supported');
                authReq.pending = null;
                getNextField(); // try the next field
            }
        } else {
            // no more input fields to fetch from mooltipass, send credentials to the web page
            chrome.runtime.sendMessage(authReq.senderId, {type: 'credentials', fields: authReq.credentials});
            message.innerHTML += 'sent credentials to '+authReq.senderId+'<br />';
            authReq = null;
        }
    }
    else
    {
        message.innerHTML += 'no authReq<br />';
    }
}


/**
 * Set the next credential field value from the mooltipass
 * The pending credential is set to the next one, and
 * a request is sent to the mooltipass to set its value.
 */
function setNextField()
{
    if (authReq && authReq.type == 'update')
    {
        if (authReq.inputs.length > 0) 
        {
            authReq.pending = authReq.inputs.pop();
            var type = authReq.pending.type;

            if (type in setFieldMap)
            {
                console.log('set '+type+' for '+authReq.context+' '+authReq.pending.type+' to '+authReq.pending.value);
                message.innerHTML += 'set '+type+' = "'+authReq.pending.value+'"<br />';
                sendString(setFieldMap[type], authReq.pending.value);
            }
            else
            {
                console.log('setNextField: type "'+authReq.pending.type+'" not supported');
                authReq.pending = null;
                getSetField(); // try the next field
            }
        } else {
            // no more input fields to set on mooltipass
            // XXX todo add an error check / ACK back to the web page?
            message.innerHTML += 'update finished <br />';
            authReq = null;
        }
    }
    else
    {
        message.innerHTML += 'no authReq<br />';
    }
}

function setContext(create)
{
    createContext = create;
    if (connection) 
    {
        message.innerHTML += 'Context: "'+authReq.context+'" ';
        // get credentials from mooltipass
        contextGood = false;
        sendString(CMD_CONTEXT, authReq.context);
    }
    else 
    {
        // not currently connected, attempt to connect
        console.log('app: not connected');
        console.log('Connecting to mooltipass...');
        message.innerHTML += 'Connecting... <br />';
        chrome.hid.getDevices(device_info, onDeviceFound);
    }
}

function saveToEntry(entry, data) 
{
    entry.createWriter(function(fileWriter) 
    {
        fileWriter.onwriteend = function(e) 
        {
            if (this.error)
            {
                exportLog.innerHTML += 'Error during write: ' + this.error.toString() + '<br />';
            }
            else
            {
                if (fileWriter.length === 0) {
                    // truncate has finished
                    var blob = new Blob([data], {type: 'application/octet-binary'});
                    exportLog.innerHTML += 'writing '+data.length+' bytes<br />';
                    fileWriter.write(blob);
                    exportLog.innerHTML += 'Save complete<br />';
                } else {
                }
            }
        }
        fileWriter.truncate(0);
    });
}



/**
 * Initialise the app window, setup message handlers.
 */
function initWindow()
{
    var connectButton = document.getElementById("connect");
    var receiveButton = document.getElementById("receiveResponse");
    var clearButton = document.getElementById("clear");
    var clearDebugButton = document.getElementById("clearDebug");
    var exportFlashButton = document.getElementById("exportFlash");
    var exportEepromButton = document.getElementById("exportEeprom");
    message = document.getElementById("messageLog");
    debug = document.getElementById("debugLog");
    exportLog = document.getElementById("exportLog");

    connectButton.addEventListener('click', function() 
    {
        console.log('Getting device...');
        chrome.hid.getDevices(device_info, onDeviceFound);
    });

    clearButton.addEventListener('click', function() 
    {
        console.log('Clearing log');
        message.innerHTML = '';
    });
    clearDebugButton.addEventListener('click', function() 
    {
        console.log('Clearing debug');
        debug.innerHTML = '';
    });
    
    exportFlashButton.addEventListener('click', function() 
    {
        chrome.fileSystem.chooseEntry({type:'saveFile', suggestedName:'mpflash.bin'}, function(entry) {
            exportLog.innerHTML = 'save mpflash.img <br />';
            exportDataEntry = entry;
            sendRequest(CMD_EXPORT_FLASH);
        });
    });

    exportEepromButton.addEventListener('click', function() 
    {
        chrome.fileSystem.chooseEntry({type:'saveFile', suggestedName:'mpeeprom.bin'}, function(entry) {
            exportLog.innerHTML = 'save mpeeprom.img <br />';
            exportDataEntry = entry;
            sendRequest(CMD_EXPORT_EEPROM);
        });
    });

    receiveButton.addEventListener('click', function() 
    {
        if (connection) 
        {
            console.log('Polling for response...');
            chrome.hid.receive(connection, packetSize, onDataReceived);
        } 
        else 
        {
            console.log('Not connected');
        }
    });

    chrome.runtime.onMessageExternal.addListener(function(request, sender, sendResponse) 
    {
        console.log(sender.tab ?  'from a content script:' + sender.tab.url : 'from the extension');

        switch (request.type)
        {
            case 'inputs':
                console.log('URL: '+request.url);

                // sort the fields so that the login is first
                request.inputs.sort(function(a, b)
                {
                    if (a == 'login')
                    {
                        return 0;
                    } else {
                        return 1;
                    }
                });

                console.log('inputs:');
                for (var ind=0; ind<request.inputs.length; ind++)
                {
                    console.log('    "'+request.inputs[ind].id+'" type='+request.inputs[ind].type);
                }
                authReq = request;
                authReq.senderId = sender.id;
                authReq.credentials = [];
                if (!context) {
                    //request.context = getContext(request); URL -> context
                    match = reContext.exec(request.url);
                    if (match.length > 0) {
                        console.log('match: '+JSON.stringify(match));
                        context = match[1];
                        console.log('context: '+context);
                    }
                    //context = 'accounts.google.com';
                }
                authReq.context = context;

                setContext(false);
                break;

            case 'update':
                authReq = request;
                authReq.senderId = sender.id;
                if (!context) {
                    //request.context = getContext(request); URL -> context
                    context = 'accounts.google.com';
                }
                authReq.context = context;

                // sort the fields so that the login is first
                authReq.inputs.sort(function(a, b)
                {
                    if (a == 'login')
                    {
                        return 0;
                    } else {
                        return 1;
                    }
                });

                if (!contextGood || (context != authReq.context)) {
                    setContext(true);
                } else {
                    setNextField();
                }
                break;

            default:
                break;
        }

    });

	$("#manage").accordion();
	importProrgessBar = $("#importProgressbar").progressbar({ value: 0 });
	exportProgressBar = $("#exportProgressbar").progressbar({ value: 0 });
    $("#connect").button();
    $("#receiveResponse").button();
    $("#clear").button();
    $("#clearDebug").button();
    $("#exportFlash").button();
    $("#exportEeprom").button();
    $("#tabs").tabs();
};


/**
 * Handler for receiving new data from the mooltipass.
 * Decodes the HID message and updates the HTML message divider with
 * to report the received message.
 * @param data the received data
 */
function onDataReceived(data) 
{
    var bytes = new Uint8Array(data);
    var len = bytes[0]
    var cmd = bytes[1]

    //console.log('Received data CMD ' + cmd + ', len ' + len + ' ' + JSON.stringify(bytes));

    switch (cmd) 
    {
        case CMD_DEBUG:
        {
            var msg = "";
            for (var i = 0; i < len; i++) 
            {
                    msg += String.fromCharCode(bytes[i+2]);
            }
            debug.innerHTML += msg + '<br />\n';
            break;
        }
        case CMD_PING:
            message.innerHTML += "command: ping<br />\n";
            break;
        case CMD_VERSION:
        {
            var version = "" + bytes[2] + "." + bytes[3];
            message.innerHTML += 'Connected to Mooltipass ' + version + '<br />\n';
            if (authReq) 
            {
                message.innerHTML += 'Context: "'+authReq.context+'" ';
                sendString(CMD_CONTEXT, authReq.context);
            }
            break;
        }
        case CMD_ADD_CONTEXT:
            contextGood = (bytes[2] == 1);
            if (!contextGood)
            {
                message.innerHTML += 'failed to create context '+authReq.context+'<br />';
            } else {
                message.innerHTML += 'created context '+authReq.context+'<br />';
                sendString(CMD_CONTEXT, authReq.context);
            }
            break;

        case CMD_CONTEXT:
            contextGood = (bytes[2] == 1);
            contextGood ? message.innerHTML += '(existing)<br />' : message.innerHTML += '(new)<br />';

            if (authReq) 
            {
                if (!contextGood && createContext) {
                    createContext = false;
                    message.innerHTML += 'add new context "'+authReq.context+'"<br />';
                    sendString(CMD_ADD_CONTEXT, authReq.context);
                    break;
                }

                switch (authReq.type)
                {
                    case 'inputs':
                        // Start getting each input field value
                        getNextField();
                        break;
                    case 'update':
                        setNextField();
                        break;
                    default:
                        break;
                }
            }
            break;

        // Input Fields
        case CMD_GET_LOGIN:
            if ((len > 1) && (loginValue == null)) {
                loginValue = arrayToStr(new Uint8Array(data.slice(2)));
            }
        case CMD_GET_PASSWORD:
        {
            if (len > 1) 
            {
                if (authReq && authReq.pending) {
                    message.innerHTML += authReq.pending.type;
                    var value = arrayToStr(new Uint8Array(data.slice(2)));
                    message.innerHTML += ': "'+value+'"<br />';
                    storeField(value);
                } else {
                    // no pending credential request
                }
            }
            else 
            {
                message.innerHTML += 'no value found for '+authReq.pending.type+'<br />';
            }
            getNextField();
            break;
        }

        // update and set results
        case CMD_SET_LOGIN:
            if (authReq && authReq.type == 'inputs' && authReq.pending) {
                if (bytes[2] == 1)
                {
                    loginSet = true;
                    console.log('get '+authReq.pending.type+' for '+authReq.context);
                    message.innerHTML += 'get '+authReq.pending.type+'<br />';
                    sendRequest(getFieldMap[authReq.pending.type]);
                    break;
                }
                // fallthrough
            }
        case CMD_SET_PASSWORD:
        {
            var type = (authReq && authReq.pending) ? authReq.pending.type : '(unknown type)';
            if (bytes[2] == 1) 
            {
                // success
                message.innerHTML += 'set '+type+' on mooltipass<br />';
            }
            else 
            {
                // failed
                message.innerHTML += 'set failed for '+type+'<br />';
            }
            setNextField();
            
            break;
        }

        case CMD_EXPORT_FLASH:
        case CMD_EXPORT_EEPROM:
            if (exportData == null)
            {
                console.log('new export');
                if (cmd == CMD_EXPORT_FLASH)
                {
                    exportData = new ArrayBuffer(FLASH_PAGE_COUNT*FLASH_PAGE_SIZE);
                }
                else
                {
                    exportData = new ArrayBuffer(EEPROM_SIZE);
                }
                exportDataUint8 = new Uint8Array(exportData);
                exportDataOffset = 0;
                exportPacketCount = 0;
                console.log('new export ready');
                exportProgressBar.progressbar('value', 0);
            }
            // flash packet
            packetId = new Uint16Array(data.slice(2,4));
            packet = new Uint8Array(data.slice(4,2+len));
            exportPacketCount += 1;
            if (exportPacketCount != packetId[0]) {
                console.log('error expected packet '+exportPacketCount+' but received '+packetId[0]);
                exportPacketCount = packetId[0];
            }
            if ((packet.length + exportDataOffset) > exportDataUint8.length)
            {
                var overflow = (packet.length + exportDataOffset) - exportDataUint8.length;
                console.log('error packet overflows buffer by '+overflow+' bytes');
                exportDataOffset += packet.length;
            } else {
                exportDataUint8.set(packet, exportDataOffset);
                exportDataOffset += packet.length;
                exportProgressBar.progressbar('value', (exportDataOffset * 100) / exportDataUint8.length);
            }
            break;

        case CMD_EXPORT_FLASH_END:
        case CMD_EXPORT_EEPROM_END:
            if (exportData && exportDataEntry)
            {
                exportLog.innerHTML += 'export: saving to file<br />';
                if (exportDataOffset < exportDataUint8.length)
                {
                    console.log('WARNING: only received '+exportDataOffset+' of '+exportDataUint8.length+' bytes');
                    exportLog.innerHTML += 'WARNING: only received '+exportDataOffset+' of '+exportDataUint8.length+' bytes<br />';
                }
                saveToEntry(exportDataEntry, exportDataUint8) 
                exportData = null;
                exportDataUint8 = null;
                exportDataOffset = 0;
                exportDataEntry = null;;
            }
            else
            {
                exportLog.innerHTML += 'Error received export end ('+cmd+') with no active export<br />';
            }
            break;

        default:
            message.innerHTML += "unknown command";
            break;
    }
    chrome.hid.receive(connection, packetSize, onDataReceived);
};


/**
 * Handler invoked when new USB mooltipass devices are found.
 * Connects to the device and sends a version request.
 * @param devices array of device objects
 * @note only the last device is used, assumes that one mooltipass is present.
 * Stale entries appear to be left in chrome if the mooltipass is removed
 * and plugged in again, or the firmware is updated.
 */
function onDeviceFound(devices) 
{
    var ind = devices.length - 1;
    console.log('Found ' + devices.length + ' devices.');
    console.log('Device ' + devices[ind].deviceId + ' vendor' + devices[ind].vendorId + ' product ' + devices[ind].productId);
    console.log('Device usage 0 usage_page' + devices[ind].usages[0].usage_page + ' usage ' + devices[ind].usages[0].usage);
    var devId = devices[ind].deviceId;

    console.log('Connecting to device '+devId);
    chrome.hid.connect(devId, function(connectInfo) 
    {
        if (!chrome.runtime.lastError) 
		{
            connection = connectInfo.connectionId;
			msg = new ArrayBuffer(packetSize);
            data = new Uint8Array(msg);
            data.set([0, CMD_VERSION], 0);
            console.log('sending '+JSON.stringify(data));
            chrome.hid.send(connection, 0, msg, function() 
            {
				if (!chrome.runtime.lastError) 
				{
					console.log('Send complete');
					chrome.hid.receive(connection, packetSize, onDataReceived);
				}
				else
				{
				  console.log('Failed to send to device: '+chrome.runtime.lastError.message);
				  throw chrome.runtime.lastError.message;  
				}					
            });
        }
        else 
        {
          console.log('Failed to connect to device: '+chrome.runtime.lastError.message);
          throw chrome.runtime.lastError.message;    
        } 
    });
}

window.addEventListener('load', initWindow);
