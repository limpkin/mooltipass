
window.onbeforeunload = function(event) {
	chrome.hid.disconnect();
}

const {dialog} = require('electron').remote;
const fs = require('fs');
var HID = require('node-hid');

// Extend EVENT object to behave like chrome's
Event.prototype.listeners = [];
Event.prototype.externalListeners = [];

Event.prototype.addListener = function( callback ) {
	var listener = { callback: callback };
	this.listeners.push( listener );
};

Event.prototype.addExternalListener = function( callback ) {
	var listener = { callback: callback };
	this.externalListeners.push( listener );
}

var portSchema = { __proto__: null, name: 'port', $ref: 'runtime.Port' };
var messageSchema = { __proto__: null, name: 'message', type: 'any', optional: true };
var options = { __proto__: null, unmanaged: true };

var chrome = global.chrome = {
	runtime: {
		onMessageExternal: new Event( null, [messageSchema, portSchema], options),
		onMessage: new Event(null, [messageSchema, portSchema], options),
		dispatchOnMessage ( args ) {
			this.onMessage.listeners[0].callback( args );
		},
		dispatchOnExternalMessage () {
			this.onMessageExternal.externalListeners[0].callback( arguments[0], arguments[1], arguments[2] );
		},
		sendMessage () {
			// console.log('runtime.sendMessage', arguments.length, arguments[1] );

			if ( arguments[0] && arguments[0].source) { // comes from external
				this.dispatchOnMessage( arguments[0] );
			} else { // goes to extension
				connection.send( JSON.stringify( arguments[1] ) );
			}
		},
		lastError: false
	},
	storage: {
		sync: {
			get () {
				var callback = arguments[1];
				var output = {};

				var values = [], keys = Object.keys(localStorage), i = keys.length;
				while ( i-- ) {
					output[ keys[i] ] = localStorage.getItem( keys[i] );
					values.push( localStorage.getItem(keys[i]) );
				}
				return output;
			}
		}
	},
	syncFileSystem: {
		getServiceStatus ( callback ) {
			console.log('syncFileSystem.getServiceStatus', arguments);
			$('.exportToCloud').closest('.storage').hide();
			global.mooltipass.device.shouldCheckForMoolticute = false;
			global.mooltipass.device.usingMooltiApp = true;
			callback( 'disabled' );
		},
		onServiceStatusChanged: {
			addListener () {
				console.log('syncFileSystem.onServiceStatusChanged.addListener');
			}
		}
	},
	fileSystem: {
		chooseEntry ( options, callback ) {
			if ( options.type == 'openFile') {
				dialog.showOpenDialog(options, function( filePath ) {
					fs.readFile(filePath[0] , function(err,data) {
						var mimicOutput = { 
							name: filePath[0], 
							data: data, 
							file: function( ownCallback ) {
								// Just leaving this for the first commit
								// var str = buf2str( data );
								// var blob = new Blob( data );
								var txt = String.fromCharCode.apply(null, new Uint8Array(data)) 
								ownCallback( new Blob([txt]) );
							}
						};
						callback( mimicOutput );
					});
				});
			} else {
				dialog.showSaveDialog( { defaultPath: options.suggestedName }, function( fileName ) {
					var writableFileEntry = {
						createWriter ( writer ) {
							var arrayBuffer;
							var fileReader = new FileReader();
							var FileWritencallback = arguments[3];
							fileReader.onload = function() {
    							arrayBuffer = this.result;
    							var buf = new Buffer(arrayBuffer); // decode
    							// fs.writeFile( fileName , String.fromCharCode.apply(null, new Uint8Array(arrayBuffer)) );
    							fs.writeFile( fileName, buf );
    							FileWritencallback(true);
							};

							fileReader.readAsArrayBuffer(arguments[2]);
						}
					};
					callback ( writableFileEntry );
				});
			}
		}
	}
};

chrome.storage.local = chrome.storage.sync;

chrome.hid = { // https://developer.chrome.com/apps/hid
	connection: false,
	devices: [],
	options: false,
	getDevices(options, callback) {
		var output = [];
		this.options = options;
		this.devices = HID.devices();
		for ( var I = 0; I < this.devices.length; I++ ) {
			let populate = false;
			if ( options.productId && options.productId === this.devices[I].productId ) populate = true;
			if ( options.vendorId && options.vendorId === this.devices[I].vendorId ) populate = true;
			if ( populate ) {
				this.devices[I].deviceId = I;
				output.push(this.devices[I]);
			}
		}
		callback ( output );
	},
	getUserSelectedDevices(options, callback) {
	},
	connect(deviceId, callback) {
		console.log( 'connect', this.devices[deviceId].deviceId );
		try {
			this.connection = new HID.HID( this.devices[deviceId].path );
			this.connection.setNonBlocking(0);
			this.connection.readTimeout(500);
			this.connection.on('error', function(error) {
				console.log('got error from device', error );
			} );
		} catch(e) {
			chrome.runtime.lastError = e;
		}
		callback ( this.connection );
	},
	disconnect(connectionId, callback) {
		if (!connectionId) connectionId = this.connection;
		connectionId.close();
		this.connection = false;
		this.devices = [];

		if ( callback ) callback();
	},
	receive(connectionId, callback) {
		if (!connectionId) connectionId = this.connection;
		connectionId.read( function( err, response) {
			if ( response[1] != 185 ) console.log('Received', response );
			if (response.length > 0 ) callback(0, response );
		});
	},
	send(connectionId, reportId, data, callback) {
		if (!connectionId) connectionId = this.connection;
		try {
			connectionId.write( buf2hex( data ) );	
		} catch(e) { // Couldnt write to device
			chrome.runtime.lastError = e;
			console.warn('Could not write to device');
			this.disconnect( connectionId );
		}
		
		callback();
	},
	receiveFeatureReport(connectionId, reportId, callback) {
	},
	sendFeatureReport(connectionId, reportId, data, callback) {
	},
	onDeviceAdded: {
		addListener( callback ) {
		}
	},
	onDeviceRemoved: new Event( null, [messageSchema, portSchema], options)
}

