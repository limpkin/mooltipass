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
var payloadSize = packetSize - 2;

var reContext = /^\https?\:\/\/([\w.]+)/;   // URL regex to extract base domain for context

// Commands that the MP device can send.
var CMD_DEBUG        = 0x01;
var CMD_PING         = 0x02;
var CMD_VERSION      = 0x03;
var CMD_CONTEXT      = 0x04;
var CMD_GET_LOGIN    = 0x05;
var CMD_GET_PASSWORD = 0x06;
var CMD_SET_LOGIN    = 0x07;
var CMD_SET_PASSWORD = 0x08;
var CMD_CHECK_PASSWORD = 0x09;
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

var CMD_ERASE_EEPROM        = 0x40;
var CMD_ERASE_FLASH         = 0x41;
var CMD_ERASE_SMC           = 0x42;
var CMD_DRAW_BITMAP         = 0x43;

var connection = null;  // connection to the mooltipass
var authReq = null;     // current authentication request
var context = null;
var contextGood = false;
var createContext = false;
var loginValue = null;

var connectMsg = null;  // saved message to send after connecting

var FLASH_PAGE_COUNT = 512;
var FLASH_PAGE_SIZE = 264;
var EEPROM_SIZE = 1024;

var exportData = null;        // arraybuffer for receiving exported data
var exportDataUint8 = null;   // uint8 view of exportData 
var exportDataEntry = null;   // File entry for flash export
var exportDataOffset = 0;     // current data offset in arraybuffer

var importData = {};        // arraybuffer holding data to import exported data
var importDataOffset = 0;     // current data offset in import arraybuffer
var importDataPageOffset = 0; // current write page offset

var mediaData = null;        // arraybuffer of media file data for slot storage
var mediaDataOffset = 0;     // current data offset in mediaData array

var media = {};             // media file info from mooltipass

var importProgressBar = null;
var exportProgressBar = null;
var uploadProgressBar = null;

// map between input field types and mooltipass credential types
var getFieldMap = {
    password:   CMD_GET_PASSWORD,
    email:      CMD_GET_LOGIN,
    username:   CMD_GET_LOGIN,
    user_id:    CMD_GET_LOGIN,
    name:       CMD_SET_LOGIN
};

var setFieldMap = {
    password:   CMD_SET_PASSWORD,
    email:      CMD_SET_LOGIN,
    username:   CMD_SET_LOGIN,
    user_id:    CMD_SET_LOGIN,
    name:       CMD_SET_LOGIN
};


/**
 * convert a string to a uint8 array
 * @param str the string to convert
 * @returns the uint8 array representing the string with a null terminator
 * @note does not support unicode yet
 */
