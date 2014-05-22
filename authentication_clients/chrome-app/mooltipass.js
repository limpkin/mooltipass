var mp_vendor_id = 0x16d0; // 5840
var mp_product_id = 0x09a0; // 2464
var device_info = { "vendorId": mp_vendor_id, "productId": mp_product_id };

// Commands that the MP device can send.
var CMD_DEBUG   = 0x01
var CMD_PING    = 0x02
var CMD_VERSION = 0x03

var mp_device = null;
var message = document.getElementById("message");
var requestButton = document.getElementById("requestPermission");

var in_transfer = {
	direction: 'in',
	endpoint: 1,
	length: 64
};

var onEvent = function(usbEvent) {
	
	console.log("onEvent");
	if (usbEvent.resultCode) {
		console.log("Error: " + usbEvent.error);
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
			message.innerHTML += "debug: '" + msg + "'<br />\n";
			break;
		}
		case CMD_PING:
			message.innerHTML += "command: ping<br />\n";
			break;
		case CMD_VERSION:
		{
			var version = "" + dv.getUint8(2) + "." + dv.getUint8(3);
			message.innerHTML += "command: Version " + version + "<br />\n";
			break;
		}
		default:
			message.innerHTML += "unknown command";
			break;
	}

	chrome.usb.interruptTransfer(mp_device, in_transfer, onEvent);
};

var gotPermission = function(result) {
	requestButton.style.display = 'none';
	console.log('App was granted the "usbDevices" permission.');
	chrome.usb.findDevices(device_info, function(devices) {
		console.log('Found ' + devices.length + ' devices.');
		if (!devices || !devices.length) {
			console.log('device not found');
			return;
		}
		mp_device = devices[0];
		chrome.usb.interruptTransfer(mp_device, in_transfer, onEvent);

		var version_cmd = [0x00, CMD_VERSION];
		var version_info = {
			direction: 'out',
			endpoint: 2,
			data: new Uint8Array(version_cmd).buffer
		};

		chrome.usb.interruptTransfer(mp_device, version_info, sendCompleted);
	});
};

var permissionObj = {
	permissions: [
		{
			'usbDevices': [device_info]
		}
	]
};

requestButton.addEventListener('click', function() {
	console.log('Requesting permission...');
	chrome.permissions.request(permissionObj, function(result) {
		if (result) {
			gotPermission();
		} else {
			console.log('App was not granted the "usbDevices" permission.');
			console.log(chrome.runtime.lastError);
		}
	});
});

function sendCompleted(usbEvent) {
	if (chrome.runtime.lastError) {
		console.error("sendCompleted Error:", chrome.runtime.lastError);
	}

	if (usbEvent) {
		if (usbEvent.data) {
			var buf = new Uint8Array(usbEvent.data);
			console.log("sendCompleted Buffer:", usbEvent.data.byteLength, buf);
		}
		if (usbEvent.resultCode !== 0) {
			console.error("Error writing to device", usbEvent.resultCode);
		}
	}
}
