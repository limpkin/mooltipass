// Adding this here just in case.
window.mpJQ = $;

// Detect if we're dealing with Firefox, Safari, or Chrome
var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
var isSafari = typeof(safari) == 'object'?true:false;

// Unify messaging method - And eliminate callbacks (a message is replied with another message instead)
function messaging( message ) {
	if (content_debug_msg > 4) cipDebug.log('%c Sending message to background:','background-color: #0000FF; color: #FFF; padding: 5px; ', message);
	if ( isSafari ) safari.self.tab.dispatchMessage("messageFromContent", message);
	else chrome.runtime.sendMessage( message );
};

// contains already called method names
var _called = {};

var content_debug_msg = false;

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

// Deprecated code
function _f(fieldId) {
	var field = (fieldId) ? mpJQ("input[data-mp-id='"+fieldId+"']:first") : [];
	return (field.length > 0) ? field : null;
}

function _fs(fieldId) {
	var field = (fieldId) ? mpJQ("input[data-mp-id='"+fieldId+"']:first,select[data-mp-id='"+fieldId+"']:first").first() : [];
	return (field.length > 0) ? field : null;
}

var cipPassword = {};

cipPassword.observedIcons = [];
cipPassword.observingLock = false;

cipPassword.init = function() {
	if (content_debug_msg > 4) cipDebug.log('%c cipPassword: %c init','background-color: #ff8e1b','color: #333333');
	if("initPasswordGenerator" in _called) {
		return;
	}

	_called.initPasswordGenerator = true;

	window.setInterval(function() {
		cipPassword.checkObservedElements();
	}, 400);
}

cipPassword.initField = function(field, inputs, pos) {
	if (content_debug_msg > 4) cipDebug.log('%c cipPassword: %c initField','background-color: #ff8e1b','color: #333333', field);
	if(!field || field.length != 1) {
		return;
	}

	if(field.data("mp-password-generator")) {
		return;
	}

	// Prevent showing icon if password field is hidden by width or height.
	if ( field[0].clientWidth < 2 || field[0].clientHeight < 2 ) {
		return;
	}

	// Prevent showing icon if the password field has a tabIndex of -1 
	if ( field.prop('tabindex') == -1) {
		return;
	}

	field.data("mp-password-generator", true);

	cipPassword.createIcon(field);
	mpDialog.precreate( inputs, field );

	var $found = false;
	if(inputs) {
		for(var i = pos + 1; i < inputs.length; i++) {
			if(inputs[i] && inputs[i].attr("type") && inputs[i].attr("type").toLowerCase() == "password") {
				field.data("mp-genpw-next-field-id", inputs[i].data("mp-id"));
				field.data("mp-genpw-next-is-password-field", (i == 0));
				$found = true;
				break;
			}
		}
	}

	field.data("mp-genpw-next-field-exists", $found);
}

cipPassword.generatePassword = function() {
	messaging( { action: 'generate_password', args: [ cip.settings['usePasswordGeneratorLength'] ] } );
}

cipPassword.generatePasswordFromSettings = function( passwordSettings ) {
	var charactersLowercase = 'abcdefghijklmnopqrstuvwxyz';
	var charactersUppercase = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
	var charactersNumbers = '1234567890';
	var charactersSpecial = '!$%*()_+{}-[]:"|;\'?,./';
	var hash = "";
	var possible = "";
	var length = cip.settings['usePasswordGeneratorLength'];

	if( passwordSettings.settings["usePasswordGeneratorLowercase"] ) possible += charactersLowercase;
	if( passwordSettings.settings["usePasswordGeneratorUppercase"]) possible += charactersUppercase;
	if( passwordSettings.settings["usePasswordGeneratorNumbers"]) possible += charactersNumbers;
	if( passwordSettings.settings["usePasswordGeneratorSpecial"]) possible += charactersSpecial;
        
	for( var i = 0; i < length; i++ ) {
		hash += possible.charAt( Math.floor(passwordSettings.seeds[i] * possible.length) );
	}
	return hash;
} 

cipPassword.createIcon = function(field) {
	if (content_debug_msg > 4) cipDebug.log('%c cipPassword: %c createIcon','background-color: #ff8e1b','color: #333333', field);

	// Check if there are other icons in the page
	var currentIcons = mpJQ('.mp-genpw-icon');
	var iconIndex = currentIcons.length;
	if ( iconIndex > 0 ) {
		for ( var I = 0; I < iconIndex; I++ ) {
			if ( field.data("mp-id") === mpJQ(currentIcons[I]).data('mp-genpw-field-id') ) { // An icon for this field already exists
				mpJQ(currentIcons[I]).remove();
			}
		}
	}

	var $className = (field.outerHeight() > 28) ? "mp-icon-key-big" : "mp-icon-key-small";
	var $size = (field.outerHeight() > 28) ? 24 : 16;
	var $offset = Math.floor((field.outerHeight() - $size) / 3);
	$offset = ($offset < 0) ? 0 : $offset;

	var $zIndex = 0;
	var $zIndexField = field;
	var z;
	var c = 0;
	while($zIndexField.length > 0) {
		if(c > 100 || $zIndexField[0].nodeName == "#document") {
			break;
		}
		z = $zIndexField.css("z-index");
		if(!isNaN(z) && parseInt(z) > $zIndex) {
			$zIndex = parseInt(z);
		}
		$zIndexField = $zIndexField.parent();
		c++;
	}

	if(isNaN($zIndex) || $zIndex < 1) {
		$zIndex = 1;
	}
	$zIndex += 1;

	var $icon = mpJQ("<div>").addClass("mp-genpw-icon")
		.addClass($className)
		.css("z-index", $zIndex)
		.data("size", $size)
		.data("offset", $offset)
		.data("index", iconIndex)
		.data("mp-genpw-field-id", field.data("mp-id"));

	cipPassword.setIconPosition($icon, field);

	$icon.click(function(e) {
		e.preventDefault();
		e.stopPropagation();

		// Use event target for cases when there are more than 1 password in the screen
		var field = $(e.target);
		if(!field.is(":visible")) {
			$icon.remove();
			field.removeData("mp-password-generator");
			return;
		}

		mpDialog.toggle( e );

		// Check if the current form has a combination associated to it
		var fieldID = mpJQ(e.target).data('mp-genpw-field-id');

		var associatedInput = mpJQ('#' + fieldID + ',input[data-mp-id=' + fieldID + ']' );
		var containerForm = associatedInput.closest('form');
		var comb = false;

		// Search for combination departing from FORM (probably refactor to be a sole function in mcCombs)
		if ( containerForm.length == 0 ) comb = mcCombs.forms.noform.combination;
		else {
			for (form in mcCombs.forms) {
				if ( form === containerForm.prop('id') || form === containerForm.data('mp-id') ) { // Match found
					comb = mcCombs.forms[form].combination;
				}
			}
		}
		if ( comb && comb.isPasswordOnly ) mpDialog.showLoginArea();
	});

	cipPassword.observedIcons.push($icon);

	// TODO: Move icons to the field area instead of the body
	// $icon.insertAfter( field ); 
	mpJQ("body").append($icon);
}

