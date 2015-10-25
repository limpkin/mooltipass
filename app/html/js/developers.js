var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.developers = mooltipass.ui.developers || {};


/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */

function test_getCredentials() {
    mooltipass.device.interface.send({
        'command': 'getCredentials',
        'contexts': Array.prototype.slice.call(arguments),
        'callbackFunction': function(_status) {
            console.log('getCredentials:', _status);
        },
    });
}

function test_addCredentials(context, username, password) {
    mooltipass.device.interface.send({
        'command': 'addCredentials',
        'context': context,
        'username': username,
        'password': password,
        'callbackFunction': function(_status) {
            console.log('updateCredentials:', _status);
        },
    });
}

function test_updateCredentials(context, username, password) {
    mooltipass.device.interface.send({
        'command': 'updateCredentials',
        'context': context,
        'username': username,
        'password': password,
        'callbackFunction': function(_status) {
            console.log('updateCredentials:', _status);
        },
    });
}

mooltipass.ui.developers.init = function () {
    $('#dev-button1').click(function() {
        mooltipass.device.interface.send({
            'command': 'getRandomNumber',
            'callbackFunction': function(_status) {
                console.log('random-string:', _status.value);
            },
        });
    });
};

$(function() {
    mooltipass.ui.developers.init();
})