function strToArray(str) 
{
    var buf = new Uint8Array(str.length+1);
    for (var ind=0; ind<str.length; ind++) 
    {
        buf[ind] = str.charCodeAt(ind);
    }
    buf[ind] = 0;
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
 * reset all state data
 */
function reset()
{
    connection = null;  // connection to the mooltipass
    //authReq = null;     // current authentication request
    //context = null;
    //contextGood = false;
    //createContext = false;
    loginValue = null;

    exportData = null;        // arraybuffer for receiving exported data
    exportDataUint8 = null;   // uint8 view of exportData 
    exportDataEntry = null;   // File entry for flash export
    exportDataOffset = 0;     // current data offset in arraybuffer
}

/**
 * Connect to the mooltipass
 */
function connect(msg)
{
    reset();
    console.log('Connecting...');
    if (msg)
    {
        connectMsg = msg;
    }
    chrome.hid.getDevices(device_info, onDeviceFound);
}


/**
 * Send a binary message to the mooltipass
 * @param type the request type to send (e.g. CMD_VERSION)
 * @param content Uint8 array message content (optional)
 */
function sendRequest(type, content)
{
    msg = new ArrayBuffer(packetSize);
    header = new Uint8Array(msg, 0);
    body = new Uint8Array(msg, 2);

    if (content)
    {
        header.set([content.length, type], 0);
        body.set(content, 0);
        if (false && type != CMD_EXPORT_FLASH && type != CMD_EXPORT_EEPROM && type != CMD_IMPORT_FLASH)
        {
            log('#messageLog','body '+JSON.stringify(body)+'\n');
        }
    }
    else
    {
        header.set([0, type], 0);
    }

    if (!connection)
    {
        connect(msg);
        return;
    }

    chrome.hid.send(connection, 0, msg, function() 
    {
        if (!chrome.runtime.lastError) 
        {
            //log('#messageLog','Send complete\n')
            //chrome.hid.receive(connection, packetSize, onContextAck);
        }
        else
        {
          log('#messageLog', 'Failed to send to device: '+chrome.runtime.lastError.message+'\n');
          reset();
        }					
    });
}


/**
 * Send a command and string to the mooltipass
 * @param type the command type (e.g. CMD_SET_PASSWORD)
 * @param str the string to send with the command
 */
function sendString(type, str)
{
    sendRequest(type, strToArray(str));
}


/**
 * Send a command and string to the mooltipass
 * @param type the command type (e.g. CMD_SET_PASSWORD)
 * @param data array of uint8arrays to combine into a message
 */
function sendRequestArray(type, data)
{
    payload = new ArrayBuffer(packetSize);
    body = new Uint8Array(payload, 0);
    var offset = 0;

    log('#messageLog', 'building request from array length '+data.length+'\n');

    for (var ind=0; ind<data.size; ind++)
    {
        log('#messageLog', 'adding array length '+data[ind].length+' '+JSON.stringify(data[ind])+'\n');
        body.set(data[ind], offset);
        offset += data[ind].length;
    }
    sendRequest(type, body);
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
                if (type == 'login')
                {
                    loginValue = null;
                }
                console.log('get '+type+' for '+authReq.context+' '+authReq.pending.type);
                log('#messageLog',  'get '+type+'\n');
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
            log('#messageLog','sent credentials to '+authReq.senderId+'\n');
            authReq = null;
        }
    }
    else
    {
        log('#messageLog', 'no authReq\n');
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
                log('#messageLog', 'set '+type+' = "'+authReq.pending.value+'"\n');
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
            chrome.runtime.sendMessage(authReq.senderId, {type: 'updateComplete'});
            log('#messageLog', 'update finished \n');
            authReq = null;
        }
    }
    else
    {
        log('#messageLog',  'no authReq\n');
    }
}

function setContext(create)
{
    createContext = create;
    log('#messageLog', 'Set context: "'+authReq.context+'" \n');
    // get credentials from mooltipass
    contextGood = false;
    sendString(CMD_CONTEXT, authReq.context);
}

function saveToEntry(entry, data) 
{
    entry.createWriter(function(fileWriter) 
    {
        fileWriter.onwriteend = function(e) 
        {
            if (this.error)
            {
                log('#exportLog', 'Error during write: ' + this.error.toString() + '\n');
            }
            else
            {
                if (fileWriter.length === 0) {
                    // truncate has finished
                    var blob = new Blob([data], {type: 'application/octet-binary'});
                    log('#exportLog',  'writing '+data.length+' bytes\n');
                    fileWriter.write(blob);
                    log('#exportLog', 'Save complete\n');
                } else {
                }
            }
        }
        fileWriter.truncate(0);
    });
}

function log(logId, text)
{
    var box = $(logId);
    if (text) {
        box.val(box.val() + text);
        box.scrollTop(box[0].scrollHeight);
    } else {
        box.val('');
    }
}

/**
 * Initialise the app window, setup message handlers.
 */
