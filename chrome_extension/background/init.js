
// Wait until everything has loaded

startMooltipass = function() {
	if ( !mooltipassEvent || !mooltipassEvent.eventLoaded || !page || !page.pageLoaded ) {
		setTimeout( startMooltipass, 100 );
		return;
	}

	// since version 2.0 the extension is using a keyRing instead of a single key-name-pair
	// keepass.convertKeyToKeyRing();
	// load settings
	page.initSettings();
	// create tab information structure for every opened tab
	page.initOpenedTabs();
	// initial connection with KeePassHttp
	// keepass.getDatabaseHash(null);
	// set initial tab-ID

	if ( !isSafari ) {
		chrome.tabs.query({"active": true, "windowId": chrome.windows.WINDOW_ID_CURRENT}, function(tabs) {
			if (tabs.length === 0)
				return; // For example: only the background devtools or a popup are opened
			page.currentTabId = tabs[0].id;
		});	
	}

	// Milliseconds for intervall (e.g. to update browserAction)
	var _interval = 250;
	// Milliseconds for intervall to check for new input fields
	var _intervalCheckForNewInputs = 500;


	/**
	 * Generate information structure for created tab and invoke all needed
	 * functions if tab is created in foreground
	 * @param {object} tab
	 */

	if ( isSafari ) {
		safari.application.addEventListener( "message", mooltipassEvent.onMessage, false );

		safari.application.addEventListener("open", function(tab) {
			// TODO: Gaston, this can be removed
		}, true);

		safari.application.addEventListener("activate", function( evt ) {
			page.currentTabId = evt.target.activeTab;
		}, true);
	} else {
		chrome.runtime.onMessage.addListener( mooltipassEvent.onMessage );


		chrome.tabs.onCreated.addListener(function(tab) {
			if(tab.id > 0) {
				if(tab.selected) {
					page.currentTabId = tab.id;
					event.invoke(page.switchTab, null, tab.id, []);
				}
			}
		});

		/**
		 * Remove information structure of closed tab for freeing memory
		 * @param {integer} tabId
		 * @param {object} removeInfo
		 */
		chrome.tabs.onRemoved.addListener(function(tabId, removeInfo) {
			delete page.tabs[tabId];
			if(page.currentTabId == tabId) {
				page.currentTabId = -1;
			}
		    mooltipass.device.onTabClosed(tabId, removeInfo);
		});
		
		/**
		 * Set active tab if switched to another window.
		 */
		chrome.windows.onFocusChanged.addListener(function() {
			chrome.tabs.query({active: true, lastFocusedWindow: true}, function(tabs) {
				if (tabs && tabs[0] && tabs[0].id) {
					page.currentTabId = tabs[0].id;
					if(tabs[0].status === "complete") {
						event.invoke(page.switchTab, null, tabs[0].id, []);
					}
				}
			});
		});

		/**
		 * Remove stored credentials on switching tabs.
		 * Invoke functions to retrieve credentials for focused tab
		 * @param {object} activeInfo
		 */
		chrome.tabs.onActivated.addListener(function(activeInfo) {
			// remove possible credentials from old tab information
		    page.clearCredentials(page.currentTabId, true);
			//browserAction.removeRememberPopup(null, {"id": page.currentTabId}, true);

			chrome.tabs.get(activeInfo.tabId, function(info) {
				if(info && info.id) {
					page.currentTabId = info.id;
					if(info.status == "complete") {
						event.invoke(page.switchTab, null, info.id, []);
					}
				}
			});
		});

		/**
		 * Update browserAction on every update of the page
		 * @param {integer} tabId
		 * @param {object} changeInfo
		 */
		chrome.tabs.onUpdated.addListener(function(tabId, changeInfo, tab) {
			if(changeInfo.status == "complete") {
				//event.invoke(browserAction.removeRememberPopup, null, tabId, []);
			}
		    mooltipass.device.onTabUpdated(tabId, changeInfo);
		});

		/**
		 * Detect POST requests and call the content script to check if it is waiting for it
		 */

		// Firefox 50+ allows requestBody in the options (not, because of a bug, once fixed, switch the following IF statements)
		//if ( isFirefox && typeof( Symbol.hasInstance ) == 'undefined' ) var webRequestOptions = ['blocking'];
		if ( isFirefox ) var webRequestOptions = ['blocking'];
		else var webRequestOptions = ['blocking','requestBody'];

		chrome.webRequest.onBeforeRequest.addListener( function (details) {
			if ( details.method == 'POST' && background_debug_msg > 4) mpDebug.log('%c init: onBeforeRequest - Post Interception','background-color: #4CAF50; color: #FFF', details);

			// Test for captcha calls (we don't want to submit if there's a captcha)
			var b = new RegExp('recaptcha');
			if (b.test(details.url)) {
				chrome.tabs.sendMessage( details.tabId, {action: 'captcha_detected', details: details});
			}

			// Intercept posts
			if (details.method == "POST") {
				// Deal both with RAW DATA and FORM DATA
				// If we notice the extension being slow because of it intercepting too much data, uncomment the following line
				//if (details && details.type === "xmlhttprequest") { // Raw data (multipart posts, etc)
					if (details && details.requestBody && details.requestBody.raw && details.requestBody.raw[0]) {
						var buffer = details.requestBody.raw[0].bytes;
						var parsed = arrayBufferToData.toJSON(buffer);
						if ( details.tabId ) chrome.tabs.sendMessage( details.tabId, {action: 'post_detected', post_data: parsed });	
					//} 
				} else { // Standard POST
					chrome.tabs.sendMessage( details.tabId, {action: 'post_detected', details: details});	
				}
			}
		}, { urls: ["<all_urls>"]},webRequestOptions);

		/**
		 * Retrieve Credentials and try auto-login for HTTPAuth requests
		 */
		if ( chrome.webRequest.onAuthRequired ) {
			chrome.webRequest.onAuthRequired.addListener(httpAuth.handleRequest,
				{ urls: ["<all_urls>"] }, [isFirefox ? "blocking" : "asyncBlocking"]
			);	
		}
	}

	if ( !isSafari ) {
		/**
		 * Add context menu entry for filling in username + password
		 */
		chrome.contextMenus.create({
			"title": "Fill &User + Pass",
			"contexts": [ "editable" ],
			"onclick": function(info, tab) {
				chrome.tabs.sendMessage(tab.id, {
					action: "fill_user_pass"
				});
			}
		});

		/**
		 * Add context menu entry for filling in only password which matches for given username
		 */
		chrome.contextMenus.create({
			"title": "Fill &Pass Only",
			"contexts": [ "editable" ],
			"onclick": function(info, tab) {
				chrome.tabs.sendMessage(tab.id, {
					action: "fill_pass_only"
				});
			}
		});

		/**
		 * Add context menu entry for creating icon for generate-password dialog
		 */
		chrome.contextMenus.create({
			"title": "Show Password &Generator Icons",
			"contexts": [ "editable" ],
			"onclick": function(info, tab) {
				chrome.tabs.sendMessage(tab.id, {
					action: "activate_password_generator"
				});
			}
		});

		/**
		 * Add context menu entry for creating icon for generate-password dialog
		 */
		chrome.contextMenus.create({
			"title": "&Save credentials",
			"contexts": [ "editable" ],
			"onclick": function(info, tab) {
				chrome.tabs.sendMessage(tab.id, {
					action: "remember_credentials"
				});
			}
		});
	}


	/**
	 * Interval which updates the browserAction (e.g. blinking icon)
	 */
	window.setInterval(function() {
		browserAction.update(_interval);
	}, _interval);

	window.setInterval(function() {
		if(page.currentTabId && page.currentTabId != -1 && page.allLoaded) {
			messaging( { action: "check_for_new_input_fields" }, page.currentTabId );
		}
	}, _intervalCheckForNewInputs);

	// ArrayBuffer to JSON (by Gaston)
	var arrayBufferToData = {
		toBase64: function (arrayBuffer) {
			var binary = '';
			var bytes = new Uint8Array(arrayBuffer);
			var len = bytes.byteLength;
			for (var i = 0; i < len; i++) {
				binary += String.fromCharCode(bytes[i]);
			}
			return window.btoa(binary);
		},
		toString: function (arrayBuffer) {
			var base64 = this.toBase64(arrayBuffer);
			try {
				var decoded = decodeURIComponent(escape(window.atob(base64)));	
			} catch( e ) {
				if (background_debug_msg != false) mpDebug.error( e, window.atob(base64) );
				var decoded = false;
			}
			return decoded;
		},
		toJSON: function (arrayBuffer) {
			try {
				var string = this.toString(arrayBuffer);
				return JSON.parse(string);
			} catch (e) {
				// Failed to parse as JSON, try a few options:
				var string = this.toString(arrayBuffer);
				if ( string.substr(0,1) == '<' ) { // Posibly XML
					return string;
				} else {
					return JSON.parse('{"' + decodeURI(string).replace(/"/g, '\\"').replace(/&/g, '","').replace(/=/g,'":"') + '"}');	
				}
				
			}
		}
	};
};

startMooltipass();
