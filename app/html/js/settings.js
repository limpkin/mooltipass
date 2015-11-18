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
        $(this).data('old-value', $(this).val());
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keyboardLayout',
            'value': parseInt($(this).val()),
            'callbackFunction': function(_response) {
                var $field = $('#settings-keyboardLayout');
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Keyboard layout changed');
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.val($field.data('old-value'));
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
        $(this).data('old-value', $(this).prop('checked'));
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'lockTimeoutEnabled',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                var $field = $('#settings-lockTimeoutEnabled');
                var enabledDisabled = ($field.prop('checked')) ? 'enabled' : 'disabled';
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Inactivity timeout ' + enabledDisabled);
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.prop('checked', $field.data('old-value'));
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
            var $field = $('#settings-lockTimeout');
            if(_response.success) {
                $field.val(Number(_response.value) / 60);
            }
            else {
                mooltipass.ui.status.error($field, _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initLockTimeout = function() {
    $('#settings-lockTimeout').change(function() {
        $(this).data('old-value', $(this).val());
        var value = $(this).val();

        if(isNaN(value)) {
            mooltipass.ui.status.error($('#settings-lockTimeout'), 'Please enter a valid number');
            return;
        }

        // Convert value to float number and bit-wise convert it to integer
        value = Number(value) | 0;
        $(this).val(value);
        $(this).data('old-value', $(this).val());

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
                var $field = $('#settings-lockTimeout');
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Inactivity timeout changed');
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.val($field.data('old-value'));
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
        $(this).data('old-value', $(this).prop('checked'));
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'screensaver',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                var $field = $('#settings-screensaver');
                var enabledDisabled = ($field.prop('checked')) ? 'enabled' : 'disabled';
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Screensaver ' + enabledDisabled);
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.prop('checked', $field.data('old-value'));
                }
            }
        });
    });
};

// Reset Parameter 15 to value 0
mooltipass.ui.settings.getUserRequestCancel = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'userRequestCancel',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-userRequestCancel').prop('checked', Boolean(Number(_response.value)));

                // Start quickfix: Set it to 0 if enabled
                if (Boolean(Number(_response.value))) {
                    mooltipass.device.interface.send({
                        'command': 'setMooltipassParameter',
                        'parameter': 'userRequestCancel',
                        'value': 0,
                        'callbackFunction': function(_response) {
                            if(!_response.success) {
                                mooltipass.ui.status.error($('#settings-userRequestCancel'), _response.msg);
                            }
                        }
                    });                
                }
                // End quickfix
            }
            else {
                mooltipass.ui.status.error($('#settings-userRequestCancel'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initUserRequestCancel = function() {
    $('#settings-userRequestCancel').change(function() {
        $(this).data('old-value', $(this).prop('checked'));
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'userRequestCancel',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                var $field = $('#settings-userRequestCancel');
                var enabledDisabled = ($field.prop('checked')) ? 'enabled' : 'disabled';
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'User interaction timeout ' + enabledDisabled);
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.prop('checked', $field.data('old-value'));
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
        $(this).data('old-value', $(this).val());
        var value = $(this).val();

        if(isNaN(value)) {
            mooltipass.ui.status.error($('#settings-userInteractionTimeout'), 'Please enter a valid number');
            return;
        }

        // Convert value to float number and bit-wise convert it to integer
        value = Number(value) | 0;
        $(this).val(value);
        $(this).data('old-value', $(this).val());

        if(value < 5) {
            mooltipass.ui.status.error($('#settings-userInteractionTimeout'), 'Please enter a number between 5-200');
            return;
        }

        if(value > 200) {
            // Maximum is 231, otherwise it's blocked to 231
            mooltipass.ui.status.error($('#settings-userInteractionTimeout'), 'Please enter a number between 5-200');
            return;
        }

        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'userInteractionTimeout',
            'value': value,
            'callbackFunction': function(_response) {
                var $field = $('#settings-userInteractionTimeout');
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Credentials request timeout changed');
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.val($field.data('old-value'));
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
        $(this).data('old-value', $(this).prop('checked'));
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'flashScreen',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                var $field = $('#settings-flashScreen');
                var enabledDisabled = ($field.prop('checked')) ? 'enabled' : 'disabled';
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Flash screen ' + enabledDisabled);
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.prop('checked', $field.data('old-value'));
                }
            }
        });
    });
};

mooltipass.ui.settings.getOfflineMode = function() {
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
        $(this).data('old-value', $(this).prop('checked'));
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'offlineMode',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                var $field = $('#settings-offlineMode');
                var enabledDisabled = ($field.prop('checked')) ? 'enabled' : 'disabled';
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Offline mode ' + enabledDisabled);
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.prop('checked', $field.data('old-value'));
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
        $(this).data('old-value', $(this).prop('checked'));
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'tutorialEnabled',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                var $field = $('#settings-tutorialEnabled');
                var enabledDisabled = ($field.prop('checked')) ? 'enabled' : 'disabled';
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Tutorial ' + enabledDisabled);
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.prop('checked', $field.data('old-value'));
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
    // Keep the next line for disabling parameter 15!
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
    //mooltipass.ui.settings.initUserRequestCancel();
    mooltipass.ui.settings.initUserInteractionTimeout();
    mooltipass.ui.settings.initFlashScreen();
    mooltipass.ui.settings.initOfflineMode();
    mooltipass.ui.settings.initTutorialEnabled();
};