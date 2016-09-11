var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.status = mooltipass.ui.status || {};

mooltipass.ui.status.success = function(element, message) {
    mooltipass.ui.message.show("success", message);
};

mooltipass.ui.status.error = function(element, message) {
    mooltipass.ui.message.show("error", message);
};