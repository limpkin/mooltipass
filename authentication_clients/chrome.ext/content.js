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

/*!      \file content.js
*        \brief        Mooltipass Chrome Authentication plugin
*        Created: 14/6/2014
*        Author: Darran Hunt
*
*        Scans web pages for authentication inputs and then requests values for those inputs
*        from the Mooltipass.
*/


console.log('mooltipass content script loaded');

function getLogin(both) 
{
    var fields = ['password', 'email'];
    var elements = document.getElementsByTagName('input');
    var result = [];

    for (var i=0; i<elements.length; i++) {
        var input = elements[i] 
        var type = input.type.toLowerCase()
        console.log('input: "'+input.id+'" ('+input.type+') ')
        if ($.inArray(type, fields) != -1) {
            console.log('pushed "'+input.id+'" ('+input.type+') ')
            result.push({id: input.id, type: input.type});
        }
    }

    return result;
}

addEventListener('DOMContentLoaded', function f() {
    removeEventListener('DOMContentLoaded', f, false);
    console.log('mooltipass content script triggered');
    var res = getLogin(true);
    if (res.length > 0) {
        // send a message back to the extension
        chrome.runtime.sendMessage({type: 'inputs', url: window.location.href, inputs: res}, function(response) {
            console.log('content: got response ' + JSON.stringify(response));
        });
    }
});

chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) 
{
        if (request.type == 'credentials') 
        {
            // update the inputs
            for (var ind=0; ind<request.fields.length; ind++) {
                console.log('set: "'+request.fields[ind].id+'" = "'+request.fields[ind].value+'"');
                document.getElementById(request.fields[ind].id).value = request.fields[ind].value;
            }
        }
});
