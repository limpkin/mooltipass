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

var mpCreds = null;
var credFields = null;
var credFieldsArray = null;
var checkSubmit = true;     // when true intercept the submit and check for update
var passForm = null;


console.log('mooltipass content script loaded');

var loginFieldTypes = {
    'user': 6,
    'username': 5,
    'user_id': 4,
    'user_login': 3,
    'name': 2,
    'email': 1
};

function isLogin(input)
{
    if (input.id.toLowerCase() in loginFieldTypes)
    {
        return loginFieldTypes[input.id.toLowerCase()];
    }
    if (input.name.toLowerCase() in loginFieldTypes)
    {
        return loginFieldTypes[input.name.toLowerCase()];
    }
    return 0;
}

function getCredentials(submitted)
{
    var pass = $(':password');
    var login = null;
    var loginPrecedence = 0;

    if (typeof pass == 'undefined' || !pass || typeof pass[0] == 'undefined') {
        console.log('no password input');
        return null;
    }

    console.log('getCredentials: password id='+pass[0].id+' name='+pass[0].name+'\n');

    // check for login input
    $(':input').each( function(i) {
        var newLogin = isLogin(this);
        if (newLogin > loginPrecedence) {
            if (submitted && this.value != '') {
                loginPrecedence = newLogin;
                login = this;
            }
        }
    });

    if (!login) {
        pindex = $(':input').index( pass );
        if (pindex > 0) {
            login = $(':input').get(pindex-1);
        }
    }
    console.log('getCredentials: login id='+login.id+' name='+login.name+'\n');

    passForm = $(pass[0]).closest('form');

    return { login: {id: login.id, name: login.name}, password: {id: pass[0].id, name: pass[0].name} };
}


// see if the field differs from the mooltipass value
function fieldChanged(input)
{
    if (!mpCreds)
    {
        return true;
    }

    for (var key in mpCreds.inputs)
    {
        console.log('check '+key);
        if ((input.id) && (mpCreds.inputs[key].id == input.id)) {
            return (input.value != mpCreds.inputs[key].value) ? true : false;
        } else if ((input.name) && (mpCreds.inputs[key].name == input.name)) {
            return (input.value != mpCreds.inputs[key].value) ? true : false;
        } 
    }
    console.log('fieldChanged: input not found');
    // input not found, consider it changed 
    return true;
}


function credentialsChanged(creds)
{
    var changed = false;

    for (var key in creds) {
        if (creds[key].id)
        {
            console.log('changeCheck: cred '+creds[key].id);
            input = document.getElementById(creds[key].id);
        }
        else
        {
            console.log('changeCheck: cred '+creds[key].name);
            input = document.getElementsByName(creds[key].name)[0];
        }
        creds[key].value = input.value;
        if (fieldChanged(input)) 
        {
            changed = true;
        }
    }
    return changed;
}


function checkSubmittedCredentials(form, event)
{
    if (!credFields) 
    {
        // no credential fields, nothing to do here
        return;
    }

    var updateNeeded = false;
    console.log('check: creds');
    if (credFields) {
        // we have some, see if the values differ from what the mooltipass has
        if (credentialsChanged(credFields))
        {
            // Offer to update the mooltpass with the new value(s)
            if (!document.getElementById('mpDialog')) {
                console.log('content: creating  dialog div');
                var layerNode= document.createElement('div');
                layerNode.setAttribute('id', 'mpDialog');
                layerNode.setAttribute('title','Mooltipass');
                var pNode= document.createElement('p');
                pNode.innerHTML = '';
                layerNode.appendChild(pNode);
                document.body.appendChild(layerNode);
            }
            console.log('content: update dialog');
            event.preventDefault();
            $( "#mpDialog" ).dialog({
                autoOpen: true,
                show: {
                    effect: 'drop',
                    direction: 'up',
                    duration: 500 
                },
                hide: {
                    effect: 'puff',
                    duration: 500
                },
                buttons: {
                    "Update Mooltipass credentials": function() 
                    {
                        chrome.runtime.sendMessage({type: 'update', url: window.location.href, inputs: credFields});
                        $(this).dialog('close');
                    },
                    Skip: function() 
                    {
                        form.submit();
                        $(this).dialog('close');
                    }
                }
            });
            console.log('content: after update dialog');
        }
        else
        {
            form.submit();
        }
    } else {
        // we don't have any, see if the user wants to add some
    }
}

function hasSecret(credlist)
{
    for (var ind=0; ind<credlist.length; ind++)
    {
        if (credlist[ind].type == 'password')
            return true;
    }
    return false;
}

addEventListener('DOMContentLoaded', function f() 
{
    removeEventListener('DOMContentLoaded', f, false);
    var forms = document.getElementsByTagName('form');
    $('form').submit(function(event) 
    {
        var form = this;
        if (checkSubmit)
        {
            console.log('checking submitted credentials');
            // see if we should store the credentials
            checkSubmittedCredentials(form, event);
        }
    });
    credFields = getCredentials();

    if (credFields == null) {
        // no credentials
        console.log('no credentials, not sending');
        return;
    }

    console.log('found credentials '+ JSON.stringify(credFields)+'\n');

    // send an array of the input fields to the mooltipass
    if ('password' in credFields)
    {
        // send a message back to the extension
        console.log('sending credentials '+JSON.stringify(credFields)+'\n');
        chrome.runtime.sendMessage({type: 'inputs', url: window.location.href, inputs: credFields}, function(response) 
        {
            console.log('content: got response ' + JSON.stringify(response));
        });
    }
    else
    {
        console.log('no password in credentials, not sending');
    }
});


chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) 
{
    switch (request.type) 
    {
        case 'credentials':
            mpCreds = request;
            //console.log('got credentials: '+JSON.stringify(request.inputs));
            // update the inputs
            for (var key in request.inputs) {
                if (request.inputs[key].id) {
                    //console.log('set: "'+request.inputs[key].id+'" = "'+request.inputs[key].value+'"');
                    document.getElementById(request.inputs[key].id).value = request.inputs[key].value;
                } else {
                    //console.log('set: "'+request.inputs[key].name+'" = "'+request.inputs[key].value+'"');
                    document.getElementsByName(request.inputs[key].name)[0].value = request.inputs[key].value;
                }
            }
            // only submit if all credentials have been supplied?
            if (passForm) {
                checkSubmit = false;
                passForm.submit();
                checkSubmit = true;
            }
            break;

        case 'updateComplete':
            console.log('updateComplete, sending submit.');
            checkSubmit = false;
            $('form').submit();
            checkSubmit = true;
            break;

        default:
            break;
    }
});
