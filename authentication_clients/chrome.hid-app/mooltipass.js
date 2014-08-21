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
var debug = false;

var packetSize = 64;    // number of bytes in an HID packet
var payloadSize = packetSize - 2;

var AUTH_REQ_TIMEOUT = 15000;   // timeout for requests sent to mooltipass

var reContext = /^\https?\:\/\/([\w.\-\_]+)/;   // URL regex to extract base domain for context
//var reContext = /(^.{1,254}$)(^(((?!-)[a-zA-Z0-9-]{1,63}(?<!-))|((?!-)[a-zA-Z0-9-]{1,63}(?<!-)\.)+[a-zA-Z]{2,63})$/;
//var reContext = /https?\:\/\/(?www\.)?([-a-zA-Z0-9@:%._\+~#=]{2,256}\.[a-z]{2,4})\b(?[-a-zA-Z0-9@:%_\+.~#?&//=]*)/;

// Commands that the MP device can send.
var CMD_DEBUG               = 0x01;
var CMD_PING                = 0x02;
var CMD_VERSION             = 0x03;
var CMD_CONTEXT             = 0x04;
var CMD_GET_LOGIN           = 0x05;
var CMD_GET_PASSWORD        = 0x06;
var CMD_SET_LOGIN           = 0x07;
var CMD_SET_PASSWORD        = 0x08;
var CMD_CHECK_PASSWORD      = 0x09;
var CMD_ADD_CONTEXT         = 0x0A;
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
var CMD_ERASE_SMARTCARD     = 0x42;
var CMD_DRAW_BITMAP         = 0x43;	
var CMD_SET_FONT            = 0x44;
var CMD_EXPORT_FLASH_START  = 0x45;
var CMD_EXPORT_EEPROM_START = 0x46;
var CMD_SET_BOOTLOADER_PWD  = 0x47;
var CMD_JUMP_TO_BOOTLOADER  = 0x48;
var CMD_CLONE_SMARTCARD     = 0x49;
var CMD_STACK_FREE          = 0x50;

// supported flash chips
// 264,   512,  128   1MB   0001 ID:00010=2  5  7 12, 6 2 16 S: 3 - 8,120,128
// 264,  1024,  128   2MB   0010 ID:00011=3  5  7 12, 5 3 16 S: 7 - 8,120,128
// 264,  2048,  256   4MB   0100 ID:00100=4  4  8 12, 4 5 17 S: 7 - 8,248,256
// 264,  4096,  256   8MB   1000 ID:00101=5  3  9 12, 3 4 17 S: 15 - 8,248,256
// 528,  4096,  256  16MB  10000 ID:00110=6  2  9 13, 2 4 18 S: 15 - 8,248,256
// 528,  8192,  128  32MB 100000 ID:00111=7  1 10 13, 1 6 17 S: 63 - 8,120,128

var FLASH_CHIP_1M           = 1;   // 1M Flash Chip (AT45DB011D)
var FLASH_CHIP_2M           = 2;   // 2M Flash Chip (AT45DB021E)
var FLASH_CHIP_4M           = 4;   // 4M Flash Chip (AT45DB041E)
var FLASH_CHIP_8M           = 8;   // 8M Flash Chip (AT45DB081E)
var FLASH_CHIP_16M          = 16;  // 16M Flash Chip (AT45DB161E)
var FLASH_CHIP_32M          = 32;  // 32M Flash Chip (AT45DB321E)
var FLASH_MEDIA_START_PAGE  = 8;

var flashInfo = {
     1: { pageSize: 264, pageCount:  512, pagesPerSector: 128 },
     2: { pageSize: 264, pageCount: 1024, pagesPerSector: 128 },
     4: { pageSize: 264, pageCount: 2048, pagesPerSector: 256 },
     8: { pageSize: 264, pageCount: 4096, pagesPerSector: 256 },
    16: { pageSize: 528, pageCount: 4096, pagesPerSector: 256 },
    32: { pageSize: 528, pageCount: 8192, pagesPerSector: 128 }
};