var _remote = require('electron').remote
// var _require = require; // in case node binding is disabled
process.once('loaded', () => {
  global.REMOTE = _remote;

  // Disable moolticute check after load
  setTimeout( function() {
  	global.mooltipass.device.shouldCheckForMoolticute = false;
  	global.mooltipass.device.usingMoolticute = false;
  	serverStartListening();
  	global.mooltipass.ui._.reset();
  },500);
  // global.nodeRequire = _require; // in case node binding is disabled
})


function buf2hex(buf) {
  var output = [];
  var uint = new Uint8Array(buf);

  for( var i = 0; i < uint.length; i++) {
    if ( i == 0 ) output.push(0);
    output.push( uint[i] );
  }
  
  return output;
}

function buf2str(uint8Array) {
    var output = '';
    for (var i=0; i < uint8Array.length; i++) {
        if (uint8Array[i] == 0) {
            return output;
        }
        else {
            output += String.fromCharCode( uint8Array[i] );
        }
    }
    return output;
};

var WebSocketServer = require('websocket').server;
var http = require('http');

var server = http.createServer(function(request, response) {
    console.log((new Date()) + ' Received request for ' + request.url);
    response.writeHead(404);
    response.end();
}).on('error', (err) => {
  // handle errors here
  setTimeout( serverStartListening, 500);
});

serverStartListening = function() {
	server.listen(30035, function() {
		console.warn((new Date()) + ' Server is listening on port 30035');
	});
};

var wsServer = new WebSocketServer({
    httpServer: server,
    autoAcceptConnections: false
});

function originIsAllowed(origin) {
  // put logic here to detect whether the specified origin is allowed.
  return true;
}

var clients = {};
var connection;
wsServer.on('request', function(request) {
    if (!originIsAllowed(request.origin)) {
      request.reject();
      console.log( (new Date()) + ' Connection from origin ' + request.origin + ' rejected.');
      return;
    }

    connection = request.accept();
    console.log((new Date()) + ' Connection accepted.');
    connection.on('message', function(message) {
    	//console.log('message from extension', message );
        if (message.type === 'utf8') {
            var json = JSON.parse( message.utf8Data );
            if ( !clients[ json.client_id ] ) clients[ json.client_id ] = {
            	connection: connection
            };

            console.log('Message from extension:', json );

            // Keeping a close look at every command here
            if ( json.msg == 'ask_password' ) {
            	var newMessage = {
            		command: 'getCredentials',
            		contexts: [ json.data.service, json.data.fallback_service ],
            		reqid: json.data.request_id
            	};
            	chrome.runtime.dispatchOnExternalMessage( newMessage, json.client_id);
            } else if ( json.msg == 'get_random_numbers' ) {
            	var newMessage = {
            		command: 'getRandomNumber'
            	};
            	chrome.runtime.dispatchOnExternalMessage( newMessage, json.client_id);
            } else if ( json.msg == 'set_credential') {
            	var newMessage = {
            		command: 'updateCredentials',
            		context: json.data.service,
            		username: json.data.login,
            		password: json.data.password
            	};
            	chrome.runtime.dispatchOnExternalMessage( newMessage, json.client_id);
            } else {
				chrome.runtime.dispatchOnExternalMessage( json, json.client_id);	
            }
        }
        else if (message.type === 'binary') {
            console.log('Received Binary Message of ' + message.binaryData.length + ' bytes');
            connection.sendBytes(message.binaryData);
        }
    });
    connection.on('close', function(reasonCode, description) {
        console.log((new Date()) + ' Peer ' + connection.remoteAddress + ' disconnected.');
    });
});



chrome.runtime.onMessageExternal.addExternalListener( function(message, sender, callbackFunction) {
	//console.log('chrome.runtime.onMessageExternal (' + sender + '):', message);
	var data = {'id': sender, 'message': message, 'source': 'external'};
	chrome.runtime.sendMessage(data, callbackFunction);
});