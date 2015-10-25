/* global chrome */
/* global chrome.runtime */
var mooltipass = mooltipass || {};
mooltipass.device = mooltipass.device || {};
mooltipass.device.clients = mooltipass.device.clients || {};

// List of known client ids which are connected to the app
mooltipass.device.clients._list = [];


/**
 * Reset clients
 */
mooltipass.device.clients.reset = function() {
    mooltipass.device.clients._list = [];
};


/**
 * Add new client id to list of connected clients
 * @param _id string of the client id (e.g. extension id)
 */
mooltipass.device.clients.add = function(_id) {
    if(!contains(mooltipass.device.clients._list, _id)) {
        mooltipass.device.clients._list.push(_id);
    }
};


/**
 * Remove client id from list of connected clients
 * @param _id string of the client id (e.g. extension id)
 */
mooltipass.device.clients.remove = function(_id) {
    if(contains(mooltipass.device.clients._list, _id)) {
        for(var i = 0; i < mooltipass.device.clients._list.length; i++) {
            if(mooltipass.device.clients._list[i] == _id) {
                mooltipass.device.clients._list.splice(i, 1);
                return;
            }
        }
    }
};


/**
 * Send data to clients
 *   if called with 1 parameter, all available clients are contacted with data from parameter
 *   if called with 2 parameters, the first is the client id, the second is the data
 */
mooltipass.device.clients.send = function() {
    if(arguments.length == 1) {
        // Send to all clients
        var data = arguments[0];

        for(var i = 0; i < mooltipass.device.clients._list.length; i++) {
            mooltipass.device.clients._send(mooltipass.device.clients._list[i], data);
        }
    }
    else if(arguments.length > 1) {
        // First parameter is client id
        var id = arguments[0];
        // Second parameter is data to send
        var data = arguments[1];

        mooltipass.device.clients._send(id, data);
    }
    else {
        console.error('Wrong method call of mooltipass.device.clients.send()');
    }
};


/**
 * Internal send method which uses a callback to remove clients on error
 *   The receiving client id is not transmitted on error --> use locale scope of _id to identify client
 * @param _id string of the client id (e.g. extension id)
 * @param _data any data to transfer to client
 * @private
 */
mooltipass.device.clients._send = function(_id, _data) {
    chrome.runtime.sendMessage(_id, _data, function() {
        if(chrome.runtime.lastError) {
            // Remove client on connection error
            mooltipass.device.clients.remove(_id);
        }
    });
};