cipPassword.setIconPosition = function($icon, $field) {
	// TODO: Move icons to the field area instead of the body
	// $icon.css('left', $field.outerWidth() - $icon.data("size") );
	$icon.css("top", $field.offset().top + $icon.data("offset") + 1)
		.css("left", $field.offset().left + $field.outerWidth() - $icon.data("size") - $icon.data("offset"))
}

cipPassword.callbackPasswordCopied = function(bool) {
	if(bool) {
		mpJQ("#mp-genpw-btn-clipboard").addClass("mp-bt-btn-success");
	}
}

cipPassword.callbackGeneratedPassword = function(entries) {
	if(entries && entries.length >= 1) {
		mpJQ("#mp-genpw-btn-clipboard:first").removeClass("mp-bt-btn-success");
		mpJQ("input#mp-genpw-textfield-password:first").val(entries[0].Password);
		if(isNaN(entries[0].Login)) {
			mpJQ("#mp-genpw-quality:first").text("??? Bits");
		}
		else {
			mpJQ("#mp-genpw-quality:first").text(entries[0].Login + " Bits");
		}
	}
	else {
		if(mpJQ("div#mp-genpw-error:first").length == 0) {
			mpJQ("button#mp-genpw-btn-generate:first").after("<div style='block' id='mp-genpw-error'>Cannot receive generated password.<br />Is your version of KeePassHttp up-to-date?<br /><br /><a href='https://github.com/pfn/keepasshttp/'>Please visit the KeePassHttp homepage</a></div>");
			mpJQ("input#mp-genpw-textfield-password:first").parent().hide();
			mpJQ("button#mp-genpw-btn-generate").hide();
			mpJQ("button#mp-genpw-btn-clipboard").hide();
			mpJQ("button#mp-genpw-btn-fillin").hide();
		}
	}
}

cipPassword.onRequestPassword = function() {
	chrome.runtime.sendMessage({
		'action': 'generate_password'
	}, cipPassword.callbackGeneratedPassword);
}

cipPassword.checkObservedElements = function() {
	if ( typeof(mpJQ) === 'undefined') return;
	if(cipPassword.observingLock) {
		return;
	}

	cipPassword.observingLock = true;
	mpJQ.each(cipPassword.observedIcons, function(index, iconField) {
		if(iconField && iconField.length == 1) {
			var fieldId = iconField.data("mp-genpw-field-id");
			var field = mpJQ("input[data-mp-id='"+fieldId+"']:first");
			if(!field || field.length != 1) {
				iconField.remove();
				cipPassword.observedIcons.splice(index, 1);
			}
			else if(!field.is(":visible")) {
				iconField.hide();
			}
			else if(field.is(":visible")) {
				iconField.show();
				cipPassword.setIconPosition(iconField, field);
				field.data("mp-password-generator", true);
			}
		}
		else {
			cipPassword.observedIcons.splice(index, 1);
		}
	});
	cipPassword.observingLock = false;
}

var cipDefine = {};

cipDefine.selection = {
	"username": null,
	"password": null,
	"fields": {}
};
cipDefine.eventFieldClick = null;

cipDefine.init = function () {
	var $backdrop = mpJQ("<div>").attr("id", "mp-bt-backdrop").addClass("mp-bt-modal-backdrop");
	mpJQ("body").append($backdrop);

	var $chooser = mpJQ("<div>").attr("id", "mp-bt-cipDefine-fields");
	mpJQ("body").append($chooser);

	var $description = mpJQ("<div>").attr("id", "mp-bt-cipDefine-description");
	$backdrop.append($description);

	cipFields.getAllFields();
	cipFields.prepareVisibleFieldsWithID("select");

	cipDefine.initDescription();

	cipDefine.resetSelection();

	if ( $('#mp-genpw-dialog').data('mpPasswordOnlyCombination') ) {
		cipDefine.selection.username = 'mooltipass-username';
		cipDefine.prepareStep2();
		cipDefine.markAllPasswordFields($chooser);
		return;
	} else {
		cipDefine.prepareStep1();
		cipDefine.markAllUsernameFields($chooser);
	}

	
}

