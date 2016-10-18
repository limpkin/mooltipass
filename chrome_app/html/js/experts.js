var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.experts = mooltipass.ui.experts || {};


/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */
mooltipass.ui.experts.init = function () {
    $("#experts-request-uid-password").click(function(e) {
        e.preventDefault();
        window.open("mailto:support@themooltipass.com?subject=UID Request Code&body=My serial number is: XXX");
    });

    $('#experts-uid-password').keypress(function(e) {
        if(e.keyCode == 13) {
            e.preventDefault();
            $("#experts-uid-submit").click();
        }
    })

    $("#experts-uid-submit").click(mooltipass.ui.experts.onClickUIDSubmit);
    $("#experts-import-csv-file").click(mooltipass.ui.experts.onClickImportCSVFile);
};

mooltipass.ui.experts.onClickUIDSubmit = function (e) {
    e.preventDefault();

    var $input = $('#experts-uid-password');
    var value = $input.val().trim();
    var $response = $('#experts-uid-response');

    $response.removeClass('success').removeClass('alert');

    if(value.length != 32) {
        $response.addClass('alert');
        $response.text('Invalid password');
        $input.focus();
        return;
    }

    $("#experts-uid-submit").addClass("disabled").prop('disabled', true);
    $input.addClass("disabled").prop('disabled', true);
    $response.text('Waiting for response');

    mooltipass.device.interface.send({
        'command': 'getMooltipassUID',
        'callbackFunction': mooltipass.ui.sync.callbackImportMediaBundle,
        'password': value,
        'callbackFunction': function(_response) {
            $("#experts-uid-submit").removeClass("disabled").prop('disabled', false);
            $input.removeClass("disabled").prop('disabled', false);
            if(_response.success) {
                $response.addClass('success').html('Your device UID is:&nbsp;&nbsp;&nbsp;' + _response.value.toUpperCase());
            }
            else {
                $response.addClass('alert').text('Could not fetch the UID from device. Maybe the password is wrong?');
            }
        }
    });
}

mooltipass.ui.experts.onClickImportCSVFile = function(e) {
    e.preventDefault();

    if(!mooltipass.ui._.isDeviceUnlocked()) {
        mooltipass.ui.status.error($(this), 'The device has to be unlocked.');
        return;
    }

    if(mooltipass.ui._.isDeviceInMMM()) {
        mooltipass.ui.status.error($(this), 'The device is not allowed to be in MemoryManagementMode for this function.');
        return;
    }

    setTimeout(function(){
        $("#modal-confirm-on-device").show();
    }, 200);

    mooltipass.device.interface.send({
        'command': 'startSingleCommunicationMode',
        'callbackFunction': mooltipass.ui.experts.callbackImportFromCSVFile,
        'reason': 'synchronisationmode',
        'callbackFunctionStart': function() {
            mooltipass.device.singleCommunicationModeEntered = true;
            mooltipass.memmgmt.mergeCsvCredentialFileToMooltipassStart(mooltipass.ui.experts.callbackImportFromCSVFile, mooltipass.ui.experts.callbackImportProgress);
        }
    });
};

mooltipass.ui.experts.callbackImportProgress = function(_progress) {
    $("#modal-confirm-on-device").hide();
    $("#modal-import").show();

    if (_progress.progress == 0) {
        $("#modal-import").hide();
        return;
    }
    $("#modal-import span.meter").css("width", _progress.progress + "%");
};

mooltipass.ui.experts.callbackImportFromCSVFile = function(_status) {
    var $button = $("#experts-import-csv-file");

    console.log(_status);

    if(_status.success) {
        mooltipass.ui.status.success($button, _status.msg);
    }
    else {
        mooltipass.ui.status.error($button, _status.msg);
    }

    $("#modal-confirm-on-device").hide();
    $("#modal-import").hide();

    mooltipass.device.endSingleCommunicationMode(_status.skipEndingSingleCommunicationMode);
};