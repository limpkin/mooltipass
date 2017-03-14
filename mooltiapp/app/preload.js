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

// Simulate Chrome and set it global in Electron Environment
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
			// Send Message can come with different arguments. 
			// https://developer.chrome.com/extensions/runtime#method-sendMessage
			// [string extensionId], any message, object options, [function callback]
			// Also, it could be used to send a message from the emulated background script in the APP to the APP content script OR
			// from the APP to the extension. 

			// if ( arguments[1] && arguments[1].command && arguments[1].command != 'getMooltipassStatus') console.log('runtime.sendMessage', arguments.length, arguments[1] );

			// If we have 'source' (added from onMessageExternal) pass it to the APP
			if ( arguments[0] && arguments[0].source) { // comes from external
				this.dispatchOnMessage( arguments[0] );
			} else { // If we don't have a 'source', it means it goes to extension
				//console.log( (new Date()) + ' Connection send.', arguments );
				connections[ arguments[0] ].send( JSON.stringify( arguments[1] ) );
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

chrome.runtime.getPlatformInfo = function (callback)
{
	/* not implemented in nodejs: android / cros */
	running_os = require('os').platform();
	if (running_os == "darwin")
	{
		callback({"os":"mac"});
	}
	else if (running_os == "freebsd")
	{
		callback({"os":"openbsd"});
	}
	else if (running_os == "linux")
	{
		callback({"os":"linux"});
	}
	else if (running_os == "sunos")
	{
		callback({"os":"linux"});
	}
	else if (running_os == "win32")
	{
		callback({"os":"win"});
	}
	else
	{
		callback({"os":"win"});
	}
}

// Emulate the HID interface management of Chrome by wrapping it to NODE
// https://developer.chrome.com/apps/hid
chrome.hid = {
	connection: false,
	devices: [],
	options: false,
	getDevices(options, callback) {
		// Returns an array of matching devices, more info here: https://developer.chrome.com/apps/hid#method-getDevices
		var output = [];
		this.options = options;
		this.devices = HID.devices();
		for ( var I = 0; I < this.devices.length; I++ ) {
			if (options.filters[0].productId === this.devices[I].productId && options.filters[0].vendorId === this.devices[I].vendorId && (mooltipass.app.os == "linux" || options.filters[0].usagePage === this.devices[I].usagePage)) {
				/* see https://github.com/signal11/hidapi/pull/6 for linux */
				//console.log(this.devices[I])
				this.devices[I].deviceId = I;
				output.push(this.devices[I]);
			}
		}
		callback ( output );
	},
	getUserSelectedDevices(options, callback) { // Not implemented as we are not using it
	},
	connect(deviceId, callback) {
		// Open a connection to an HID device for communication: https://developer.chrome.com/apps/hid#method-connect
		//console.log( 'connect', this.devices[deviceId].deviceId );
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
			//if ( response[1] != 185 ) console.log('Received', response );
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

  // Disable moolticute check after load (wait 500ms for security)
  setTimeout( function() {
	global.mooltipass.device.shouldCheckForMoolticute = false;
	global.mooltipass.device.usingMoolticute = false;
	serverStartListening();
	global.mooltipass.ui._.reset();

	// Insert a CSS file for Electron without touching the original CHROME_APP
	var head = document.head;
	var link = document.createElement('link');

	link.type = 'text/css';
	link.rel = 'stylesheet';
	link.href = 'stylesheets/mooltiapp.css';

	head.appendChild(link);
  },500);
  // global.nodeRequire = _require; // in case node binding is disabled
})


// Converts buffer data into a string. 
// Adds a 0 as the first byte (required by USB driver)
function buf2hex(buf) {
  var output = [];
  var uint = new Uint8Array(buf);

  for( var i = 0; i < uint.length; i++) {
	if ( i == 0 ) output.push(0);
	output.push( uint[i] );
  }
  
  return output;
}

// Not being used at the moment, Converts buffer data into a string
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

// Create the listening server
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
  // We're accepting every origin here.
  return true;
}

var connection;
var connections = [];

// Socket server messaging 
wsServer.on('request', function(request) {
	if (!originIsAllowed(request.origin)) {
	  request.reject();
	  console.log( (new Date()) + ' Connection from origin ' + request.origin + ' rejected.');
	  return;
	}

	// Store the connection in an Array so we can reference it later
	connection = request.accept();
	var connectionIndex = connections.length;
	connections[ connectionIndex ] = connection;

	console.log((new Date()) + ' Connection accepted.', connectionIndex);

	connection.on('message', function(message) {
		//console.log('message from extension', message );
		if (message.type === 'utf8') {
			var json = JSON.parse( message.utf8Data );
			json.client_id = connectionIndex;

			// console.log('Message from extension:', json );

			// Keeping a close look at every command here
			// When we receive a message, we need to translate it to a format the APP will understand (as the extension is thinking it is connected with moolticute)
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
			} else if ( json.msg == 'cancel_request') {
				var newMessage = {
					command: 'cancelGetCredentials',
					reqid: json.data.request_id
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

	// Shoot when the connection from the extension to the MooltiApp is closed.
	connection.on('close', function(reasonCode, description) {
		console.log((new Date()) + ' Peer ' + connection.remoteAddress + ' disconnected.');
	});
});

// Send message internally, emulating init from the APP adding the 'source' into the message to easier identification of source
// Basically: we got a message from the EXTENSION and we send it to the APP
// We are emulating the doings of init.js script that's available in a Chrome_APP but not in Electron
chrome.runtime.onMessageExternal.addExternalListener( function(message, sender, callbackFunction) {
	//console.log('chrome.runtime.onMessageExternal (' + sender + '):', message);
	var data = {'id': sender, 'message': message, 'source': 'external'};
	chrome.runtime.sendMessage(data, callbackFunction);
});

// Disconnect from the HID device
window.onbeforeunload = function(event) {
	chrome.hid.disconnect();
}