var flashChipId = null;


var clientId = null;     // chrome extension address
var connection = null;   // connection to the mooltipass
var connected = false;   // current connection state
var version = 'unknown'; // connected mooltipass version
var authReq = null;      // current authentication request
var authReqQueue = [];
var context = null;
var contextGood = false;
var createContext = false;
var loginValue = null;

var connectMsg = null;  // saved message to send after connecting

var FLASH_PAGE_COUNT = 512;
var FLASH_PAGE_SIZE = 264;
var EEPROM_SIZE = 1024;
var FLASH_EXPORT_ALL = false;

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

var authReqTimeout = null;  // timer for auth requests sent to mooltipass

// map between input field types and mooltipass credential types
var getFieldMap = {
    password:   CMD_GET_PASSWORD,
    login:      CMD_GET_LOGIN,
};

var setFieldMap = {
    password:   CMD_SET_PASSWORD,
    login:      CMD_SET_LOGIN,
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
    connected = false;
    authReq = null;     // current authentication request
    context = null;
    contextGood = false;
    createContext = false;
    loginValue = null;
    connectMsg = null;

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
    sendMsg(msg);
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
 * Get the next credential field value from the mooltipass
 * The pending credential is set to the next one, and
 * a request is sent to the mooltipass to get its value.
 */
function getNextField()
{
    if (authReq && authReq.type == 'inputs')
    {
        if (authReq.keys.length > 0) {
            authReq.pending = authReq.keys.pop();
            //console.log('getNextField(): request '+authReq.pending);
            sendRequest(getFieldMap[authReq.pending]);
        } else {
            // got all the credentials
            //console.log('getNextField(): got all fields '+JSON.stringify(authReq.inputs));
            chrome.runtime.sendMessage(authReq.senderId, {type: 'credentials', inputs: authReq.inputs});
            log('#messageLog','sent credentials\n');
            authReq = null;
        }
    }
    else
    {
        log('#messageLog',  'getNextField(): no authReq\n');
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
        if (authReq.keys.length > 0) 
        {
            var key = authReq.keys.pop();
            authReq.pending = key;

            if (key in setFieldMap)
            {
                sendString(setFieldMap[key], authReq.inputs[key].value);
            }
            else
            {
                console.log('setNextField: type "'+authReq.key+'" not supported');
                authReq.pending = null;
                setNextField(); // try the next field
            }
        } else {
            // no more input fields to set on mooltipass
            chrome.runtime.sendMessage(authReq.senderId, {type: 'updateComplete'});
            log('#messageLog', 'update finished \n');
            endAuthRequest();
        }
    }
    else
    {
        log('#messageLog',  'setNextField: no authReq\n');
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
 * Return a sorted list of field keys
 */
function getKeys(fields)
{
    return Object.keys(fields).sort(function(a, b)
    {
        if (a == 'login')
        {
            return 1;
        } else {
            return 0;
        }
    });
}

/**
 * timeout waiting for current auth request to complete
 */
function timeoutAuthRequest()
{
    console.log('authreq timeout');
    authReqTimeout = null;
    endAuthRequest();
}

/**
 * end the current auth request
 */
function endAuthRequest()
{
    if (debug) {
        if (authReq) {
            console.log('endAuthRequest '+authReq.type);
        } else {
            console.log('endAuthRequest - no active request');
        }
    }
    if (authReqTimeout != null) {
        console.log('clear authreq timeout');
        clearTimeout(authReqTimeout);
        authReqTimeout = null;
    } else {
        console.log('no timeout to clear');
    }
    if (authReqQueue.length > 0) {
        authReq = authReqQueue.shift();
        startAuthRequest(authReq);
    } else {
        authReq = null;
    }
}

/**
 * start a new auth request
 */
function startAuthRequest(request)
{
    switch (request.type) {
    case 'ping':    // hellow from extension
        clientId = request.senderId;
        // Send current mooltipass status
        if (connected) {
            console.log('got extension ping, sending connected');
            chrome.runtime.sendMessage(request.senderId, {type: 'connected', version: version});
        } else {
            console.log('got extension ping, sending disconnected');
            chrome.runtime.sendMessage(request.senderId, {type: 'disconnected'});
        }
        break;
    case 'inputs':
        clientId = request.senderId;
        authReq = request;
        console.log('URL: '+request.url);
        authReq.keys = getKeys(request.inputs);

        console.log('keys: '+JSON.stringify(authReq.keys))

        match = reContext.exec(request.url);
        if (match.length > 0) {
            if (!context || context != match[1]) {
                context = match[1];
                console.log('context: '+context);
            } else {
                console.log('not updating context '+context+' to '+match[1]);
            }
        }
        authReq.context = context;

        authReqTimeout = setTimeout(timeoutAuthRequest, AUTH_REQ_TIMEOUT);
        setContext(false);
        break;

    case 'update':
        clientId = request.senderId;
        authReq = request;
        match = reContext.exec(request.url);
        if (match.length > 0) {
            authReq.context = match[1];
            console.log('auth context: '+authReq.context);
        }
        log('#messageLog', 'update:\n');
        for (var key in request.inputs)
        {
            id = (request.inputs[key].id) ? request.inputs[key].id : request.inputs[key].name;
            if (key == 'password') {
                log('#messageLog', '    set "'+id+'" = "'+request.inputs[key].value.replace(/./gi, '*')+'"\n');
            } else {
                log('#messageLog', '    set "'+id+'" = "'+request.inputs[key].value+'"\n');
            }
        }

        authReq.keys = getKeys(request.inputs);
        authReqTimeout = setTimeout(timeoutAuthRequest, AUTH_REQ_TIMEOUT);

        if (!contextGood || (context != authReq.context)) {
            setContext(true);
        } else {
            setNextField();
        }
        break;

    default:
        // not a supported request type
        endAuthRequest();
        break;
    }
}


/**
 * Initialise the app window, setup message handlers.
 */
function initWindow()
{
    var clearButton = document.getElementById("clear");
    var clearDebugButton = document.getElementById("clearDebug");
    var exportFlashButton = document.getElementById("exportFlash");
    var exportEepromButton = document.getElementById("exportEeprom");
    var exportMediaButton = document.getElementById("exportMedia");
    var importFlashButton = document.getElementById("importFlash");
    var importEepromButton = document.getElementById("importEeprom");
    var importMediaButton = document.getElementById("importMedia");
    var sendCMDButton = document.getElementById("sendCMD");
    var jumpToBootloader = document.getElementById("jumpToBootloader");
    var cloneSmartcard = document.getElementById("cloneSmartcard");
    var drawBitmapButton = document.getElementById("drawBitmap");
    var setFontButton = document.getElementById("setFont");
    var fillButton = document.getElementById("fill");

    // clear contents of logs
    $('#messageLog').html('');
    $('#debugLog').html('');
    $('#exportLog').html('');
    $('#developerLog').html('');
    var messageLog = $('#messageLog');

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
                sendRequest(CMD_EXPORT_FLASH_START);
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
                sendRequest(CMD_EXPORT_EEPROM_START);
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

    sendCMDButton.addEventListener('click', function()
    {
        var command = parseInt($('#sendCMDvalue').val(), 16);
	if (command >= 0 && command <= 255){ 
	    log('#messageLog', 'Sending '+ $('#sendCMDvalue').val() + '\n');
            sendRequest(command);
        } else{
            log('#messageLog', 'command out of range\n');
        }
    });

    jumpToBootloader.addEventListener('click', function() 
    {
        log('#messageLog', 'Sending JUMP_TO_BOOTLOADER\n');
        sendRequest(CMD_JUMP_TO_BOOTLOADER);
    });

    cloneSmartcard.addEventListener('click', function() 
    {
        log('#messageLog', 'Cloning smartcard\n');
        sendRequest(CMD_CLONE_SMARTCARD);
    });

    drawBitmapButton.addEventListener('click', function() 
    {
        args = new Uint8Array([$('#bitmapId').val(), $('#bitmap_x').val(), $('#bitmap_y').val(), $("#bitmap_clear").is(':checked') ? 1 : 0]);
        log('#messageLog', 'draw bitmap '+args[0]+' x='+args[1]+', y='+args[2]+', clear='+args[3]+'\n');
        sendRequest(CMD_DRAW_BITMAP, args);
    });

    setFontButton.addEventListener('click', function() 
    {
        var args = strToArray(String.fromCharCode($('#fontId').val()) + $('#fontTestString').val());
        log('#messageLog', 'set font '+args[0]+' "'+$('#fontTestString').val()+'"\n');
        sendRequest(CMD_SET_FONT, args);
    });

    $('#enableDebug').change(function() {
        if ($(this).is(":checked")) {
            log('#messageLog', 'enabled debug\n');
            debug = true;
        } else {
            log('#messageLog', 'disabled debug\n');
            debug = false;
        }
    });

    // configure jquery ui elements
	$("#manage").accordion();
	$("#developer").accordion();
	importProgressBar = $("#importProgressbar").progressbar({ value: 0 });
	exportProgressBar = $("#exportProgressbar").progressbar({ value: 0 });
    $("#clear").button();
    $("#clearDebug").button();
    $("#exportFlash").button();
    $("#exportEeprom").button();
    $("#exportMedia").button();
    $("#importFlash").button();
    $("#importEeprom").button();
    $("#importMedia").button();
    $("#sendCMD").button();
    $("#jumpToBootloader").button();
    $("#cloneSmartcard").button();
    $("#drawBitmap").button();
    $("#setFont").button();
    $("#fill").button();
    $("#tabs").tabs();

    var eraseOptions = {
        'eeprom and flash': { query: 'Erase EEPROM and Flash?', cmd: CMD_ERASE_EEPROM },
        'flash':            { query: 'Erase Flash?',            cmd: CMD_ERASE_FLASH },
        'smartcard':        { query: 'Erase smartcard?',        cmd: CMD_ERASE_SMARTCARD }
    };
                             
    $("#erase").menu({
        select: function(event, ui) {
            var option = ui.item.text();
            if (option in eraseOptions) {
                query = eraseOptions[option].query;
                var buts = {};
                buts[query] = function() {
                        log('#developerLog', 'Erasing '+option+'... ');
                        sendRequest(eraseOptions[option].cmd);
                        $(this).dialog('close');
                    }
                buts['Cancel'] = function() {
                        $(this).dialog('close');
                    }
                $('#eraseConfirm').dialog({buttons: buts});
            }
        }
    });

    chrome.runtime.onMessageExternal.addListener(function(request, sender, sendResponse) 
    {
        request.senderId = sender.id;
        console.log('received request '+request.type);

        if (authReq == null) {
            startAuthRequest(request)
        } else {
            authReqQueue.push(request);
        }
    });

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
        sendRequest(cmd+1);     // END
        return 0;
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

    return importer.data.byteLength - importer.offset;
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
function onDataReceived(reportId, data) 
{
    if (typeof reportId === "undefined" || typeof data === "undefined")
    {
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

    var bytes = new Uint8Array(data);
    var msg = new Uint8Array(data,2);
    var len = bytes[0]
    var cmd = bytes[1]

    if (debug && (cmd != CMD_VERSION) && (cmd != CMD_DEBUG) && ((cmd < CMD_EXPORT_FLASH) || (cmd >= CMD_EXPORT_EEPROM)))
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
            version = arrayToStr(new Uint8Array(data.slice(3)));
            if (!connected)
            {
                flashChipId = msg[0];
                log('#messageLog', 'Connected to Mooltipass ' + version + ' flashId '+flashChipId+'\n');
                connected = true;
                if (clientId) {
                    chrome.runtime.sendMessage(clientId, {type: 'connected', version: version});
                }
            }
            break;
        }
        case CMD_ADD_CONTEXT:
            contextGood = (bytes[2] == 1);
            if (!contextGood)
            {
                log('#messageLog',  'failed to create context '+authReq.context+'\n');
                endAuthRequest();
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
                    if (authReq.type == 'inputs') {
                        getNextField();
                    } else {
                        setNextField();
                    }
                }
                else if (createContext) 
                {
                    createContext = false;
                    log('#messageLog','add new context "'+authReq.context+'" for '+authReq.type+'\n');
                    sendString(CMD_ADD_CONTEXT, authReq.context);
                } else {
                    console.log('Failed to set up context "'+authReq.context+'"');
                    // failed to set up context
                    endAuthRequest();
                }
            }
            break;

        // Input Fields
        case CMD_GET_LOGIN:
        case CMD_GET_PASSWORD:
            if (authReq && authReq.pending) {
                if (len > 1) {
                    var key = authReq.pending;
                    var value = arrayToStr(new Uint8Array(data.slice(2)));
                    if (key == 'password') {
                        log('#messageLog',  'got '+key+' = "'+value.replace(/./gi, '*')+'"\n');
                    } else {
                        log('#messageLog',  'got '+key+' = "'+value+'"\n');
                    }
                    authReq.inputs[key].value = value;

                    if (authReq.type == 'inputs') {
                        getNextField();
                    } else {
                        setNextField();
                    }
                } else {
                    // failed
                    endAuthRequest();
                }
            }
            break;

        // update and set results
        case CMD_SET_LOGIN:
            if (authReq && authReq.type == 'inputs' && authReq.pending) {
                if (bytes[2] == 1)
                {
                    console.log('set '+authReq.pending+' for '+authReq.context);
                    log('#messageLog', 'get '+authReq.pending+'\n');
                    // XXX is this still needed?
                    sendRequest(getFieldMap[authReq.pending]);
                    break;
                }
                // fallthrough
            }
        case CMD_SET_PASSWORD:
        {
            var type = (authReq && authReq.pending) ? authReq.pending : '(unknown type)';
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

        case CMD_EXPORT_FLASH_START:
        {
            var ok = (bytes[2] == 1);

            if (ok)
            {
                // proceed
                args = new Uint8Array([0]);     // restart export from 0
                sendRequest(CMD_EXPORT_FLASH, args);
            }
            break;
        }

        case CMD_EXPORT_EEPROM_START:
        {
            var ok = (bytes[2] == 1);

            if (ok)
            {
                // proceed
                args = new Uint8Array([0]);     // restart export from 0
                sendRequest(CMD_EXPORT_EEPROM, args);
            }
            break;
        }

        case CMD_EXPORT_FLASH:
        case CMD_EXPORT_EEPROM:
            if (!exportData)
            {
                console.log('new export');
                var size;
                if (cmd == CMD_EXPORT_FLASH)
                {
                    console.log('flashChipId '+flashChipId + 
                                ' pageSize ' + flashInfo[flashChipId].pageSize + 
                                ' pages '+ flashInfo[flashChipId].pageCount +
                                ' media start page '+ FLASH_MEDIA_START_PAGE);

                    size = (flashInfo[flashChipId].pageSize * flashInfo[flashChipId].pageCount);

                    if (!FLASH_EXPORT_ALL) {
                        // export skips the media partition
                        size -= (flashInfo[flashChipId].pageSize * (flashInfo[flashChipId].pagesPerSector - FLASH_MEDIA_START_PAGE));
                    }
                }
                else
                {
                    size = EEPROM_SIZE;
                }
                console.log('exporting '+size+' bytes');
                exportData = new ArrayBuffer(size);
                exportDataUint8 = new Uint8Array(exportData);
                exportDataOffset = 0;
                console.log('new export ready');
                exportProgressBar.progressbar('value', 0);
            }
            // data packet
            packet = new Uint8Array(data.slice(2,2+len));
            if ((packet.length + exportDataOffset) < exportDataUint8.length)
            {
                exportDataUint8.set(packet, exportDataOffset);
                exportDataOffset += packet.length;
                exportProgressBar.progressbar('value', (exportDataOffset * 100) / exportDataUint8.length);
                args = new Uint8Array([1]);     // request next packet
                sendRequest(cmd, args);
            } else {
                if ((packet.length + exportDataOffset) > exportDataUint8.length)
                {
                    var overflow = (packet.length + exportDataOffset) - exportDataUint8.length;
                    console.log('error packet overflows buffer by '+overflow+' bytes');
                }

                // done, write the file to disk
                saveToEntry(exportDataEntry, exportDataUint8) 
                exportData = null;
                exportDataUint8 = null;
                exportDataOffset = 0;
                exportDataEntry = null;;
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
        case CMD_ERASE_SMARTCARD:
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
                remainder = importData.data.byteLength - importData.offset;
                if (remainder > 0) {
                    log('#importLog', 'import halted, '+remainder+' bytes left\n');
                } else {
                    log('#importLog', 'import finished\n');
                }
                importData = null;
            } else {
                sendNextPacket(CMD_IMPORT_EEPROM, importData);
            }
            break;
        }

        default:
            log('#messageLog', 'unknown command '+cmd+'\n');
            break;
    }
    if (connection) {
        chrome.hid.receive(connection, onDataReceived);
    }
};

function sendMsg(msg)
{
    if (debug) {
        msgUint8 = new Uint8Array(msg);
        // don't output the CMD_VERSION command since this is the keep alive
        if (msgUint8[1] != CMD_VERSION) {
            console.log('sending '+JSON.stringify(new Uint8Array(msg)));
        }
    }
    chrome.hid.send(connection, 0, msg, function() 
    {
        if (!chrome.runtime.lastError) 
        {
            chrome.hid.receive(connection, onDataReceived);
        }
        else
        {
            if (connected)
            {
                if (debug) {
                    console.log('Failed to send to device: '+chrome.runtime.lastError.message);
                }
                log('#messageLog', 'Disconnected from mooltipass\n');
                if (clientId) {
                    chrome.runtime.sendMessage(clientId, {type: 'disconnected'});
                }
                reset();
            }
        }					
    });
}

function sendPing()
{
    msg = new ArrayBuffer(packetSize);
    data = new Uint8Array(msg);
    data.set([0, CMD_VERSION], 0);
    sendMsg(msg);
}


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
    if (devices.length <= 0)
    {
        return;
    }

    var ind = devices.length - 1;
    console.log('Found ' + devices.length + ' devices.');
    console.log('Device ' + devices[ind].deviceId + ' vendor' + devices[ind].vendorId + ' product ' + devices[ind].productId);
    //console.log('Device usage 0 usage_page' + devices[ind].usages[0].usage_page + ' usage ' + devices[ind].usages[0].usage);
    var devId = devices[ind].deviceId;

    console.log('Connecting to device '+devId);
    log('#messageLog', 'Connecting to device...\n');
    chrome.hid.connect(devId, function(connectInfo) 
    {
        if (!chrome.runtime.lastError) 
		{
            connection = connectInfo.connectionId;

            if (connectMsg)
            {
                sendMsg(connectMsg);
            }
            else
            {
                sendPing();
            }
        }
        else 
        {
          console.log('Failed to connect to device: '+chrome.runtime.lastError.message);
          reset();
        } 
    });
}

function checkConnection()
{
    if (!connected) {
        connect();
    } else {
        sendPing();
    }
}

setInterval(checkConnection,2000);

window.addEventListener('load', initWindow);
