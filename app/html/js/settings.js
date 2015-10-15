var _s = {};

_s.getKeyboardLayout = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'keyboardLayout',
        'callbackFunction': function(_response) {
            if(_response.status == 'success') {
                $('#settings-keyboardLayout').val(_response.value);
            }
            else {
                // TODO: Show alert
            }
        }
    });
};

_s.initKeyboardLayout = function() {
    $('#settings-keyboardLayout').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keyboardLayout',
            'value': $(this).val(),
            'callbackFunction': function(_response) {
                if(_response.status != 'success') {
                    // TODO: Show alert
                }
            },
            'callbackParameters': null
        });
    });
};

_s.getLockTimeoutEnabled = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'lockTimeoutEnabled',
        'callbackFunction': function(_response) {
            if(_response.status == 'success') {
                $('#settings-lockTimeoutEnabled').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                // TODO: Show alert
            }
        }
    });
};

_s.initLockTimeoutEnabled = function() {
    $('#settings-lockTimeoutEnabled').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'lockTimeoutEnabled',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                if(_response.status != 'success') {
                    // TODO: Show alert
                }
            }
        });
    });
};

_s.getLockTimeout = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'lockTimeout',
        'callbackFunction': function(_response) {
            if(_response.status == 'success') {
                $('#settings-lockTimeout').val(Number(_response.value) / 60);
            }
            else {
                // TODO: Show alert
            }
        }
    });
};

_s.initLockTimeout = function() {
    $('#settings-lockTimeout').change(function() {
        var value = $(this).val();

        if(isNaN(value)) {
            // TODO: Not a number entered
            return;
        }

        // Convert value to float number and bit-wise convert it to integer
        value = Number(value) | 0;

        if(value < 1) {
            // TODO: Invalid range for
            return;
        }

        if(value > 4) {
            // TODO: Maximum is 4, otherwise an overflow occurs
            return;
        }

        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'lockTimeout',
            'value': value * 60,
            'callbackFunction': function(_response) {
                if(_response.status != 'success') {
                    // TODO: Show alert
                }
            }
        });
    });
};

_s.getScreensaver = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'screensaver',
        'callbackFunction': function(_response) {
            if(_response.status == 'success') {
                $('#settings-screensaver').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                // TODO: Show alert
            }
        }
    });
};

_s.initScreensaver = function() {
    $('#settings-screensaver').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'screensaver',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                if(_response.status != 'success') {
                    // TODO: Show alert
                }
            }
        });
    });
};

_s.getUserRequestCancel = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'userRequestCancel',
        'callbackFunction': function(_response) {
            if(_response.status == 'success') {
                $('#settings-userRequestCancel').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                // TODO: Show alert
            }
        }
    });
};

_s.initUserRequestCancel = function() {
    $('#settings-userRequestCancel').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'userRequestCancel',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                if(_response.status != 'success') {
                    // TODO: Show alert
                }
            }
        });
    });
};

_s.getUserInteractionTimeout = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'userInteractionTimeout',
        'callbackFunction': function(_response) {
            if(_response.status == 'success') {
                $('#settings-userInteractionTimeout').val(Number(_response.value));
            }
            else {
                // TODO: Show alert
            }
        }
    });
};

_s.initUserInteractionTimeout = function() {
    $('#settings-userInteractionTimeout').change(function() {
        var value = $(this).val();

        if(isNaN(value)) {
            // TODO: Not a number entered
            return;
        }

        // Convert value to float number and bit-wise convert it to integer
        value = Number(value) | 0;

        if(value < 1) {
            // TODO: Invalid range for
            return;
        }

        if(value > 200) {
            // TODO: Maximum is 231, otherwise it's blocked to 231
            return;
        }

        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'userInteractionTimeout',
            'value': value,
            'callbackFunction': function(_response) {
                if(_response.status != 'success') {
                    // TODO: Show alert
                }
            }
        });
    });
};

_s.getFlashScreen = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'flashScreen',
        'callbackFunction': function(_response) {
            if(_response.status == 'success') {
                $('#settings-flashScreen').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                // TODO: Show alert
            }
        }
    });
};

_s.initFlashScreen = function() {
    $('#settings-flashScreen').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'flashScreen',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                if(_response.status != 'success') {
                    // TODO: Show alert
                }
            }
        });
    });
};

_s.getOfflineMode = function() {
    console.log('get offline mode')
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'offlineMode',
        'callbackFunction': function(_response) {
            console.log('Response', _response);
            if(_response.status == 'success') {
                $('#settings-offlineMode').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                // TODO: Show alert
            }
        }
    });
};

_s.initOfflineMode = function() {
    $('#settings-offlineMode').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'offlineMode',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                console.log('Response SET', _response)
                if(_response.status != 'success') {
                    // TODO: Show alert
                }
            }
        });
    });
};

_s.getTutorialEnabled = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'tutorialEnabled',
        'callbackFunction': function(_response) {
            if(_response.status == 'success') {
                $('#settings-tutorialEnabled').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                // TODO: Show alert
            }
        }
    });
};

_s.initTutorialEnabled = function() {
    $('#settings-tutorialEnabled').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'tutorialEnabled',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                if(_response.status != 'success') {
                    // TODO: Show alert
                }
            }
        });
    });
};



$(function() {
    _s.initKeyboardLayout();
    _s.getKeyboardLayout();

    _s.initLockTimeoutEnabled();
    _s.getLockTimeoutEnabled();
    _s.initLockTimeout();
    _s.getLockTimeout();

    _s.initScreensaver();
    _s.getScreensaver();

    _s.initUserRequestCancel();
    _s.getUserRequestCancel();
    _s.initUserInteractionTimeout();
    _s.getUserInteractionTimeout();

    _s.initFlashScreen();
    _s.getFlashScreen();

    _s.initOfflineMode();
    _s.getOfflineMode();

    _s.initTutorialEnabled();
    _s.getTutorialEnabled();
});