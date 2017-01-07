
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
    $("#btn-settings").click(function() {
        close();
        chrome.tabs.create({
            url: "/options/options.html"
        })
    });

    $("#btn-open-app").click(function(e) {
        e.preventDefault();
        close();
        var global = chrome.extension.getBackgroundPage();
        chrome.runtime.sendMessage(global.mooltipass.device._app.id, { 'show': true });
    });

    $("#btn-report-error").click(function() {
        mooltipass.website.reportError(function(target_url){
            chrome.tabs.create({
                url: target_url
            })
        });        
    });

    $("#btn-select-credential-fields").click(function() {
        var global = chrome.extension.getBackgroundPage();
        mooltipass.website.chooseCredentialFields();
        close();
    });

    $("#btn-add-site-to-blacklist").click(function() {
        chrome.tabs.query({currentWindow: true, active: true}, function(tabs) {
            chrome.runtime.sendMessage({
                action: 'blacklist_url',
                args: [tabs[0].url]
            }, function() {});
        });
        close();
    });

    $("#btn-remove-site-from-blacklist").click(function() {
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
    console.log( object );
     $('#status-bar .status > span').hide();

    // Connection to app established, device connected and unlocked
    if (object.status.deviceUnlocked && object.status.connectedToDevice && object.status.connectedToApp) {
        $('#device-unlocked').show();
    }
    // Connection to app established, device connected but locked
    else if (!object.status.deviceUnlocked && object.status.connectedToDevice && object.status.connectedToApp) {
        $('#device-locked').show();
    }
    // Connection to app established, but no device connected
    else if (!object.status.connectedToDevice && object.status.connectedToApp) {
        $('#device-disconnected').show();
    }
    // No app found
    else if(!object.status.connectedToApp) {
        $('#app-missing').show();
    }
    // Unknown error
    else {
        $('#unknown-error').show();
    }

    if ( object.blacklisted ) {
        $('#btn-remove-site-from-blacklist').show();
        $('#btn-add-site-to-blacklist').hide();
    } else {
        $('#btn-add-site-to-blacklist').show();
        $('#btn-remove-site-from-blacklist').hide();
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
                    $("#notifications-disabled").show();
                }
            });        
        }
    }    
}

function _updateStatusInfo() {
    updateStatusInfo();
    setTimeout(_updateStatusInfo, 1000);
}

$(function() {
    initSettings();
    $('#status-bar .status > span').hide();
    $('#initial-state').show();
        
    _updateStatusInfo();
});