cipDefine.initDescription = function() {
	var $description = mpJQ("div#mp-bt-cipDefine-description").css("min-width", "300px");
	var $h1 = mpJQ("<div>").addClass("mp-bt-chooser-headline");
	
	var $help = mpJQ("<div>").addClass("mp-bt-chooser-help").attr("id", "mp-bt-help");

	var $buttonWrap = mpJQ("<div>").attr("id", "mp-bt-buttonWrap")
						.addClass("mooltipass-text-right")
						.hide();

	var $btnDismiss = mpJQ("<a>").text("Dismiss").attr("id", "mp-bt-btn-dismiss").attr("href",'#')
		.click(function(e) {
			mpJQ("div#mp-bt-backdrop").remove();
			mpJQ("div#mp-bt-cipDefine-fields").remove();
		});

	var $btnSkip = mpJQ("<button>").text("Skip").attr("id", "mp-bt-btn-skip")
		.css("margin-right", "5px")
		.click(function() {
			if(mpJQ(this).data("step") == 1) {
				cipDefine.selection.username = null;
				cipDefine.prepareStep2();
				cipDefine.markAllPasswordFields(mpJQ("#mp-bt-cipDefine-fields"));
				mpJQ("#mp-bt-btn-again").hide();
			}
			else if(mpJQ(this).data("step") == 2) {
				cipDefine.selection.password = null;
				cipDefine.prepareStep3();
				cipDefine.markAllStringFields(mpJQ("#mp-bt-cipDefine-fields"));
				mpJQ("#mp-bt-btn-again").show();
			}
		});
	var $btnAgain = mpJQ("<a>").text("Undo").attr("id", "mp-bt-btn-again").attr("href",'#')
		.click(function(e) {
			cipDefine.resetSelection();
			cipDefine.prepareStep1();
			cipDefine.markAllUsernameFields(mpJQ("#mp-bt-cipDefine-fields"));
		})
		.hide();
	var $btnConfirm = mpJQ("<button>").text("Confirm").attr("id", "mp-bt-btn-confirm")
		.css("margin-right", "15px")
		.hide()
		.click(function(e) {
			if(!cip.settings["defined-credential-fields"]) {
				cip.settings["defined-credential-fields"] = {};
			}

			if(cipDefine.selection.username) {
				cipDefine.selection.username = cipFields.prepareId(cipDefine.selection.username);
			}

			var passwordId = mpJQ("div#mp-bt-cipDefine-fields").data("password");
			if(cipDefine.selection.password) {
				cipDefine.selection.password = cipFields.prepareId(cipDefine.selection.password);
			}

			var fieldIds = [];
			var fieldKeys = Object.keys(cipDefine.selection.fields);
			for(var i = 0; i < fieldKeys.length; i++) {
				fieldIds.push(cipFields.prepareId(fieldKeys[i]));
			}

			cip.settings["defined-credential-fields"][document.location.origin] = {
				"username": cipDefine.selection.username,
				"password": cipDefine.selection.password,
				"fields": fieldIds
			};

			chrome.runtime.sendMessage({
				action: 'save_settings',
				args: [cip.settings]
			});

			mpJQ("#mp-bt-btn-dismiss").click();
		})
		.hide();

	$h1.append($btnDismiss);

	$description.append($h1);
	$description.append($btnDismiss);
	
	$buttonWrap.append($btnAgain);

	$buttonWrap.append($btnConfirm);

	if(cip.settings["defined-credential-fields"] && cip.settings["defined-credential-fields"][document.location.origin]) {
		var $p = mpJQ("<p id='mp-already-existent-message'>").html("For this page credential fields are already selected and will be overwritten.");
		$description.append($p);
	}

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

cipDefine.isFieldSelected = function($cipId) {
	return (
		$cipId == cipDefine.selection.username ||
		$cipId == cipDefine.selection.password ||
		$cipId in cipDefine.selection.fields
	);
}

cipDefine.markAllUsernameFields = function($chooser) {
	cipDefine.eventFieldClick = function(e) {
		cipDefine.selection.username = mpJQ(this).data("mp-id");
		mpJQ(this).addClass("mp-bt-fixed-username-field").text("Username").unbind("click");
		cipDefine.prepareStep2();
		cipDefine.markAllPasswordFields(mpJQ("#mp-bt-cipDefine-fields"));
	};
	cipDefine.markFields($chooser, cipFields.inputQueryPattern);
}

cipDefine.markAllPasswordFields = function($chooser) {
	cipDefine.eventFieldClick = function(e) {
		cipDefine.selection.password = mpJQ(this).data("mp-id");
		mpJQ(this).addClass("mp-bt-fixed-password-field").text("Password").unbind("click");
		cipDefine.prepareStep3();
		cipDefine.markAllStringFields(mpJQ("#mp-bt-cipDefine-fields"));
	};
	cipDefine.markFields($chooser, "input[type='password']");
}

cipDefine.markAllStringFields = function($chooser) {
	cipDefine.eventFieldClick = function(e) {
		cipDefine.selection.fields[mpJQ(this).data("mp-id")] = true;
		var count = Object.keys(cipDefine.selection.fields).length;
		mpJQ(this).addClass("mp-bt-fixed-string-field").text("String field #"+count.toString()).unbind("click");

		mpJQ("button#mp-bt-btn-confirm:first").addClass("mp-bt-btn-primary").attr("disabled", false);
	};
	cipDefine.markFields($chooser, cipFields.inputQueryPattern + ", select");
}

cipDefine.markFields = function ($chooser, $pattern) {
	//var $found = false;
	mpJQ($pattern).each(function() {
		if(cipDefine.isFieldSelected(mpJQ(this).data("mp-id"))) {
			//continue
			return true;
		}

		if(mpJQ(this).is(":visible") && mpJQ(this).css("visibility") != "hidden" && mpJQ(this).css("visibility") != "collapsed") {
			var $field = mpJQ("<div>").addClass("mp-bt-fixed-field")
				.css("top", mpJQ(this).offset().top)
				.css("left", mpJQ(this).offset().left)
				.css("width", mpJQ(this).outerWidth())
				.css("height", mpJQ(this).outerHeight())
				.attr("data-mp-id", mpJQ(this).attr("data-mp-id"))
				.click(cipDefine.eventFieldClick)
				.hover(function() {mpJQ(this).addClass("mp-bt-fixed-hover-field");}, function() {mpJQ(this).removeClass("mp-bt-fixed-hover-field");});
			$chooser.append($field);
			//$found = true;
		}
	});

	/* skip step if no entry was found
	if(!$found) {
		alert("No username field found.\nContinue with choosing a password field.");
		mpJQ("button#mp-bt-btn-skip").click();
	}
	*/
}

cipDefine.prepareStep1 = function() {
	mpJQ("div#mp-bt-help").text("").css("margin-bottom", 0);
	mpJQ("div#mp-bt-cipDefine-fields").removeData("username");
	mpJQ("div#mp-bt-cipDefine-fields").removeData("password");
	mpJQ("div.mp-bt-fixed-field", mpJQ("div#mp-bt-cipDefine-fields")).remove();
	mpJQ("div:first", mpJQ("div#mp-bt-cipDefine-description")).text("1. Choose a username field");
	mpJQ("button#mp-bt-btn-skip:first").data("step", "1").show();
	mpJQ("button#mp-bt-btn-confirm:first").hide();
	mpJQ("button#mp-bt-btn-again:first").hide();
}

cipDefine.prepareStep2 = function() {
	mpJQ("div#mp-bt-help").text("").css("margin-bottom", 0);
	mpJQ("div.mp-bt-fixed-field:not(.mp-bt-fixed-username-field)", mpJQ("div#mp-bt-cipDefine-fields")).remove();
	mpJQ("div:first", mpJQ("div#mp-bt-cipDefine-description")).text("2. Now choose a password field");
	mpJQ("button#mp-bt-btn-skip:first").data("step", "2");
	mpJQ("button#mp-bt-btn-again:first").show();
}

cipDefine.prepareStep3 = function() {
	/* skip step if no entry was found
	if(!mpJQ("div#mp-bt-cipDefine-fields").data("username") && !mpJQ("div#mp-bt-cipDefine-fields").data("password")) {
		alert("Neither an username field nor a password field were selected.\nNothing will be changed and chooser will be closed now.");
		mpJQ("button#mp-bt-btn-dismiss").click();
		return;
	}
	 */

	if(!cipDefine.selection.username && !cipDefine.selection.password) {
		mpJQ("button#mp-bt-btn-confirm:first").removeClass("mp-bt-btn-primary").attr("disabled", true);
	}

	mpJQ("div#mp-bt-help").html("Please confirm your selection or choose more fields as <em>String fields</em>.").css("margin-bottom", "5px");
	mpJQ("div.mp-bt-fixed-field:not(.mp-bt-fixed-password-field,.mp-bt-fixed-username-field)", mpJQ("div#mp-bt-cipDefine-fields")).remove();
	mpJQ("button#mp-bt-btn-confirm:first").show();
	mpJQ("div#mp-bt-buttonWrap").show();
	mpJQ("button#mp-bt-btn-skip:first").data("step", "3").hide();
	mpJQ("div:first", mpJQ("div#mp-bt-cipDefine-description")).text("3. Confirm selection");
}



cipFields = {}

cipFields.inputQueryPattern = "input[type='text'], input[type='email'], input[type='password'], input[type='tel'], input[type='number'], input:not([type])";
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
		hash += fields[i].data('mp-id');
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
	if(cip.settings["defined-credential-fields"] && cip.settings["defined-credential-fields"][document.location.origin]) {
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
	if(cip.settings["defined-credential-fields"] && cip.settings["defined-credential-fields"][document.location.origin]) {
		var creds = cip.settings["defined-credential-fields"][document.location.origin];

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



var cip = {};

// settings of chromeIPass
cip.settings = {};
// username field which will be set on focus
cip.u = null;
// password field which will be set on focus
cip.p = null;
// document.location
cip.url = null;
// request-url of the form in which the field is located
cip.submitUrl = null;
// received credentials from KeePassHTTP
cip.credentials = [];

cip.trapSubmit = true;

cip.visibleInputsHash = 0;

// Post could be either by form submit or XHR. Either way, we wait for it.
cip.waitingForPost = true;

//Auto-submit form
cip.autoSubmit = true;

// If the form has captcha, we can't autosubmit.
cip.formHasCaptcha = false;

// Flag setting that we only wish to fill in the password
cip.fillPasswordOnly = false;

cip.init = function() {
	cipDebug.warn('Starting CIP');
	// Grab settings from mcCombinations.
	cip.settings = mcCombs.settings;
	cip.initCredentialFields();
};

cip.initCredentialFields = function(forceCall) {
	if(_called.initCredentialFields && !forceCall) {
		return;
	}
	_called.initCredentialFields = true;

	var inputs = cipFields.getAllFields();
	cipDebug.log('initCredentialFields(): ' + inputs.length + ' input fields found');

	cip.visibleInputsHash = cipFields.getHashForVisibleFields(inputs);

	cipFields.prepareVisibleFieldsWithID("select");
	cipDebug.log('about to start pwd gen');
	cip.initPasswordGenerator(inputs);

	var searchForAllCombinations = true;
	var manualSpecifiedFields = false;
	if(cipFields.useDefinedCredentialFields()) {
		searchForAllCombinations = false;
		manualSpecifiedFields = true;
	}

	var inputs = cipFields.getAllFields();
	
	// If we detected a captcha procedure in the form, we will prevent auto-submit
	if ( cip.formHasCaptcha) cip.autoSubmit = false; 

	/*
	 * Uncomment next line of code for development tests (will prevent forms to auto-submit)
	*/
	//cip.autoSubmit = false; // Temporarily forbid auto-submit

	// get all combinations of username + password fields
	cipFields.combinations = cipFields.getAllCombinations(inputs);

	cipFields.prepareCombinations(cipFields.combinations);

	//cipDebug.log('Combinations found:', cipFields.combinations );
	if(cipFields.combinations.length == 0) {
		if ( !isSafari ) chrome.runtime.sendMessage({
			'action': 'show_default_browseraction'
		});
		return;
	}

	// If manual specified credential fields are not available on the current page (defined, 2-page login)
	// --> don't trigger request for credentials to device
	if(manualSpecifiedFields) {
		if(!cipFields.isSpecifiedFieldAvailable(cipFields.combinations[0].username)
		&& !cipFields.isSpecifiedFieldAvailable(cipFields.combinations[0].password)
		&& (!cipFields.combinations[0].fields || (cipFields.combinations[0].fields.length > 0 && !cipFields.isSpecifiedFieldAvailable(cipFields.combinations[0].fields[0])))) {
			chrome.runtime.sendMessage({
				'action': 'show_default_browseraction'
			});
			return;
		}
	}

	cip.url = document.location.origin;
	cip.submitUrl = cip.getFormActionUrl(cipFields.combinations[0]);

	chrome.runtime.sendMessage({
		'action': 'retrieve_credentials',
		'args': [ cip.url, cip.submitUrl, true, true]
	}, cip.retrieveCredentialsCallback);
} // end function init

cip.initPasswordGenerator = function(inputs) {
	if (content_debug_msg > 4) cipDebug.log('%c cip: %c initPasswordGenerator','background-color: #b78e1b','color: #333333');
	if(mcCombs.settings.usePasswordGenerator) {
		cipPassword.init();

		for(var i = 0; i < inputs.length; i++) {
			if(inputs[i] && inputs[i].attr("type") && inputs[i].attr("type").toLowerCase() == "password") {
				if ( !mpJQ(inputs[i]).hasClass('mooltipass-password-do-not-update') ) {
					cipPassword.initField(inputs[i], inputs, i);	
				}
			}
		}
	}
}

/**
 * Checks for visible input fields and triggers redetection of credential fields if the visible fields change
 */
cip.checkForNewInputs = function() {
	var fields = cipFields.getAllFields();
	var hash = cipFields.getHashForVisibleFields(fields);

	if(hash != cip.visibleInputsHash) {
		// WAIT for Mooltipass App or Moolticute to answer before sending init to fields.
		if ( cip.settings.status ) {
			//cip.initCredentialFields( true );
			cip.visibleInputsHash = hash;
			mcCombs.detectCombination();
		}
	}
}

/**
 * Submit the credentials to the server
 */
cip.doSubmit = function (pass) {
	cip.trapSubmit = false; // don't trap this submit, let it through

	cipDebug.log('doSubmit: pass field');

	// locate best submit option
	var forms = mpJQ(pass).closest('form');
	cipDebug.log("forms length: " + forms.length);
	if (forms.length > 0) {		
		cipDebug.log(mpJQ(forms[0]));
		var submits = forms.find(':submit');
		cipDebug.log("submits length: " + submits.length);
		if (submits.length > 0) {
			cipDebug.log('submitting form '+forms[0].id+' via ',submits[0]);
			cipDebug.log(mpJQ(submits[0]));
			mpJQ(submits[0]).click();
		} else {
			if(!mpJQ(forms[0]).action)
			{
				// This is wrong, if there's no action, submits to the same page. it is known... 
				cipDebug.log("Submitting an empty action form");
				mpJQ(forms[0]).submit();
			}
			else
			{
				cipDebug.log('submitting form '+forms[0].id);
				mpJQ(forms[0]).submit();		
			}
		}
	} else {
		// uh? No forms... what are we trying to submit?
		cipDebug.log('submitting default form '+mpJQ('form').id);
		cipDebug.log(mpJQ('form'));		
		mpJQ('form').submit();

		setTimeout( function() {
			// Last resource: try common btn ID and classes
			mpJQ('#sign-in, .btn-submit').click();
		},1500);
	}
}

cip.retrieveCredentialsCallback = function (credentials, dontAutoFillIn) {
	cipDebug.trace('cip.retrieveCredentialsCallback()', credentials);
	if (!credentials) return;

	if (cipFields.combinations.length > 0) {
		cip.u = _f(cipFields.combinations[0].username);
		cip.p = _f(cipFields.combinations[0].password);
	}

	if ( cip.winningCombination ) {
		// Associate the fields to the winning combination (just in case they dissapear -> Now with values!!!!)
		for (field in cip.winningCombination.fields) {
			cip.winningCombination.savedFields[field] = {
				name: cip.winningCombination.fields[field].attr( cip.winningCombination.savedFields[field].submitPropertyName?cip.winningCombination.savedFields[field].submitPropertyName:'name' ),
			}
		}
	}

	if (credentials.length > 0) {
		cip.credentials = credentials;
		cip.prepareFieldsForCredentials(!Boolean(dontAutoFillIn));


		if ( cip.winningCombination ) {
			// Associate the fields to the winning combination (just in case they dissapear -> Now with values!!!!)
			for (field in cip.winningCombination.fields) {
				cip.winningCombination.savedFields[field].value = cip.winningCombination.fields[field].val();
			}

			// Check if WinningCombination wants to run something after values have been filled
			if ( cip.winningCombination.postFunction ) cip.winningCombination.postFunction.call( this, cip.winningCombination.fields );
		}
	}
}

cip.prepareFieldsForCredentials = function(autoFillInForSingle) {
	cipDebug.log('cip.prepareFieldsForCredentials()',cipFields.combinations[0], cip.credentials);

	// only one login returned by mooltipass
	var combination = null;
	if(!cip.u && cipFields.combinations.length > 0) {
		cip.u = _f(cipFields.combinations[0].username);
	}
	if(!cip.p && cipFields.combinations.length > 0) {
		cip.p = _f(cipFields.combinations[0].password);
	}

	if (cip.u && cip.fillPasswordOnly === false) {
		
			cip.u.val('');
			cip.u.sendkeys(cip.credentials[0].Login);
			//cip.u.val(cip.credentials[0].Login);
			// Due to browser extension sand-boxing, and basic jQuery functionality, you cannot trigger a non-jQuery click event with trigger or click.
			cip.u[0].dispatchEvent(new Event('change'));
		
	}
	if (cip.p) {
			cip.p.val('');
			cip.p.sendkeys(cip.credentials[0].Password);
			// Due to browser extension sand-boxing, and basic jQuery functionality, you cannot trigger a non-jQuery click event with trigger or click.
			cip.p[0].dispatchEvent(new Event('change'));
	}
	cip.fillPasswordOnly = false;
}

cip.getFormActionUrl = function(combination) {
	var field = _f(combination.password) || _f(combination.username);

	if(field == null) {
		return null;
	}

	var form = field.closest("form");
	var action = null;

	if(form && form.length > 0) {
		action = form[0].action;
	}

	if(typeof(action) != "string" || action == "" || action.indexOf('{') > -1) {
		action = document.location.origin + document.location.pathname;
	}
	
	cipDebug.log("action url: " + action);
	return action;
}

cip.fillInCredentials = function(combination, onlyPassword, suppressWarnings) {
	var action = cip.getFormActionUrl(combination);

	var u = _f(combination.username);
	var p = _f(combination.password);

	if(combination.isNew) {
		// initialize form-submit for remembering credentials
		var fieldId = combination.password || combination.username;
		var field = _f(fieldId);
	}

	if(u) {
		cip.u = u;
	}
	if(p) {
		cip.p = p;
	}

	if(cip.url == document.location.origin && cip.submitUrl == action && cip.credentials.length > 0) {
		cip.fillIn(combination, onlyPassword, suppressWarnings);
	}
	else {
		cip.url = document.location.origin;
		cip.submitUrl = action;

		chrome.runtime.sendMessage({
			'action': 'retrieve_credentials',
			'args': [ cip.url, cip.submitUrl, true, true]
		}, function(credentials) {
			cipDebug.log('cip.fillInCredentials()');
			cip.retrieveCredentialsCallback(credentials, true);
			cip.fillIn(combination, onlyPassword, suppressWarnings);
		});
	}
}

/*
	Function called from context menu, to retrieve and show credentials
*/
cip.retrieveAndFillUserAndPassword = function() {
	this.initCredentialFields(true);
}

/*
	Called from context menu, to retrieve credentials and show password only
*/
cip.retrieveAndFillPassword = function() {
	cip.fillPasswordOnly = true;
	this.initCredentialFields(true);
}

cip.fillInFromActiveElement = function(suppressWarnings) {
	var el = document.activeElement;
	if (el.tagName.toLowerCase() != "input") {
		if(cipFields.combinations.length > 0) {
			cip.fillInCredentials(cipFields.combinations[0], false, suppressWarnings);
		}
		return;
	}

	cipFields.setUniqueId(mpJQ(el));
	var fieldId = cipFields.prepareId(mpJQ(el).attr("data-mp-id"));
	var combination = null;
	if(el.type && el.type.toLowerCase() == "password") {
		combination = cipFields.getCombination("password", fieldId);
	}
	else {
		combination = cipFields.getCombination("username", fieldId);
	}
	delete combination.loginId;

	cip.fillInCredentials(combination, false, suppressWarnings);
}

cip.fillInFromActiveElementPassOnly = function(suppressWarnings) {
	var el = document.activeElement;
	if (el.tagName.toLowerCase() != "input") {
		if(cipFields.combinations.length > 0) {
			cip.fillInCredentials(cipFields.combinations[0], false, suppressWarnings);
		}
		return;
	}

	cipFields.setUniqueId(mpJQ(el));
	var fieldId = cipFields.prepareId(mpJQ(el).attr("data-mp-id"));
	var combination = null;
	if(el.type && el.type.toLowerCase() == "password") {
		combination = cipFields.getCombination("password", fieldId);
	}
	else {
		combination = cipFields.getCombination("username", fieldId);
	}

	if(!_f(combination.password)) {
		var message = "Unable to find a password field";
		chrome.runtime.sendMessage({
			action: 'alert',
			args: [message]
		});
		return;
	}

	delete combination.loginId;

	cip.fillInCredentials(combination, true, suppressWarnings);
}

cip.setValue = function(field, value) {
	if(field.is("select")) {
		value = value.toLowerCase().trim();
		mpJQ("option", field).each(function() {
			if(mpJQ(this).text().toLowerCase().trim() == value) {
				field.val(mpJQ(this).val());
				return false;
			}
		});
	}
	else {
		field.val(value);
		field.trigger('input');
	}
}

cip.fillInStringFields = function(fields, StringFields, filledInFields) {
	cipDebug.log('cip.fillInStringFields()');
	var $filledIn = false;

	filledInFields.list = [];
	if(fields && StringFields && fields.length > 0 && StringFields.length > 0) {
		for(var i = 0; i < fields.length; i++) {
			var $sf = _fs(fields[i]);
			if($sf && StringFields[i]) {
				//$sf.val(StringFields[i].Value);
				cip.setValue($sf, StringFields[i].Value);
				filledInFields.list.push(fields[i]);
				$filledIn = true;
			}
		}
	}

	return $filledIn;
}

cip.fillIn = function(combination, onlyPassword, suppressWarnings) {
	// no credentials available
	if (cip.credentials.length == 0 && !suppressWarnings) {
		var message = "No logins found.";
		chrome.runtime.sendMessage({
			action: 'alert',
			args: [message]
		});
		return;
	}

	var uField = _f(combination.username);
	var pField = _f(combination.password);

	// exactly one pair of credentials available
	if (cip.credentials.length == 1) {
		var filledIn = false;
		if(uField && !onlyPassword) {
			uField.val(cip.credentials[0].Login);
			filledIn = true;
		}
		if(pField) {
			pField.attr("type", "password");
			pField.val(cip.credentials[0].Password);
			pField.data("unchanged", true);
			filledIn = true;
		}

		var list = {};
		if(cip.fillInStringFields(combination.fields, cip.credentials[0].StringFields, list)) {
			filledIn = true;
		}

		if(!filledIn) {
			if(!suppressWarnings) {
				var message = "Error #101\nCannot find fields to fill in.";
				chrome.runtime.sendMessage({
					action: 'alert',
					args: [message]
				});
			}
		}
	}
	// specific login id given
	else if(combination.loginId != undefined && cip.credentials[combination.loginId]) {
		var filledIn = false;
		if(uField) {
			uField.val(cip.credentials[combination.loginId].Login);
			filledIn = true;
		}

		if(pField) {
			pField.val(cip.credentials[combination.loginId].Password);
			pField.data("unchanged", true);
			filledIn = true;
		}

		var list = {};
		if(cip.fillInStringFields(combination.fields, cip.credentials[combination.loginId].StringFields, list)) {
			filledIn = true;
		}

		if(!filledIn) {
			if(!suppressWarnings) {
				var message = "Error #102\nCannot find fields to fill in.";
				chrome.runtime.sendMessage({
					action: 'alert',
					args: [message]
				});
			}
		}
	}
	// multiple credentials available
	else {
		// check if only one password for given username exists
		var countPasswords = 0;

		if(uField) {
			var valPassword = "";
			var valUsername = "";
			var valStringFields = [];
			var valQueryUsername = uField.val().toLowerCase();

			// find passwords to given username (even those with empty username)
			for (var i = 0; i < cip.credentials.length; i++) {
				if(cip.credentials[i].Login.toLowerCase() == valQueryUsername) {
					countPasswords += 1;
					valPassword = cip.credentials[i].Password;
					valUsername = cip.credentials[i].Login;
					valStringFields = cip.credentials[i].StringFields;
				}
			}

			// for the correct alert message: 0 = no logins, X > 1 = too many logins
			if(countPasswords == 0) {
				countPasswords = cip.credentials.length;
			}

			// only one mapping username found
			if(countPasswords == 1) {
				if(!onlyPassword) {
					uField.val(valUsername);
				}
				if(pField) {
					pField.val(valPassword);
					pField.data("unchanged", true);
				}

				var list = {};
			}

			// user has to select correct credentials by himself
			if(countPasswords > 1) {
				if(!suppressWarnings) {
					var message = "Error #105\nMore than one login was found in KeePass!\n" +
					"Press the chromeIPass icon for more options.";
					chrome.runtime.sendMessage({
						action: 'alert',
						args: [message]
					});
				}
			}
			else if(countPasswords < 1) {
				if(!suppressWarnings) {
					var message = "Error #103\nNo credentials for given username found.";
					chrome.runtime.sendMessage({
						action: 'alert',
						args: [message]
					});
				}
			}
		}
		else {
			if(!suppressWarnings) {
					var message = "Error #104\nMore than one login was found in KeePass!\n" +
					"Press the chromeIPass icon for more options.";
				chrome.runtime.sendMessage({
					action: 'alert',
					args: [message]
				});
			}
		}
	}
}

cip.contextMenuRememberCredentials = function() {
	var el = document.activeElement;
	if (el.tagName.toLowerCase() != "input") {
		return;
	}

	cipFields.setUniqueId(mpJQ(el));
	var fieldId = cipFields.prepareId(mpJQ(el).attr("data-mp-id"));
	var combination = null;
	if(el.type && el.type.toLowerCase() == "password") {
		combination = cipFields.getCombination("password", fieldId);
	}
	else {
		combination = cipFields.getCombination("username", fieldId);
	}

	var usernameValue = "";
	var passwordValue = "";

	var usernameField = _f(combination.username);
	var passwordField = _f(combination.password);

	if(usernameField) {
		usernameValue = usernameField.val();
	}
	if(passwordField) {
		passwordValue = passwordField.val();
	}

	if(!cip.rememberCredentials(null, usernameField, usernameValue, passwordField, passwordValue)) {
		alert("Could not detect changed credentials.");
	}
};

cip.rememberCredentials = function(event, usernameField, usernameValue, passwordField, passwordValue) {
	cipDebug.log('rememberCredentials()', arguments);
	// no password given or field cleaned by a site-running script
	// --> no password to save
	if(passwordValue == "") {
		cipDebug.log('rememberCredentials() no password value');
		return false;
	}

	if (!cip.trapSubmit) {
		cipDebug.log('rememberCredentials() trap disabled');
		cip.trapSubmit = true;
		return false;
	}


	var usernameExists = false;
	var nothingChanged = false;
	for(var i = 0; i < cip.credentials.length; i++) {
		if(cip.credentials[i].Login == usernameValue && cip.credentials[i].Password == passwordValue) {
			nothingChanged = true;
			break;
		}

		if(cip.credentials[i].Login == usernameValue) {
			usernameExists = true;
		}
	}

	if(!nothingChanged) {
		if(!usernameExists) {
			for(var i = 0; i < cip.credentials.length; i++) {
				if(cip.credentials[i].Login == usernameValue) {
					usernameExists = true;
					break;
				}
			}
		}
		var credentialsList = [];
		for(var i = 0; i < cip.credentials.length; i++) {
			credentialsList.push({
				"Login": cip.credentials[i].Login,
				"Name": cip.credentials[i].Name,
				"Uuid": cip.credentials[i].Uuid
			});
		}

		var url = mpJQ(this)[0].action;
		if(!url) {
			url = document.location.href;
			if(url.indexOf("?") > 0) {
				url = url.substring(0, url.indexOf("?"));
				if(url.length < document.location.origin.length) {
					url = document.location.origin;
				}
			}
		}
		
		cipDebug.log('rememberCredentials - sending update_notify');
		messaging({
			'action': 'update_notify',
			'args': [usernameValue, passwordValue, url, usernameExists, credentialsList]
		});

		return true;
	} else {
		cipDebug.log('rememberCredentials - nothing changed');
	}

	return false;
};

cipEvents = {};

/*
 * CipEvents is action based. Sometimes we need to either catch the normal path, or add one.
 * temporaryActions does that. just add an object with the action name and the callback.
 * temporaryActions.ACTIONNAME = function( REQ ) {}. After usage, it gets cleaned automagically
 */
cipEvents.temporaryActions = {};

/*
* Starts listening for messages, keypresses, and other events.
*/
cipEvents.startEventHandling = function() {
	/*
	* Receive a message from WS_SOCKET or MooltiPass APP
	*/
	listenerCallback = function(req, sender, callback) {
		if ( isSafari ) req = req.message;
		if (content_debug_msg > 5) cipDebug.log('%c onMessage: %c ' + req.action,'background-color: #68b5dc','color: #000000');
		else if (content_debug_msg > 4 && req.action != 'check_for_new_input_fields') cipDebug.log('%c onMessage: %c ' + req.action,'background-color: #68b5dc','color: #000000');

		// Check if we want to catch the standard path
		if ( cipEvents.temporaryActions[req.action] ) {
			cipEvents.temporaryActions[req.action]( req );
			delete cipEvents.temporaryActions[req.action];
			return;
		}

		// Safari Specific
		switch(req.action) {
			case 'response-content_script_loaded':
				mcCombs.init( function() {
					cip.settings = mcCombs.settings;
				});
				break;
			case 'response-get_settings':
				mcCombs.gotSettings( req.data );
				break;
			case 'response-retrieve_credentials':
				mcCombs.retrieveCredentialsCallback( req.data );
				break;
			case 'response-generate_password':
				var randomPassword = cipPassword.generatePasswordFromSettings( req.data );
				mpJQ(".mooltipass-password-generator").val(randomPassword);
				break;
		}

		// TODO: change IF for SWITCH
		if ('action' in req) {
			if(req.action == "fill_user_pass_with_specific_login") {
				if(cip.credentials[req.id]) {
					var combination = null;
					if (cip.u) {
						cip.u.val(cip.credentials[req.id].Login);
						combination = cipFields.getCombination("username", cip.u);
						cip.u.focus();
					}
					if (cip.p) {
						cip.p.val(cip.credentials[req.id].Password);
						combination = cipFields.getCombination("password", cip.p);
					}

					var list = {};
				}
				// wish I could clear out _logins and _u, but a subsequent
				// selection may be requested.
			}
			else if (req.action == "fill_user_pass") {
				cip.retrieveAndFillUserAndPassword();
			}
			else if (req.action == "fill_pass_only") {
				cip.retrieveAndFillPassword();
			}
			else if (req.action == "activate_password_generator") {
				cip.initPasswordGenerator(cipFields.getAllFields());
			}
			else if(req.action == "remember_credentials") {
				cip.contextMenuRememberCredentials();
			}
			else if (req.action == "choose_credential_fields") {
				cipDefine.init();
			}
			else if (req.action == "clear_credentials") {
				cipEvents.clearCredentials();
			}
			else if (req.action == "activated_tab") {
				cipEvents.triggerActivatedTab();
			}
			else if (req.action == "check_for_new_input_fields") {
				cip.checkForNewInputs();
			}
			else if (req.action == "redetect_fields") {
				chrome.runtime.sendMessage({
					"action": "get_settings",
				}, function(response) {
					cip.settings = response.data;
					cip.initCredentialFields(true);
				});
			}
			else if (req.action == "get_website_info") {
				data = {
					"url" : window.location.href,
					"html" : mpJQ("html").html()
				};
				callback(data);
			}
			else if (req.action == "post_detected") {
				mcCombs.postDetected( req.details?req.details:req.post_data );
			}
			else if (req.action == "captcha_detected") {
				cip.formHasCaptcha = true;
			}
		}
	};

	if ( isSafari ) safari.self.addEventListener("message", listenerCallback ,false);
	else {
		chrome.runtime.onMessage.removeListener( listenerCallback );
		chrome.runtime.onMessage.addListener( listenerCallback );
	}

	// Hotkeys for every page
	// ctrl + shift + p = fill only password
	// ctrl + shift + u = fill username + password
	window.addEventListener('keydown.mooltipass', function(e) {
		if (e.ctrlKey && e.shiftKey) {
			if (e.keyCode == 80) { // P
				e.preventDefault();
				cip.fillInFromActiveElementPassOnly(false);
			} else if (e.keyCode == 85) { // U
				e.preventDefault();
				cip.fillInFromActiveElement(false);
			}
		}
	}, false);

	window.addEventListener('focus.mooltipass', function() {
		chrome.runtime.sendMessage({ "action": "set_current_tab" });
	});
}

/*
* Clears cached credentials
*/
cipEvents.clearCredentials = function() {
	cip.credentials = [];
}

/*
* Triggered when a tab becomes active
*/
cipEvents.triggerActivatedTab = function() {
	mcCombs.init();
}

/* Delay execution a bit if JQuery isn't ready yet ~happens 1/1000, but still worth it~ */
while( !mpJQ ) {};

cipEvents.startEventHandling();

var mcCombs = new mcCombinations();
mcCombs.settings.debugLevel = content_debug_msg;

messaging( {'action': 'content_script_loaded' } );

var mpDialog = {
	shown: false,
	dialog: false,
	$pwField: false,
	inputs: false,
	created: false,
	toggle: function( event ) {
		if ( this.shown ) this.hide();
		else this.show( event );
	},
	precreate: function( inputs, $pwField ) {
		this.inputs = inputs;
		this.$pwField = $pwField;
	},
	create: function( inputs, $pwField ) {
		var overlay = mpJQ('<div>').addClass('mp-genpw-overlay');
		var dom = this.domDialog( $pwField );
		this.dialog = mpJQ("<div>").addClass('mp-genpw-dialog').append( overlay, dom );
		mpJQ("body").append( this.dialog );
	},
	showLoginArea: function() {
		this.dialog.find('.mp-first').removeClass('mp-first');
		this.dialog.find('.login-area').addClass('mp-first').show();
	},
	show: function( event ) {
		if ( !this.created ) {
			this.create( this.inputs, this.$pwField );
		}

		var posX = event.clientX + 20;
		var posY = event.clientY - 20;

		this.dialog.find('.mp-genpw-overlay').on('click.mooltipass', function( e ) {
			if ( mpJQ(e.target).hasClass('mp-genpw-overlay') ) this.hide();
		}.bind(this));

		this.dialog.find('.mooltipass-box').css({ top: posY, left: posX });
		this.dialog.show( function() {
			var mpBox = this.dialog.find('.mooltipass-box');

			// Generate password if empty
			if ( mpBox.find('.mooltipass-password-generator').val() === '' ) {
				cipPassword.generatePassword();
			}

			// Move dialog if exceeding right area
			if ( posX + mpBox.outerWidth() > window.innerWidth ) {
				mpBox.css({ left: posX - mpBox.outerWidth() - 50 + 'px' });
				mpBox.addClass('inverted-triangle');
			}

			// Move dialog if exceeding bottom
			var exceedingBottom = ( posY + mpBox.innerHeight() ) - mpJQ( window.top ).innerHeight();
			if ( exceedingBottom > 0 ) mpBox.css({ top: mpBox.position().top - exceedingBottom + 'px' });

			// Move Arrows to the right place
			if ( exceedingBottom > 0 ) mpBox.find('.mp-triangle-in, .mp-triangle-out').css({ top: 8 + exceedingBottom + 'px' });

		}.bind(this));
		this.shown = true;
	},
	hide: function() {
		this.dialog.hide();
		this.dialog.find('.mp-genpw-overlay').off('click.mooltipass');
		this.shown = false;
	},
	domDialog: function( $pwField ) {
		// DomDialog creates the dialog upon request (No dialog is created if the user doesn't click on the blue-key)
		var output = document.createElement('div');
		output.className = 'mooltipass-box';

		// Triangle
		mpJQ('<div>').addClass('mp-triangle-out').appendTo( output );
		mpJQ('<div>').addClass('mp-triangle-in').appendTo( output );

		// Current Login information + field
		var oTitle = mpJQ('<div>').addClass('mp-title').text('Current Login');
		var oText = mpJQ('<p>').text('If not correct, modify it below');
		var oInput = mpJQ('<input>').prop('type','text').addClass('mooltipass-hash-ignore').prop('id','mooltipass-username');
		mpJQ('<div>').addClass('login-area').addClass('mp-area').append(oTitle, oText, oInput).appendTo( output );

		// Check if we retreived credentials
		if ( mcCombs.credentialsCache && mcCombs.credentialsCache.length && mcCombs.credentialsCache[0].Login ) oInput.val( mcCombs.credentialsCache[0].Login );
		
		// Credentials Actions
		oTitle = mpJQ('<div>').addClass('mp-title').text('Credential Storage');
		oText = mpJQ('<p>').text('You can store your entered credentials in the Mooltipass device to securely store and easily access them.');
		var oButton = mpJQ('<button>').prop('id','mooltipass-store-credentials').addClass('mooltipass-button').text('Store or update current credentials');
		var oLink = mpJQ('<a>').addClass('mooltipass-select-custom').text('Select custom credential fields');
		var oEncloser = mpJQ('<div>').addClass('mp-encloser').append(oButton, oLink);
		mpJQ('<div>').addClass('mp-area').addClass('mp-first').append(oTitle, oText, oEncloser).appendTo( output );

		// Credential Actions Event Listeners
		oButton.hover( function() {
			if ( cipDefine.selection.password ) $pwField = cipFields.getPasswordField( cipDefine.selection.password , true);
			$userField = cipFields.getUsernameField( $pwField.data("mp-id") );
			
			if ( $userField ) $userField.addClass("mp-hover-username");
			if ( $pwField ) $pwField.addClass("mp-hover-password");
		}, function() {
			mpJQ(".mp-hover-username").removeClass("mp-hover-username");
			mpJQ(".mp-hover-password").removeClass("mp-hover-password");
		}).click( function( e ) {
			e.preventDefault();

			var url = (document.URL.split("://")[1]).split("/")[0];
			if ( cipDefine.selection.password ) $pwField = cipFields.getPasswordField( cipDefine.selection.password , true);
			$userField = cipFields.getUsernameField( $pwField.data("mp-id") );

			if ( mpJQ('#mooltipass-username').val() ) {
				var username = mpJQ('#mooltipass-username').val();
			} else if ( $userField.length ) {
				var username = $userField.val();	
			} else {
				var username = '';
			}
			var password = $pwField.val();

			mpJQ(".mp-hover-username").removeClass("mp-hover-username");
			mpJQ(".mp-hover-password").removeClass("mp-hover-password");

			mpJQ("#mp-update-credentials-wrap").html('<p style="font-size: 12px !important;">Follow the instructions on your Mooltipass device to store the credentials.</p>');

			if(cip.rememberCredentials(null, $userField, username, $pwField, password)) {
				mpJQ("#mp-update-credentials-wrap").html('<p style="font-size: 12px !important;">Credentials are added to your Mooltipass KeyCard</p>');
			}
		});

		oLink.on('click.mooltipass', function( e ) {
			e.preventDefault();
			cipDefine.init();
		});

		// Password Generator
		oTitle = mpJQ('<div>').addClass('mp-title').text('Password Generator');
		oInput = mpJQ('<input>').prop('type','text').addClass('mooltipass-password-generator').addClass('mooltipass-hash-ignore');
		oLink = mpJQ('<a>').addClass('mooltipass-new-password').text('Re-generate');
		oButton = mpJQ('<button>').addClass('mp-bottom-button').text('Copy to all password fields');
		var oButtonB = mpJQ('<button>').addClass('mp-bottom-button').text('Copy to Clipboard');
		mpJQ('<div>').addClass('mp-area').append(oTitle, oInput, oLink, oButton, oButtonB).appendTo( output );

		// Password Generator Event Listeners
		oLink.on('click.mooltipass', function( e ) {
			e.preventDefault();
			cipPassword.generatePassword();
		});

		oButton.on('click.mooltipass', function( e ) {
			e.preventDefault();
			e.stopPropagation();

			var password = mpJQ(".mooltipass-password-generator").val();
			var passwordFields = mpJQ("input[type='password']:not('.mooltipass-password-do-not-update')");

			passwordFields.val('').sendkeys( password );
			passwordFields.trigger('change');
		});

		oButtonB.on('click.mooltipass', function( e ) {
			e.preventDefault();
			e.stopPropagation();

			var copyInput = document.querySelector('.mooltipass-password-generator');
			copyInput.select();

			try {
				var successful = document.execCommand('copy');
				var msg = successful ? 'successful' : 'unsuccessful';
				cipDebug.info('Copying password to clipboard was ' + msg);
				mpJQ(this).addClass("mooltipass-button-success");
			} catch(err) {
				cipDebug.warn('Unable to copy password to clipboard');
				mpJQ(this).addClass("mooltipass-button-error");
			}
		});

		return output;
	}
}