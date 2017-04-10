
// Unify messaging method - And eliminate callbacks (a message is replied with another message instead)
function messaging( message, tab ) {
	if (background_debug_msg > 5) mpDebug.log('%c Sending message to content:','background-color: #0000FF; color: #FFF; padding: 3px; ', message);
	else if (background_debug_msg > 4 && message.action != 'check_for_new_input_fields') mpDebug.log('%c Sending message to content:','background-color: #0000FF; color: #FFF; padding: 3px; ', message);

	if ( isSafari ) tab.page.dispatchMessage("messageFromBackground", message);
	else chrome.tabs.sendMessage( typeof(tab) == 'number'?tab:tab.id, message, function(response) {});
};

function cross_notification( notificationId, options ) {
	if ( isSafari ) {
		options.tag = notificationId;
		options.body = options.message;
		var n = new Notification( options.title, options );
		n.onclose = mooltipassEvent.onNotifyClosed;
	} else {
		chrome.notifications.create( notificationId, options );
	}
}

// Masquerade event var into a different variable name ( while event is not reserved, many websites use it and creates problems )
var mooltipassEvent = {
	eventLoaded: true
};

// Keep event for backwards compatibility
var event = mooltipassEvent;

/**
 * Message listener - Handles the messages from the content script
 * @request {object}  The request received from content. Varies if it comes from Safari or Chrome/FF
 * @sender {object} Tab object sending the message
**/
mooltipassEvent.onMessage = function( request, sender, callback ) {
	if ( isSafari ) { // Safari sends an EVENT
		sender = request.target;
		request = request.message;
		tab = sender;
	} else { // Chrome and FF sends Request and Sender separately
		tab = sender.tab;
	}

	if (background_debug_msg > 4) mpDebug.log('%c mooltipassEvent: onMessage ' + request.action, mpDebug.css('e2eef9'), tab, arguments);

	if (request.action in mooltipassEvent.messageHandlers) {
		if ( tab ) {
			var callback = function( data, tab ) {
				messaging( { 'action': 'response-' + request.action, 'data': data }, tab );
			};	
		}

		mooltipassEvent.invoke(mooltipassEvent.messageHandlers[request.action], callback, tab, request.args);
	}

	return true;
}

/**
 * Get interesting information about the given tab.
 * Function adapted from AdBlock-Plus.
 *
 * @param {function} handler to call after invoke
 * @param {function} callback to call after handler or null
 * @param {integer} senderTabId
 * @param {array} args
 * @param {bool} secondTime
 * @returns null (asynchronous)
 */
mooltipassEvent.invoke = function(handler, callback, senderTab, args, secondTime) {
	if (background_debug_msg > 4) mpDebug.log('%c mooltipassEvent: invoke ', mpDebug.css('e2eef9'), arguments);
	if ( typeof(senderTab) == 'number' ) senderTab = { id: senderTab };

	if ( senderTab && senderTab.id && !page.tabs[senderTab.id]) {
		page.createTabEntry( senderTab.id );
	};

	args = args || [];
	// Preppend the tab and the callback function to the arguments list
	args.unshift(senderTab);
	args.unshift(callback);
	handler.apply(this, args);
	return;
}


mooltipassEvent.onShowAlert = function(callback, tab, message) {
	alert(message);
}

mooltipassEvent.onLoadSettings = function(callback, tab) {
	page.settings = (typeof(localStorage.settings) == 'undefined') ? {} : JSON.parse(localStorage.settings);
	if (isFirefox || isSafari) page.settings.useMoolticute = true;
    mooltipass.backend.loadSettings();
}

mooltipassEvent.onLoadKeyRing = function(callback, tab) {
    //TODO: Gaston: I think this can be removed
}

