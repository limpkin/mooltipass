var mooltipass = mooltipass || {};
mooltipass.emulator = mooltipass.emulator || {};

/**
 * Boolean, allows to develop without a device
 * Emulate connection with the app/device
*/
mooltipass.emulator.active = false;

mooltipass.emulator._updateCredentials = function(inputObject) {
    console.log('STORING CREDENTIALS *************************************************');
    var context = inputObject.context;
    var username = inputObject.username;
    var password = inputObject.password;

    if($.trim(context) == "") {
        mooltipass.device.interface._returnError(inputObject, 104, 'missing context for add/update credentials');
        return;
    }

    if(password == "") {
        mooltipass.device.interface._returnError(inputObject, 105, 'missing password for add/update credentials');
        return;
    }

    request = {context: context, 'username': username, 'password': password};
    chrome.storage.local.get('mooltipass_emulator', function(result) {
			credentials = result.mooltipass_emulator?JSON.parse(result.mooltipass_emulator):{};
			credentials[context] = request;
			chrome.storage.local.set({'mooltipass_emulator': JSON.stringify(credentials)}, function() {
				chrome.storage.local.get('mooltipass_emulator', function(result) {
					console.log( result )
				});
			});
	});
}

mooltipass.emulator._getCredentials = function(inputObject) {
	console.log('RETRIEVING CREDENTIALS *************************************************');
    var contexts = inputObject.contexts;

    if(contexts.length < 1) {
        mooltipass.device.interface._returnError(inputObject, 103, 'missing context for getting credentials');
        return;
    }
    
    var firstContext = contexts.splice(0, 1);

    console.log('Context:', firstContext[0] );
	if ( 'undefined' !== typeof( firstContext[0] ) ) {
		chrome.storage.local.get('mooltipass_emulator', function(result) {
			console.log('mooltipass_emulator:', result.mooltipass_emulator );
			credentials = result.mooltipass_emulator?JSON.parse(result.mooltipass_emulator):{};
			var creds = {};
			if ( 'undefined' !== typeof credentials[ firstContext[0] ] ) {
				creds = credentials[ firstContext[0] ];
				creds.success = true;
			} else {
				creds.success = false;
			}
			console.log( 'credentials: ', creds );
			inputObject.callbackFunction ( creds );
		})
	} else {
		inputObject.callbackFunction ( inputObject.callbackParameters );
	}
};