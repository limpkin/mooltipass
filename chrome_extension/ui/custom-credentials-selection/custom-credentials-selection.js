/*
 * Custom credentials selection overlay.
 * 
 * @param settings {Object}
 * @param origin {String}
 */

window.data = JSON.parse(decodeURIComponent(window.location.search.slice(1)))

$(function() {
	startEventHandling()
	cipDefine.init()

	function startEventHandling() {
		/*
		* Receive a message from WS_SOCKET or MooltiPass APP
		*/
		listenerCallback = function(req, sender, callback) {
			if ( isSafari ) req = req.message;

			if ('action' in req) {
				switch (req.action) {
					case 'custom_credentials_selection_mark_fields_data':
						cipDefine.onMarkFieldsData(req.args.fields)
						break
				}
			}
		};

		if ( isSafari ) safari.self.addEventListener("message", listenerCallback ,false);
		else {
			chrome.runtime.onMessage.removeListener( listenerCallback );
			chrome.runtime.onMessage.addListener( listenerCallback );
		}
	}
});

cipFields = {}

cipFields.inputQueryPattern = "input[type='text'], input[type='username'], input[type='email'], input[type='password'], input[type='tel'], input[type='number'], input:not([type])";
// unique number as new IDs for input fields
cipFields.uniqueNumber = 342845638;
// objects with combination of username + password fields
cipFields.combinations = [];

cipFields.setUniqueId = function(field) {
	if(field && !field.attr("data-mp-id")) {
		// use ID of field if it is unique
		// yes, it should be, but there are many bad developers outside...
		var fieldId = field.attr("id");
		if(fieldId) {
			var foundIds = mpJQ("input#" + cipFields.prepareId(fieldId));
			if(foundIds.length == 1) {
				field.attr("data-mp-id", fieldId);
				return;
			}
		}

		// create own ID if no ID is set for this field
		cipFields.uniqueNumber += 1;
		field.attr("data-mp-id", "mpJQ"+String(cipFields.uniqueNumber));
	}
}

cipFields.setFormUniqueId = function(field) {
	if(field && !field.attr("data-mp-id")) {
		// use ID of field if it is unique
		// yes, it should be, but there are many bad developers outside...
		var fieldId = field.attr("id");
		if(fieldId) {
			var foundIds = mpJQ("form#" + cipFields.prepareId(fieldId));
			if(foundIds.length == 1) {
				field.attr("data-mp-id", fieldId);
				return;
			}
		}

		// create own ID if no ID is set for this field
		cipFields.uniqueNumber += 1;
		field.attr("data-mp-id", "mpJQ"+String(cipFields.uniqueNumber));
	}
}

cipFields.prepareId = function(id) {
	return id.replace(/[:#.,\[\]\(\)' "]/g, function(m) {
												return "\\"+m
											});
}

cipFields.getAllFields = function() {
	//cipDebug.log('field call!');
	var fields = [];
	// get all input fields which are text, email or password and visible
	mpJQ(cipFields.inputQueryPattern).each(function() {
		if( mpJQ(this).hasClass('mooltipass-hash-ignore') ) {
			return;
		}
		if(cipFields.isAvailableField(this)) {
			cipFields.setUniqueId(mpJQ(this));
			fields.push(mpJQ(this));
			//cipDebug.log('field detection!', mpJQ(this));
		}
	});

	return fields;
};



cipFields.isSpecifiedFieldAvailable = function(fieldId) {
	return Boolean(_f(fieldId));
}

/**
 * Generates a hash based on the input fields currently visible to the user
 * @param fields array of input fields
 * @returns {string}
 */
cipFields.getHashForVisibleFields = function(fields) {
	var hash = '';
	for (var i = 0; i < fields.length; i++) {
		if( mpJQ(this).hasClass('mooltipass-hash-ignore') ) {
			continue;
		}
		hash += fields[i].attr('type') + fields[i].data('mp-id');
	};

	return hash;
}

cipFields.prepareVisibleFieldsWithID = function($pattern) {
	mpJQ($pattern).each(function() {
		if(cipFields.isAvailableField(this)) {
			cipFields.setUniqueId(mpJQ(this));
		}
	});
};

cipFields.isAvailableField = function($field) {
	return (
			mpJQ($field).is(":visible")
			&& mpJQ($field).css("visibility") != "hidden"
			&& !mpJQ($field).is(':disabled')
			&& mpJQ($field).css("visibility") != "collapsed"
			&& mpJQ($field).css("visibility") != "collapsed"
		);
}

