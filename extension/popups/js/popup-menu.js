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
		chrome.tabs.sendMessage(global.page.currentTabId, {
			action: "choose_credential_fields"
		});
		close();
	});
}


$(function() {
	initSettings();
	chrome.extension.sendMessage({ action: "update_available_client" }, updateAvailableResponse);
	chrome.extension.sendMessage({ action: "update_available_chromeipass" }, updateAvailableResponse);
});
