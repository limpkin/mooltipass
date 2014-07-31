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


console.log('mooltipass content script loaded');

var credentialFieldTypes = {
    'password':'password',
    'email':'login',
    'user_id':'login',
    'user_login':'login',
    'username':'login',
    'name':'login'
};


function getCredential(input)
{
    console.log('getCredential('+input.id+')');
    var type = input.type.toLowerCase()
    var name = input.name.toLowerCase()
    var id = input.id.toLowerCase()
    var ctype = null;

    if (type in credentialFieldTypes)
    {
        ctype = type;
    }
    else if (name in credentialFieldTypes)
    {
        ctype = name;
    }
    else if (id in credentialFieldTypes)
    {
        ctype = id;
    }

    if (ctype) {
        if (input.id)
        {
            return {id: input.id, type: ctype};
        }
        else
        {
            return {name: input.name, type: ctype}
        }
    }
    return null;
}

function getCredentialFields() 
{
    var result = [];

    $('input').each( function(i) 
    {
        cred = getCredential(this);
        if (cred)
        {
            console.log('pushed '+JSON.stringify(cred));
            result.push(cred);
        }
    });

    return result;
}

// see if the field differs from the mooltipass value
function fieldChanged(field)
{
    if (!mpCreds)
    {
        return true;
    }
    for (var ind=0; ind<mpCreds.fields.length; ind++)
    {
        if (mpCreds.fields[ind].id == input.id) {
            if (input.value != mpCreds.fields[ind].value) {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    // field not found, so it is changed
    return true;
}

function credentialsChanged(creds)
{
    var changed = false;
    for (var ind=0; ind<creds.length; ind++) 
    {
        if ('id' in creds[ind])
        {
            console.log('changeCheck: cred '+creds[ind].id);
            input = document.getElementById(creds[ind].id);
        }
        else
        {
            console.log('changeCheck: cred '+creds[ind].name);
            input = document.getElementsByName(creds[ind].name)[0];
        }
        creds[ind].value = input.value;
        if (fieldChanged(input)) 
        {
            changed = true;
        }
    }
    return changed;
}

function checkSubmittedCredentials(form)
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
    console.log('mooltipass content script triggered');
    var forms = document.getElementsByTagName('form');
    $('form').submit(function(event) 
    {
        var form = this;
        if (checkSubmit)
        {
            console.log('checking submitted credentials');
            // see if we should store the credentials
            checkSubmittedCredentials(form);
            event.preventDefault();
        }
    });
    credFields = getCredentialFields();

    // send an array of the input fields to the mooltipass
    if (hasSecret(credFields))
    {
        // send a message back to the extension
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
            // update the inputs
            for (var ind=0; ind<request.fields.length; ind++) {
                if ('id' in request.fields[ind]) {
                    console.log('set: "'+request.fields[ind].id+'" = "'+request.fields[ind].value+'"');
                    document.getElementById(request.fields[ind].id).value = request.fields[ind].value;
                } else {
                    console.log('set: "'+request.fields[ind].name+'" = "'+request.fields[ind].value+'"');
                    document.getElementsByName(request.fields[ind].name)[0].value = request.fields[ind].value;
                }
            }
            // only submit if all credentials have been supplied?
            //checkSubmit = false;
            //$('form').submit();
            //checkSubmit = true;
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
