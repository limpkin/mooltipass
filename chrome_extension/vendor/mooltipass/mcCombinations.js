/*
 * Extendable objects for special cases
 *
 */
var extendedCombinations = {
	skype: function( forms ) {
		//console.log('skype combination');
		if ( mcCombs.getAllForms() == 0 ) return;
		for( form in forms ) {
			var currentForm = forms[ form ];
			if ( currentForm.element ) { // Skip noform form
				currentForm.combination = {
					special: true,
					fields: {
						username: '',
						password: ''
					},
					savedFields: {
						username: '',
						password: ''
					},
					autoSubmit: false
				}

				if ( mpJQ('input[type=password]:visible').length > 0 ) { // Step 1: Email
					currentForm.combination.fields.password = mpJQ('input[type=password]');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('input[name=loginfmt]:visible').length > 0 ) { // Step 1: Email
					currentForm.combination.fields.username = mpJQ('input[name=loginfmt]');
					currentForm.combination.autoSubmit = true;
				}
			}
		}
	},
	autodesk: function( forms ) {
		//console.log('autodesk combination');
		if ( mcCombs.getAllForms() == 0 ) return;
		for( form in forms ) {
			var currentForm = forms[ form ];
			if ( currentForm.element ) { // Skip noform form
				currentForm.combination = {
					special: true,
					fields: {
						username: '',
						password: ''
					},
					savedFields: {
						username: '',
						password: ''
					},
					autoSubmit: false
				}

				if ( mpJQ('input[type=password]:visible').length > 0 ) { // Step 1: Email
					currentForm.combination.fields.password = mpJQ('input[type=password]');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('input[type=text]:visible').length > 0 ) { // Step 1: Email
					currentForm.combination.fields.username = mpJQ('input[type=text]');
					currentForm.combination.autoSubmit = true;
				}
			}
		}
	},
	evernote: function( forms ) {
		if ( mcCombs.getAllForms() == 0 ) return;
		for( form in forms ) {
			var currentForm = forms[ form ];
			if ( currentForm.element ) { // Skip noform form
				currentForm.combination = {
					special: true,
					fields: {
						username: '',
						password: ''
					},
					savedFields: {
						username: '',
						password: ''
					},
					autoSubmit: false
				}

				if ( mpJQ('input[type=text]:visible').length > 0 ) {
					currentForm.combination.fields.username = mpJQ('input[type=text]');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('input[type=password]:visible').length > 0 ) {
					currentForm.combination.fields.password = mpJQ('input[type=password]');
					currentForm.combination.autoSubmit = true;
				}

				// Skip special combination for password restore form.
				if ( mpJQ('input[type=password]:visible').length > 1 ) return 'skip'
			}
		}
	},
	pwc: function( forms ) {
		if ( mcCombs.getAllForms() == 0 ) return;
		for( form in forms ) {
			var currentForm = forms[ form ];
			if ( currentForm.element ) { // Skip noform form
				currentForm.combination = {
					special: true,
					fields: {
						username: '',
						password: ''
					},
					savedFields: {
						username: '',
						password: ''
					},
					autoSubmit: false
				}

				if ( mpJQ('input[type=text]:visible').length > 0 ) {
					currentForm.combination.fields.username = mpJQ('input[type=text]');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('input[type=password]:visible').length > 0 ) {
					currentForm.combination.fields.password = mpJQ('input[type=password]');
					currentForm.combination.autoSubmit = true;
				}
			}
		}
	},
	google: function( forms ) {
		if ( mcCombs.getAllForms() == 0 ) return;
		for( form in forms ) {
			var currentForm = forms[ form ];
			if ( currentForm.element ) { // Skip noform form
				currentForm.combination = {
					special: true,
					fields: {
						username: '',
						password: ''
					},
					savedFields: {
						username: '',
						password: ''
					},
					autoSubmit: false
				}

				if ( mpJQ('input[type=password]:visible').length > 0 ) { // Step 1: Email
					currentForm.combination.fields.password = mpJQ('input[type=password]');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('input[type=email]:visible').length > 0 ) { // Step 1: Email
					currentForm.combination.fields.username = mpJQ('input[type=email]');
					currentForm.combination.autoSubmit = true;
				}
			}
		}
	},
	soundcloud: function( forms ) {
		if ( mcCombs.getAllForms() == 0 ) return;
		for( form in forms ) {
			var currentForm = forms[ form ];
			if ( currentForm.element ) { // Skip noform form
				currentForm.combination = {
					special: true,
					fields: {
						username: '',
						password: ''
					},
					savedFields: {
						username: '',
						password: ''
					},
					autoSubmit: false
				}

				if ( mpJQ('input[name=password]:visible').length > 0 ) {
					currentForm.combination.fields.password = mpJQ('input[name=password]');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('input[name=username]:visible').length > 0 ) {
					currentForm.combination.fields.username = mpJQ('input[name=username]');
					currentForm.combination.autoSubmit = true;
				}
			}
		}
	},
	citi: function( forms ) {
		if ( mcCombs.getAllForms() == 0 ) return;
		for( form in forms ) {
			var currentForm = forms[ form ];
			if ( currentForm.element ) { // Skip noform form
				currentForm.combination = {
					special: true,
					fields: {
						username: '',
						password: ''
					},
					savedFields: {
						username: '',
						password: ''
					},
					autoSubmit: false
				}

				if ( mpJQ('input[id=username]:visible').length > 0 ) {
					currentForm.combination.fields.username = mpJQ('input[id=username]');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('input[id=password]:visible').length > 0 ) {
					currentForm.combination.fields.password = mpJQ('input[id=password]');
					currentForm.combination.autoSubmit = true;
				}
			}
		}
	},
	upwork: function( forms ) {
		if ( mcCombs.getAllForms() == 0 ) return;
		for( form in forms ) {
			var currentForm = forms[ form ];
			if ( currentForm.element ) { // Skip noform form
				currentForm.combination = {
					special: true,
					fields: {
						username: '',
						password: ''
					},
					savedFields: {
						username: '',
						password: ''
					},
					autoSubmit: false
				}

				if ( mpJQ('input[id=login_username]:visible').length > 0 ) {
					currentForm.combination.fields.username = mpJQ('input[id=login_username]');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('input[id=login_password]:visible').length > 0 ) {
					currentForm.combination.fields.password = mpJQ('input[id=login_password]');
					currentForm.combination.autoSubmit = true;
				}
			}
		}
	},
	hsbc: function( forms ) {
		if ( mcCombs.getAllForms() == 0 ) return;
		for( form in forms ) {
			var currentForm = forms[ form ];
			if ( currentForm.element ) { // Skip noform form
				currentForm.combination = {
					special: true,
					fields: {
						username: '',
						password: ''
					},
					savedFields: {
						username: '',
						password: ''
					},
					autoSubmit: false
				}
				
				var usernameSelector = 'input[name=u_UserID]:visible, input[name=userid]:visible',
						passwordSelector = 'input[type=password]:visible'

				if ( mpJQ(usernameSelector).length > 0 ) {
					currentForm.combination.fields.username = mpJQ(usernameSelector);
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ(passwordSelector).length > 0 ) {
					currentForm.combination.fields.password = mpJQ(passwordSelector);
					currentForm.combination.autoSubmit = true;
				}
			}
		}
	},
	yahoo: function( forms ) {
		if ( mcCombs.getAllForms() == 0 ) return;
		for( form in forms ) {
			var currentForm = forms[ form ];
			if ( currentForm.element ) { // Skip noform form
				currentForm.combination = {
					special: true,
					fields: {
						username: '',
						password: ''
					},
					savedFields: {
						username: '',
						password: ''
					},
					autoSubmit: false
				}

				if ( mpJQ('#login-passwd:visible').length > 0 ) { // Step 1: Email
					currentForm.combination.fields.password = mpJQ('#login-passwd');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('#login-username:visible').length > 0 ) { // Step 1: Email
					currentForm.combination.fields.username = mpJQ('#login-username');
					currentForm.combination.autoSubmit = true;
				}
			}
		}
	}
};

