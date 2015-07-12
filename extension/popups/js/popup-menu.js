var $ = cIPJQ.noConflict(true);
var _settings = typeof(localStorage.settings)=='undefined' ? {} : JSON.parse(localStorage.settings);
//var global = chrome.extension.getBackgroundPage();

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
	chrome.extension.sendMessage({ action: "update_available_client" }, updateAvailableResponse);
	chrome.extension.sendMessage({ action: "update_available_chromeipass" }, updateAvailableResponse);

	mooltipass.device.isConnected(function(isConnected) {
		$("#initial-state").hide();
		if (isConnected) {
			$("#device-connected").show();
		} else {
			$("#device-disconnected").show();		
		}
	});

	mooltipass.website.hasCredentialFields(function(hasCredentialFields) {

	});
});