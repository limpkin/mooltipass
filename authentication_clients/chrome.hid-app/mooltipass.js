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

var packetSize = 32;    // number of bytes in an HID packet

// Commands that the MP device can send.
var CMD_DEBUG        = 0x01;
var CMD_PING         = 0x02;
var CMD_VERSION      = 0x03;
var CMD_CONTEXT      = 0x04;
var CMD_GET_LOGIN    = 0x05;
var CMD_GET_PASSWORD = 0x06;
var CMD_SET_LOGIN    = 0x07;
var CMD_SET_PASSWORD = 0x08;

var message = null;     // reference to the message div in the app HTML for logging

var connection = null;  // connection to the mooltipass
var authReq = null;     // current authentication request

// map between input field types and mooltipass credential types
var fieldMap = {
    password: CMD_GET_PASSWORD,
    email: CMD_GET_LOGIN,
    username: CMD_GET_LOGIN
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
    var len = 2 + str.length + 1;
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
    if (authReq)
    {
        if (authReq.inputs.length > 0) 
        {
            authReq.pending = authReq.inputs.pop();
            var type = authReq.pending.type;

            if (type in fieldMap)
            {
                console.log('get '+type+' for '+authReq.context+' '+authReq.pending.type);
                message.innerHTML += 'get '+type+'<br />';
                sendRequest(fieldMap[type]);
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
 * Initialise the app window, setup message handlers.
 */
function initWindow()
{
    var connectButton = document.getElementById("connect");
    var receiveButton = document.getElementById("receiveResponse");
    message = document.getElementById("message");

    connectButton.addEventListener('click', function() 
    {
        console.log('Getting device...');
        chrome.hid.getDevices(device_info, onDeviceFound);
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
        if (request.type == 'inputs') {
            console.log('URL: '+request.url);
            console.log('inputs:');
            for (var i=0; i<request.inputs.length; i++) {
                console.log('    "'+request.inputs[i].id+'" '+request.inputs[i].type);
            }
        }
        
        authReq = request;
        authReq.senderId = sender.id;
        authReq.credentials = [];
        //request.context = getContext(request); URL -> context
        authReq.context = 'accounts.google.com';

        if (connection) 
        {
            message.innerHTML = 'Context: "'+authReq.context+'" ';
            // get credentials from mooltipass
            sendString(CMD_CONTEXT, authReq.context);
        }
        else 
        {
            // not currently connected, attempt to connect
            console.log('app: not connected');
            console.log('Connecting to mooltipass...');
            message.innerHTML = 'Connecting... <br />';
            chrome.hid.getDevices(device_info, onDeviceFound);
        }
    });

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

    console.log('Received data CMD ' + cmd + ', len ' + len + ' ' + JSON.stringify(bytes));

    switch (cmd) 
    {
        case CMD_DEBUG:
        {
            var msg = "";
            for (var i = 0; i < len; i++) 
            {
                    msg += String.fromCharCode(bytes[i+2]);
            }
            message.innerHTML += "debug: '" + msg + "'<br />\n";
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
        case CMD_CONTEXT:
        {
            if (bytes[2] == 0) 
            {
                message.innerHTML += '(existing)<br />';
            }
            else
            {
                message.innerHTML += '(new)<br />';
            }

            // Start getting each input field value
            getNextField();
            break;
        }

        // Input Fields
        case CMD_GET_LOGIN:
        case CMD_GET_PASSWORD:
        {
            if (len > 1) 
            {
                message.innerHTML += authReq.pending.type;
                var value = arrayToStr(new Uint8Array(data.slice(2)));
                message.innerHTML += ': "'+value+'"<br />';
                storeField(value);
            }
            else 
            {
                message.innerHTML += 'no value found for '+authReq.pending.type+'<br />';
            }
            getNextField();
            break;
        }

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
 * @note only device 0 is used, assumes that one mooltipass is present.
 */
function onDeviceFound(devices) 
{
    console.log('Found ' + devices.length + ' devices.');
    console.log('Device ' + devices[0].deviceId + ' vendor' + devices[0].vendorId + ' product ' + devices[0].productId);
    console.log('Device 0 usage 0 usage_page' + devices[0].usages[0].usage_page + ' usage ' + devices[0].usages[0].usage);
    var devId = devices[0].deviceId;

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