var extendedPost = {
	'techmania.ch': function( details ) {
		if ( details.requestParams && details.requestParams.login ) {
			details.usernameValue = details.requestParams.login;
			details.passwordValue = details.requestParams.password;
		}
		return details;
	},
	'seeedstudio.com': function( details ) {
		if ( details.email && details.password ) {
			details.usernameValue = details.email;
			details.passwordValue = details.password;
		}
		return details;
	}
}

/*
/ Form Detection by combinations.
/ Searches the DOM for a predefined set of combinations and retrieves credentials or prepares everything to be saved
/ This will, eventually, replace cip.* 
*/

/* Debug Levels:

0: No debug info
1: Important Messages
2: Informational Messages
3: Available
4: Verbose Debug messages
5: Show every function call
6: Special case, show "check_for_new_input_fields" as well

*/

function mcCombinations() {}

mcCombinations.prototype = ( function() {
	return {
		constructor:mcCombinations,
		inputQueryPattern: "input[type='text']:not([class='search']), input[type='email'], input[type='login'], input[type='password']:not(.notinview):not([tabindex='-1']), input[type='tel'], input[type='number'], input:not([type]), input[name='username']",
		forms: {
			noform: { fields: [] }
		},
		usernameFieldId: null,
		passwordFieldId: null,
		settings: {
			debugLevel: 0,
			postDetectionFeature: true
		}
	};
})();

/*
* Init the mcCombinations procedure.
* This should be called just once per body/frame!
*/
mcCombinations.prototype.init = function( callback ) {
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c Init','background-color: #c3c6b4','color: #333333');

	this.callback = callback;
	messaging( { "action": "get_settings" } );
};

/*
 * Parse settings received from background script
*/
mcCombinations.prototype.gotSettings = function( response ) {
	if (this.settings.debugLevel > 3) cipDebug.log('%c mcCombinations: %c Got settings', 'background-color: #c3c6b4','color: #FF0000',response);
	if ( typeof(response) !== 'undefined') {
		mpJQ.extend(this.settings, response.data);
		if (this.settings.debugLevel > 0) cipDebug.warn('mcCombinations: Status is: ', this.settings.status);

		if ( this.callback ) this.callback.apply( this, response );
		this.detectCombination();
	} else {
		if (this.settings.debugLevel > 0) cipDebug.warn('Get settings returned empty!', runtime.lastError);
	}
}