mooltipassEvent.onGetSettings = function(callback, tab) {
	if (background_debug_msg > 4) mpDebug.log('%c mooltipassEvent: %c onGetSettings','background-color: #e2eef9','color: #246', tab);
	mooltipassEvent.onLoadSettings();
	var settings = page.settings;
	
	settings.status = mooltipass.device._status;
	settings.tabId = tab.id;
	callback({ data: settings }, tab );
}

mooltipassEvent.onSaveSettings = function(callback, tab, settings) {
	localStorage.settings = JSON.stringify(settings);
	mooltipassEvent.onLoadSettings();
}

mooltipassEvent.onGetStatus = function(callback, tab) {
	if (background_debug_msg > 5) mpDebug.log('%c mooltipassEvent: %c onGetStatus','background-color: #e2eef9','color: #246', tab);

	if ( tab ) {
		browserAction.showDefault(null, tab);
    	page.tabs[tab.id].errorMessage = undefined;  // XXX debug
    }

    var toReturn = {
		status: mooltipass.device.getStatus(),
		error: undefined,
		blacklisted: false
	};

    if ( tab && tab.url ) {
    	var tabStatus = mooltipass.backend.extractDomainAndSubdomain( tab.url );
    	toReturn.blacklisted = tabStatus.blacklisted;
    }

	callback( toReturn );
}

mooltipassEvent.onPopStack = function(callback, tab) {
	browserAction.stackPop(tab.id);
	browserAction.show(null, tab);
}

mooltipassEvent.onGetTabInformation = function(callback, tab) {
	var id = tab.id || page.currentTabId;

	callback(page.tabs[id]);
}

mooltipassEvent.onGetConnectedDatabase = function(callback, tab) {
	callback({
		"count": 10,
		"identifier": 'my_mp_db_id'
	});
}

mooltipassEvent.onRemoveCredentialsFromTabInformation = function(callback, tab) {
	var id = tab.id || page.currentTabId;

	page.clearCredentials(id);
}

mooltipassEvent.onNotifyButtonClick = function(id, buttonIndex) 
{
	// Check the kind of notification
	if (id.indexOf('mpNotConnected') == 0 || id.indexOf('mpNotUnlocked') == 0)
	{
		mooltipass.backend.disableNonUnlockedNotifications = true;
	}
	else
	{
		// Check notification type
		if(mooltipassEvent.mpUpdate[id].type == "singledomainadd")
		{
			// Adding a single domain notification
			if (buttonIndex == 0) 
			{
				// Blacklist
				//console.log('notification blacklist ',mooltipassEvent.mpUpdate[id].url);
				mooltipass.backend.blacklistUrl(mooltipassEvent.mpUpdate[id].url);
			} 
			else 
			{
			}
		}
		else if(mooltipassEvent.mpUpdate[id].type == "subdomainadd")
		{
			// Adding a sub domain notification
			if (buttonIndex == 0) 
			{
				// Store credentials
				//console.log('notification update with subdomain',mooltipassEvent.mpUpdate[id].username,'on',mooltipassEvent.mpUpdate[id].url);
				mooltipass.device.updateCredentials(null, mooltipassEvent.mpUpdate[id].tab, 0, mooltipassEvent.mpUpdate[id].username, mooltipassEvent.mpUpdate[id].password, mooltipassEvent.mpUpdate[id].url);
			} 
			else 
			{
				// Store credentials
				//console.log('notification update',mooltipassEvent.mpUpdate[id].username,'on',mooltipassEvent.mpUpdate[id].url2);
				mooltipass.device.updateCredentials(null, mooltipassEvent.mpUpdate[id].tab, 0, mooltipassEvent.mpUpdate[id].username, mooltipassEvent.mpUpdate[id].password, mooltipassEvent.mpUpdate[id].url2);
			}
		}
		delete mooltipassEvent.mpUpdate[id];		
	}

	// Close notification
	chrome.notifications.clear(id);
}

mooltipassEvent.onNotifyClosed = function(id) {
    delete mooltipassEvent.mpUpdate[id];
}

