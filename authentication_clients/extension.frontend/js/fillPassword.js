function fillPassword(url, username, password) {
	// Check if url is still the same
	chrome.tabs.getSelected(null, function(tab){
		console.log(tab);
	});
}


mooltipass.device.onConfirmCredentials.addListener(fillPassword);