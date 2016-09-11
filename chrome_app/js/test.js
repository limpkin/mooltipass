$(function() {
    $('#buttonPing').click(function(e) {
        e.preventDefault();
        mooltipass.device.interface.send({
            'command': 'ping',
            'callbackFunction': function(_response) {
                console.log('Response:', _response);
            }
        });
    });
    $('#buttonStatus').click(function(e) {
        e.preventDefault();
        mooltipass.device.interface.send({
            'command': 'getMooltipassStatus',
            'callbackFunction': function(_response) {
                console.log('Response:', _response);
            }
        });
    });
    $('#buttonGetParameter').click(function(e) {
        e.preventDefault();
        mooltipass.device.interface.send({
            'command': 'getMooltipassParameter',
            'parameter': $('#textCommand').val(),
            'callbackFunction': function(_response) {
                console.log('Response:', _response);
            }
        })
    });
    $('#buttonSetParameter').click(function(e) {
        e.preventDefault();
        mooltipass.device.interface.send({
            'command': 'setMooltipassParameter',
            'parameter': $('#textCommand').val(),
            'value': $('#textPayload').val(),
            'callbackFunction': function(_response) {
                console.log('Response:', _response);
            }
        })
    });
    $('#clearLog').click(function(e) {
        e.preventDefault();
        $('#log').empty();
    });
    $('#buttonCommand').click(function(e) {
        e.preventDefault();

        var cmd = $('#textCommand').val();
        var payload = [];
        var split_value = $('#textPayload').val().split(',');
        for (var key in split_value) {
            payload.push(parseInt((split_value[key])));
        }

        if (!cmd) {
            console.log($('#textCommand').val());
            console.log('command not found!');
            return;
        }
        console.log('payload:', payload);
        mooltipass.device.addToQueue(cmd, payload);
    });
    $('#lastError').click(function(e) {
        e.preventDefault();
        console.log('chrome.runtime.lastError', chrome.runtime.lastError);
    });
});