/*
* Array containing all the possible combinations we support
*/
mcCombinations.prototype.possibleCombinations = [
	{
		combinationId: 'skypeTwoPageAuth',
		combinationName: 'Skype Two Page Login Procedure',
		requiredUrl: 'login.live.com',
		callback: extendedCombinations.skype
	},
	{
		combinationId: 'autodeskTwoPageAuth',
		combinationName: 'Autodesk Two Page Login Procedure',
		requiredUrl: 'accounts.autodesk.com',
		callback: extendedCombinations.autodesk
		
	},
	{
		combinationId: 'googleTwoPageAuth',
		combinationName: 'Google Two Page Login Procedure',
		requiredUrl: 'accounts.google.com',
		callback: extendedCombinations.google
	},
	{
		combinationId: 'soundcloudTwoPageAuth',
		combinationName: 'SoundCloud Two Page Login Procedure',
		requiredUrl: 'soundcloud.com',
		callback: extendedCombinations.soundcloud
	},
	{
		combinationId: 'evernoteTwoPageAuth',
		combinationName: 'Evernote Two Page Login Procedure',
		requiredUrl: 'www.evernote.com',
		callback: extendedCombinations.evernote
	},
	{
		combinationId: 'pwcTwoPageAuth',
		combinationName: 'PWC Two Page Login Procedure',
		requiredUrl: 'mymobility.pwc.com',
		callback: extendedCombinations.pwc
	},
	{
		combinationId: 'hsbcAuth',
		combinationName: 'HSBC Login Procedure',
		requiredUrl: 'hsbc.com',
		callback: extendedCombinations.hsbc
	},
	{
		combinationId: 'citiAuth',
		combinationName: 'Citi Login Procedure',
		requiredUrl: 'online.citi.com',
		callback: extendedCombinations.citi
	},
	{
		combinationId: 'upworkAuth',
		combinationName: 'Upwork Login Procedure',
		requiredUrl: 'upwork.com',
		callback: extendedCombinations.upwork
	},
	{
		combinationId: 'googleTwoPageAuth',
		combinationName: 'Google Two Page Login Procedure',
		requiredUrl: 'login.yahoo.com',
		callback: extendedCombinations.yahoo
	},
	{
		// Seen at icloud.com, seems to comform to an Apple's proprietary identity management system (IDMS)
		combinationId: 'canFieldBased',
		combinationName: 'Login Form with can-field properties',
		requiredFields: [
			{
				selector: 'input[can-field=accountName]',
				submitPropertyName: 'can-field',
				mapsTo: 'username'
			},
			{
				selector: 'input[can-field=password]',
				submitPropertyName: 'can-field',
				mapsTo: 'password'	
			}
		],
		scorePerMatch: 50,
		score: 0,
		maxfields: 3,
		extraFunction: function( fields ) {
			setTimeout( function() {
				mpJQ('#sign-in, .btn-submit').click();
			},500);
		}
	},
	{
		combinationId: 'loginform001',
		combinationName: 'Simple Login Form with Email',
		requiredFields: [
			{
				selector: 'input[type=email]',
				submitPropertyName: 'name',
				mapsTo: 'username'
			},
			{
				selector: 'input[type=password]',
				submitPropertyName: 'name',
				mapsTo: 'password'
			},
		],
		scorePerMatch: 50,
		score: 0,
		autoSubmit: true,
		maxfields: 2,
		extraFunction: function( fields ) {
			/* This function will be called if the combination is found, in this case: enable any disabled field in the form */
			if ( fields[0] && fields[0].closest ) fields[0].closest('form').find('input:disabled').prop('disabled',false);
		}
	},
	{
		combinationId: 'loginform002',
		combinationName: 'Simple Login Form with Text',
		requiredFields: [
			{
				selector: 'input[type=text],input[type=login],input[type=tel],input:not([type]),input[name=username]',
				mapsTo: 'username'
			},
			{
				selector: 'input[type=password],input[realtype=password]',
				mapsTo: 'password'
			},
		],
		scorePerMatch: 50,
		score: 0,
		autoSubmit: true,
		maxfields: 2,
		extraFunction: function( fields ) {
			/* This function will be called if the combination is found, in this case: enable any disabled field in the form */
			if ( fields[0] && fields[0].closest ) fields[0].closest('form').find('input:disabled').prop('disabled',false);
		}
	},
	{
		combinationId: 'registerformsimple',
		combinationName: 'Simple Registration (1 username, 2 passwords)',
		mustFollowOrder: true,
		requiredFields: [
			{
				selector: 'input[type=text],input[type=email],input:not([type])',
				mapsTo: 'username'
			},
			{
				selector: 'input[type=password]',
				mapsTo: 'password'
			},
			{
				selector: 'input[type=password]:visible',
			}
		],
		scorePerMatch: 33,
		score: 1,
		autoSubmit: true,
		maxfields: 3,
		extraFunction: function( fields ) {
			/* This function will be called if the combination is found, in this case: enable any disabled field in the form */
			if ( fields[0] && fields[0].closest ) fields[0].closest('form').find('input:disabled').prop('disabled',false);
		}
	},
	{
		combinationId: 'loginform004',
		combinationName: 'Login Form mixed with Registration Form (ie: showroomprive.com)',
		requiredFields: [
			{
				selector: 'input[type=email], input[type=text]:not([name=fakeusernameremembered])',
				mapsTo: 'username'
			},
			{
				selector: 'input[type=password]:not([name=fakepasswordremembered]), input[type=text].DocControlPassword',
				mapsTo: 'password'
			},
		],
		scorePerMatch: 50,
		score: 0,
		autoSubmit: false,
		enterFromPassword: true,
		maxfields: 12,
		extraFunction: function( fields ) {
			if (!this.fields.username) {
				this.fields.username = cipFields.getUsernameField( fields.password.prop('id') );
			}
		}
	},
	{
		combinationId: 'loginform005',
		combinationName: 'Login Form with Text and Search field',
		requiredFields: [
			{
				selector: 'input[type=password]',
				mapsTo: 'password'
			},
			{
				selector: 'input[type=text]:not([name*=Search]),input:not([type])',
				mapsTo: 'username'
			},
			{
				selector: 'input[type=text][name*=Search]',
				mapsTo: 'search'
			}
		],
		scorePerMatch: 33,
		score: 1,
		autoSubmit: true,
		maxfields: 3,
		extraFunction: function( fields ) {
			/* This function will be called if the combination is found, in this case: enable any disabled field in the form */
			if ( fields[0] && fields[0].closest ) fields[0].closest('form').find('input:disabled').prop('disabled',false);
		}
	},
	{
		combinationId: 'loginform003',
		combinationName: 'Login Form with Text and 3 fields instead of 2',
		requiredFields: [
			{
				selector: 'input[type=password]',
				mapsTo: 'password'
			},
			{
				selector: 'input[type=text],input:not([type])',
				mapsTo: 'username',
				closeToPrevious: true,
			}
		],
		scorePerMatch: 50,
		score: 0,
		autoSubmit: false,
		maxfields: 3,
		extraFunction: function( fields ) {
			/* This function will be called if the combination is found, in this case: enable any disabled field in the form */
			if ( fields[0] && fields[0].closest ) fields[0].closest('form').find('input:disabled').prop('disabled',false);
		}
	},
	{
		combinationId: 'passwordreset002',
		combinationName: 'Password Reset with Confirmation',
		combinationDescription: 'Searches for 3 password fields in the form, retrieves password for the first one, stores the value from the second',
		requiredFields: [
			{
				selector: 'input[type=password]:visible',
				mapsTo: 'password'
			},
			{
				selector: 'input[type=password]:visible',
			},
			{
				selector: 'input[type=password]:visible'
			},
		],
		isPasswordOnly: true,
		scorePerMatch: 33,
		score: 1,
		autoSubmit: false,
		usePasswordGenerator: true,
		preExtraFunction: function() { // Pre-extra is run before retrieving credentials
			mpJQ(this.fields.password[0]).addClass('mooltipass-password-do-not-update');
		},
		extraFunction: function( fields ) {
			// We need LOGIN information. Try to retrieve credentials from cache.
			cipEvents.temporaryActions['response-cache_retrieve'] = function( response ) {
				var r = response.data;

				if ( r.Login ) { // We got a login!
					this.savedFields.username = r.Login;
					this.fields.username = r.Login;
				} else { // No login information. Just ask the user.
					var currentForm = this.fields.password.parents('form');
					var url = document.location.origin;
					var submitUrl = currentForm?mcCombs.getFormActionUrl( currentForm ):url;

					cipEvents.temporaryActions['response-retrieve_credentials'] = function( response ) {
						var r = response.data;
						if ( r[0] && r[0].Login ) {
							this.savedFields.username = r[0].Login;
							this.fields.username = r[0].Login;
						}
					}.bind(this);
					messaging({ 'action': 'retrieve_credentials', 'args': [ url, submitUrl, true, true] });	
				}
			}.bind(this);
			messaging({'action': 'cache_retrieve' });

			
			// We also change the password field for the next one (as we retrieve in the first field, but store the value from the second!)
			this.fields.password = this.fields.password.parents('form').find("input[type='password']:not('.mooltipass-password-do-not-update')");
	
			// for( field in fields ) {
			// 	fields[field].addClass('mooltipass-password-do-not-update');	
			// }

			// Use password generated for both new password fields
			// Disabled feature, uncomment to have mooltipass pre-fill your new password for you
			// mpJQ('input[type=password]:visible:not(".mooltipass-password-do-not-update")').val('').sendkeys( mpJQ('#mooltipass-password-generator').val() );
		}
	},
	{
		combinationId: 'passwordreset001',
		usePasswordGenerator: true,
		combinationName: 'Password Reset without Confirmation',
		requiredFields: [
			{
				selector: 'input[type=password]:visible'
			},
			{
				selector: 'input[type=password]:visible',
				mapsTo: 'password'
			},
		],
		scorePerMatch: 50,
		score: 0,
		maxfields: 2,
		isPasswordOnly: true,
		autoSubmit: false,
		extraFunction: function( fields ) {
			// We need LOGIN information. Try to retrieve credentials from cache.
			cipEvents.temporaryActions['response-cache_retrieve'] = function( response ) {
				var r = response.data;
				
				if ( r.Login ) { // We got a login!
					this.savedFields.username = r.Login;
					this.fields.username = r.Login;
				} else { // No login information. Just ask the user.
					var currentForm = this.fields.password.parents('form');
					var url = document.location.origin;
					var submitUrl = currentForm?mcCombs.getFormActionUrl( currentForm ):url;

					cipEvents.temporaryActions['response-retrieve_credentials'] = function( response ) {
						var r = response.data;
						if ( r[0] && r[0].Login ) {
							this.savedFields.username = r[0].Login;
							this.fields.username = r[0].Login;
						}
					}.bind(this);
					messaging({ 'action': 'retrieve_credentials', 'args': [ url, submitUrl, true, true] });	
				}
			}.bind(this);
			messaging({'action': 'cache_retrieve' });
		}
	},
	{
		usePasswordGenerator: true,
		isPasswordOnly: true,
		combinationId: 'enterpassword',
		combinationName: 'A password fill-in form',
		requiredFields: [
			{
				selector: 'input[type=password]',
				mapsTo: 'password'
			}
		],
		scorePerMatch: 100,
		maxfields: 1,
		score: 0,
		autoSubmit: false
	}
];

