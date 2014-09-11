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
var credObjs = null;
var credFieldsArray = null;
var checkSubmit = true;     // when true intercept the submit and check for update
var passForm = null;
var formSubs = {}
var passwords = [];
var activeCredentials = null;
var mooltipassConnected = false;    // Active mooltipass connected to app

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
    if (input.type.toLowerCase() in loginFieldTypes)
    {
        return loginFieldTypes[input.type.toLowerCase()];
    }
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
    var pass = $(':password')[0];
    var loginInput = null;
    var loginPrecedence = 0;

    if (typeof pass == 'undefined' || !pass) {
        console.log('no password input');
        return null;
    }

    // this needs to be extended to support multiple credential entries in a page
    // with the correct association of username and submit.
    passwords = []
    $(':password').each(function(index) {
        var forms = $(this).closest('form');
        if (forms.length > 0) {
            var submits = forms.find(':submit');
            if (submits.length > 0) {
                passwords.push( {form: forms[0], password: this, submit: submits[0]} );
                console.log('getCredentials: pass id='+this.id+' name='+this.name+' form='+forms[0].id+' submit='+submits[0].value);
            } else {
                passwords.push( {form: forms[0], password: this, submit: null} );
                console.log('getCredentials: pass id='+this.id+' name='+this.name+' form='+forms[0].id+' no submit button');
            }
        }
    });

    if (passwords.length > 0) {
        passForm = passwords[0].form;
        pass = passwords[0].password;
        activeCredentials = passwords[0];
    } else {
        console.log('getCredentials: ERROR no passwords credentials found');
    }

    console.log('getCredentials: password id='+pass.id+' name='+pass.name+'\n');

    // check for login input
    $(':input').each( function(i) {
        if (this.id) {
            id = this.id;
        } else {
            id = this.name;
        }
        console.log('checking '+id);
        var newLogin = isLogin(this);
        if (newLogin > loginPrecedence) {
            if (submitted) {
                if (this.value != '') {
                    loginPrecedence = newLogin;
                    loginInput = this;
                    console.log('using '+id);
                }
            } else {
                loginPrecedence = newLogin;
                loginInput = this;
                console.log('using '+id);
            }
        }
    });

    if (!loginInput) {
        loginInput = $(pass).closest('input:text')[0];
        if (!loginInput) {
            loginInput = $(pass).closest('input[type=email]')[0];
        }
        if (!loginInput) {
            pindex = $('input:text, input:password').index( pass );
            if (pindex > 0) {
                loginInput = $('input:text, input:password').get(pindex-1);
            }
        }
    }
    if (!loginInput) {
        console.log('Failed to find login input');
        return null;
    } else {
        console.log('getCredentials: login id='+(loginInput.id?loginInput.id:'none')+' name='+(loginInput.name?loginInput.name:'none')+'\n');
    }

    credObjs = { login: loginInput, password: pass };

    passwords[0]['login'] = loginInput;
    return { login: {id: loginInput.id, name: loginInput.name}, password: {id: pass.id, name: pass.name} };
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
            return ($(input).val() != mpCreds.inputs[key].value) ? true : false;
        } else if ((input.name) && (mpCreds.inputs[key].name == input.name)) {
            return ($(input).val() != mpCreds.inputs[key].value) ? true : false;
        } 
    }
    console.log('fieldChanged: input not found');
    // input not found, consider it changed 
    return true;
}


function credentialsChanged(creds)
{
    var changed = false;
    var input;

    for (var key in creds) {
        input = credObjs[key];
        if (!input) {
            console.log('no input found for key '+key);
        }
        if (false) {
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
        }
        console.log('value = '+input.value);
        creds[key].value = input.value;
        if (fieldChanged(input)) 
        {
            changed = true;
        }
    }
    return changed;
}


function checkSubmittedCredentials(event)
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
                var pNode= document.createElement('moolti');
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
                        doSubmit(activeCredentials);
                        $(this).dialog('close');
                    }
                }
            });
            console.log('content: after update dialog');
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

function handleSubmit(event)
{
    if (!mooltipassConnected) return;

    if (checkSubmit)
    {
        console.log('click: checking submitted credentials');
        // see if we should store the credentials
        checkSubmittedCredentials(event);
    }
    else 
    {
        console.log('click: skipping submit check');
        return true;
    }
}

