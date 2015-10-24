var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.status = mooltipass.ui.status || {};

mooltipass.ui.status.success = function(element, message) {
    // TODO: element = invoked the status
    console.log('Status Success:', message);
};

mooltipass.ui.status.error = function(element, message) {
    // TODO: element = invoked the status
    console.warn('Status Error:', message);
};