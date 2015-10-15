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
                $('#settings-lockTimeoutEnabled').prop('checked', Boolean(_response.value));
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
            'value': $(this).prop('checked'),
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
                $('#settings-screensaver').prop('checked', Boolean(_response.value));
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
            'value': $(this).prop('checked'),
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

    

    return;
    // Load settings
    $("#page-settings select, #page-settings input").each(function() {
        var parameter = $(this).attr("name");
        mooltipass.device.interface.send({
            'command': 'getMooltipassParameter',
            'parameter': parameter,
            'callbackFunction': function(_response) {
                console.log(parameter + ':' + _response);
                // TODO #as: show parameter in ui
            },
            'callbackParameters': null
        });
    });

    // Save settings on change
    $("#page-settings select, #page-settings input").on('change keyup', function() {
        key = $(this).attr("name");
        if ($(this).attr('type') == 'checkbox') {
            value = this.checked;
        } else {
            value = $(this).val();
        }

        // TODO #as: send parameters to device
        console.log(key + ": " + value);
    });
});