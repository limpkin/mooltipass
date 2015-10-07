var _s = {};

_s.getKeyboardLayout = function() {

}

$(function() {
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': 'keyboardLayout',
        'callbackFunction': function(_response) {
            $('#settings-keyboardLayout').val(_response.value);
        },
        'callbackParameters': null
    });

    $('#settings-keyboardLayout').change(function() {
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': 'keyboardLayout',
            'value': $(this).val(),
            'callbackFunction': function(_response) {
                console.log('KeyboardLayout SET:', _response);
            },
            'callbackParameters': null
        });
    });

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