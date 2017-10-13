
// Detect if we're dealing with Firefox, Safari, or Chrome
var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
var isSafari = typeof(safari) == 'object'?true:false;

var _settings = typeof(localStorage.settings)=='undefined' ? {} : JSON.parse(localStorage.settings);

if ( isSafari ) {
    if ( safari.extension.globalPage ) messaging = safari.extension.globalPage.contentWindow.messaging;
    else messaging = function() {}
} else {
    // Unify messaging method - And eliminate callbacks (a message is replied with another message instead)
    function messaging( message, callback ) {
        chrome.runtime.sendMessage( message, callback );
    };    
}

function initSettings() {
    if ( isSafari ) {
        // Safari new method requires this to open a link
        mpJQ('#btn-link').click(function() {
            safari.application.activeBrowserWindow.openTab().url = "http://themooltipass.com/";
        });
    }

    mpJQ(".pure-menu-list .pure-menu-item").click(function() {
        if (isSafari) {
            safari.self.hide();
        }
    });

    mpJQ("#btn-settings").click(function( e ) {
        if ( isSafari ) {
            e.preventDefault();
            safari.application.activeBrowserWindow.openTab().url = safari.extension.baseURI + "options/options.html";
        } else {
            chrome.tabs.create({
                url: "/options/options.html"
            });
        }
    });

    mpJQ("#btn-open-app").click(function(e) {
        e.preventDefault();
        messaging( { action: "show_app" }, function() {} );
        if ( !isSafari ) close();
    });

    mpJQ("#btn-report-error").click(function(e) {
        if ( isSafari ) {
            e.preventDefault();
            safari.application.activeBrowserWindow.openTab().url = "https://docs.google.com/forms/d/1lFKaTR3LQxySyGsZwtHudVE6aGErGU2DHfn-YpuW8aE/viewform?entry.449375470=" + safari.application.activeBrowserWindow.activeTab.url;
        } else {
            mooltipass.website.reportError( function(target_url){
                chrome.tabs.create({
                    url: target_url
                })
            });
        }
    });

    mpJQ("#btn-select-credential-fields").click(function( e ) {
        e.preventDefault()
        if ($(this).hasClass('disabled')) return
      
        if ( isSafari ) {
            var global = safari.extension.globalPage.contentWindow;
            global.mooltipass.website.chooseCredentialFields();
        } else {
            var global = chrome.extension.getBackgroundPage();
            mooltipass.website.chooseCredentialFields();
            close();
        }
    });

    mpJQ("#btn-add-site-to-blacklist").click(function( e ) {
        if ( isSafari ) {
            e.preventDefault();
            var message = { action: "blacklist_url", args: [safari.application.activeBrowserWindow.activeTab.url] };
            messaging( message, function() {} );
        } else {
            chrome.tabs.query({currentWindow: true, active: true}, function(tabs) {
                chrome.runtime.sendMessage({
                    action: 'blacklist_url',
                    args: [tabs[0].url]
                }, function() {});
                close();
            });
        }
    });

    mpJQ("#btn-remove-site-from-blacklist").click(function(e) {
        if ( isSafari ) {
            e.preventDefault();
            var message = { action: "unblacklist_url", args: [safari.application.activeBrowserWindow.activeTab.url] };
            messaging( message, function() {} );
        } else {
            chrome.tabs.query({currentWindow: true, active: true}, function(tabs) {
                chrome.runtime.sendMessage({
                    action: 'unblacklist_url',
                    args: [tabs[0].url]
                }, function() {});
                close();
            });
        }
    });
}

function getStatusCallback( object ) {
     mpJQ('#status-bar .status > span').hide();
     console.log(object)

     mpJQ('#btn-select-credential-fields').toggleClass('disabled', object.hideCustomCredentials)
     
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
        if ( safari.extension.globalPage && safari.extension.globalPage.contentWindow.mooltipassEvent) {
            safari.extension.globalPage.contentWindow.mooltipassEvent.onGetStatus(getStatusCallback, {
                id: getSafariTabId(safari.application.activeBrowserWindow.activeTab),
                url: safari.application.activeBrowserWindow.activeTab.url
            });
        }
    } else {
        chrome.tabs.query({currentWindow: true, active: true}, function(tabs) {
            messaging( { action: "get_status", overwrite_tab: tabs[0] }, getStatusCallback);
        }); 

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