mooltipassEvent.notificationCount = 0;
mooltipassEvent.mpUpdate = {};

mooltipassEvent.isMooltipassUnlocked = function()
{
	if (background_debug_msg > 4) mpDebug.log('%c mooltipassEvent: %c isMooltipassUnlocked','background-color: #e2eef9','color: #246', arguments);
	// prevents "Failed to send to device: Transfer failed" error when device is suddenly unplugged
	if(typeof mooltipass.device._status.state == 'undefined') {
		return false;
	}

	// If the device is not connected and not unlocked and the user disabled the notifications, return
	if (mooltipass.device._status.state != 'Unlocked') {
		if (mooltipass.backend.disableNonUnlockedNotifications)
		{
			//console.log('Not showing a notification as they are disabled');
			return false;
		}
	}

	// Increment notification count
	mooltipassEvent.notificationCount++;
	var noteId = 'mpNotUnlocked.'+mooltipassEvent.notificationCount.toString();

	// Check that the Mooltipass app is installed and enabled
	if (!mooltipass.device._app || mooltipass.device._app['enabled'] !== true)
	{
		//console.log('notify: mooltipass app not ready');

		noteId = "mpNotUnlockedStaticMooltipassAppNotReady";

		// Create notification to inform user
		cross_notification(noteId,
			{   type: 'basic',
				title: 'Mooltipass App not ready!',
				message: 'The Mooltipass app is not installed or disabled',
				iconUrl: '/icons/warning_icon.png',
				buttons: [{title: 'Don\'t show these notifications', iconUrl: '/icons/forbidden-icon.png'}]});

		return false;
	}

	// Check that our device actually is connected
	if (mooltipass.device._status.state == 'NotConnected')
	{
		//console.log('notify: device not connected');

		noteId = "mpNotUnlockedStaticMooltipassNotConnected";

		// Create notification to inform user
		cross_notification(noteId,
			{   type: 'basic',
				title: 'Mooltipass Not Connected!',
				message: 'Please Connect Your Mooltipass',
				iconUrl: '/icons/warning_icon.png',
				buttons: [{title: 'Don\'t show these notifications', iconUrl: '/icons/forbidden-icon.png'}]});

		return false;
	}
	else if (mooltipass.device._status.state == 'Locked')
	{
		//console.log('notify: device locked');

		noteId = "mpNotUnlockedStaticMooltipassDeviceLocked";

		cross_notification( noteId, {
			type: 'basic',
			title: 'Mooltipass Locked!',
			message: 'Please Unlock Your Mooltipass',
			iconUrl: '/icons/warning_icon.png',
			buttons: [{title: 'Don\'t show these notifications', iconUrl: '/icons/forbidden-icon.png'}]
		});

		return false;
	}
	else if (mooltipass.device._status.state == 'NoCard')
	{
		//console.log('notify: device without card');

		noteId = "mpNotUnlockedStaticMooltipassDeviceWithoutCard";

		// Create notification to inform user
		cross_notification(noteId,
				{   type: 'basic',
					title: 'No Card in Mooltipass!',
					message: 'Please Insert Your Smartcard and Enter Your PIN',
					iconUrl: '/icons/warning_icon.png',
					buttons: [{title: 'Don\'t show these notifications', iconUrl: '/icons/forbidden-icon.png'}]});
					
		return false;
	}
	else if (mooltipass.device._status.state == 'ManageMode')
	{
		//console.log('notify: management mode');

		noteId = "mpNotUnlockedStaticMooltipassDeviceInManagementMode";

		var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
		var notification = {   
			type: 'basic',
			title: 'Mooltipass in Management Mode!',
			message: 'Please leave management mode in the App',
			iconUrl: '/icons/warning_icon.png'
		};

		if (!isFirefox) notification.buttons = [{title: 'Don\'t show these notifications', iconUrl: '/icons/forbidden-icon.png'}];

		// Create notification to inform user
		cross_notification(noteId,notification);
					
		return false;
	}
	
	return true;
}

