var $ = mpJQ.noConflict(true);
var _settings = typeof(localStorage.settings)=='undefined' ? {} : JSON.parse(localStorage.settings);

function updateAvailableResponse(available) {
    if(available) {
        $("#update-available").show();
    }
    else {
        $("#update-available").hide();
    }
}

function initSettings() {
    $("#btn-settings").click(function() {
        close();
        chrome.tabs.create({
            url: "/options/options.html"
        })
    });

    $("#btn-select-credential-fields").click(function() {
        var global = chrome.extension.getBackgroundPage();
        mooltipass.website.chooseCredentialFields();
        close();
    });
}

$(function() {
    initSettings();
    chrome.extension.sendMessage({ action: "update_available_firmware" }, updateAvailableResponse);

    $('#status-bar .status > span').hide();
    $('#initial-state').show();

    chrome.extension.sendMessage({
        action: "get_status"
    }, function(object) {
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
    });

    // Check if notifications are enabled
    chrome.notifications.getPermissionLevel(function(response) {
        if (response == 'denied') {
            $("#notifications-disabled").show();
        }
    });
});