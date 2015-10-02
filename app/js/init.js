function launch() {
    // TODO: In case of an update, the app can be launched twice
    chrome.app.window.create('html/index.html', { 'bounds': { 'width': 800, 'height': 600 }, "resizable": false });

    chrome.runtime.onMessage.addListener(function(message, sender, callbackFunction) {
        mooltipass.device.interface._sendFromListener(message, sender, callbackFunction);
    });
}

chrome.runtime.onInstalled.addListener(launch);
chrome.runtime.onStartup.addListener(launch);