function initWindow()
{
    var connectButton = document.getElementById("connect");
    var clearButton = document.getElementById("clear");
    var clearDebugButton = document.getElementById("clearDebug");
    var exportFlashButton = document.getElementById("exportFlash");
    var exportEepromButton = document.getElementById("exportEeprom");
    var exportMediaButton = document.getElementById("exportMedia");
    var importFlashButton = document.getElementById("importFlash");
    var importEepromButton = document.getElementById("importEeprom");
    var importMediaButton = document.getElementById("importMedia");
    var eraseEepromButton = document.getElementById("eraseEeprom");
    var eraseFlashButton = document.getElementById("eraseFlash");
    var eraseSmartcardButton = document.getElementById("eraseSmartcard");
    var eraseMediaButton = document.getElementById("eraseMedia");
    var drawBitmapButton = document.getElementById("drawBitmap");

    // clear contents of logs
    $('#messageLog').html('');
    $('#debugLog').html('');
    $('#exportLog').html('');
    $('#developerLog').html('');
    var messageLog = $('#messageLog');

    connectButton.addEventListener('click', function() { connect(); });
    clearButton.addEventListener('click', function() { log('#messageLog'); });
    clearDebugButton.addEventListener('click', function() {  log('#debugLog'); });
    
    exportFlashButton.addEventListener('click', function() 
    {
        chrome.fileSystem.chooseEntry({type:'saveFile', suggestedName:'mpflash.bin'}, function(entry) {
            if (entry)
            {
                log('#exportLog');  // clear log
                log('#exportLog', 'save mpflash.img\n');
                exportDataEntry = entry;
                exportData = null;
                exportProgressBar.progressbar('value', 0);
                args = new Uint8Array([0]);     // restart export from 0
                sendRequest(CMD_EXPORT_FLASH, args);
            }
        });
    });

    exportEepromButton.addEventListener('click', function() 
    {
        chrome.fileSystem.chooseEntry({type:'saveFile', suggestedName:'mpeeprom.bin'}, function(entry) {
            if (entry)
            {
                log('#exportLog');  // clear log
                log('#exportLog', 'save mpeeprom.img\n');
                exportDataEntry = entry;
                exportData = null;
                exportProgressBar.progressbar('value', 0);
                args = new Uint8Array([0]);     // restart export from 0
                sendRequest(CMD_EXPORT_EEPROM, args);
            }
        });
    });

    exportMediaButton.addEventListener('click', function() 
    {
        chrome.fileSystem.chooseEntry({type:'saveFile', suggestedName:'media.bin'}, function(entry) {
            if (entry)
            {
                log('#exportLog');  // clear log
                log('#exportLog', 'save media.bin\n');
                exportDataEntry = entry;
                exportData = null;
                exportProgressBar.progressbar('value', 0);
                args = new Uint8Array([1]);     // media
                sendRequest(CMD_EXPORT_FLASH, args);
            }
        });
    });

    importFlashButton.addEventListener('click', function() 
    {
        chrome.fileSystem.chooseEntry({type: 'openFile'}, function(entry) {
            entry.file(function(file) {
                var reader = new FileReader();
                reader.onerror = function(e)
                {
                    log('#importLog', 'Failed to read file\n');
                    log('#importLog', e+'\n');
                };
                reader.onloadend = function(e) {
                    importData = { data:reader.result, log:'#importLog', bar:importProgressBar };
                    importProgressBar.progressbar('value', 0);
                    // Request permission to send
                    args = new Uint8Array([0]);         // user space, 1 = media
                    sendRequest(CMD_IMPORT_FLASH_BEGIN, args);
                    log('#importLog');  // clear log
                };

                reader.readAsArrayBuffer(file);
            });
        });
    });

    importEepromButton.addEventListener('click', function() 
    {
        chrome.fileSystem.chooseEntry({type: 'openFile'}, function(entry) {
            entry.file(function(file) {
                var reader = new FileReader();

                reader.onerror = function(e)
                {
                    log('#importLog', 'Failed to read file\n');
                    log('#importLog', e+'\n');
                };
                reader.onloadend = function(e) {
                    importData = { data:reader.result, log:'#importLog', bar:importProgressBar };
                    importProgressBar.progressbar('value', 0);
                    // Request permission to send
                    args = new Uint8Array([0]);
                    sendRequest(CMD_IMPORT_EEPROM_BEGIN, args);
                    log('#importLog');  // clear log
                };

                reader.readAsArrayBuffer(file);
            });
        });
    });

    importMediaButton.addEventListener('click', function() 
    {
        chrome.fileSystem.chooseEntry({type: 'openFile'}, function(entry) {
            entry.file(function(file) {
                var reader = new FileReader();

                reader.onerror = function(e)
                {
                    log('#importLog', 'Failed to read file\n');
                    log('#importLog', e+'\n');
                };
                reader.onloadend = function(e) {
                    importData = { data:reader.result, log:'#importLog', bar:importProgressBar };
                    importProgressBar.progressbar('value', 0);

                    // Request permission to send
                    args = new Uint8Array([1]);     // media
                    sendRequest(CMD_IMPORT_FLASH_BEGIN, args);
                    log('#importLog');  // clear log
                };

                reader.readAsArrayBuffer(file);
            });
        });
    });

    eraseFlashButton.addEventListener('click', function() 
    {
        $('#eraseConfirm').dialog({
            buttons: {
                "Erase Mooltipass Flash?": function() 
                {
                    log('#developerLog', 'Erasing flash... ');
                    sendRequest(CMD_ERASE_FLASH);
                    $(this).dialog('close');
                },
                Cancel: function() 
                {
                    $(this).dialog('close');
                }
            }
        });
    });

    eraseEepromButton.addEventListener('click', function() 
    {
        $('#eraseConfirm').dialog({
            buttons: {
                "Erase Mooltipass EEPROM?": function() 
                {
                    log('#developerLog', 'Erasing EEPROM... ');
                    sendRequest(CMD_ERASE_EEPROM);
                    $(this).dialog('close');
                },
                Cancel: function() 
                {
                    $(this).dialog('close');
                }
            }
        });
    });

    eraseSmartcardButton.addEventListener('click', function() 
    {
        $('#eraseConfirm').dialog({
            buttons: {
                "Erase Mooltipass Smartcard?": function() 
                {
                    log('#developerLog', 'Erasing smartcard... ');
                    sendRequest(CMD_ERASE_SMC);
                    $(this).dialog('close');
                },
                Cancel: function() 
                {
                    $(this).dialog('close');
                }
            }
        });
    });

    eraseMediaButton.addEventListener('click', function() 
    {
        $('#eraseConfirm').dialog({
            buttons: {
                "Erase media files?": function() 
                {
                    log('#developerLog', 'Erasing media files... ');
                    //sendRequest(CMD_MEDIA_ERASE);
                    $(this).dialog('close');
                },
                Cancel: function() 
                {
                    $(this).dialog('close');
                }
            }
        });
    });

    drawBitmapButton.addEventListener('click', function() 
    {
        log('#messageLog', 'drawing bitmap 0');
        args = new Uint8Array([0]);
        sendRequest(CMD_DRAW_BITMAP, args);
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
                    id = 'id' in request.inputs[ind] ? request.inputs[ind].id : request.inputs[ind].name;
                    console.log('    "'+id+'" type='+request.inputs[ind].type);
                }
                authReq = request;
                authReq.senderId = sender.id;
                authReq.credentials = [];
                //request.context = getContext(request); URL -> context
                match = reContext.exec(request.url);
                if (match.length > 0) {
                    if (!context || context != match[1]) {
                        context = match[1];
                        console.log('context: '+context);
                    } else {
                        console.log('not updaing context '+context+' to '+match[1]);
                    }
                }
                authReq.context = context;

                setContext(false);
                break;

            case 'update':
                authReq = request;
                authReq.senderId = sender.id;
                match = reContext.exec(request.url);
                if (match.length > 0) {
                    authReq.context = match[1];
                    console.log('auth context: '+authReq.context);
                }
                console.log('update:');
                for (var ind=0; ind<request.inputs.length; ind++)
                {
                    id = 'id' in request.inputs[ind] ? request.inputs[ind].id : request.inputs[ind].name;
                    console.log('    "'+id+'" type='+request.inputs[ind].type+', value="'+request.inputs[ind].value);
                }

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

    // configure jquery ui elements
	$("#manage").accordion();
	$("#developer").accordion();
	importProgressBar = $("#importProgressbar").progressbar({ value: 0 });
	exportProgressBar = $("#exportProgressbar").progressbar({ value: 0 });
    $("#connect").button();
    $("#clear").button();
    $("#clearDebug").button();
    $("#exportFlash").button();
    $("#exportEeprom").button();
    $("#exportMedia").button();
    $("#importFlash").button();
    $("#importEeprom").button();
    $("#importMedia").button();
    $("#eraseFlash").button();
    $("#eraseEeprom").button();
    $("#eraseSmartcard").button();
    $("#eraseMedia").button();
    $("#drawBitmap").button();
    $("#tabs").tabs();
};

/**
/* importer = { data: ArrayBuffer of data to send
 *               offset: current offset in data
 *               pageSpace: space left in page
 *               log: optional output log
 *               bar: optional progress bar
 *               }
 */
function sendNextPacket(cmd, importer)
{
    // need to match data to page sizes
    if (!importer.pageSpace)
    {
        importer.pageSpace = FLASH_PAGE_SIZE;
        importer.offset = 0;
    }
    var size = Math.min(importer.data.byteLength - importer.offset, payloadSize, importer.pageSpace);

    if (size <= 0)
    {
        // finished
        log(importer.log, 'import complete.\n');
        sendRequest(cmd+1);     // END
        return;
    }

    data = new Uint8Array(importer.data, importer.offset, size);

    // debug
    if (false && importer.log && ((importer.offset * 100)/importer.data.byteLength) > 95)
    {
        log(importer.log, 'import: offset '+importer.offset+' size '+size+' pageSpace '+importer.pageSpace+'\n');
    }

    importer.pageSpace -= size;
    if (importer.pageSpace <= 0)
    {
        importer.pageSpace = FLASH_PAGE_SIZE;
    }
    importer.offset += size;

    if (importer.bar)
    {
        importer.bar.progressbar('value', (importer.offset * 100)/ importer.data.byteLength);
    }
    sendRequest(cmd, data);
}

function updateMedia(data)
{
    update = new Uint16Array(data,2,8);
    files = new Uint16Array(data,6,40);
    fileSizes = new Uint16Array(data,46,20);

    media.pageSize = update[0];
    media.size = update[1];
    media.file = new Uint16Array(files.length);
    media.file.set(files);
    media.fileSize = new Uint8Array(fileSizes.length);
    media.fileSize.set(fileSizes);
}

function allocateMediaPage(size)
{
    var ind=0;
}

/**
 * Handler for receiving new data from the mooltipass.
 * Decodes the HID message and updates the HTML message divider with
 * to report the received message.
 * @param data the received data
 */
function onDataReceived(data) 
{
    var bytes = new Uint8Array(data);
    var msg = new Uint8Array(data,2);
    var len = bytes[0]
    var cmd = bytes[1]

    if ((cmd != CMD_DEBUG) && (cmd < CMD_EXPORT_FLASH))
    {
        console.log('Received CMD ' + cmd + ', len ' + len + ' ' + JSON.stringify(msg));
    }

    switch (cmd) 
    {
        case CMD_DEBUG:
        {
            var msg = "";
            for (var i = 0; i < len; i++) 
            {
                    msg += String.fromCharCode(bytes[i+2]);
            }
            log('#debugLog', msg);
            break;
        }
        case CMD_PING:
            log('#messageLog', 'command: ping\n');
            break;
        case CMD_VERSION:
        {
            var version = "" + bytes[2] + "." + bytes[3];
            log('#messageLog', 'Connected to Mooltipass ' + version + '\n');
            if (authReq) 
            {
                log('#messageLog', 'Context: "'+authReq.context+'"\n');
                sendString(CMD_CONTEXT, authReq.context);
                console.log('Initial set context "'+authReq.context+'"');
            }
            break;
        }
        case CMD_ADD_CONTEXT:
            contextGood = (bytes[2] == 1);
            if (!contextGood)
            {
                log('#messageLog',  'failed to create context '+authReq.context+'\n');
            } else {
                log('#messageLog', 'created context "'+authReq.context+'" for '+authReq.type+'\n');
                log('#messageLog', 'setting context "'+authReq.context+'" for '+authReq.type+'\n');
                console.log('Added context, now set context "'+authReq.context+'" for '+authReq.type);
                sendString(CMD_CONTEXT, authReq.context);
            }
            break;

        case CMD_CONTEXT:
            contextGood = (bytes[2] == 1);

            if (contextGood) {
                log('#messageLog', 'Active: "'+authReq.context+'" for '+authReq.type+'\n');
                console.log('Successfully set context "'+authReq.context+'" for '+authReq.type);
            } else {
                console.log('Failed to set context "'+authReq.context+'"');
                log('#messageLog','Unknown context "'+authReq.context+'" for '+authReq.type+'\n');
            }

            if (authReq) 
            {
                if (contextGood)
                {
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
                else if (createContext) 
                {
                    createContext = false;
                    log('#messageLog','add new context "'+authReq.context+'" for '+authReq.type+'\n');
                    sendString(CMD_ADD_CONTEXT, authReq.context);
                } else {
                    // failed to set up context
                    authReq = null;
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
                    log('#messageLog',  authReq.pending.type);
                    var value = arrayToStr(new Uint8Array(data.slice(2)));
                    log('#messageLog', ': "'+value+'"\n');
                    storeField(value);
                } else {
                    // no pending credential request
                }
            }
            else 
            {
                log('#messageLog', 'no value found for '+authReq.pending.type+'\n');
            }
            getNextField();
            break;
        }

        // update and set results
        case CMD_SET_LOGIN:
            if (authReq && authReq.type == 'inputs' && authReq.pending) {
                if (bytes[2] == 1)
                {
                    console.log('get '+authReq.pending.type+' for '+authReq.context);
                    log('#messageLog', 'get '+authReq.pending.type+'\n');
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
                log('#messageLog', 'set '+type+' on mooltipass\n');
            }
            else 
            {
                // failed
                log('#messageLog', 'set failed for '+type+'\n');
            }
            setNextField();
            
            break;
        }

        case CMD_EXPORT_FLASH:
        case CMD_EXPORT_EEPROM:
            if (!exportData)
            {
                console.log('new export');
                var size = (cmd == CMD_EXPORT_FLASH) ? (FLASH_PAGE_COUNT*FLASH_PAGE_SIZE) : EEPROM_SIZE;
                exportData = new ArrayBuffer(size);
                exportDataUint8 = new Uint8Array(exportData);
                exportDataOffset = 0;
                console.log('new export ready');
                exportProgressBar.progressbar('value', 0);
            }
            // data packet
            packet = new Uint8Array(data.slice(2,2+len));
            if ((packet.length + exportDataOffset) > exportDataUint8.length)
            {
                var overflow = (packet.length + exportDataOffset) - exportDataUint8.length;
                console.log('error packet overflows buffer by '+overflow+' bytes');
                exportDataOffset += packet.length;
            } else {
                exportDataUint8.set(packet, exportDataOffset);
                exportDataOffset += packet.length;
                exportProgressBar.progressbar('value', (exportDataOffset * 100) / exportDataUint8.length);
                args = new Uint8Array([1]);     // request next packet
                sendRequest(cmd, args);
            }
            break;

        case CMD_EXPORT_FLASH_END:
        case CMD_EXPORT_EEPROM_END:
            if (exportData && exportDataEntry)
            {
                log('#exportLog', 'export: saving to file\n');
                if (exportDataOffset < exportDataUint8.length)
                {
                    console.log('WARNING: only received '+exportDataOffset+' of '+exportDataUint8.length+' bytes');
                    log('#exportLog', 'WARNING: only received '+exportDataOffset+' of '+exportDataUint8.length+' bytes\n');
                }
                saveToEntry(exportDataEntry, exportDataUint8) 
            }
            else
            {
                log('#exportLog', 'Error received export end ('+cmd+') with no active export\n');
            }
            exportData = null;
            exportDataUint8 = null;
            exportDataOffset = 0;
            exportDataEntry = null;;
            break;

        case CMD_ERASE_EEPROM:
        case CMD_ERASE_FLASH:
        case CMD_ERASE_SMC:
            log('#developerLog', (bytes[2] == 1) ? 'succeeded\n' : 'failed\n');
            break;

        case CMD_IMPORT_FLASH_BEGIN: 
        case CMD_IMPORT_FLASH: 
        {
            var ok = bytes[2];
            if (ok == 0) {
                log('#importLog', 'import denied\n');
            } else {
                sendNextPacket(CMD_IMPORT_FLASH, importData);
            }
            break;
        }

        case CMD_IMPORT_FLASH_END:
        case CMD_IMPORT_EEPROM_END:
            importData = null;
            log('#importLog', 'import finished\n');
            break;

        case CMD_IMPORT_EEPROM_BEGIN: 
        case CMD_IMPORT_EEPROM: 
        {
            var ok = bytes[2];
            if (ok == 0) {
                log('#importLog', 'import denied\n');
            } else {
                sendNextPacket(CMD_IMPORT_EEPROM, importData);
            }
            break;
        }

        default:
            log('#messageLog', 'unknown command '+cmd+'\n');
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

            if (connectMsg)
            {
                // message pending to send
                msg = connectMsg;
                connectMsg = null;
                console.log('sending '+JSON.stringify(new Uint8Array(msg)));
            }
            else
            {
                msg = new ArrayBuffer(packetSize);
                data = new Uint8Array(msg);
                data.set([0, CMD_VERSION], 0);
                console.log('sending '+JSON.stringify(data));
            }
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
                  log('#messageLog','Unable to connect.\n');
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