function sendCredentials(credFields)
{

    if (!credFields) {
        return false;
    }

    // send an array of the input fields to the mooltipass
    if ('password' in credFields)
    {

        if (activeCredentials) {
            console.log('using submit click technique');
            if (activeCredentials.submit) {
                $(activeCredentials.submit).click(handleSubmit);
            } else {
                $(activeCredentials.form).submit(handleSubmit);
            }
        } else {
            $('form').each(function(i,e)
            {
                var onsubmit = $(e).attr("onsubmit");
                if (onsubmit != null) {
                    $(e).removeAttr('onsubmit');
                    console.log('form '+e.id+' removed submit "'+onsubmit+'"');
                    onsubmit = new Function(onsubmit);
                    formSubs[$(e).id] = onsubmit;
                    $(e).attr('onsubmit', 'alert("replaced submit")');
                } else {
                    formSubs[$(e).id] = null;
                }
                $(e).submit(handleSubmit);
            });
        }

        // send a message back to the extension
        //console.log('sending credentials '+JSON.stringify(credFields)+'\n');
        console.log('sending credentials ');
        chrome.runtime.sendMessage({type: 'inputs', url: window.location.href, inputs: credFields}, function(response) 
        {
            console.log('content: got response ' + JSON.stringify(response));
        });

        return true;
    }
    else
    {
        console.log('no password in credentials, not sending');
        return false;
    }
}

function recheckCredentials()
{
    credFields = getCredentials();
    if (credFields == null) {
        console.log('recheck: no credentials');
    }
    sendCredentials(credFields);
}

// Scan for credentials as late as possible.
$(window).load(function() 
{
    credFields = getCredentials();

    if (credFields == null) {
        // no credentials
        console.log('no credentials, not sending');

        // Rescan after a small timeout.  Scripts may
        // add credential fields after the window is loaded.
        setTimeout(recheckCredentials,500);
        return;
    }

    sendCredentials(credFields);
});

/**
 * Submit the credentials to the server
 */
function doSubmit(creds)
{
    checkSubmit = false;
    if (creds) {
        if (creds.submit) {
            console.log('submitting form '+creds.form.id+' via '+creds.submit.id);
            $(creds.submit).click();
        } else {
            console.log('submitting form '+creds.form.id);
            $(creds.form).submit();
        }
    } else {
        console.log('submitting default form '+$('form').id);
        $('form').submit();
    }
    checkSubmit = true;
}

chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) 
{
    switch (request.type) 
    {
        case 'credentials':
            mpCreds = request;
            //console.log('got credentials: '+JSON.stringify(request.inputs));
            // update the inputs
            for (var key in request.inputs) {
                if (passwords.length > 0) {
                    switch (key) {
                        case 'login':
                            $(passwords[0].login).val(request.inputs[key].value);
                        break;

                        case 'password':
                            $(passwords[0].password).val(request.inputs[key].value);
                        break;

                        default:
                        break;
                    }
                } else {
                    if (request.inputs[key].id) {
                        //console.log('set: "'+request.inputs[key].id+'" = "'+request.inputs[key].value+'"');
                        document.getElementById(request.inputs[key].id).value = request.inputs[key].value;
                    } else {
                        //console.log('set: "'+request.inputs[key].name+'" = "'+request.inputs[key].value+'"');
                        document.getElementsByName(request.inputs[key].name)[0].value = request.inputs[key].value;
                    }
                }
            }

            doSubmit(activeCredentials);
            break;

        case 'updateComplete':
            console.log('updateComplete, sending submit.');
            doSubmit(activeCredentials);
            break;

        case 'connected':
            mooltipassConnected = true;
            console.log('content: connected to mooltipass '+request.version);
            break;
        case 'disconnected':
            mooltipassConnected = false;
            console.log('content: disconnected from mooltipass ');
            break;

        case 'rescan':
            // Rescan the page for credentials
            console.log('rescanning page for credentials');
            credFields = getCredentials();
            if (!credFields) {
                console.log('no credentials found');
            } else {
                console.log('found credentials, sending to mooltipass');
                sendCredentials(credFields);
            }
            break;
        default:
            break;
    }
});

// say hi to background and app
chrome.runtime.sendMessage({type: 'ping'});
