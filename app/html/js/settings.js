var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.settings = mooltipass.ui.settings || {};



mooltipass.ui.settings.initKeyboardLayout = function() {
    $('#settings-keyboardLayout').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keyboardLayout',
            'value': Number($(this).val()),
            'callbackParameters': null
        });
    });
};

mooltipass.ui.settings.getKeyboardLayout = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'keyboardLayout'
    });
};

mooltipass.ui.settings.callbackGetKeyboardLayout = function(_success, _msg, _value) {
    var $field = $('#settings-keyboardLayout');
    if(_success) {
        $field.val(_value).data('value', _value);
    }
    else {
        mooltipass.ui.status.error($field, _response.msg);
    }
};

mooltipass.ui.settings.callbackSetKeyboardLayout = function(_success, _msg) {
    var $field = $('#settings-keyboardLayout');
    if(_success) {
        $field.data('value', $field.val());
        mooltipass.ui.status.success($field, 'Keyboard Layout successfully changed');
    }
    else {
        $field.val($field.data('value'));
        mooltipass.ui.status.error($field, _msg);
    }
};



mooltipass.ui.settings.getLockTimeoutEnabled = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'lockTimeoutEnabled'
    });
};

mooltipass.ui.settings.initLockTimeoutEnabled = function() {
    $('#settings-lockTimeoutEnabled').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'lockTimeoutEnabled',
            'value': Number($(this).prop('checked')),
        });
    });
};

mooltipass.ui.settings.callbackGetLockTimeoutEnabled = function(_success, _msg, _value) {
    var $field = $('#settings-lockTimeoutEnabled');
    if(_success) {
        _value = Boolean(Number(_value));
        $field.prop('checked', _value).data('value', _value);
    }
    else {
        mooltipass.ui.status.error($field, _response.msg);
    }
};

mooltipass.ui.settings.callbackSetLockTimeoutEnabled = function(_success, _msg) {
    var $field = $('#settings-lockTimeoutEnabled');
    if(_success) {
        $field.data('value', $field.prop('checked'));

        if($field.prop('checked')) {
            mooltipass.ui.status.success($field, 'Lock Inactivity successfully enabled');
        }
        else {
            mooltipass.ui.status.success($field, 'Lock Inactivity successfully disabled');
        }
    }
    else {
        $field.prop('checked', $field.data('value'));
        mooltipass.ui.status.error($field, _msg);
    }
};


mooltipass.ui.settings.getLockTimeout = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'lockTimeout',
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
        });
    });
};

mooltipass.ui.settings.callbackGetLockTimeout = function(_success, _msg, _value) {
    var $field = $('#settings-lockTimeout');
    if(_success) {
        _value = Number(_value) / 60;
        $field.val(_value).data('value', _value);
    }
    else {
        mooltipass.ui.status.error($field, _response.msg);
    }
};

mooltipass.ui.settings.callbackSetLockTimeout = function(_success, _msg) {
    var $field = $('#settings-lockTimeout');
    if(_success) {
        $field.data('value', $field.val());
        mooltipass.ui.status.success($field, 'Inactivity time successfully changed');
    }
    else {
        $field.val($field.data('value'));
        mooltipass.ui.status.error($field, _msg);
    }
};


mooltipass.ui.settings.getScreensaver = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'screensaver',
    });
};

mooltipass.ui.settings.initScreensaver = function() {
    $('#settings-screensaver').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'screensaver',
            'value': Number($(this).prop('checked')),
        });
    });
};

mooltipass.ui.settings.callbackGetScreensaver = function(_success, _msg, _value) {
    var $field = $('#settings-screensaver');
    if(_success) {
        _value = Boolean(Number(_value));
        $field.prop('checked', _value).data('value', _value);
    }
    else {
        mooltipass.ui.status.error($field, _response.msg);
    }
};

mooltipass.ui.settings.callbackSetScreensaver = function(_success, _msg) {
    var $field = $('#settings-screensaver');
    if(_success) {
        $field.data('value', $field.prop('checked'));
        if($field.prop('checked')) {
            mooltipass.ui.status.success($field, 'Screensaver successfully enabled');
        }
        else {
            mooltipass.ui.status.success($field, 'Screensaver successfully disabled');
        }
    }
    else {
        $field.prop('checked', $field.data('value'));
        mooltipass.ui.status.error($field, _msg);
    }
};


mooltipass.ui.settings.getUserInteractionTimeout = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'userInteractionTimeout',
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
        });
    });
};

mooltipass.ui.settings.callbackGetUserInteractionTimeout = function(_success, _msg, _value) {
    var $field = $('#settings-userInteractionTimeout');
    if(_success) {
        $field.val(_value).data('value', _value);
    }
    else {
        mooltipass.ui.status.error($field, _response.msg);
    }
};

mooltipass.ui.settings.callbackSetUserInteractionTimeout = function(_success, _msg) {
    var $field = $('#settings-userInteractionTimeout');
    if(_success) {
        $field.data('value', $field.val());
        mooltipass.ui.status.success($field, 'User Interaction Timeout successfully changed');
    }
    else {
        $field.val($field.data('value'));
        mooltipass.ui.status.error($field, _msg);
    }
};


mooltipass.ui.settings.getFlashScreen = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'flashScreen',
    });
};

mooltipass.ui.settings.initFlashScreen = function() {
    $('#settings-flashScreen').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'flashScreen',
            'value': Number($(this).prop('checked')),
        });
    });
};

