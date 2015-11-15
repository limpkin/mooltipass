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
            },
        });
    });
};