/*
* The main function for mcCombinations.
* Searches for input fields in the DOM and matches them against a combination of our own
*/
mcCombinations.prototype.detectCombination = function() {
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c detectCombination','background-color: #c3c6b4','color: #333333');
	var numberOfFields = this.getAllForms();
	if (this.settings.debugLevel > 1) cipDebug.log('detectCombination has found ' + numberOfFields + ' fields.');
	
	// Init the password generator as always.
	if ( mcCombs.settings.usePasswordGenerator ) {
		var inputs = cipFields.getAllFields();
		cip.initPasswordGenerator(inputs);
	}

	if ( numberOfFields > 0 ) {
		// Check for special cases first 
		for (var I = 0; I < this.possibleCombinations.length; I++) {
			if ( this.possibleCombinations[I].requiredUrl &&  window.location.hostname.match(this.possibleCombinations[I].requiredUrl) ) { // Found a special case
				if (this.settings.debugLevel > 1) cipDebug.log('Dealing with special case for ' + window.location.hostname);

				if (this.possibleCombinations[I].callback(this.forms) == 'skip') break
				
				// Handle sumbit event on submit button click or return keydown.
				for (form in this.forms) {
					var currentForm = this.forms[form]
					if (currentForm.element) {
						var field = currentForm.combination.fields.password || currentForm.combination.fields.username,
								submitButton = this.detectSubmitButton(field, field.parent())
							
						this.usernameFieldId =
							currentForm.combination.fields.username &&
							currentForm.combination.fields.username.data('mp-id')
						this.passwordFieldId =
							currentForm.combination.fields.password &&
							currentForm.combination.fields.password.data('mp-id')
					
						mpJQ(submitButton)
							.unbind('click.mooltipass')
							.on('click.mooltipass', this.onSubmit.bind(this, { target: currentForm.element[0] }))
							
						mpJQ(mpJQ.map(currentForm.fields, function (field) { return field.get() }))
							.unbind('keydown.mooltipass')
							.on('keydown.mooltipass', function(currentForm, event) {
								if (event.which == 13) { this.onSubmit.call(this, { target: currentForm.element[0] }) }
							}.bind(this, currentForm))
					}
				}

				var url = document.location.origin;
				var submitUrl = url;
				if ( this.credentialsCache && this.credentialsCache.length > 0 && this.credentialsCache[0].Login) {
					if (this.settings.debugLevel > 1) cipDebug.log('%c mcCombinations - %c Using credentials from cache', 'background-color: #c3c6b4','color: #777777' );
					this.retrieveCredentialsCallback( this.credentialsCache );
				} else {
					messaging({ 'action': 'retrieve_credentials', 'args': [ url, submitUrl, true, true] });	
				}
				return;
			}
		}
	
		this.detectForms();	

		for( form in this.forms ) {
			var currentForm = this.forms[ form ];

			// Unsure about this restriction. Probably should always make a retrieve credentials call (need to think about it)
			if ( currentForm.combination ) {
				var url = document.location.origin;
				var submitUrl = currentForm.element?this.getFormActionUrl( currentForm.element ):url;
				
				if ( this.credentialsCache && this.credentialsCache.length > 0) {
					// Sometimes the form changes when typing in. Issuing a new detectCombination.. we use a temporary cache to avoid double request in the device
					if (this.settings.debugLevel > 1) cipDebug.log('%c mcCombinations - %c Using credentials from cache', 'background-color: #c3c6b4','color: #777777', currentForm.element );
					this.retrieveCredentialsCallback( this.credentialsCache );
				} else {
					if (this.settings.debugLevel > 1) cipDebug.trace('%c mcCombinations - %c Retrieving credentials', 'background-color: #c3c6b4','color: #777777', currentForm.element );
					messaging({ 'action': 'retrieve_credentials', 'args': [ url, submitUrl, true, true] });
				}
			}
		}
	}
}

/*
* Returns the action URL for a form
*/
mcCombinations.prototype.getFormActionUrl = function( formElement ) {
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c getFormActionUrl','background-color: #c3c6b4','color: #333333');
	var action = formElement[0].action;

	if(typeof(action) != "string" || action == "" || action.indexOf('{') > -1 || action == 'javascript:void(0)') {
		action = document.location.origin + document.location.pathname;
	}

	if (this.settings.debugLevel > 3) cipDebug.log('%c mcCombinations - %c Form Action URL','background-color: #c3c6b4','color: #777777', action);
	return action;
}

