/* Initialize mooltipass lib */
if (typeof mooltipass == 'undefined') {
    mooltipass = {};
}
mooltipass.backend = mooltipass.backend || {};

mooltipass.backend.setStatusIcon = function(icon_name) {
    chrome.browserAction.setIcon({
        tabId: page.currentTabId,
        path: "/images/icon_" + icon_name + "_19.png"
    });    
}

mooltipass.backend.updateStatusIcon = function() {
    if (mooltipass.device.getStatus()['deviceUnlocked']) {
        iconName = "normal";
    } else {
        iconName = "cross";
    }
    chrome.notifications.getPermissionLevel(function(response) {
        if (response == 'denied') {
            iconName += "_warning";
        }
        mooltipass.backend.setStatusIcon(iconName);
    });
}
mooltipass.backend._updateStatusIcon = function() {
    mooltipass.backend.updateStatusIcon();
    setTimeout(mooltipass.backend._updateStatusIcon, 500);
}
mooltipass.backend._updateStatusIcon();