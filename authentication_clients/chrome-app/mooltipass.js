var mp_vendor_id = 0x16d0; // 5840
var mp_product_id = 0x09a0; // 2464
var device_info = { "vendorId": mp_vendor_id, "productId": mp_product_id };

// Commands that the MP device can send.
// TODO: Add new commands.
var CMD_DEBUG   = 0x01
var CMD_PING    = 0x02
var CMD_VERSION = 0x03

var mp_device = null;

var in_transfer = {
	direction: 'in',
	endpoint: 1,
	length: 64
};

function message(msg) {
	message_div = document.getElementById("message");
	message_div.innerHTML += msg;
}

function messageln(msg) {
	message(msg + '<br />\n');
}

function warning(msg) {
	message('<font color="#a0a000">Warning:</font> ' + msg);
}

function warningln(msg) {
	warning(msg + '<br />\n');
}

function error(msg) {
	message('<font color="#c00000">Error:</font> ' + msg);
}

function errorln(msg) {
	error(msg + '<br />\n');
}

var onEvent = function(usbEvent) {
	console.log("onEvent");
	if (usbEvent.resultCode) {
		errorln(chrome.runtime.lastError.message + " [onEvent]");
		return;
	}

	var dv = new DataView(usbEvent.data);
	var len = dv.getUint8(0);
	var cmd = dv.getUint8(1);

	switch (cmd) {
		case CMD_DEBUG:
		{
			var msg = "";
			for (var i = 0; i < len; i++) {
				msg += String.fromCharCode(dv.getUint8(i+2));
			}
			messageln("<b>debug:</b> '" + msg + "'");
			break;
		}
		case CMD_PING:
			messageln("<b>ping</b>");
			break;
		case CMD_VERSION:
		{
			var version = "" + dv.getUint8(2) + "." + dv.getUint8(3);
			messageln("<b>command:</b> Version " + version);
			break;
		}
		default:
			errorln("unknown command");
			break;
	}

	chrome.usb.interruptTransfer(mp_device, in_transfer, onEvent);
};

function showMainDiv() {
	var requestDiv = document.getElementById('requestDiv');
	var mainDiv = document.getElementById('mainDiv');
	requestDiv.style.display = 'none';
	mainDiv.style.display = 'block';
}

function showRequestDiv() {
	var requestDiv = document.getElementById('requestDiv');
	var mainDiv = document.getElementById('mainDiv');
	requestDiv.style.display = 'block';
	mainDiv.style.display = 'none';
}

function sendCommand(cmd) {
	var command = [0, cmd];
	var info = {
		direction: 'out',
		endpoint: 2,
		data: new Uint8Array(command).buffer
	};

	chrome.usb.interruptTransfer(mp_device, info, sendCompleted);
}

var setupUSBEventHandlers = function(result) {
	showMainDiv();

	console.log('App was granted the "usbDevices" permission.');
	chrome.usb.findDevices(device_info, function(devices) {
		console.log('Found ' + devices.length + ' devices.');
		if (!devices || !devices.length) {
			errorln('No Mooltipass device was found.');
			return;
		}
		mp_device = devices[0];
		chrome.usb.interruptTransfer(mp_device, in_transfer, onEvent);
		messageln("Mooltipass device found and set up.");
	});
};

window.addEventListener('load', function() {
	var USBPermissions = {
		permissions: [
			{
				'usbDevices': [device_info]
			}
		]
	};
	chrome.permissions.contains(USBPermissions, function(result) {
		if (result) {
			setupUSBEventHandlers();
		} else {
			showRequestDiv();
			var requestButton = document.getElementById('requestPermissionsButton');
			requestButton.addEventListener('click', function(ev) {
				chrome.permissions.request(USBPermissions, function(result) {
					if (result) {
						setupUSBEventHandlers();
					} else {
						showMainDiv();
						errorln('App was not granted the "usbDevices" persionssion');
					}
				});
			});
		}
	});

	// Set up the command button click handlers.
	var pingButton = document.getElementById("ping");
	pingButton.addEventListener("click", function(ev) {
		sendCommand(CMD_PING);
	});

	var versionButton = document.getElementById("version");
	versionButton.addEventListener("click", function(ev) {
		sendCommand(CMD_VERSION);
	});

});

function sendCompleted(usbEvent) {
	if (chrome.runtime.lastError) {
		console.error("sendCompleted Error:", chrome.runtime.lastError.message);
	}

	if (usbEvent) {
		if (usbEvent.data) {
			var buf = new Uint8Array(usbEvent.data);
			console.log("sendCompleted Buffer:", usbEvent.data.byteLength, buf);
		}
		if (usbEvent.resultCode !== 0) {
			errorln("Error writing to device", usbEvent.resultCode);
		}
	}
}