mooltipassEvent.onUpdateNotify = function(callback, tab, username, password, url, usernameExists, credentialsList) {
	if (background_debug_msg > 2) mpDebug.log('%c mooltipassEvent: %c onUpdateNotify','background-color: #e2eef9','color: #246', arguments);

	// No password? Return
	if ( !password ) return;

	// Parse URL
	var parsed_url = mooltipass.backend.extractDomainAndSubdomain(url);
	var valid_url = false;
	var subdomain;
	var domain;

	//console.log('onUpdateNotify', 'parsed_url', parsed_url);
	
	// See if our script detected a valid domain & subdomain
	if(parsed_url.valid == true)
	{
		valid_url = true;
		domain = parsed_url.domain;
		subdomain = parsed_url.subdomain;
	}
	
	// Check if URL is valid
	if(valid_url == true)
	{
		// Check if blacklisted
		if (mooltipass.backend.isBlacklisted(domain))
		{
			//console.log('notify: ignoring blacklisted url',url);
			return;
		}
		
		// Check that the Mooltipass is unlocked
		var mp_unlocked = mooltipassEvent.isMooltipassUnlocked();
		
		// Increment notification count
		mooltipassEvent.notificationCount++;
		
		if(mp_unlocked && password.length > 31)
		{		
			var noteId = 'mpPasswordTooLong.'+ mooltipassEvent.notificationCount.toString();
			
			cross_notification(noteId,
				{   type: 'basic',
					title: 'Password Too Long!',
					message: "We are sorry, Mooltipass only supports passwords that are less than 31 characters",
					iconUrl: '/icons/warning_icon.png'});
			return;
		}

		if(subdomain == null)
		{
			// Single domain
			// Here we should send a request to the mooltipass to know if the username exists!
			if(true)
			{
				// Unknown user
				var noteId = 'mpUpdate.'+mooltipassEvent.notificationCount.toString();

				// Store our event
				mooltipassEvent.mpUpdate[noteId] = { tab: tab, username: username, password: password, url: domain, url2: domain, type: "singledomainadd"};
				
				// Send request by default
				mooltipass.device.updateCredentials(null, tab, 0, username, password, domain);

				// Create notification to blacklist
				if (mooltipass.device._status.unlocked) {
					cross_notification(noteId,
						{   type: 'basic',
							title: 'Credentials Detected!',
							message: 'Please Approve their Storage on the Mooltipass',
							iconUrl: '/icons/mooltipass-128.png',
							buttons: [{title: 'Black list this website', iconUrl: '/icons/forbidden-icon.png'}] },
							function(id) 
							{
								//console.log('notification created for',id);
							});
				}
			}
			else
			{}
		}
		else
		{
			// Subdomain exists
			// Here we should send a request to the mooltipass to know if the username exists!
			
			// first let's check to make sure the device is connected
			if(mooltipass.device._status.state == 'NotConnected')
			{
				//console.log('mooltipass not connected - do not ask which domain to store');
			}
			else{

				// Unknown user
				var noteId = 'mpUpdate.'+mooltipassEvent.notificationCount.toString();

				// Store our event
				mooltipassEvent.mpUpdate[noteId] = { tab: tab, username: username, password: password, url: domain, url2: subdomain + "." + domain, type: "subdomainadd"};

				var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
				var notification = {   
					type: 'basic',
					title: 'Subdomain Detected!',
					message: 'What domain do you want to store?',
					iconUrl: '/icons/question.png',
				};

				// Firefox doesn't support buttons on notifications
				if (!isFirefox && !isSafari) {
					notification.buttons = [{title: 'Store ' + domain}, {title: 'Store ' + subdomain + '.' + domain}];
				} else {
					// Firefox: Use domain (we should check against subdomain and later domain if missing tho...)
					notification.message = 'Please approve Domain storage';
					mooltipass.device.updateCredentials(null, tab, 0, username, password, subdomain + '.' + domain);
				}

				cross_notification(noteId,notification);
			}
		}		
	}	
}

