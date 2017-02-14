
// Detect if we're dealing with Firefox, Safari, or Chrome
var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
var isSafari = typeof(safari) == 'object'?true:false;

var _settings = typeof(localStorage.settings)=='undefined' ? {} : JSON.parse(localStorage.settings);

if ( isSafari ) messaging = safari.extension.globalPage.contentWindow.messaging;
else {
    // Unify messaging method - And eliminate callbacks (a message is replied with another message instead)
    function messaging( message, callback ) {
        chrome.runtime.sendMessage( message, callback );
    };    
}

function initSettings() {
    mpJQ("#btn-settings").click(function() {
        chrome.tabs.create({
            url: "/options/options.html"
        });
    });

    mpJQ("#btn-open-app").click(function(e) {
        e.preventDefault();
        messaging( { action: "show_app" }, function() {} );
        close();
    });

    mpJQ("#btn-report-error").click(function() {
        mooltipass.website.reportError(function(target_url){
            chrome.tabs.create({
                url: target_url
            })
        });        
    });

    mpJQ("#btn-select-credential-fields").click(function() {
        var global = chrome.extension.getBackgroundPage();
        mooltipass.website.chooseCredentialFields();
        close();
    });

    mpJQ("#btn-add-site-to-blacklist").click(function() {
        chrome.tabs.query({currentWindow: true, active: true}, function(tabs) {
            chrome.runtime.sendMessage({
                action: 'blacklist_url',
                args: [tabs[0].url]
            }, function() {});
        });
        close();
    });

    mpJQ("#btn-remove-site-from-blacklist").click(function() {
        chrome.tabs.query({currentWindow: true, active: true}, function(tabs) {
            chrome.runtime.sendMessage({
                action: 'unblacklist_url',
                args: [tabs[0].url]
            }, function() {});
        });
        close();
    });
}

function getStatusCallback( object ) {
     mpJQ('#status-bar .status > span').hide();

    // Connection to app established, device connected and unlocked
    if (object.status.deviceUnlocked && object.status.connectedToDevice && object.status.connectedToApp) {
        mpJQ('#device-unlocked').show();
    }
    // Connection to app established, device connected but locked
    else if (!object.status.deviceUnlocked && object.status.connectedToDevice && object.status.connectedToApp) {
        mpJQ('#device-locked').show();
    }
    // Connection to app established, but no device connected
    else if (!object.status.connectedToDevice && object.status.connectedToApp) {
        mpJQ('#device-disconnected').show();
    }
    // No app found
    else if(!object.status.connectedToApp) {
        mpJQ('#app-missing').show();
    }
    // Unknown error
    else {
        mpJQ('#unknown-error').show();
    }

    if ( object.blacklisted ) {
        mpJQ('#btn-remove-site-from-blacklist').show();
        mpJQ('#btn-add-site-to-blacklist').hide();
    } else {
        mpJQ('#btn-add-site-to-blacklist').show();
        mpJQ('#btn-remove-site-from-blacklist').hide();
    }
}

function updateStatusInfo() {
    if( isSafari ) {
        safari.extension.globalPage.contentWindow.mooltipassEvent.onGetStatus(getStatusCallback, { id: 'safari' });
    } else {
        messaging( { action: "get_status" }, getStatusCallback );    

        if ( typeof chrome.notifications.getPermissionLevel == 'function' ) {
            // Check if notifications are enabled
            chrome.notifications.getPermissionLevel(function(response) {
                if (response == 'denied') {
                    mpJQ("#notifications-disabled").show();
                }
            });        
        }
    }    
}

function _updateStatusInfo() {
    updateStatusInfo();
    setTimeout(_updateStatusInfo, 1000);
}

mpJQ(function() {
    initSettings();
    mpJQ('#status-bar .status > span').hide();
    mpJQ('#initial-state').show();
        
    _updateStatusInfo();
});