mooltipass.ui.settings.callbackGetFlashScreen = function(_success, _msg, _value) {
    var $field = $('#settings-flashScreen');
    if(_success) {
        _value = Boolean(Number(_value));
        $field.prop('checked', _value).data('value', _value);
    }
    else {
        mooltipass.ui.status.error($field, _response.msg);
    }
};

mooltipass.ui.settings.callbackSetFlashScreen = function(_success, _msg) {
    var $field = $('#settings-flashScreen');
    if(_success) {
        $field.data('value', $field.prop('checked'));
        if($field.prop('checked')) {
            mooltipass.ui.status.success($field, 'Flash Screen successfully enabled');
        }
        else {
            mooltipass.ui.status.success($field, 'Flash Screen successfully disabled');
        }
    }
    else {
        $field.prop('checked', $field.data('value'));
        mooltipass.ui.status.error($field, _msg);
    }
};


mooltipass.ui.settings.getOfflineMode = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'offlineMode',
    });
};

mooltipass.ui.settings.initOfflineMode = function() {
    $('#settings-offlineMode').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'offlineMode',
            'value': Number($(this).prop('checked')),
        });
    });
};

mooltipass.ui.settings.callbackGetOfflineMode = function(_success, _msg, _value) {
    var $field = $('#settings-offlineMode');
    if(_success) {
        _value = Boolean(Number(_value));
        $field.prop('checked', _value).data('value', _value);
    }
    else {
        mooltipass.ui.status.error($field, _response.msg);
    }
};

mooltipass.ui.settings.callbackSetOfflineMode = function(_success, _msg) {
    var $field = $('#settings-offlineMode');
    if(_success) {
        $field.data('value', $field.prop('checked'));
        if($field.prop('checked')) {
            mooltipass.ui.status.success($field, 'Offline Mode successfully enabled');
        }
        else {
            mooltipass.ui.status.success($field, 'Offline Mode successfully disabled');
        }
    }
    else {
        $field.prop('checked', $field.data('value'));
        mooltipass.ui.status.error($field, _msg);
    }
};


mooltipass.ui.settings.getTutorialEnabled = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'tutorialEnabled',
    });
};

mooltipass.ui.settings.initTutorialEnabled = function() {
    $('#settings-tutorialEnabled').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'tutorialEnabled',
            'value': Number($(this).prop('checked')),
        });
    });
};

mooltipass.ui.settings.callbackGetTutorialEnabled = function(_success, _msg, _value) {
    var $field = $('#settings-tutorialEnabled');
    if(_success) {
        _value = Boolean(Number(_value));
        $field.prop('checked', _value).data('value', _value);
    }
    else {
        mooltipass.ui.status.error($field, _response.msg);
    }
};

mooltipass.ui.settings.callbackSetTutorialEnabled = function(_success, _msg) {
    var $field = $('#settings-tutorialEnabled');
    if(_success) {
        $field.data('value', $field.prop('checked'));
        if($field.prop('checked')) {
            mooltipass.ui.status.success($field, 'Tutorial successfully enabled');
        }
        else {
            mooltipass.ui.status.success($field, 'Tutorial successfully disabled');
        }
    }
    else {
        $field.prop('checked', $field.data('value'));
        mooltipass.ui.status.error($field, _msg);
    }
};


mooltipass.ui.settings.getSettings = function() {
    mooltipass.ui.settings.getKeyboardLayout();
    mooltipass.ui.settings.getLockTimeoutEnabled();
    mooltipass.ui.settings.getLockTimeout();
    mooltipass.ui.settings.getScreensaver();
    mooltipass.ui.settings.getUserInteractionTimeout();
    mooltipass.ui.settings.getFlashScreen();
    mooltipass.ui.settings.getOfflineMode();
    mooltipass.ui.settings.getTutorialEnabled();
};

mooltipass.ui.settings.getParameter = function(_success, _msg, _parameter, _value) {
    if(_parameter == 'undefined' || typeof(_value) == 'undefined') {
        console.error('Invalid mooltipass parameter or value given | Parameter:', _parameter, '| Value:', _value);
        return;
    }

    mooltipass.ui.settings['callbackGet'+capitalizeFirstLetter(_parameter)](_success, _msg, _value);
};

mooltipass.ui.settings.setParameter = function(_success, _msg, _parameter) {
    if(_parameter == 'undefined') {
        console.error('Invalid mooltipass parameter | Parameter:', _parameter);
        return;
    }

    mooltipass.ui.settings['callbackSet'+capitalizeFirstLetter(_parameter)](_success, _msg);
};


/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */
mooltipass.ui.settings.init = function() {
    mooltipass.ui.settings.initKeyboardLayout();
    mooltipass.ui.settings.initLockTimeoutEnabled();
    mooltipass.ui.settings.initLockTimeout();
    mooltipass.ui.settings.initScreensaver();
    mooltipass.ui.settings.initUserInteractionTimeout();
    mooltipass.ui.settings.initFlashScreen();
    mooltipass.ui.settings.initOfflineMode();
    mooltipass.ui.settings.initTutorialEnabled();

    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'userRequestCancel',
        'callbackFunction': function(_response) {
            if(_response.success) {
                var value = Boolean(Number(_response.value));
                if(value) {
                    mooltipass.device.interface.send({
                        'command': 'setMooltipassParameter',
                        'parameter': 'userRequestCancel',
                        'value': 0,
                        'callbackFunction': function(_response) {
                            if(!_response.success) {
                                console.error('could not disable userRequestCancel:', _response.msg);
                            }
                        }
                    });
                }
            }
        }
    });
};