/*
* Match found fields with available combinations
* Set debug to 4 to see all the traversing
*/
mcCombinations.prototype.detectForms = function() {
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c detectTypeofForm','background-color: #c3c6b4','color: #333333');
	var combinations = 0;
	
	// Check for custom fields.
	var definedCredentialFields =
		this.settings["defined-credential-fields"] && this.settings["defined-credential-fields"][document.location.origin]
	if (definedCredentialFields) {
		this.forms['noform'].definedCredentialFields = true
		this.forms['noform'].fields = [
			$('[data-mp-id=' + definedCredentialFields.username + ']'),
			$('[data-mp-id=' + definedCredentialFields.password + ']')
		]
	}
		
	
	// Traverse Forms
	for( form in this.forms ) {
		var currentForm = this.forms[ form ];
		if (this.settings.debugLevel > 3) cipDebug.log('%c mcCombinations - Form Detection: %c Traversing forms ','background-color: #c3c6b4','color: #777777', currentForm);
		if ( currentForm.fields.length == 0 ) continue; // Form has no fields.

		// Check combination against current form
		this.possibleCombinations.some( function( combination_data, index ) {
			if ( combination_data.requiredUrl ) { // Combination is a special case.
				return false;
			}

			if (this.settings.debugLevel > 3) cipDebug.log('\t %c mcCombinations  - Form Detection: %c Checking combination ' + combination_data.combinationName,'background-color: #c3c6b4','color: #777777; font-weight: bold;');
			if ( combination_data.maxfields && combination_data.maxfields < currentForm.fields.length ) {
				if (this.settings.debugLevel > 3) cipDebug.log('\t %c mcCombinations  - Form Detection: %c Form has more fields than expected ', 'background-color: #c3c6b4','color: #777777;');
				return false; // Form has more fields than expected
			}
			if ( combination_data.requiredFields.length > currentForm.fields.length) return false; // Form has less fields than required by this combination

			// Assign the combination to the form
			currentForm.combination = mpJQ.extend( true, {}, combination_data);

			// Check against the existence of extra selectors
			if (currentForm.combination.addFields) {
				if ( currentForm.element.find( currentForm.combination.addFields ).length == 1 ) {
					currentForm.combination.score += currentForm.combination.scorePerMatch;
				}
			}

			// Traverse fields in form and match against combination
			var matching = currentForm.fields.some( function( field ) {
				if (this.settings.debugLevel > 3) cipDebug.log('\t\t %c mcCombinations - Form Detection: %c Checking field ','background-color: #c3c6b4','color: #777777', field.prop('type') );

				// Initialize field as not passed
				field.data('passed', false);
				
				// Traverve required fields in combination to find a match
				var matching = currentForm.combination.requiredFields.some( function( requiredField, index, theArray ) {
					//console.log( 'matching ', combination_data.combinationName, field.is( requiredField.selector ), field[0].name, requiredField.selector, requiredField.mapsTo, requiredField.found, field.data('passed') );
					
					// Check if we already matched this field with another requirement
					if( field.data('passed') == true ) return false;


					// if ( requiredField.found ) {
					// 	// Check if we're looking for a close match
					// 	if ( !requiredField.closeToPrevious ) {
					// 		// return false;
					// 	} else { // We found the field, let's check if this one is closer
					// 		console.log('index', index, theArray );
					// 	}
					// }

					if ( field.is( requiredField.selector ) && !requiredField.found) {
						requiredField.found = true;
						requiredField.index = index;
						field.data('passed', true);
						currentForm.combination.score += currentForm.combination.scorePerMatch;
						if (this.settings.debugLevel > 3) cipDebug.log('\t\t\t %c mcCombinations - Form Detection: %c Field Match! Combination Score set to ','background-color: #c3c6b4','color: #777777', currentForm.combination.score );

						// Map fields into the combination.
						if ( requiredField.mapsTo ) {
							if (!currentForm.combination.savedFields) currentForm.combination.savedFields = {};
							currentForm.combination.savedFields[ requiredField.mapsTo ] = {
								value: '',
								submitPropertyName: typeof(requiredField.submitPropertyName) == 'function'?requiredField.submitPropertyName( field ):requiredField.submitPropertyName
							};

							if (!currentForm.combination.fields) currentForm.combination.fields = {};
							currentForm.combination.fields[ requiredField.mapsTo ] = field;
							requiredField.mapsTo = null;
						}

						// Check current score
						if ( currentForm.combination.score == 100 ) {
							this.waitingForPost = true;
							combinations++;
							if (this.settings.debugLevel > 3) cipDebug.log('\t\t\t %c mcCombinations - Form Detection: %c Combination Match!','background-color: #c3c6b4','color: #800000', currentForm.combination.combinationName );
							return true;
						}
						return false;
					} else {
						// If field doesn't match, and combination requires a specific order, then just leave.
						if ( combination_data.mustFollowOrder ) {
							currentForm.combination.score = -100;
							return false;
						}
					}
				}.bind(this));
				return matching;
			}.bind(this));
			return matching;
		}.bind(this));

		if (currentForm.combination.score < 100) {
			currentForm.combination = false;
			cipDebug.log('\t\t\t %c mcCombinations - Form Detection: %c No viable combination found!','background-color: #c3c6b4','color: #800000');
		} else {
			if ( currentForm.combination.preExtraFunction ) {
				if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c Running PreExtraFunction for combination','background-color: #c3c6b4','color: #333333');
				currentForm.combination.preExtraFunction( currentForm.combination.fields );
			}
			
			// Handle sumbit event on submit button click or return keydown.
			var field = currentForm.combination.fields.password || currentForm.combination.fields.username,
					submitButton = this.detectSubmitButton(field, field.parent())
				
			this.usernameFieldId =
				currentForm.combination.fields.username &&
				currentForm.combination.fields.username.data('mp-id')
			this.passwordFieldId =
				currentForm.combination.fields.password &&
				currentForm.combination.fields.password.data('mp-id')
			
			mpJQ(submitButton)
				.unbind('click.mooltipass')
				.on('click.mooltipass', this.onSubmit.bind(this, { target: currentForm.element && currentForm.element[0] }))
				
			mpJQ(mpJQ.map(currentForm.fields, function (field) { return field.get() }))
				.unbind('keydown.mooltipass')
				.on('keydown.mooltipass', function(currentForm, event) {
					if (event.which == 13) { this.onSubmit.call(this, { target: currentForm.element && currentForm.element[0] }) }
				}.bind(this, currentForm))
		}
	}

	// If there are no combinations detected, init the old method as well.
	if ( combinations == 0 ) cip.init();
	return;
}

/*
* Get all Fields on the DOM and store them in fields variable.
* Returns the number of fields found
*/
mcCombinations.prototype.getAllForms = function() {
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c getAllForms','background-color: #c3c6b4','color: #333333');
	var found = 0;

	for ( form in this.forms ) {
		this.forms[form].fields = [];
	}
	

	// get all input fields which are text, email or password and visible
	mpJQ( this.inputQueryPattern ).each( function( index, field ) {
		field = mpJQ( field );

		// Ignore our fields and search fields.
		if (field.attr('id') == 'mooltipass-password-generator' ||
				field.clone().children().remove().end()[0].outerHTML.match(/search/i)) {
			return;
		}
		
		// Check for field availability
		if( this.isAvailableField( field ) ) {
			found++;
			if (this.settings.debugLevel > 3) cipDebug.log('%c mcCombinations: %c Available Field ', 'background-color: #c3c6b4','color: #00FF00', field[0]);
			this.setUniqueId( field );

			// Store fields in FORMS
			containerForm = field.closest('form');
			if ( containerForm.length == 0 ) {
				var currentForm = this.forms.noform; // Field isn't in a Form
			} else {
				if ( !containerForm.data('mp-id') ) {
					this.setUniqueId( containerForm );
				}

				if ( !this.forms[ containerForm.data('mp-id') ] ) {
					this.forms[ containerForm.data('mp-id') ] = {
						fields: [],
						element: containerForm
					};
					
					containerForm.submit( mpJQ.proxy(this.onSubmit,this) );
				}
				var currentForm = this.forms[ containerForm.data('mp-id') ];
			}
			currentForm.fields.push( field );
		} else {
			if (this.settings.debugLevel > 3) cipDebug.log('%c mcCombinations: %c Unavailable Field ', 'background-color: #c3c6b4','color: #FF0000', field[0]);
		}
	}.bind(this));

	return found;
};