cipFields.getAllCombinations = function(inputs) {
	cipDebug.log('cipFields.getAllCombinations');
	var fields = [];
	var uField = null;
	for(var i = 0; i < inputs.length; i++) {
		if(!inputs[i] || inputs[i].length < 1) {
			cipDebug.log("input discredited:");
			cipDebug.log(inputs[i]);
			continue;
		}
		else
		{
			cipDebug.log("examining input: ", inputs[i]);
		}

		if((inputs[i].attr("type") && inputs[i].attr("type").toLowerCase() == "password") || (inputs[i].attr("data-placeholder-type") && inputs[i].attr("data-placeholder-type").toLowerCase() == "password")){
			var uId = (!uField || uField.length < 1) ? null : cipFields.prepareId(uField.attr("data-mp-id"));

			var combination = {
				"username": uId,
				"password": cipFields.prepareId(inputs[i].attr("data-mp-id"))
			};
			fields.push(combination);

			// reset selected username field
			uField = null;
		}
		else {
			// username field
			uField = inputs[i];
		}
	}

	return fields;
}

cipFields.getCombination = function(givenType, fieldId) {
	cipDebug.log("cipFields.getCombination");

	if(cipFields.combinations.length == 0) {
		if(cipFields.useDefinedCredentialFields()) {
			return cipFields.combinations[0];
		}
	}
	// use defined credential fields (already loaded into combinations)
	if(cip.settings["defined-credential-fields"] && cip.settings["defined-credential-fields"][data.origin]) {
		return cipFields.combinations[0];
	}

	for(var i = 0; i < cipFields.combinations.length; i++) {
		if(cipFields.combinations[i][givenType] == fieldId) {
			return cipFields.combinations[i];
		}
	}

	// find new combination
	var combination = {
		"username": null,
		"password": null
	};

	var newCombi = false;
	if(givenType == "username") {
		var passwordField = cipFields.getPasswordField(fieldId, true);
		var passwordId = null;
		if(passwordField && passwordField.length > 0) {
			passwordId = cipFields.prepareId(passwordField.attr("data-mp-id"));
		}
		combination = {
			"username": fieldId,
			"password": passwordId
		};
		newCombi = true;
	}
	else if(givenType == "password") {
		var usernameField = cipFields.getUsernameField(fieldId, true);
		var usernameId = null;
		if(usernameField && usernameField.length > 0) {
			usernameId = cipFields.prepareId(usernameField.attr("data-mp-id"));
		}
		combination = {
			"username": usernameId,
			"password": fieldId
		};
		newCombi = true;
	}

	if(combination.username || combination.password) {
		cipFields.combinations.push(combination);
	}

	if(newCombi) {
		combination.isNew = true;
	}
	return combination;
}

/**
* return the username field or null if it not exists
*/
cipFields.getUsernameField = function(passwordId, checkDisabled) {
	var passwordField = _f(passwordId);
	if(!passwordField) {
		return null;
	}

	if ( cipDefine.selection && cipDefine.selection.username !== null ) {
		return mpJQ('#' + cipDefine.selection.username);
	}

	var form = passwordField.closest("form")[0];
	var usernameField = null;

	// search all inputs on this one form
	if(form) {
		mpJQ(cipFields.inputQueryPattern, form).each(function() {
			cipFields.setUniqueId(mpJQ(this));
			if(mpJQ(this).attr("data-mp-id") == passwordId) {
				// break
				return false;
			}

			if(mpJQ(this).attr("type") && mpJQ(this).attr("type").toLowerCase() == "password") {
				// continue
				return true;
			}

			usernameField = mpJQ(this);
		});
	}
	// search all inputs on page
	else {
		var inputs = cipFields.getAllFields();
		cip.initPasswordGenerator(inputs);
		for(var i = 0; i < inputs.length; i++) {
			if(inputs[i].attr("data-mp-id") == passwordId) {
				break;
			}

			if(inputs[i].attr("type") && inputs[i].attr("type").toLowerCase() == "password") {
				continue;
			}

			usernameField = inputs[i];
		}
	}

	if(usernameField && !checkDisabled) {
		var usernameId = usernameField.attr("data-mp-id");
		// check if usernameField is already used by another combination
		for(var i = 0; i < cipFields.combinations.length; i++) {
			if(cipFields.combinations[i].username == usernameId) {
				usernameField = null;
				break;
			}
		}
	}

	cipFields.setUniqueId(usernameField);

	return usernameField;
}

