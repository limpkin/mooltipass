chrome.app.runtime.onLaunched.addListener(function() {
	chrome.app.window.create('window.html', {
		'bounds': {
			'width': 400,
			'height': 500
		}
	});
});

chrome.runtime.onInstalled.addListener(function() {
	chrome.app.window.create('window.html', {
		'bounds': {
			'width': 400,
			'height': 500
		}
	});
});

chrome.runtime.onStartup.addListener(function() {
	chrome.app.window.create('window.html', {
		'bounds': {
			'width': 400,
			'height': 500
		}
	});
});
