var mp_vendor_id = 0x16d0; // 5840
var mp_product_id = 0x09a0; // 2464
var device_info = { "vendorId": mp_vendor_id, "productId": mp_product_id };

// Commands that the MP device can send.
// TODO: Add new commands.
var CMD_DEBUG   = 0x01;
var CMD_PING    = 0x02;
var CMD_VERSION = 0x03;

// The following error happens when device has been connected but no successful
// communication has been achieved yet.
var USB_ERROR_TRANSFER_FAILED = 1;
var USB_ERROR_DEVICE_DISCONNECTED = 5;

var mp_device = null;
var setupIntervalID = null;
var currentState = null;

var USBPermissions = {
	permissions: [
		{
			'usbDevices': [device_info]
		}
	]
};

var in_transfer = {
	direction: 'in',
	endpoint: 1,
	length: 64
};


var state = {
	// The app does not yet have permission to use USB.
	request_permissions: { value: 0, name: "Request permissions", color: "black" },
	// The MP device is not connected to the computer.
	disconnected:        { value: 1, name: "Disconnected",        color: "#a05050" },
	// The device is connected but we haven't succeeded in communicating with it yet.
	connected:           { value: 2, name: "Connected...",        color: "#a0a050" },
	// The device is connected and ready to service the app.
	ready:               { value: 3, name: "Ready",               color: "#50a050" }
};

window.addEventListener('load', windowLoadListener);

function changeState(newState) {
	var stateSpan = document.getElementById("state");
	stateSpan.innerHTML = newState.name;
	stateSpan.style.color = newState.color;

	switch (newState) {
	case state.request_permissions:
		showRequestDiv();
		if (setupIntervalID) {
			window.clearInterval(setupIntervalID);
			setupIntervalID = null;
		}
		document.getElementById("ping").disabled    = true;
		document.getElementById("version").disabled = true;
		break;
	case state.disconnected:
		showMainDiv();
		if (!setupIntervalID) {
			setupIntervalID = window.setInterval(setupUSBEventHandlers, 1000);
		}
		document.getElementById("ping").disabled    = true;
		document.getElementById("version").disabled = true;
		break;
	case state.connected:
		showMainDiv();
		if (!setupIntervalID) {
			setupIntervalID = window.setInterval(setupUSBEventHandlers, 1000);
		}
		document.getElementById("ping").disabled    = true;
		document.getElementById("version").disabled = true;
		break;
	case state.ready:
		showMainDiv();
		if (setupIntervalID) {
			window.clearInterval(setupIntervalID);
			setupIntervalID = null;
		}
		document.getElementById("ping").disabled    = false;
		document.getElementById("version").disabled = false;
		break;
	default:
		errorln("Unknown state");
		break;
	}
}

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

function windowLoadListener(ev) {
	chrome.permissions.contains(USBPermissions, function(result) {
		if (result) {
			changeState(state.disconnected);
		} else {
			changeState(state.request_permissions);
		}
	});

	var pingButton = document.getElementById("ping");
	pingButton.addEventListener("click", function(ev) {
		sendCommand(CMD_PING);
	});

	var versionButton = document.getElementById("version");
	versionButton.addEventListener("click", function(ev) {
		sendCommand(CMD_VERSION);
	});

}

// Function to show the main div, hiding the USB permissions request div.
function showMainDiv() {
	var requestDiv = document.getElementById('requestDiv');
	var mainDiv = document.getElementById('mainDiv');
	requestDiv.style.display = 'none';
	mainDiv.style.display = 'block';
}

// Function to show the permission request div, hiding the main div. We need
// this because requesting USB permissions can only be done in a method
// initiated by a user gesture, such as the click-handler of a form button.
function showRequestDiv() {
	var requestDiv = document.getElementById('requestDiv');
	var mainDiv = document.getElementById('mainDiv');
	requestDiv.style.display = 'block';
	mainDiv.style.display = 'none';

	var requestButton = document.getElementById('requestPermissionsButton');
	requestButton.addEventListener('click', function(ev) {
		chrome.permissions.request(USBPermissions, function(result) {
			if (result) {
				changeState(state.disconnected);
			} else {
				showMainDiv();
				errorln('App was not granted the "usbDevices" persionssion');
			}
		});
	});
}

// Function to set up the USB transfer event handler.
function setupUSBEventHandlers(result) {
	console.log('App was granted the "usbDevices" permission.');
	chrome.usb.findDevices(device_info, function(devices) {
		console.log('Found ' + devices.length + ' devices.');
		if (!devices || !devices.length) {
			return;
		}
		mp_device = devices[0];
		chrome.usb.interruptTransfer(mp_device, in_transfer, onEvent);

		changeState(state.ready);
	});
}

// Callback to handle USB transfer events.
function onEvent(usbEvent) {
	console.log("onEvent");
	if (usbEvent.resultCode) {
		if (usbEvent.resultCode == USB_ERROR_TRANSFER_FAILED) {
			// Device is connected but we failed to send data.
			changeState(state.connected);
		} else if (usbEvent.resultCode == USB_ERROR_DEVICE_DISCONNECTED) {
			changeState(state.disconnected);
		}
		console.log(chrome.runtime.lastError.message + "(code: " + usbEvent.resultCode + ") [onEvent]");
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
}

// Function to send a command over USB.
// TODO: Extend the function to allow data with commands.
function sendCommand(cmd) {
	var command = [0, cmd];
	var info = {
		direction: 'out',
		endpoint: 2,
		data: new Uint8Array(command).buffer
	};

	chrome.usb.interruptTransfer(mp_device, info, sendCompleted);
}

// Callback to handle the result of sending data over USB.
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
			changeState(state.connected);
			errorln("Error writing to device: " + chrome.runtime.lastError.message);
		}
	}
}
