function launch() {
    chrome.app.window.create('html/index.html', { 'bounds': { 'width': 800, 'height': 600 }, "resizable": false });

    chrome.runtime.onMessage.addListener(mooltipass.device.interface._sendFromListener);
}

chrome.runtime.onInstalled.addListener(launch);
chrome.runtime.onStartup.addListener(launch);
