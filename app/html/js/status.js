var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.status = mooltipass.ui.status || {};

mooltipass.ui.status.success = function(message) {
    console.log('Status Success:', message);
};

mooltipass.ui.status.error = function(message) {
    console.log('Status Error:', message);
};