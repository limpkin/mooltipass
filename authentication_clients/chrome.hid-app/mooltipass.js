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

var device_info = { "vendorId": 0x16d0, "productId": 0x09a0 };
//var device_info = { "vendorId": 0x16c0, "productId": 0x0486 };    // Teensy 3.1

var packetSize = 32;    // number of bytes in an HID packet

// Commands that the MP device can send.
var CMD_DEBUG   = 0x01
var CMD_PING    = 0x02
var CMD_VERSION = 0x03

var message = null;

var connection = -1;

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
        if (connection != -1) {
            console.log('Polling for response...');
            chrome.hid.receive(connection, packetSize, onDataReceived);
        } else {
            console.log('Not connected');
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

    console.log('Received data CMD ' + cmd + ', len ' + len);

    switch (cmd) 
    {
        case CMD_DEBUG:
        {
            var msg = "";
            for (var i = 0; i < len; i++) {
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
            message.innerHTML += "command: Version " + version + "<br />\n";
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
    devId = devices[0].deviceId;

    console.log('Connecting to device '+devId);
    chrome.hid.connect(devId, function(connectInfo) 
    {
        if (!chrome.runtime.lastError) {
            connection = connectInfo.connectionId;
            var version_cmd = [0x00, CMD_VERSION];
            data = new Uint8Array(version_cmd).buffer;
            console.log('sending '+version_cmd);
            chrome.hid.send(connection, 0, data, function() 
            {
                console.log('Send complete');
                chrome.hid.receive(connection, packetSize, onDataReceived);
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
