var _initLock = false;
var _inBackground = true;


/* ######################################################################################################### */


function launch(details) {
    // TODO: As of 11/2014 this event is not fired in case of granting new permissions
    //   http://stackoverflow.com/questions/2399389/detect-chrome-extension-first-run-update#comment32831961_14957674
    if(details.reason == "install"){
        //console.log("This is a first install!");
    }else if(details.reason == "update"){
        //var thisVersion = chrome.runtime.getManifest().version;
        //console.log("Updated from " + details.previousVersion + " to " + thisVersion + "!");
    }

    init();
}


function init() {
    if(_initLock) {
        return;
    }

    chrome.runtime.onInstalled.addListener(launch);
    chrome.runtime.onStartup.addListener(launch);
    chrome.app.runtime.onLaunched.addListener(launchWindow);

    // Listen for external messages (e.g. from extension) and send them to the app
    // /vendor/mooltipass/app.js is listening for incoming internal messages
    chrome.runtime.onMessageExternal.addListener(
        function(message, sender, callbackFunction) {
            console.log('chrome.runtime.onMessageExternal(' + sender.id + '):', message);

            // Keep callbackFunction separated to react on chrome.runtime.lastError
            mooltipass.messages.onExternalMessage(sender.id, message, callbackFunction);
        });

    // Listen for internal messages from frontend
    chrome.runtime.onMessage.addListener(
        function(message, sender, callbackFunction) {
            //console.warn('chrome.runtime.onMessage(', data.id, ')');
            mooltipass.messages.onInternalMessage(message, callbackFunction);
        }
    );


    mooltipass.prefstorage.getStoredPreferences(mooltipass.memmgmt.preferencesCallback);
    mooltipass.prefstorage.getStoredPreferences(mooltipass.datamemmgmt.preferencesCallback);
    mooltipass.filehandler.getSyncableFileSystemStatus(mooltipass.memmgmt.syncableFSStateCallback);
    mooltipass.filehandler.setSyncFSStateChangeCallback(mooltipass.memmgmt.syncableFSStateChangeCallback);

    mooltipass.device.init();

    console.log('Listener installed');

    _initLock = true;
}


function launchWindow() {
    // AppWindow is already opened -> do not open another one
    var windows = chrome.app.window.getAll();

    if(windows.length > 0) {
        return;
    }

    chrome.app.window.create('html/index.html', {'bounds': {'width': 800, 'height': 500}, "resizable": false});
}


/* ######################################################################################################### */


init();
launchWindow();