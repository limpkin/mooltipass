var device = {connection: null, connecting: false, waitingForMessage: false};
var device_info = { "vendorId": 0x16d0, "productId": 0x09a0 };      // Mooltipass

/**
 * Handler invoked when new USB mooltipass devices are found.
 * @param devices array of device objects
 * @note only the last device is used, assumes that one mooltipass is present.
 * Stale entries appear to be left in chrome if the mooltipass is removed
 * and plugged in again, or the firmware is updated.
 */
onDeviceFound = function (devices)
{
    if (devices.length <= 0)
    {
        return;
    }

    var ind = devices.length - 1;
    var devId = devices[ind].deviceId;

    chrome.hid.connect(devId, function(connectInfo)
    {
        if (!chrome.runtime.lastError)
		{
            device.connection = connectInfo.connectionId;
            deviceSendToElm({setHidConnected:true});
            deviceSendToElm({appendToLog:"device found, connection made"});
        }
        clearTimeout(device.timeoutId);
        device.connecting = false;
    });
}

device.connect = function ()
{
    if (device.connecting)
        return;
    device.connecting = true;
    deviceSendToElm({appendToLog:"> looking for device"});
    device.timeoutId = setTimeout(function () {
        if (device.connecting) {
            deviceSendToElm({appendToLog:"device search timed out"});
            device.connecting = false;
        }
    }, 5000)
    chrome.hid.getDevices(device_info, onDeviceFound);
}


function onDataReceived(reportId, data)
{
    var bytes = new Uint8Array(data);
    var ints = [];
    for (var i = 0, len = bytes.length; i < len; i++)
    {
        ints[i] = bytes[i];
    }
    deviceSendToElm({receiveCommand: ints});
    device.waitingForMessage = false;
}

function sendMsg(msg)
{
    if (device.waitingForMessage)
        return;
    if (msg[1] !== 112)
        console.log("sending: ", msg);
    device.waitingForMessage = true;
    //buffer creation is a bit awkward because windows doesn't like us using
    //the Uint8Array.buffer directly (or maybe it's something to do with the
    //ArrayBuffer size argument?)
    var buffer = new ArrayBuffer(64);
    var array = new Uint8Array(buffer);
    array.set(msg,0);
    chrome.hid.send(device.connection, 0, buffer, function()
    {
        if (!chrome.runtime.lastError)
        {
            chrome.hid.receive(device.connection, onDataReceived);
        }
        else
        {
            console.log("hid error", chrome.runtime.lastError);
            deviceSendToElm({setHidConnected:false});
            device.waitingForMessage = false;
        }
    });
}