/*
* Intercept form submit
*/
mcCombinations.prototype.onSubmit = function( event ) {
	var currentForm = this.forms[ mpJQ(event.target).data('mp-id') ] || this.forms['noform'];
	if (this.forms['noform'].definedCredentialFields) currentForm = this.forms['noform']
	if (!currentForm.combination) return
	
	// Return if onSubmit has been already triggered by other events.
	if (this.onSubmitInProgress) return
	this.onSubmitInProgress = true
	setTimeout(function() {
		this.onSubmitInProgress = false
	}.bind(this), 100)
	
	if (this.settings.debugLevel > 1) cipDebug.log('%c mcCombinations: %c onSubmit','background-color: #c3c6b4','color: #333333');
	this.waitingForPost = false;
	
	// Check if there's a difference between what we retrieved and what is being submitted
	if ( !currentForm.combination.savedFields.username && this.credentialsCache) {
		if ( this.credentialsCache[0].TempLogin ) {
			this.credentialsCache[0].Login = this.credentialsCache[0].TempLogin
		}
		if ( this.credentialsCache[0].Login ) {
			currentForm.combination.savedFields.username = this.credentialsCache[0].Login;
			if ( !currentForm.combination.fields.username ) currentForm.combination.fields.username = this.credentialsCache[0].Login;
		}
	}

	if ( !currentForm.combination.savedFields.password && this.credentialsCache && this.credentialsCache[0].Password ) {
		currentForm.combination.savedFields.password = this.credentialsCache[0].Password;
		if ( !currentForm.combination.fields.password ) currentForm.combination.fields.password = this.credentialsCache[0].Password;
	}

	var storedUsernameValue = this.parseElement( currentForm.combination.savedFields.username, 'value');
	var storedPasswordValue = this.parseElement( currentForm.combination.savedFields.password, 'value');

	var submittedUsernameValue = this.parseElement( currentForm.combination.fields.username, 'value');
	var submittedPasswordValue = this.parseElement( currentForm.combination.fields.password, 'value');

	if ( mpJQ('#mooltipass-username').val() && mpJQ('#mooltipass-username').val() !== submittedUsernameValue ) {
		submittedUsernameValue = mpJQ('#mooltipass-username').val();
	}

	if ( !storedUsernameValue && !this.credentialsCache && submittedUsernameValue && !submittedPasswordValue) { // In case there's a 2 pages login. Store the username in cache
		this.credentialsCache = [{ TempLogin: submittedUsernameValue, Login: false, Password: false }];
	}

	if ( storedUsernameValue != submittedUsernameValue || storedPasswordValue != submittedPasswordValue ) { // Only save when they differ
		cip.rememberCredentials( event, 'unused', submittedUsernameValue, 'unused', submittedPasswordValue);
	}
	
	// Reset detected forms to prevent later usage.
	this.forms = { noform: { fields: [] } }
}



/*
* Check if a field is visible and ready to get input
* $field: The field selector as a JQUERY reference obj
* Returns true is the field appears to be available and visible
*/
mcCombinations.prototype.isAvailableField = function($field) {
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c isAvailableField','background-color: #c3c6b4','color: #333333');

	return (
		$field.is(":visible")
		&& $field.css("visibility") != "hidden"
		&& !$field.is(':disabled')
		&& $field.css("visibility") != "collapsed"
		&& $field.css("visibility") != "collapsed"
		&& $field.attr('aria-hidden') != 'true'
	);
}