/**
* return the password field or null if it not exists
*/
cipFields.getPasswordField = function(usernameId, checkDisabled) {
	cipDebug.log('cipFields.getPasswordField');
	var usernameField = _f(usernameId);
	if(!usernameField) {
		return null;
	}

	if ( cipDefine.selection && cipDefine.selection.password !== null ) {
		return mpJQ('#' + cipDefine.selection.password);
	}

	var form = usernameField.closest("form")[0];
	var passwordField = null;

	// search all inputs on this one form
	if(form) {
		passwordField = mpJQ("input[type='password']:first", form);
		if(passwordField.length < 1) {
			passwordField = null;
		}

		cipPassword.init();
		cipPassword.initField(passwordField);
	}
	// search all inputs on page
	else {
		var inputs = cipFields.getAllFields();
		cip.initPasswordGenerator(inputs);

		var active = false;
		for(var i = 0; i < inputs.length; i++) {
			if(inputs[i].attr("data-mp-id") == usernameId) {
				active = true;
			}
			if(active && mpJQ(inputs[i]).attr("type") && mpJQ(inputs[i]).attr("type").toLowerCase() == "password") {
				passwordField = inputs[i];
				break;
			}
		}
	}

	if(passwordField && !checkDisabled) {
		var passwordId = passwordField.attr("data-mp-id");
		// check if passwordField is already used by another combination
		for(var i = 0; i < cipFields.combinations.length; i++) {
			if(cipFields.combinations[i].password == passwordId) {
				passwordField = null;
				break;
			}
		}
	}

	cipFields.setUniqueId(passwordField);

	return passwordField;
}

cipFields.prepareCombinations = function(combinations) {
	cipDebug.log("prepareCombinations, length: " + combinations.length);
	for(var i = 0; i < combinations.length; i++) {
		// disable autocomplete for username field
		if(_f(combinations[i].username)) {
			_f(combinations[i].username).attr("autocomplete", "off");
		}

		var pwField = _f(combinations[i].password);
		// needed for auto-complete: don't overwrite manually filled-in password field
		if(pwField && !pwField.data("cipFields-onChange")) {
			pwField.data("cipFields-onChange", true);
			pwField.change(function() {
				mpJQ(this).data("unchanged", false);
			});
		}

		// initialize form-submit for remembering credentials
		var fieldId = combinations[i].password || combinations[i].username;
		var field = _f(fieldId);
	}
}

cipFields.useDefinedCredentialFields = function() {
	if(cip.settings["defined-credential-fields"] && cip.settings["defined-credential-fields"][data.origin]) {
		var creds = cip.settings["defined-credential-fields"][data.origin];

		var $found = _f(creds.username) || _f(creds.password);
		for(var i = 0; i < creds.fields.length; i++) {
			if(_fs(creds.fields[i])) {
				$found = true;
				break;
			}
		}

		if($found) {
			var fields = {
				"username": creds.username,
				"password": creds.password,
				"fields": creds.fields
			};
			cipFields.combinations = [];
			cipFields.combinations.push(fields);

			return true;
		}
	}

	return false;
}

var cipDefine = {}
window.mpJQ = $;

cipDefine.eventFieldClick = null;

cipDefine.init = function () {
	var $backdrop = mpJQ("<div>").attr("id", "mp-bt-backdrop").addClass("mp-bt-modal-backdrop");
	mpJQ("body").append($backdrop);

	cipDefine.$chooser = mpJQ("<div>").attr("id", "mp-bt-cipDefine-fields");
	mpJQ("body").append(cipDefine.$chooser);

	var $description = mpJQ("<div>").attr("id", "mp-bt-cipDefine-description");
	mpJQ("body").append($description);

	cipFields.getAllFields();
	cipFields.prepareVisibleFieldsWithID("select");
	cipDefine.initDescription();
	cipDefine.resetSelection();

	cipDefine.prepareStep1();
	cipDefine.markAllUsernameFields();
}

