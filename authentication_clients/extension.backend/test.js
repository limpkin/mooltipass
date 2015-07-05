$(function() {
    $('#button').click(function(e) {
       e.preventDefault();
       mp.checkConnection();
    });
    $('#lastError').click(function(e) {
        e.preventDefault();
        console.log('chrome.runtime.lastError', chrome.runtime.lastError);
    })
});