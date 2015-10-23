var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.status = mooltipass.ui.status || {};

mooltipass.ui.status.showSuccess = function(message) {
    console.log('Status Success:', message);
};

mooltipass.ui.status.showError = function(message) {
    console.log('Status Error:', message);
};