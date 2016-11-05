var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.developers = mooltipass.ui.developers || {};


/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */

mooltipass.ui.developers.init = function () {
    $('#page-developers #resetCardCheckbox').change(function() {
        $('#page-developers button.resetCard').prop('disabled', !$(this).prop('checked'));
    });
    $('#page-developers button.resetCard').click(function() {
        $(this).prop('disabled', true);
        $('#page-developers #resetCardCheckbox').prop('disabled', true).prop('checked', false).data('active', 1);
        mooltipass.device.interface.send({
            'command': 'resetCard',
            'callbackFunction': function(_status) {
                if(_status.status == 1) {
                    mooltipass.ui.status.success($('#page-developers button.resetCard'), 'Factory reset for card successful');
                }
                else {
                    mooltipass.ui.status.error($('#page-developers button.resetCard'), 'Could not do a factory reset for the inserted card');
                }
                $('#page-developers #resetCardCheckbox').prop('disabled', false).data('active', 0);
            }
        });
    });

    $('#page-developers button.import').click(function() {

        $("#modal-confirm-on-device").show();
		
		// Quick hack to help debugging process for users...
		mooltipass.memmgmt.debugLog = true;

        mooltipass.device.interface.send({
            'command': 'startSingleCommunicationMode',
            'callbackFunction': mooltipass.ui.developers.callbackImportMediaBundle,
            'reason': 'mediabundlerupload',
            'callbackFunctionStart': function() {
                mooltipass.device.singleCommunicationModeEntered = true;

                var password = $('#importMediaBundlePassword').val();
                mooltipass.memmgmt.mediaBundlerUpload(mooltipass.ui.developers.callbackImportMediaBundle, password, mooltipass.ui.developers.progressImportMediaBundle);
            }
        });

    });

    $('#page-developers button.jump').click(function() {
        if($('#jumpToBootloaderPassword').val().length != 124) {
            mooltipass.ui.status.error($('#page-developers button.jump'), 'Wrong password length');
            return;
        }

        $("#modal-confirm-on-device").show();
		
		// Convert the password
		tempPassword = new Uint8Array(62);
		for(var i = 0; i < $('#jumpToBootloaderPassword').val().length; i+= 2)
		{
			tempPassword[i/2] = parseInt($('#jumpToBootloaderPassword').val().substr(i, 2), 16);
		}

        mooltipass.device.interface.send({
            'command': 'jumpToBootloader',
			'payload': tempPassword,
            'callbackFunction': null
        });

        mooltipass.ui.status.success($('#page-developers button.jump'), 'Sent jump packet');

        setTimeout(function() {
            console.warn('reset device connection');
            mooltipass.device.reset();
            mooltipass.device.restartProcessingQueue();
            $("#modal-confirm-on-device").hide();
        }, 2000);

    });
};

mooltipass.ui.developers.progressImportMediaBundle = function(_progress) {
    $("#modal-confirm-on-device").hide();
    $("#modal-import-media-bundle").show();

    if (_progress.progress == 0) {
        $('#modal-import-media-bundle').hide();
        return
    }
    $("#modal-import-media-bundle span.meter").css("width", _progress.progress + "%");
};

mooltipass.ui.developers.callbackImportMediaBundle = function(_statusObject) {
    if(_statusObject.success) {
        mooltipass.ui.status.success($('#page-developers button.import'), _statusObject.msg);
    }
    else {
        mooltipass.ui.status.error($('#page-developers button.import'), _statusObject.msg);
    }

    $('#modal-import-media-bundle').hide();
    $("#modal-confirm-on-device").hide();

    mooltipass.device.endSingleCommunicationMode(_statusObject.skipEndingSingleCommunicationMode);
};

mooltipass.ui.developers.callbackJumpToBootloader = function(_statusObject) {
    if(_statusObject.success) {
        mooltipass.ui.status.success($('#page-developers button.jump'), 'Successfully jumped to bootloader');
    }
    else {
        mooltipass.ui.status.error($('#page-developers button.jump'), _statusObject.msg);
    }

    $("#modal-confirm-on-device").hide();

    mooltipass.device.endSingleCommunicationMode(_statusObject.skipEndingSingleCommunicationMode);
};