mooltipassEvent.onUpdate = function(callback, tab, username, password, url, usernameExists, credentialsList) {
    mooltipass.device.updateCredentials(callback, tab, 0, username, password, url);
}

mooltipassEvent.onLoginPopup = function(callback, tab, logins) {
	var stackData = {
		level: 1,
		iconType: "questionmark",
		popup: "popup_login.html"
	}
	browserAction.stackUnshift(stackData, tab.id);

	page.tabs[tab.id].loginList = logins;

	browserAction.show(null, tab);
}

mooltipassEvent.onHTTPAuthPopup = function(callback, tab, data) {
	var stackData = {
		level: 1,
		iconType: "questionmark",
		popup: "popup_httpauth.html"
	}
	browserAction.stackUnshift(stackData, tab.id);

	page.tabs[tab.id].loginList = data;

	browserAction.show(null, tab);
}

mooltipassEvent.onMultipleFieldsPopup = function(callback, tab) {
	var stackData = {
		level: 1,
		iconType: "normal",
		popup: "popup_multiple-fields.html"
	}
	browserAction.stackUnshift(stackData, tab.id);

	browserAction.show(null, tab);
}

/*
 * Open either Chrome App or Moolticute Interface
 *
 */
mooltipassEvent.showApp = function() {
	if (moolticute.connectedToDaemon) {
		moolticute.sendRequest( {"msg": "show_app"} );
	} else {
		var global = chrome.extension.getBackgroundPage();
        chrome.runtime.sendMessage(global.mooltipass.device._app.id, { 'show': true });
	}
}

// all methods named in this object have to be declared BEFORE this!
mooltipassEvent.messageHandlers = {
	'update': mooltipassEvent.onUpdate,
	'add_credentials': mooltipass.device.addCredentials,
	'blacklist_url': mooltipass.backend.handlerBlacklistUrl,
	'unblacklist_url': mooltipass.backend.handlerUnBlacklistUrl,
	'blacklistUrl': mooltipass.backend.blacklistUrl,
	'alert': mooltipassEvent.onShowAlert,
	'get_connected_database': mooltipassEvent.onGetConnectedDatabase,
	'get_settings': mooltipassEvent.onGetSettings,
	'get_status': mooltipassEvent.onGetStatus,
	'get_tab_information': mooltipassEvent.onGetTabInformation,
	'load_keyring': mooltipassEvent.onLoadKeyRing,
	'load_settings': mooltipassEvent.onLoadSettings,
	'pop_stack': mooltipassEvent.onPopStack,
	'popup_login': mooltipassEvent.onLoginPopup,
	'popup_multiple-fields': mooltipassEvent.onMultipleFieldsPopup,
	'remove_credentials_from_tab_information': mooltipassEvent.onRemoveCredentialsFromTabInformation,
	'retrieve_credentials': mooltipass.device.retrieveCredentials,
	'show_default_browseraction': browserAction.showDefault,
	'update_credentials': mooltipass.device.updateCredentials,
	'save_settings': mooltipassEvent.onSaveSettings,
	'update_notify': mooltipassEvent.onUpdateNotify,
	'stack_add': browserAction.stackAdd,
	'generate_password': mooltipass.device.generatePassword,
    'set_current_tab': page.setCurrentTab,
    'cache_login': page.cacheLogin,
    'cache_retrieve': page.cacheRetrieve,
    'content_script_loaded': page.setAllLoaded,
    'show_app': mooltipassEvent.showApp
};

if (!isSafari) {
	chrome.notifications.onButtonClicked.addListener(mooltipassEvent.onNotifyButtonClick);
	chrome.notifications.onClosed.addListener(mooltipassEvent.onNotifyClosed);
}