/*
* Sets an unique ID for an element (field or form)
*/
mcCombinations.prototype.setUniqueId = function( element ) {
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c setUniqueId','background-color: #c3c6b4','color: #333333');
	if(element && !element.attr("data-mp-id")) {
		var elementId = element.attr("id");
		if( elementId ) {
			elementId = elementId.replace(/[:#.,\[\]\(\)' "]/g, '');
			element.attr("data-mp-id", elementId);
			return;
		} else {
			cipFields.uniqueNumber += 1;
			element.attr( "data-mp-id", "mpJQ" + String( cipFields.uniqueNumber ) );
		}
	}
}

/*
 * Trigger change event with new value for input.
 * Used to update value for fields handled by React.
 * https://github.com/vitalyq/react-trigger-change
 *
 * @param node {DOM node}
 * @param value {String}
 * @return undefined
 */
mcCombinations.prototype.triggerChangeEvent = function(node, value) {
  // React 16
  // Cache artificial value property descriptor.
  // Property doesn't exist in React <16, descriptor is undefined.
  descriptor = Object.getOwnPropertyDescriptor(node, 'value');

  // React 0.14: IE9
  // React 15: IE9-IE11
  // React 16: IE9
  // Dispatch focus.
  event = document.createEvent('UIEvents');
  event.initEvent('focus', false, false);
  node.dispatchEvent(event);

  // React 0.14: IE9
  // React 15: IE9-IE11
  // React 16
  // In IE9-10 imperative change of node value triggers propertychange event.
  // Update inputValueTracking cached value.
  // Remove artificial value property.
  // Restore initial value to trigger event with it.
  initialValue = node.value;
  node.value = value + '#';
  deletePropertySafe(node, 'value');
  node.value = value;

  // React 15: IE11
  // For unknown reason React 15 added listener for propertychange with addEventListener.
  // This doesn't work, propertychange events are deprecated in IE11,
  // but allows us to dispatch fake propertychange which is handled by IE11.
  event = document.createEvent('HTMLEvents');
  event.initEvent('propertychange', false, false);
  event.propertyName = 'value';
  node.dispatchEvent(event);

  // React 0.14: IE10-IE11, non-IE
  // React 15: non-IE
  // React 16: IE10-IE11, non-IE
  event = document.createEvent('HTMLEvents');
  event.initEvent('input', true, false);
  node.dispatchEvent(event);

  // React 16
  // Restore artificial value property descriptor.
  if (descriptor) {
    Object.defineProperty(node, 'value', descriptor);
  }
  
  // Do not try to delete non-configurable properties.
  // Value and checked properties on DOM elements are non-configurable in PhantomJS.
  function deletePropertySafe(elem, prop) {
    var desc = Object.getOwnPropertyDescriptor(elem, prop);
    if (desc && desc.configurable) {
      delete elem[prop];
    }
  }
}
	
/*
* Parses the credentials obtained
*/
mcCombinations.prototype.retrieveCredentialsCallback = function (credentials) {
	if (!credentials || credentials.length < 1) {
		if (this.settings.debugLevel > 1) cipDebug.log('%c mcCombinations: %c retrieveCredentialsCallback returned empty','background-color: #c3c6b4','color: #FF0000');
		return;
	}
	
	this.credentialsCache = credentials;
	
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c retrieveCredentialsCallback','background-color: #c3c6b4','color: #333333', credentials);

	// Credentials callback gets called when there's a hashChange in the fields. If we modified the username, keep the modified one
	if ( mpJQ('#mooltipass-username').val() ) credentials[0].Login = mpJQ('#mooltipass-username').val();
	mpJQ('#mooltipass-login-info').show();
	mpJQ('#mooltipass-username').val( credentials[0].Login );

	if (!isSafari) {
		// Store retrieved username as a cache
		chrome.runtime.sendMessage({'action': 'cache_login', 'args': [ credentials[0].Login ] }, function( r ) {
			var lastError = chrome.runtime.lastError;
			if (lastError) {
				if (this.settings.debugLevel > 0) cipDebug.log('%c mcCombinations: %c Error: ','background-color: #c3c6b4','color: #333333', lastError.message);
				return;
			} else {
				if (this.settings.debugLevel > 1) cipDebug.log('%c mcCombinations: %c retrieveCredentialsCallback Cache: ','background-color: #c3c6b4','color: #333333', r);
			}
		}.bind(this));	
	}
	
	for( form in this.forms ) {
		currentForm = this.forms[ form ];
		if (this.settings.debugLevel > 1) cipDebug.log('%c mcCombinations - %c retrieveCredentialsCallback filling form','background-color: #c3c6b4','color: #FF0000', currentForm);
		if (!currentForm.combination || !currentForm.combination.fields) continue;

		if (currentForm.combination && currentForm.combination.submitHandler)
		{
			currentForm.combination.submitHandler(credentials[0]);
		}
		// Unsure about this restriction. Probably should always make a retrieve credentials call (need to think about it)
		else if ( currentForm.combination ) {
			if ( credentials[0].Login && currentForm.combination.fields.username ) {
				if (this.settings.debugLevel > 3) cipDebug.log('%c mcCombinations - %c retrieveCredentialsCallback filling form - Username','background-color: #c3c6b4','color: #FF0000');
				// Fill-in Username
				if ( currentForm.combination.fields.username && typeof currentForm.combination.fields.username !== 'string' ) {
					currentForm.combination.fields.username.val('');
					currentForm.combination.fields.username.click();
					try {
						this.triggerChangeEvent(currentForm.combination.fields.username[0], credentials[0].Login)
						currentForm.combination.fields.username.trigger('blur')
					} catch (e) {}					
					currentForm.combination.fields.username[0].dispatchEvent(new Event('change'));
					currentForm.combination.savedFields.username.value = credentials[0].Login;	
				}
			}
			
			if ( credentials[0].Password && currentForm.combination.fields.password && currentForm.combination.combinationId != 'passwordreset001') {
				if (this.settings.debugLevel > 3) cipDebug.log('%c mcCombinations - %c retrieveCredentialsCallback filling form - Password','background-color: #c3c6b4','color: #FF0000');
				// Fill-in Password
				if ( 
					typeof currentForm.combination.fields.password === 'object' &&  // It is a field and not a string
					currentForm.combination.fields.password.length > 0 // && // It exists
					// !currentForm.combination.fields.password.hasClass('mooltipass-password-do-not-update')
				) {
					currentForm.combination.fields.password.val('');
					
					try {
						currentForm.combination.fields.password.click();
					} catch(e) {}
					
					try {
						currentForm.combination.fields.password.sendkeys( credentials[0].Password );
						this.triggerChangeEvent(currentForm.combination.fields.password[0], credentials[0].Password)
						currentForm.combination.fields.password.trigger('blur')
					} catch (e) {}					
					currentForm.combination.fields.password[0].dispatchEvent(new Event('change'));
					currentForm.combination.savedFields.password.value = credentials[0].Password;
				}
			}

			//console.log( currentForm.combination );
			if ( currentForm.combination.extraFunction ) {
				if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c Running ExtraFunction for combination','background-color: #c3c6b4','color: #333333');
				currentForm.combination.extraFunction( currentForm.combination.fields );
			}

			if ( currentForm.combination.usePasswordGenerator && mcCombs.settings.usePasswordGenerator ) {
				if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c Running Password generator for combination','background-color: #c3c6b4','color: #333333');
				var inputs = cipFields.getAllFields();
				cip.initPasswordGenerator(inputs);
			}
			
			if (currentForm.combination.autoSubmit &&
				  !currentForm.definedCredentialFields &&
				  !this.settings.doNotSubmitAfterFill &&
				  (!currentForm.combination.fields.username || mpJQ.contains(document, currentForm.combination.fields.username[0])) &&
				  (!currentForm.combination.fields.password || mpJQ.contains(document, currentForm.combination.fields.password[0]))) {
				this.doSubmit( currentForm );

				// Stop processing forms if we're going to submit
				// TODO: Weight the importance of each form and submit the most important, not the first!
				return;
			} else if ( currentForm.combination.enterFromPassword ) { // Try to send the enter key while focused on password
				try {currentForm.combination.fields.password.focus().sendkeys( "{enter}" );} catch(e){}
			}
		}
		
		// Don't proceed other forms when we have defined credential fields.
		if (this.forms['noform'].definedCredentialFields) break
	}
}

/*
 * Detect submit button for the given field and container.
 *
 * @param field {jQuery object}
 * @param container {jQuery object}
 * @return submitButton {DOM node} or undefined
 */
 mcCombinations.prototype.detectSubmitButton = function detectSubmitButton(field, container) {
	var ACCEPT_PATTERNS = [
		/submit/i,
		/login/i,
		/log in/i,
		/sign/i,
		/connexion/i,
		/valider/i,
		/connecter/i,
		/anmelden/i,
		/identifierNext/i,
		/passwordNext/i,
		/verify_user_btn/i,
		/change password/i,
		/continue/i,
		/weiter/i,
	],
	
	IGNORE_PATTERNS = [
		/forgot/i,
		/lost/i,
		/lostlogin/i,
		/closeModal/i,
		/vergessen/i,
		/troubleloggingin/i,
		/showpassword/i,
		/showhidepasswd/i,
		/passwordreset/i,
		/resetform/i,
		/remember_login/i,
		/sign up/i,
		/facebook/i,
		/google/i,
		/id=".*?search.*?"/i,
		/id="btnLoadMoreProducts"/i,
		/class=".*?search.*?"/i,
		/class="login_row"/i,
		/href=".*?loginpage.*?"/i,
		/href="http.*?"/i,
		/\(Logged out\) Header/i,
	],
	
	// Selectors are ordered by priority, first ones are more important.
	BUTTON_SELECTORS = [
		'td.custom-button-center:visible',
		'[type="button"]:visible, [type="submit"]:visible, a[href^="javascript:"]:visible',
		'button:visible',
		'button:visible span',
		'[role="button"]:visible',
		'[role="button"]:visible span',
		'input.button:visible',
		'a:visible',
		'input[onclick]:visible',
		'div[onclick]:visible',
		'div.button:visible'
	]
	
	var submitButton
	
	for (var selectorIndex = 0; selectorIndex < BUTTON_SELECTORS.length; selectorIndex++) {
		var selector = BUTTON_SELECTORS[selectorIndex]
		
		var buttons = container.find(selector).filter(function(index, button) {
			for (var i = 0; i < IGNORE_PATTERNS.length; i++) {
				if (mpJQ(button).clone().children().remove().end()[0].outerHTML.match(IGNORE_PATTERNS[i])) return false
			}
			
			for (var i = 0; i < ACCEPT_PATTERNS.length; i++) {
				if (mpJQ(button).clone().children().remove().end()[0].outerHTML.match(ACCEPT_PATTERNS[i])) return true
			}
		})
		
		// Sort buttons by how nearest they are from the field.
		buttons.each(function(index, button) {
			button.distance = Math.abs($(button).offset().top - field.offset().top)
		})
		
		buttons.sort(function(a, b) {
			if (a.distance > b.distance) return 1
			if (a.distance < b.distance) return -1
			if (a.distance == b.distance) return 0
		})
		
		// Button shouldn't be far more than 150px from input.
		if (buttons.length > 0 && buttons[0].distance < 150) return buttons[0]
	}
	
	// If we haven't detected submit button yet, try to find it in parent container.
	if (container[0] != mpJQ('body')[0]) {
		return this.detectSubmitButton(field, container.parent())
	}
 }

/*
 * Captcha detection.
 *
 * @param form {jQuery object}
 * @return {Boolean}
 */
mcCombinations.prototype.formHasCaptcha = function(form) {
	form = form && form.length ? form : $('body')
	
	var hasCaptcha = false
	form.find('[class*="captcha"]:visible, [id*="captcha"]:visible').each(function(index, element) {
		var $element = $(element)
		
		if ($element.width() != 0 &&
				$element.height() != 0 &&
				$element.css('display') != 'none') {
			hasCaptcha = true
		}
	})
	
	return hasCaptcha
}

/*
* Submits the form!
*/
mcCombinations.prototype.doSubmit = function doSubmit(currentForm) {
	if (this.formHasCaptcha()) {
		if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c Captcha detected','background-color: #c3c6b4','color: #800000')
		return
	}
	
	// Do not autosubmit form with two-factor auth for Steam.
	if (window.location.hostname.match(/steamcommunity.com|steampowered.com/) &&
		  mpJQ('#authcode:visible, #twofactorcode_entry:visible').length) return
			
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c doSubmit','background-color: #c3c6b4','color: #333333');
	
	// Trying to find submit button and trigger click event.
	var field = currentForm.combination.fields.password || currentForm.combination.fields.username,
			submitButton = this.detectSubmitButton(field, field.parent())
	
	if (submitButton) {
		// Select innermost element to trigger click because handler can be on it.
		// Event will be propagated anyway.
		submitButton = mpJQ(submitButton).find(':not(:has(*))')[0] || submitButton
		
		// Button can be disabled, waiting for update.
		setTimeout(function() {
			if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c doSubmit:submitButton','background-color: #c3c6b4','color: #333333', submitButton);
			submitButton.click()
		}.bind(this), 100)
	} else {
		// If we haven't found submit button, let's trigger submit event on the form.
		mpJQ(currentForm.element).trigger('submit')
	}
}


/*
* Parses an element and data and tries to retrieve value from it.
* Element: String or DOM Object, or jQuery Object containing a reference to the element. If string is given , it is used as the value.
* Data: Object containing the data to retrieve from
*/
mcCombinations.prototype.parseElement = function( element, data ) {
	if ( !element ) { // Just a false call
		return false;
	}
	
	// If it is a string, we stored the value, so return it.
	if ( typeof( element ) == 'string' ) return element;

	var attribute = 'name';
	if ( typeof ( element.submitPropertyName) != 'undefined' ) attribute = element.submitPropertyName;

	// We're searching for a property of the element
	if ( typeof ( data ) == 'string' ) {
		if ( data == 'value' && typeof( element.val ) == 'function' ) return element.val();
		return element[ data ];
	}

	// Fallback to standard:  data [ element.attribute ]
	return element?data[ element.attr( attribute ) ]:false;
}
/*
* When a POST request is detected, we check it in case there's our info
*/
mcCombinations.prototype.postDetected = function( details ) {
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c postDetected','background-color: #c3c6b4','color: #333333', details);

	// Run special cases
	for ( specialCase in extendedPost ) {
		if ( window.location.hostname.indexOf( specialCase ) > -1 ) {
			details = extendedPost[specialCase]( details );
		}
	}

	// Just act if we're waiting for a post
	if ( this.waitingForPost && this.settings.postDetectionFeature) {
		// Loop throught the forms and check if we've got a match
		for( form in this.forms ) {
			currentForm = this.forms[ form ];
			if ( currentForm.combination && currentForm.combination.savedFields ) {
				var currentCombination = currentForm.combination;
				var sent = details.requestBody?details.requestBody:details;
				var storedUsernameValue = false;
				var usernameValue = details.usernameValue?details.usernameValue:false;
				var storedPasswordValue = false;
				var passwordValue = details.passwordValue?details.passwordValue:false;

				// Username parsing
				if ( currentCombination.savedFields.username ) {
					if ( typeof currentCombination.savedFields.username == 'string') {
						usernameValue = currentCombination.savedFields.username;
						storedUsernameValue = currentCombination.savedFields.username;
					} else {
						// Special cases handle the usernameValue var.
						if ( usernameValue === false) {
							var attrUsername = 'name';
							if ( currentCombination.savedFields.username.submitPropertyName ) {
								attrUsername = currentCombination.savedFields.username.submitPropertyName;
							}

							// Some servers send DATA instead of FORMDATA
							if ( sent.data && !sent.formData ) sent.formData = sent.data;

							if ( sent.formData ) { // Form sent FORM DATA
								usernameValue = sent.formData[
									currentCombination.fields.username && currentCombination.fields.username.attr( attrUsername )
								];
							} else { // Client sent a RAW request.
								usernameValue = sent[ currentCombination.fields.username.attr( attrUsername ) ];
							}	
						}

						storedUsernameValue = currentCombination.savedFields.username.value;
					}
				}
				
				// Password parsing
				if ( currentCombination.savedFields.password ) {
					// Special cases handle the passwordValue var.
					if ( passwordValue === false ) {
						var attrPassword = 'name';
						if ( currentCombination.savedFields.password.submitPropertyName ) {
							var attrPassword = currentCombination.savedFields.password.submitPropertyName;	
						}

						if ( sent.formData ) { // Form sent FORM DATA
							passwordValue = sent.formData[ currentCombination.fields.password.attr( attrPassword ) ];
						} else { // Client sent a RAW request.
							passwordValue = sent[ currentCombination.fields.password.attr( attrPassword ) ];
						}
					} 

					storedPasswordValue = currentCombination.savedFields.password.value;
				}

				if (this.settings.debugLevel > 3) {
					cipDebug.log('%c mcCombinations: %c postDetected - Stored: ', 'background-color: #c3c6b4','color: #333333', storedUsernameValue, storedPasswordValue);
					cipDebug.log('%c mcCombinations: %c postDetected - Received: ','background-color: #c3c6b4','color: #333333', usernameValue, passwordValue);
				}

				// Only update if they differ from our database values (and if new values are filled in)
				if ( ((storedUsernameValue != usernameValue) || (storedPasswordValue != passwordValue)) && (usernameValue != '' && passwordValue != '') ) {
					var url = document.location.origin;
					chrome.runtime.sendMessage({
						'action': 'update_notify',
						'args': [usernameValue, passwordValue, url]
					});
				}
			}
		}
		return;
	}
}