cipDefine.initDescription = function() {
	var $description = mpJQ("div#mp-bt-cipDefine-description");
	var $h1 = mpJQ("<div>").addClass("mp-bt-chooser-headline");
	
	var $help = mpJQ("<div>").addClass("mp-bt-chooser-help").attr("id", "mp-bt-help");

	var $buttonWrap = mpJQ("<div>").attr("id", "mp-bt-buttonWrap").addClass("mooltipass-text-right")

	var $btnSkip = mpJQ("<button>").text("Skip").attr("id", "mp-bt-btn-skip")
		.css("margin-right", "5px")
		.click(function() {
			if(mpJQ(this).data("step") == 1) {
				cipDefine.selection.username = null;
				
				messaging({
					action: 'create_action',
					args: [{
						action: 'custom_credentials_selection_selected',
						args: {
							username: null
						}
					}]
				})
				
				cipDefine.prepareStep2();
				cipDefine.markAllPasswordFields(mpJQ("#mp-bt-cipDefine-fields"));
			}
			else if(mpJQ(this).data("step") == 2) {
				cipDefine.selection.password = null;
				
				messaging({
					action: 'create_action',
					args: [{
						action: 'custom_credentials_selection_selected',
						args: {
							password: null
						}
					}]
				})
				
				cipDefine.prepareStep3();
			}
		});
	var $btnCancel = mpJQ("<button class='alert tiny'>").text("Cancel").attr("id", "mp-bt-btn-cancel").attr("href",'#')
		.click(function(e) {
			data.settings["defined-credential-fields"][data.origin] = null
			
			messaging({
				action: 'create_action',
				args: [{
					action: 'custom_credentials_selection_cancelled',
					args: {}
				}]
			});
			
			messaging({
				action: 'save_settings',
				args: [data.settings]
			});
			
			messaging({
				action: 'create_action',
				args: [{
					action: 'custom_credentials_selection_hide',
					args: {} 
				}]
			});
		})
		
	var $btnConfirm = mpJQ("<button class='success tiny'>").text("Confirm").attr("id", "mp-bt-btn-confirm")
		.css("margin-right", "15px")
		.hide()
		.click(function(e) {
			if(cipDefine.selection.username) {
				username = cipFields.prepareId(cipDefine.selection.username);
			}

			var passwordId = mpJQ("div#mp-bt-cipDefine-fields").data("password");
			if(cipDefine.selection.password) {
				password = cipFields.prepareId(cipDefine.selection.password);
			}

			var fieldIds = [];
			var fieldKeys = Object.keys(cipDefine.selection.fields);
			for(var i = 0; i < fieldKeys.length; i++) {
				fieldIds.push(cipFields.prepareId(fieldKeys[i]));
			}

			data.settings["defined-credential-fields"][data.origin] = {
				"username": username,
				"password": password,
				"fields": fieldIds
			};

			messaging({
				action: 'create_action',
				args: [{
					action: 'custom_credentials_selection_selected',
					args: {
						username: username,
						password: password,
						fieldsIds: fieldIds,
						fields: cipDefine.selection.fields
					}
				}]
			});
			
			messaging({
				action: 'save_settings',
				args: [data.settings]
			});

			messaging({
				action: 'create_action',
				args: [{
					action: 'custom_credentials_selection_hide',
					args: {}
				}]
			});
		})
		.hide();

	$description.append($h1);
	$buttonWrap.append($btnConfirm);
	$buttonWrap.append($btnCancel);
	$description.append($buttonWrap);

	// Last piece of jquery-ui
	mpJQ("div#mp-bt-cipDefine-description").draggable();
}

cipDefine.resetSelection = function() {
	cipDefine.selection = {
		username: null,
		password: null,
		fields: {}
	};
}

cipDefine.markAllUsernameFields = function() {
	cipDefine.eventFieldClick = function(e) {
		cipDefine.selection.username = mpJQ(this).data("mp-id");
		messaging({
			action: 'create_action',
			args: [{
				action: 'custom_credentials_selection_selected',
				args: {
					username: cipDefine.selection.username
				}
			}]
		})
		
		mpJQ(this).addClass("mp-bt-fixed-username-field").text("Username").unbind("click");
		cipDefine.prepareStep2();
		cipDefine.markAllPasswordFields(mpJQ("#mp-bt-cipDefine-fields"));
	};
	
	messaging({
		action: 'create_action',
		args: [{
			action: 'custom_credentials_selection_request_mark_fields_data',
			args: {
				pattern: cipFields.inputQueryPattern
			}
		}]
	});
}

