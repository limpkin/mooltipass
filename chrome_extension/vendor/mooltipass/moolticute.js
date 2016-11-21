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

/*
 * Set this to true to see websocked input and output at the console
*/
moolticute._ws.debug = false;

moolticute._ws.onopen = function() {
    if (background_debug_msg > 4) mpDebug.log('%c Moolticute daemon connected', mpDebug.css('FFC6A0'));
    console.log();
    moolticute.connectedToDaemon = true;
    moolticute.fireEvent('statusChange');
}

moolticute._ws.onclose = function() {
    if (background_debug_msg > 4) mpDebug.log('%c Moolticute daemon disconnected', mpDebug.css('FFC6A0'));
    moolticute.connectedToDaemon = false;
    moolticute.fireEvent('statusChange');
}

moolticute._ws.onerror = function() {
    if (background_debug_msg > 4) mpDebug.log('%c Moolticute daemon connection error', mpDebug.css('FFC6A0'));
    moolticute.connectedToDaemon = false;
}

/**
 * Ask for a password
 */
moolticute.askPassword = function( request ) {
    if (background_debug_msg > 4) mpDebug.log('%c Moolticute asking for password', mpDebug.css('FFC6A0'));
    var id = moolticute._getCallbackId();

    var context = '';
    if( request.subdomain && request.domain ) {
        context = request.subdomain + '.' + request.domain;
        subcontext = request.domain;
    } else {
        context = request.domain;
        subcontext = null;
    }

    var message = {
        msg: 'ask_password',
        client_id: id,
        data: {
            service: context,
            fallback_service: subcontext,
            login: '',
            request_id: request.reqid //this id is for a potential cancel of this request
        }
    };

    if (background_debug_msg > 4) mpDebug.log('%c About to send to moolticuted:', mpDebug.css('FFC6A0'), message);
    moolticute._ws.send(JSON.stringify(message));
}

/**
 + * Get random numbers
 + */
 moolticute.getRandomNumbers = function() {
    if (background_debug_msg > 4) mpDebug.log('%c Moolticute getRandomNumbers', mpDebug.css('FFC6A0'));
    var id = moolticute._getCallbackId();

    moolticute._ws.send(JSON.stringify({
        msg: 'get_random_numbers',
        client_id: id
    }));
 }

/* Raw send request */
moolticute.sendRequest = function( request ) {
    if (background_debug_msg > 4) mpDebug.log('%c Moolticute sendRequest', mpDebug.css('FFC6A0'), request);
    if ( request.update ) {
        var message = {
            "msg": "set_credential",
            "data": {
                "service": request.update.context,
                "login": request.update.login,
                "password": request.update.password,
                "description": 'None'
            }
        };

        moolticute._ws.send(JSON.stringify( message ));
    }
}

/**
 * Process message from moolticute daemon
 */
moolticute._ws.onmessage = function(ev, delayed) {
    var d = ev.data;
    try {
        var recvMsg = JSON.parse(d);
        if (background_debug_msg > 4) mpDebug.log('%c Moolticute Received message: ', mpDebug.css('FFC6A0'), recvMsg );
    }
    catch (e) {
        if (background_debug_msg > 4) mpDebug.log('%c Moolticute Error in received message: ', mpDebug.css('FFC6A0'), e, d );
        return;
    }

    if ( moolticute.delayResponse && !delayed && recvMsg.msg == 'ask_password') {
        // Check if we want a delayed response from moolticute
        // Emulated from here
        setTimeout( function() {
            moolticute._ws.onmessage(ev, true);
        }, moolticute.delayResponse);
        return;
    }

    var wrapped = {};

    switch( recvMsg.msg ) {
        case 'mp_connected':
            moolticute.status.connected = true;
            wrapped.deviceStatus = moolticute.status;
            break;
        case 'mp_disconnected':
            moolticute.status.connected = false;
            moolticute.status.state = 'NotConnected';
            wrapped.deviceStatus = moolticute.status;
            break;
        case 'param_changed':
            break;
        case 'status_changed':
            moolticute.status.unlocked = recvMsg.data == 'Unlocked';
            moolticute.status.realState = recvMsg.data;
            moolticute.status.state = recvMsg.data;

            //Keep compatibility with mooltipass chrome App
            if (recvMsg.data == 'NoCardInserted' || recvMsg.data == 'UnkownSmartcad') {
                moolticute.status.state = 'NoCard';
            } else if (recvMsg.data == 'LockedScreen') {
                moolticute.status.state = 'Locked';
            }
            wrapped.deviceStatus = moolticute.status;
            break;
        case 'ask_password':
            if ( recvMsg.data.failed == true ) wrapped.noCredentials = true;
            else {
                wrapped.credentials = {
                    login: recvMsg.data.login,
                    password: recvMsg.data.password,
                };
            }
            break;
        case 'version_changed':
            moolticute.status.version = recvMsg.data;
            wrapped.deviceStatus = moolticute.status;
            break;
        case 'set_credential':
            wrapped.updateComplete = true;
            break;
        case 'get_random_numbers':
            wrapped.random = recvMsg.data;
            break;
        default:
            console.warn('Unknown message: ', recvMsg.msg, recvMsg );
    }
    
    mooltipass.device.messageListener( wrapped );
    return;
}

moolticute.cancelRequest = function( reqid, domain, subdomain ) {
    // TODO: need to clean up the Callbacks otherwise it might get crowded here.
    console.log('Cancel Request');

    moolticute._ws.send(JSON.stringify({
        'msg': 'cancel_request',
        'data': {
            'request_id': reqid //this id is the one that was sent with ask_password
        }
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

moolticute.on('statusChange', function(type, data) {
    if (background_debug_msg > 4) mpDebug.log('%c Moolticute statusChange event received', mpDebug.css('FFC6A0'));
    mooltipass.device._status = {
        'connected': moolticute.status.connected,
        'unlocked': moolticute.status.unlocked,
        'version': moolticute.status.version,
        'state' : moolticute.status.state
    };
    mooltipass.connectedToApp = moolticute.connectedToDaemon;
});