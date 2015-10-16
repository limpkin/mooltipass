/* Contains all methods which are accessed by the html app interface */
var mooltipass = mooltipass || {};
mooltipass.app = mooltipass.app || {};

mooltipass.app.get_password = function(_context, _username, _callback) {
    mooltipass.memmgmt.getPasswordForCredential(_context, _username, function(_status, _password) {
        _callback(_context, _username, _status, _password);
    });
};