cipDefine.markAllPasswordFields = function() {
	cipDefine.eventFieldClick = function(e) {
		cipDefine.selection.password = mpJQ(this).data("mp-id");
		messaging({
			action: 'create_action',
			args: [{
				action: 'custom_credentials_selection_selected',
				args: {
					password: cipDefine.selection.password
				}
			}]
		})
		
		mpJQ(this).addClass("mp-bt-fixed-password-field").text("Password").unbind("click");
		cipDefine.prepareStep3();
	};

	messaging({
		action: 'create_action',
		args: [{
			action: 'custom_credentials_selection_request_mark_fields_data',
			args: {
				pattern: "input[type='password']"
			}
		}]
	});
}

cipDefine.onMarkFieldsData = function(fields) {
	fields.forEach(function(field) {
		var $field = mpJQ("<div>").addClass("mp-bt-fixed-field")
			.css("top", field.top)
			.css("left", field.left)
			.css("width", field.width)
			.css("height", field.height)
			.attr("data-mp-id", field.id)
			.click(cipDefine.eventFieldClick)
			.hover(function() {mpJQ(this).addClass("mp-bt-fixed-hover-field");}, function() {mpJQ(this).removeClass("mp-bt-fixed-hover-field");});
			
		cipDefine.$chooser.append($field);
	});
}

cipDefine.prepareStep1 = function() {
	mpJQ("div#mp-bt-help").text("").css("margin-bottom", 0);
	mpJQ("div#mp-bt-cipDefine-fields").removeData("username");
	mpJQ("div#mp-bt-cipDefine-fields").removeData("password");
	mpJQ("div.mp-bt-fixed-field", mpJQ("div#mp-bt-cipDefine-fields")).remove();
	mpJQ("div:first", mpJQ("div#mp-bt-cipDefine-description")).text("1. Choose a username field");
	mpJQ("button#mp-bt-btn-skip:first").data("step", "1").show();
	mpJQ("button#mp-bt-btn-confirm:first").hide();
}

cipDefine.prepareStep2 = function() {
	mpJQ("div#mp-bt-help").text("").css("margin-bottom", 0);
	mpJQ("div.mp-bt-fixed-field:not(.mp-bt-fixed-username-field)", mpJQ("div#mp-bt-cipDefine-fields")).remove();
	mpJQ("div:first", mpJQ("div#mp-bt-cipDefine-description")).text("2. Now choose a password field");
	mpJQ("button#mp-bt-btn-skip:first").data("step", "2");
}

cipDefine.prepareStep3 = function() {
	if(!cipDefine.selection.username && !cipDefine.selection.password) {
		mpJQ("button#mp-bt-btn-confirm:first").removeClass("mp-bt-btn-primary").attr("disabled", true);
	}

	mpJQ("div#mp-bt-help").html("Please confirm your selection or choose more fields as <em>String fields</em>.").css("margin-bottom", "5px");
	mpJQ("div.mp-bt-fixed-field:not(.mp-bt-fixed-password-field,.mp-bt-fixed-username-field)", mpJQ("div#mp-bt-cipDefine-fields")).remove();
	mpJQ("button#mp-bt-btn-confirm:first").show();
	mpJQ("button#mp-bt-btn-skip:first").data("step", "3").hide();
	mpJQ("div:first", mpJQ("div#mp-bt-cipDefine-description")).text("3. Confirm selection");
}

// Unify messaging method - And eliminate callbacks (a message is replied with another message instead)
function messaging( message ) {
	if (content_debug_msg > 4) cipDebug.log('%c Sending message to background:','background-color: #0000FF; color: #FFF; padding: 5px; ', message);
	if ( isSafari ) safari.self.tab.dispatchMessage("messageFromContent", message);
	else chrome.runtime.sendMessage( message );
};

var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
var isSafari = typeof(safari) == 'object'?true:false;
var content_debug_msg = (!isFirefox && !isSafari && chrome.runtime && !('update_url' in chrome.runtime.getManifest())) ? 55 : false;

var cipDebug = {};
if (content_debug_msg) {
	cipDebug.log = function( message ) {
		this.log( message );
	}
	cipDebug.log = console.log.bind(window.console);
	cipDebug.warn = console.warn.bind(window.console);
	cipDebug.trace = console.trace.bind(window.console);
	cipDebug.error = console.error.bind(window.console);
} else {
	cipDebug.log = function() {}
	cipDebug.log = function() {}
	cipDebug.warn = function() {}
	cipDebug.trace = function() {}
	cipDebug.error = function() {}
}
