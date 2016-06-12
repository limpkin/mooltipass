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

mooltipass.ui.settings.getKeypressLoginEnabled = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'keypressLoginEnabled',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-keypressLoginEnabled').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                mooltipass.ui.status.error($('#settings-keypressLoginEnabled'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initKeypressLoginEnabled = function() {
    $('#settings-keypressLoginEnabled').change(function() {
        $(this).data('old-value', $(this).prop('checked'));
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keypressLoginEnabled',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                var $field = $('#settings-keypressLoginEnabled');
                var enabledDisabled = ($field.prop('checked')) ? 'enabled' : 'disabled';
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Login key press ' + enabledDisabled);
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.prop('checked', $field.data('old-value'));
                }
            }
        });
    });
};

mooltipass.ui.settings.getKeypressPasswordEnabled = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'keypressPasswordEnabled',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-keypressPasswordEnabled').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                mooltipass.ui.status.error($('#settings-keypressPasswordEnabled'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initKeypressPasswordEnabled = function() {
    $('#settings-keypressPasswordEnabled').change(function() {
        $(this).data('old-value', $(this).prop('checked'));
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keypressPasswordEnabled',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                var $field = $('#settings-keypressPasswordEnabled');
                var enabledDisabled = ($field.prop('checked')) ? 'enabled' : 'disabled';
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Password key press ' + enabledDisabled);
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.prop('checked', $field.data('old-value'));
                }
            }
        });
    });
};

mooltipass.ui.settings.initKnockEnabled = function() {
    $('#settings-knockEnabled').change(function() {
        $(this).data('old-value', $(this).prop('checked'));
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'knockDetectEnable',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                var $field = $('#settings-knockEnabled');
                var enabledDisabled = ($field.prop('checked')) ? 'enabled' : 'disabled';
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Knock detection feature ' + enabledDisabled);
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.prop('checked', $field.data('old-value'));
                }
            }
        });
    });
};

mooltipass.ui.settings.getKnockEnabled = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'knockDetectEnable',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-knockEnabled').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                mooltipass.ui.status.error($('#settings-knockEnabled'), _response.msg);
            }
        }
    });
};


mooltipass.ui.settings.getKeypressWaitEnabled = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'keybOutputDelayEnabled',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-keypressWaitEnabled').prop('checked', Boolean(Number(_response.value)));
            }
            else {
                mooltipass.ui.status.error($('#settings-keypressWaitEnabled'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initKeypressWaitEnabled = function() {
    $('#settings-keypressWaitEnabled').change(function() {
        $(this).data('old-value', $(this).prop('checked'));
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keybOutputDelayEnabled',
            'value': Number($(this).prop('checked')),
            'callbackFunction': function(_response) {
                var $field = $('#settings-keypressWaitEnabled');
                var enabledDisabled = ($field.prop('checked')) ? 'enabled' : 'disabled';
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Key press wait ' + enabledDisabled);
                }
                else {
                    mooltipass.ui.status.error($field, _response.msg);
                    $field.prop('checked', $field.data('old-value'));
                }
            }
        });
    });
};

mooltipass.ui.settings.getKeypressWait = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'keybOutputDelay',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-keypressWait').val(_response.value);
            }
            else {
                mooltipass.ui.status.error($('#settings-keypressWait'), _response.msg);
            }
        }
    });
};


mooltipass.ui.settings.initKeypressWait = function() {
    $('#settings-keypressWait').change(function() {
        $(this).data('old-value', $(this).val());
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keybOutputDelay',
            'value': parseInt($(this).val()),
            'callbackFunction': function(_response) {
                var $field = $('#settings-keypressWait');
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Key press delay changed');
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

mooltipass.ui.settings.initKnockSensitivity = function() {
    $('#settings-knockSensitivity').change(function() {
        $(this).data('old-value', $(this).val());
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'knockDetectThres',
            'value': parseInt($(this).val()),
            'callbackFunction': function(_response) {
                var $field = $('#settings-knockSensitivity');
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Knock detection sensitivity changed');
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

mooltipass.ui.settings.getKnockSensitivity = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'knockDetectThres',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-knockSensitivity').val(_response.value);
            }
            else {
                mooltipass.ui.status.error($('#settings-knockSensitivity'), _response.msg);
            }
        }
    });
};

mooltipass.ui.settings.initScreenBrightness = function() {
    $('#settings-screenBrightness').change(function() {
        $(this).data('old-value', $(this).val());
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'screenBrightness',
            'value': parseInt($(this).val()),
            'callbackFunction': function(_response) {
                var $field = $('#settings-screenBrightness');
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Screen brightness changed');
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

mooltipass.ui.settings.getScreenBrightness = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'screenBrightness',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-screenBrightness').val(_response.value);
            }
            else {
                mooltipass.ui.status.error($('#settings-screenBrightness'), _response.msg);
            }
        }
    });
};


mooltipass.ui.settings.getKeypressLogin = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'keypressLogin',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-keypressLogin').val(_response.value);
            }
            else {
                mooltipass.ui.status.error($('#settings-keypressLogin'), _response.msg);
            }
        }
    });
};


