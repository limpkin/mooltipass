function launch() {
    chrome.app.window.create('mooltipass.html', { 'bounds': { 'width': 400, 'height': 500 } });
    chrome.app.window.create('html/index.html', { 'bounds': { 'width': 800, 'height': 600 }, "resizable": false });

    chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) 
    {
        console.log(sender.tab ?  'from a content script:' + sender.tab.url : 'from the extension');
        if (request.type == 'inputs') {
            console.log('inputs:');
            for (var i=0; i<request.inputs.length; i++) {
                console.log('    "'+request.inputs[i].id);
            }
            sendResponse({type: 'response', msg: 'Hi From Client'});
        }
    });
}

chrome.runtime.onInstalled.addListener(launch);
chrome.runtime.onStartup.addListener(launch);
