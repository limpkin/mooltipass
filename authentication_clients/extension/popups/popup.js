function status_response(r) {
    console.log(r);
	$('.status').hide();

    // Connection to app established, device connected and unlocked
    if (r.status.deviceUnlocked && r.status.connectedToDevice && r.status.connectedToApp) {
        $('#unlocked').show();
    }
    // Connection to app established, device connected but locked
    else if (!r.status.deviceUnlocked && r.status.connectedToDevice && r.status.connectedToApp) {
        $('#locked').show();
    }
    // Connection to app established, but no device connected
    else if (!r.status.connectedToDevice && r.status.connectedToApp) {
        $('#no-device').show();
    }
    // No app found
    else if(!r.status.connectedToApp) {
        $('#no-app').show();
    }
    // Unknown error
    else {
        $('#unknown-error').show();
        $('#error-message').text(r.error);
    }
}

$(function() {
	$(".reload-status-button").click(function() {
		chrome.extension.sendMessage({
			action: "get_status"
		}, status_response);
	});

	$(".redetect-fields-button").click(function() {
		chrome.tabs.query({"active": true, "windowId": chrome.windows.WINDOW_ID_CURRENT}, function(tabs) {
			if (tabs.length === 0)
				return; // For example: only the background devtools or a popup are opened
			var tab = tabs[0];

			chrome.tabs.sendMessage(tab.id, {
				action: "redetect_fields"
			});
		});
	});
	
	// Temp patch!!!
    chrome.tabs.query({"active": true, "windowId": chrome.windows.WINDOW_ID_CURRENT}, function(tabs) {
        if (tabs.length === 0)
            return; // For example: only the background devtools or a popup are opened
        var tab = tabs[0];

        chrome.tabs.sendMessage(tab.id, {
            action: "redetect_fields"
        });
    });


    $('.status').hide();
    $('initial-state').show();
	chrome.extension.sendMessage({
		action: "get_status"
	}, status_response);
});