mooltipass.ui.settings.initKeypressLogin = function() {
    $('#settings-keypressLogin').change(function() {
        $(this).data('old-value', $(this).val());
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keypressLogin',
            'value': parseInt($(this).val()),
            'callbackFunction': function(_response) {
                var $field = $('#settings-keypressLogin');
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Login key press changed');
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


mooltipass.ui.settings.getKeypressPassword = function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'keypressPassword',
        'callbackFunction': function(_response) {
            if(_response.success) {
                $('#settings-keypressPassword').val(_response.value);
            }
            else {
                mooltipass.ui.status.error($('#settings-keypressPassword'), _response.msg);
            }
        }
    });
};


mooltipass.ui.settings.initKeypressPassword = function() {
    $('#settings-keypressPassword').change(function() {
        $(this).data('old-value', $(this).val());
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keypressPassword',
            'value': parseInt($(this).val()),
            'callbackFunction': function(_response) {
                var $field = $('#settings-keypressPassword');
                if(_response.success) {
                    mooltipass.ui.status.success($field, 'Password key press changed');
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
                $field.val(Number(_response.value));
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
            mooltipass.ui.status.error($('#settings-lockTimeout'), 'Please enter a number between 1-120');
            return;
        }

        if(value > 120) {
            mooltipass.ui.status.error($('#settings-lockTimeout'), 'Please enter a number between 1-120');
            return;
        }

        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'lockTimeout',
            'value': value,
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
                
                // Firmware v1.0 suffers a bug which prevents us from using the request cancel functionality
                if (mooltipass.ui._.getDeviceVersion() == "v1.0")
                {
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
                else if (mooltipass.ui._.getDeviceVersion().indexOf("v1.1") >= 0)
                {
                    // Might put a dedicated checkbox for that in the future... but enable it if disabled
                    if (!Boolean(Number(_response.value))) {
                        console.log("V1.1 FW, enabling cancelling feature");
                        mooltipass.device.interface.send({
                            'command': 'setMooltipassParameter',
                            'parameter': 'userRequestCancel',
                            'value': 1,
                            'callbackFunction': function(_response) {
                                if(!_response.success) {
                                    mooltipass.ui.status.error($('#settings-userRequestCancel'), _response.msg);
                                }
                            }
                        });                
                    }
                }
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

        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'userInteractionTimeout',
            'value': parseInt($(this).val()),
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

mooltipass.ui.settings.getFirmwareVersion = function() {
    mooltipass.device.interface.send({
        'command': 'getVersion',
        'payload': [],
        'callbackFunction': function(_response) {
            if(_response.success) {
                var firmware_version = _response.value.trim();
                $('#settings-tab-title').text("Device Settings - Firmware " + firmware_version);
                
                // Delete LUTs present in all non v1 firmware versions
                for(var i = 168; i < 256; i++)
                {
                    $("#settings-keyboardLayout option[value='" + i + "']").remove();                    
                }
                
                // Actions depending on the firmware version
                if (firmware_version.indexOf("v1.1") >= 0)
                {
                    // They're not alphabetically sorted but heh... they're special.
                    $("#settings-keyboardLayout").append(new Option("cz_QW", "179"));
                    $("#settings-keyboardLayout").append(new Option("en_DV", "180"));
                    $("#settings-keyboardLayout").append(new Option("mac_FR", "181"));
                    $("#settings-keyboardLayout").append(new Option("mac_FRCH", "182"));
                    $("#settings-keyboardLayout").append(new Option("mac_DECH", "183"));
                    $("#settings-keyboardLayout").append(new Option("mac_DE", "184"));
                    $("#settings-keyboardLayout").append(new Option("mac_US", "185"));
                    $(".show-if-v1.1-version").show(); 
                }
                else
                {
                    $(".show-if-v1.1-version").hide();  
                }
                
                if (firmware_version.indexOf("mini") >= 0)
                {
                    $(".show-if-mini-version").show();  
                }
            }
            else {
            }
        }
    });
};

mooltipass.ui.settings.getSettings = function() {
    mooltipass.ui.settings.getFirmwareVersion();
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
    mooltipass.ui.settings.getKeypressWaitEnabled();
    mooltipass.ui.settings.getKeypressLoginEnabled();
    mooltipass.ui.settings.getKeypressPasswordEnabled();
    mooltipass.ui.settings.getKeypressLogin();
    mooltipass.ui.settings.getKeypressPassword();
    mooltipass.ui.settings.getKeypressWait();
    mooltipass.ui.settings.getScreenBrightness();
    mooltipass.ui.settings.getKnockEnabled();
    mooltipass.ui.settings.getKnockSensitivity();
}


/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */
mooltipass.ui.settings.init = function() {
    $(".show-if-v1.1-version").hide();  
    $(".show-if-mini-version").hide();  
    mooltipass.ui.settings.initKeyboardLayout();
    mooltipass.ui.settings.initLockTimeoutEnabled();
    mooltipass.ui.settings.initLockTimeout();
    mooltipass.ui.settings.initScreensaver();
    //mooltipass.ui.settings.initUserRequestCancel();
    mooltipass.ui.settings.initUserInteractionTimeout();
    mooltipass.ui.settings.initFlashScreen();
    mooltipass.ui.settings.initOfflineMode();
    mooltipass.ui.settings.initTutorialEnabled();
    mooltipass.ui.settings.initKeypressWaitEnabled();
    mooltipass.ui.settings.initKeypressLoginEnabled();
    mooltipass.ui.settings.initKeypressPasswordEnabled();
    mooltipass.ui.settings.initKeypressPassword();
    mooltipass.ui.settings.initKeypressLogin();
    mooltipass.ui.settings.initKeypressWait();
    mooltipass.ui.settings.initScreenBrightness();
    mooltipass.ui.settings.initKnockEnabled();
    mooltipass.ui.settings.initKnockSensitivity();
};