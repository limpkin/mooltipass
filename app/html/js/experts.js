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

    $("#experts-uid-submit").click(function (e) {
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
                console.log(_response);
                $("#experts-uid-submit").removeClass("disabled").prop('disabled', false);
                $input.removeClass("disabled").prop('disabled', false);
                if(_response.success) {
                    $response.addClass('success').text(_response.value);
                }
                else {
                    $response.addClass('alert').text('Could not fetch the UID from device. Maybe the password is wrong?');
                }
            }
        });
    });
};