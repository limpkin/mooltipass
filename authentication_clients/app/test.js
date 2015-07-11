$(function() {
    $('#buttonPing').click(function(e) {
        e.preventDefault();
        _mp.sendMsg(_mp.commands.ping);
    });
    $('#buttonStatus').click(function(e) {
        e.preventDefault();
        _mp.sendMsg(_mp.commands.getMooltipassStatus);
    });
    $('#buttonCommand').click(function(e) {
        e.preventDefault();

        var cmd = _mp.commands[$('#textCommand').val()];
        var payload = parseInt($('#textPayload').val());
        if (!cmd) {
            console.log($('#textCommand').val());
            console.log('command not found!');
            return;
        }
        console.log('payload:', payload)
        _mp.sendMsg(cmd, [payload]);
    });
    $('#lastError').click(function(e) {
        e.preventDefault();
        console.log('chrome.runtime.lastError', chrome.runtime.lastError);
    });
});