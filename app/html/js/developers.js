var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.developers = mooltipass.ui.developers || {};


/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */
mooltipass.ui.developers.init = function () {
    $('#random-string').click(function() {
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