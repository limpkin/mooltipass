/*
 * Custom credentials selection overlay.
 * 
 * Emits following actions:
 * - ***
 *
 * Receives following actions:
 * - 
 *
 * @param *** {***}
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

var cipDefine = {}
window.mpJQ = $;

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

	// cipFields.getAllFields();
	// cipFields.prepareVisibleFieldsWithID("select");

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
			if(!data.settings["defined-credential-fields"]) {
				data.settings["defined-credential-fields"] = {};
			}

			if(cipDefine.selection.username) {
				// cipDefine.selection.username = cipFields.prepareId(cipDefine.selection.username);
			}

			var passwordId = mpJQ("div#mp-bt-cipDefine-fields").data("password");
			if(cipDefine.selection.password) {
				// cipDefine.selection.password = cipFields.prepareId(cipDefine.selection.password);
			}

			var fieldIds = [];
			var fieldKeys = Object.keys(cipDefine.selection.fields);
			for(var i = 0; i < fieldKeys.length; i++) {
				// fieldIds.push(cipFields.prepareId(fieldKeys[i]));
			}

			data.settings["defined-credential-fields"][document.location.origin] = {
				"username": cipDefine.selection.username,
				"password": cipDefine.selection.password,
				"fields": fieldIds
			};

			messaging({
				action: 'save_settings',
				args: [data.settings]
			});

			mpJQ("#mp-bt-btn-dismiss").click();
		})
		.hide();

	$h1.append($btnDismiss);

	$description.append($h1);
	$description.append($btnDismiss);
	
	$buttonWrap.append($btnAgain);

	$buttonWrap.append($btnConfirm);

	if(data.settings["defined-credential-fields"] && data.settings["defined-credential-fields"][document.location.origin]) {
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
	// cipDefine.markFields($chooser, cipFields.inputQueryPattern);
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
	// cipDefine.markFields($chooser, cipFields.inputQueryPattern + ", select");
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

// Unify messaging method - And eliminate callbacks (a message is replied with another message instead)
function messaging( message ) {
	if (content_debug_msg > 4) cipDebug.log('%c Sending message to background:','background-color: #0000FF; color: #FFF; padding: 5px; ', message);
	if ( isSafari ) safari.self.tab.dispatchMessage("messageFromContent", message);
	else chrome.runtime.sendMessage( message );
};

var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;
var isSafari = typeof(safari) == 'object'?true:false;
var content_debug_msg = ((window.chrome || isFirefox) && chrome.runtime && !('update_url' in chrome.runtime.getManifest()))? 55 : false;;

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
