var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.settings = mooltipass.ui.settings || {};

mooltipass.ui.settings.getKeyboardLayout = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'keyboardLayout',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-keyboardLayout').val(_response.value);
            }
            else {
                mooltipass.ui.status.error($('#settings-keyboardLayout'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initKeyboardLayout = function() {
    $('#settings-keyboardLayout').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keyboardLayout',
            'value': $(this).val(),
            'callbackFunction': function(_response) {
                if(!_response.success) {
                    mooltipass.ui.status.error($('#settings-keyboardLayout'), _response.msg);
                }
            },
            'callbackParameters': null
        });
    });
};

mooltipass.ui.settings.getLockTimeoutEnabled = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'lockTimeoutEnabled',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-lockTimeoutEnabled').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                mooltipass.ui.status.error($('#settings-lockTimeoutEnabled'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initLockTimeoutEnabled = function() {
    $('#settings-lockTimeoutEnabled').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'lockTimeoutEnabled',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                if(!_response.success) {
                    mooltipass.ui.status.error($('#settings-lockTimeoutEnabled'), _response.msg);
                }
            }
        });
    });
};

mooltipass.ui.settings.getLockTimeout = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'lockTimeout',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-lockTimeout').val(Number(_response.value) / 60);
            }
            else {
                mooltipass.ui.status.error($('#settings-lockTimeout'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initLockTimeout = function() {
    $('#settings-lockTimeout').change(function() {
        var value = $(this).val();

        if(isNaN(value)) {
            mooltipass.ui.status.error($('#settings-lockTimeout'), 'Please enter a valid number');
            return;
        }

        // Convert value to float number and bit-wise convert it to integer
        value = Number(value) | 0;

        if(value < 1) {
            mooltipass.ui.status.error($('#settings-lockTimeout'), 'Please enter a number between 1-4');
            return;
        }

        if(value > 4) {
            mooltipass.ui.status.error($('#settings-lockTimeout'), 'Please enter a number between 1-4');
            return;
        }

        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'lockTimeout',
            'value': value * 60,
            'callbackFunction': function(_response) {
                if(!_response.success) {
                    mooltipass.ui.status.error($('#settings-lockTimeout'), _response.msg);
                }
            }
        });
    });
};

mooltipass.ui.settings.getScreensaver = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'screensaver',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-screensaver').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                mooltipass.ui.status.error($('#settings-screensaver'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initScreensaver = function() {
    $('#settings-screensaver').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'screensaver',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                if(!_response.success) {
                    mooltipass.ui.status.error($('#settings-screensaver'), _response.msg);
                }
            }
        });
    });
};

mooltipass.ui.settings.getUserRequestCancel = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'userRequestCancel',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-userRequestCancel').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                mooltipass.ui.status.error($('#settings-userRequestCancel'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initUserRequestCancel = function() {
    $('#settings-userRequestCancel').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'userRequestCancel',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                if(!_response.success) {
                    mooltipass.ui.status.error($('#settings-userRequestCancel'), _response.msg);
                }
            }
        });
    });
};

mooltipass.ui.settings.getUserInteractionTimeout = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'userInteractionTimeout',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-userInteractionTimeout').val(Number(_response.value));
            }
            else {
                mooltipass.ui.status.error($('#settings-userInteractionTimeout'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initUserInteractionTimeout = function() {
    $('#settings-userInteractionTimeout').change(function() {
        var value = $(this).val();

        if(isNaN(value)) {
            mooltipass.ui.status.error($('#settings-userInteractionTimeout'), 'Please enter a valid number');
            return;
        }

        // Convert value to float number and bit-wise convert it to integer
        value = Number(value) | 0;

        if(value < 1) {
            mooltipass.ui.status.error($('#settings-userInteractionTimeout'), 'Please enter a number between 1-200');
            return;
        }

        if(value > 200) {
            // Maximum is 231, otherwise it's blocked to 231
            mooltipass.ui.status.error($('#settings-userInteractionTimeout'), 'Please enter a number between 1-200');
            return;
        }

        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'userInteractionTimeout',
            'value': value,
            'callbackFunction': function(_response) {
                if(!_response.success) {
                    mooltipass.ui.status.error($('#settings-userInteractionTimeout'), _response.msg);
                }
            }
        });
    });
};

mooltipass.ui.settings.getFlashScreen = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'flashScreen',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-flashScreen').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                mooltipass.ui.status.error($('#settings-flashScreen'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initFlashScreen = function() {
    $('#settings-flashScreen').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'flashScreen',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                if(!_response.success) {
                    mooltipass.ui.status.error($('#settings-flashScreen'), _response.msg);
                }
            }
        });
    });
};

mooltipass.ui.settings.getOfflineMode = function() {
    console.log('get offline mode')
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'offlineMode',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-offlineMode').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                mooltipass.ui.status.error($('#settings-offlineMode'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initOfflineMode = function() {
    $('#settings-offlineMode').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'offlineMode',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                console.log('Response SET', _response)
                if(!_response.success) {
                    mooltipass.ui.status.error($('#settings-offlineMode'), _response.msg);
                }
            }
        });
    });
};

mooltipass.ui.settings.getTutorialEnabled = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'tutorialEnabled',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-tutorialEnabled').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                mooltipass.ui.status.error($('#settings-tutorialEnabled'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initTutorialEnabled = function() {
    $('#settings-tutorialEnabled').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'tutorialEnabled',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                if(!_response.success) {
                    mooltipass.ui.status.error($('#settings-tutorialEnabled'), _response.msg);
                }
            }
        });
    });
};

mooltipass.ui.settings.getSettings = function() {
    mooltipass.ui.settings.getKeyboardLayout();
    mooltipass.ui.settings.getLockTimeoutEnabled();
    mooltipass.ui.settings.getLockTimeout();
    mooltipass.ui.settings.getScreensaver();
    mooltipass.ui.settings.getUserRequestCancel();
    mooltipass.ui.settings.getUserInteractionTimeout();
    mooltipass.ui.settings.getFlashScreen();
    mooltipass.ui.settings.getOfflineMode();
    mooltipass.ui.settings.getTutorialEnabled();
}


/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */
mooltipass.ui.settings.init = function() {
    mooltipass.ui.settings.initKeyboardLayout();
    mooltipass.ui.settings.initLockTimeoutEnabled();
    mooltipass.ui.settings.initLockTimeout();
    mooltipass.ui.settings.initScreensaver();
    mooltipass.ui.settings.initUserRequestCancel();
    mooltipass.ui.settings.initUserInteractionTimeout();
    mooltipass.ui.settings.initFlashScreen();
    mooltipass.ui.settings.initOfflineMode();
    mooltipass.ui.settings.initTutorialEnabled();
};