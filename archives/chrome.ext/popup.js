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

/*!      \file popup.js
*        \brief        Mooltipass Chrome extension popup
*        Created: 22/8/2014
*        Author: Darran Hunt
*/


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
    var clearButton = document.getElementById("clear");

    // clear contents of logs
    $('#messageLog').html('');
    var messageLog = $('#messageLog');

    $('#clear').click(function() { log('#messageLog'); });

    $('#rescanPage').click(function()
    {
        chrome.tabs.query({ active: true, currentWindow: true }, function(tabs) {
            console.log('got '+tabs.length+'tabs');
            console.log('tabs: '+JSON.stringify(tabs));
            chrome.tabs.sendMessage(tabs[0].id, {type: 'rescan'});
        });
    });

    $("#tabs").tabs();
    $("#rescanPage").button();
    $("#clear").button();
};

$(window).load(initWindow);
