/*
 * Password dialog.
 * 
 * @param login {String}
 * @param offsetLeft {Number}
 * @param offsetTop {Number}
 * @param isPasswordOnly {Boolean}
 * @param windowWidth {Number}
 * @param windowHeight {Number}
 * @param hideCustomCredentials {Boolean}
 */
 
window.data = JSON.parse(decodeURIComponent(window.location.search.slice(1)))

$(function() {
	startEventHandling()
	onShow(data)
	$('#mooltipass-username').val(data.login);
	
	// Credential Actions Event Listeners
	$('#mooltipass-store-credentials').hover( function() {
		messaging({
			action: 'create_action',
			args: [{
				action: 'password_dialog_highlight_fields',
				args: {
					highlight: true
				}
			}]
		});
	}, function() {
		messaging({
			action: 'create_action',
			args: [{
				action: 'password_dialog_highlight_fields',
				args: {
					highlight: false
				}
			}]
		});
	}).click( function(e) {
		e.preventDefault();
		
		messaging({
			action: 'create_action',
			args: [{
				action: 'password_dialog_store_credentials',
				args: {
					username: $('#mooltipass-username').val()
				}
			}]
		});
	});

	$('.mooltipass-select-custom').on('click', function( e ) {
		e.preventDefault();
		messaging({
			action: 'create_action',
			args: [{
				action: 'password_dialog_custom_credentials_selection',
				args: {}
			}]
		});
	});
	
	// Password Generator Event Listeners
	$('.mooltipass-new-password').on('click', function( e ) {
		e.preventDefault();
		messaging({
			action: 'create_action',
			args: [{
				action: 'password_dialog_generate_password',
				args: {}
			}]
		});
	});

	$('#copy-to-fields').on('click', function( e ) {
		e.preventDefault();
		e.stopPropagation();

		var password = $(".mooltipass-password-generator").val();
		messaging({
			action: 'create_action',
			args: [{
				action: 'password_dialog_copy_password_to_fields',
				args: {
					password: password
				}
			}]
		});
	});

	$('#copy-to-clipboard').on('click', function( e ) {
		e.preventDefault();
		e.stopPropagation();

		var copyInput = document.querySelector('.mooltipass-password-generator');
		copyInput.select();
		document.execCommand('copy');
	});
	
	$('.mp-genpw-overlay').on('click', function( e ) {
		if ( $(e.target).hasClass('mp-genpw-overlay') ) {
			messaging({
				action: 'create_action',
				args: [{
					action: 'password_dialog_hide',
					args: {}
				}]
			});
		}
	})
	
	function onShow(data) {
		var mpBox = $('.mooltipass-box');
		
		mpBox.css({ top: data.offsetTop, left: data.offsetLeft });
		mpBox.find('.mp-triangle-in, .mp-triangle-out').css('top', '')

		// Generate password if empty
		if ( mpBox.find('.mooltipass-password-generator').val() === '' ) {
			messaging({
				action: 'create_action',
				args: [{
					action: 'password_dialog_generate_password',
					args: {}
				}]
			});
		}

		// Move dialog if exceeding right area
		if (data.offsetLeft + mpBox.outerWidth() > data.windowWidth) {
			mpBox.css({ left: Math.max(0, data.offsetLeft - mpBox.outerWidth() - 50) + 'px' });
			mpBox.addClass('inverted-triangle');
		}

		// Move dialog if exceeding bottom
		var exceedingBottom = data.offsetTop + mpBox.innerHeight() - data.windowHeight
		if ( exceedingBottom > 0 ) mpBox.css({ top: Math.max(0, mpBox.position().top - exceedingBottom) + 'px' });

		// Move Arrows to the right place
		if ( exceedingBottom > 0 ) mpBox.find('.mp-triangle-in, .mp-triangle-out').css({ top: 8 + exceedingBottom + 'px' });
    
		if (data.hideCustomCredentials) {
			mpBox.find('.mooltipass-select-custom').hide()
		}
		
    if (data.isPasswordOnly) {
  		mpBox.find('.mp-first').removeClass('mp-first');
  		mpBox.find('.login-area').addClass('mp-first').show();
    }
		
		// Overflow password dialog.
		if (mpBox.innerHeight() >= data.windowHeight) {
			mpBox.css('overflow', 'auto')
			mpBox.css('top', '0')
			mpBox.css('max-height', '100vh')
		}
	}
	
	function startEventHandling() {
		/*
		* Receive a message from WS_SOCKET or MooltiPass APP
		*/
		listenerCallback = function(req, sender, callback) {
			if ( isSafari ) req = req.message;

			if ('action' in req) {
				switch (req.action) {
					case 'password_dialog_show':
						onShow(req.args);
						break;
					case 'password_dialog_generated_password':
						$('.mooltipass-password-generator').val(req.args.password);
						break;
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
