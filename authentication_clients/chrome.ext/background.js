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

/*!      \file background.js
*        \brief        Mooltipass Chrome Authentication plugin
*        Created: 14/6/2014
*        Author: Darran Hunt
*
*        Waits for requests from the web page content script for
*        credentials, asks the Mooltipass Client for them, then sends
*        the response back to the content script.
*/


var mpClient = null;
var contentAddr = null;
var connected = null;

function getAll(ext)
{
    for (var ind=0; ind<ext.length; ind++)
    {
        if (ext[ind].shortName == 'Mooltipass Client') 
        {
            mpClient = ext[ind];
            break;
        }
    }

    if (mpClient) 
    {
        chrome.runtime.sendMessage(mpClient.id, { type: 'ping' });
        console.log('found mooltipass client "'+ext[ind].shortName+'" id='+ext[ind].id);
    }
    else 
    {
        console.log('No mooltipass client found');
    }
}

// Search for the Mooltipass Client
chrome.management.getAll(getAll);

// Messages from the mooltipass client app
chrome.runtime.onMessageExternal.addListener(function(request, sender, sendResponse) 
{
    console.log('back: app req '+JSON.stringify(request));
    switch (request.type) 
    {
        case 'credentials':
            console.log('back: got credentials '+JSON.stringify(request));
            chrome.tabs.sendMessage(contentAddr, request);
            break;
        case 'updateComplete':
            console.log('back: got updateComplete');
            chrome.tabs.sendMessage(contentAddr, request);
            break;
        case 'connected':
            connected = request;
            if (contentAddr) {
                chrome.tabs.sendMessage(contentAddr, request);
            }
            break;
        case 'disconnected':
            connected = null;;
            if (contentAddr) {
                chrome.tabs.sendMessage(contentAddr, request);
            }
            break;
        case 'rescan':
            if (contentAddr) {
                chrome.tabs.sendMessage(contentAddr, request);
            }
            break;
        default:
            break;
    }
});

// Messages from the content script
chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) 
{
    console.log(sender.tab ?  'from a content script:' + sender.tab.url : 'from the extension');

    if (sender.tab) 
    {

        if (!mpClient) {
            // try to find the app
            chrome.management.getAll(getAll);
        }

        if (!mpClient) {
            console.log('back: mooltipass chrome app not running');
            return;
        }
        console.log('back: content req '+JSON.stringify(request));
        // from content script
        switch (request.type) 
        {
            case 'ping':
                contentAddr = sender.tab.id;
                if (connected) {
                    chrome.browserAction.setIcon({path: 'mooltipass-active.png'});
                    chrome.tabs.sendMessage(contentAddr, connected);
                } else {
                    if (mpClient.id) {
                        chrome.runtime.sendMessage(mpClient.id, { type: 'ping' });
                    } else {
                        sendResponse({type: 'disconnected'});
                    }
                    chrome.browserAction.setIcon({path: 'mooltipass-inactive.png'});
                }
                break;
            case 'inputs':
                contentAddr = sender.tab.id;
                console.log('inputs:');
                for (var i=0; i<request.inputs.length; i++) 
                {
                    console.log('    "'+request.inputs[i].id);
                }
                sendResponse({type: 'ack'});  // ack
                console.log('sending to '+mpClient.id);
                chrome.runtime.sendMessage(mpClient.id, request);
                break;
            case 'update':
                contentAddr = sender.tab.id;
                chrome.runtime.sendMessage(mpClient.id, request);
                break;
            default:
                break;
        }
    } 
});

