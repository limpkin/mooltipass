/*
 * Moolticute support for Mooltipass browser extension
 * (c) Raoul Hecky
 */

var moolticute = moolticute || {};

moolticute.status = {
    connected: false,
    unlocked: false,
    version: {
        flash_size: 0,
        hw_version: '0'
    },
    state: 'UnknownStatus',     //state for mooltipass ext compat
    realState: 'UnknownStatus'
}

moolticute.connectedToDaemon = false;

// Keep pending requests here until they get reponses
moolticute._qCallbacks = {};

//create a unique id to map requests to responses
moolticute._currCallbackId = 0;

/*
    Allows to have a delayed response
    Set to false for normal operation or timeout in millisecs
*/
moolticute.delayResponse = false;

moolticute._getCallbackId = function() {
    moolticute._currCallbackId += 1;
    if (moolticute._currCallbackId > 1000000) {
        moolticute._currCallbackId = 0;
    }
    return moolticute._currCallbackId;
}

/**
 * websocket object for communicating with moolticute daemon
 * TODO: make this configurable in settings page
 */
moolticute._ws = page.settings.useMoolticute? new ReconnectingWebSocket('ws://127.0.0.1:30035'):{};

moolticute._ws.onopen = function() {
    console.log("Moolticute daemon connected");
    moolticute.connectedToDaemon = true;
    moolticute.fireEvent('statusChange');
}

moolticute._ws.onclose = function() {
    console.log("Moolticute daemon disconnected");
    moolticute.connectedToDaemon = false;
    moolticute.fireEvent('statusChange');
}

moolticute._ws.onerror = function() {
    console.log("Moolticute daemon connection error");
    moolticute.connectedToDaemon = false;
}

/**
 * Ask for a password
 */
moolticute.askPassword = function(_ctx, _login, _cb) {
    var id = moolticute._getCallbackId();

    moolticute._qCallbacks[id] = {
        callback: _cb,
        context: _ctx,
        login: _login
    };

    var message = {
        msg: 'ask_password',
        client_id: id,
        data: {
            service: _ctx,
            login: _login,
        }
    };

    console.log('About to send to moolticuted:', message);
    moolticute._ws.send(JSON.stringify(message));
}

/**
 + * Get random numbers
 + */
 moolticute.getRandomNumbers = function(cb) {
    var id = moolticute._getCallbackId();

    moolticute._qCallbacks[id] = {
        callback: cb
    };

    moolticute._ws.send(JSON.stringify({
        msg: 'get_random_numbers',
        client_id: id
    }));
 }

/* Raw send request */
moolticute.sendRequest = function( request ) {
    console.log( 'Sending request -> ', request);
    if ( request.update ) {
        var message = {
            "msg": "set_credential",
            "data": {
                "service": request.update.context,
                "login": request.update.login,
                "password": request.update.password,
                "description": 'Set by Mooltipass Extension/Websocket'
            }
        };
        moolticute._ws.send(JSON.stringify( message ));    
    }
}

/**
 * Process message from moolticute daemon
 */
moolticute._ws.onmessage = function(ev, delayed) {
    // Check if we want a delayed response from moolticute
    // Emulated from here
    if ( moolticute.delayResponse && !delayed) {
        setTimeout( function() {
            moolticute._ws.onmessage(ev, true);
        }, moolticute.delayResponse);
        return;
    }

    var d = ev.data;
    try {
        var recvMsg = JSON.parse(d);
        console.log("Received message:");
        console.log(recvMsg);
    }
    catch (e) {
        console.log("Error in received message: " + e);
        return;
    }

    if (recvMsg.msg == 'mp_connected') {
        moolticute.status.connected = true;
        moolticute.fireEvent('statusChange');
    }
    else if (recvMsg.msg == 'mp_disconnected') {
        moolticute.status.connected = false;
        moolticute.status.state = 'NotConnected';
        moolticute.fireEvent('statusChange');
    }
    else if (recvMsg.msg == 'status_changed') {
        moolticute.status.unlocked = recvMsg.data == 'Unlocked';
        moolticute.status.realState = recvMsg.data;
        moolticute.status.state = recvMsg.data;

        //Keep compatibility with mooltipass chrome App
        if (recvMsg.data == 'NoCardInserted' || recvMsg.data == 'UnkownSmartcad') {
            moolticute.status.state = 'NoCard';
        }
        else if (recvMsg.data == 'LockedScreen') {
            moolticute.status.state = 'Locked';
        }
 
        moolticute.fireEvent('statusChange');
    }
    else if (recvMsg.msg == 'version_changed') {
        moolticute.status.version = recvMsg.data;
        moolticute.fireEvent('statusChange');
    }
    else if (recvMsg.msg == 'memorymgmt_changed') {
        if (recvMsg.data) {
            moolticute.status.state = 'ManageMode';
        }
        else {
             moolticute.status.state = moolticute.status.realState;

             //Keep compatibility with mooltipass chrome App
             if (recvMsg.data == 'NoCardInserted' || recvMsg.data == 'UnkownSmartcad') {
                 moolticute.status.state = 'NoCard';
             }
             else if (recvMsg.data == 'LockedScreen') {
                 moolticute.status.state = 'Locked';
             }
        }
        moolticute.fireEvent('statusChange');
    }
    else if (recvMsg.msg == 'ask_password' || recvMsg.msg == 'get_random_numbers') {
        if (moolticute._qCallbacks.hasOwnProperty(recvMsg.client_id)) {
            moolticute._qCallbacks[recvMsg.client_id].callback(recvMsg.data);
            delete moolticute._qCallbacks[recvMsg.client_id];
        }
    }
}

moolticute.cancelRequest = function( reqid, domain, subdomain ) {
    // TODO: need to clean up the Callbacks otherwise it might get crowded here.
    console.log('Cancel Request:', moolticute._qCallbacks );

    moolticute._ws.send(JSON.stringify({
        'msg': 'cancelGetInputs',
        'reqid': reqid, 
        'domain': domain, 
        'subdomain': subdomain
    }));
}

/* Simple event listener for events sent by moolticute
 + */
moolticute.on = function(type, method, scope) {
     var listeners, handlers;

     if (!(listeners = this.eventListeners)) {
         listeners = this.eventListeners = {};
     }
     if (!(handlers = listeners[type])) {
         handlers = listeners[type] = [];
     }
     scope = (scope? scope:window);
     handlers.push({
         method: method,
         scope: scope
     });
}

moolticute.fireEvent = function(type, data) {
     var listeners, handlers, i, n, handler, scope;
     if (!(listeners = this.eventListeners)) {
         return;
     }
     if (!(handlers = listeners[type])) {
         return;
     }

     for (i = 0;i < handlers.length;i++) {
         handler = handlers[i];
         handler.method.call(handler.scope, type, data);
     }
}