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

				if ( mpJQ('input[type=password]').length > 0 ) { // Step 1: Email
					currentForm.combination.fields.password = mpJQ('input[type=password]');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('input[name=loginfmt]').length > 0 ) { // Step 1: Email
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

				if ( mpJQ('#login-passwd').length > 0 ) { // Step 1: Email
					currentForm.combination.fields.password = mpJQ('#login-passwd');
					currentForm.combination.autoSubmit = true;
				} 
				if ( mpJQ('#login-username').length > 0 ) { // Step 1: Email
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
		inputQueryPattern: "input[type='text']:not([class='search']), input[type='email'], input[type='password']:not(.notinview), input[type='tel'], input[type='number'], input:not([type])",
		forms: {
			noform: { fields: [] }
		},
		uniqueNumber: 342845638,
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
				selector: 'input[type=text],input:not([type])',
				mapsTo: 'username'
			},
			{
				selector: 'input[type=password]',
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
				selector: 'input[type=email]',
				mapsTo: 'username'
			},
			{
				selector: 'input[type=password]',
				mapsTo: 'password'
			},
		],
		scorePerMatch: 50,
		score: 0,
		autoSubmit: false,
		enterFromPassword: true,
		maxfields: 12,
		extraFunction: function( fields ) {
			this.fields.username = cipFields.getUsernameField( fields.password.prop('id') );
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
				selector: 'input[type=password]:visible',
				mapsTo: 'password'
			},
			{
				selector: 'input[type=password]:visible'
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

	if ( numberOfFields > 0 ) {
		// Check for special cases first 
		for (var I = 0; I < this.possibleCombinations.length; I++) {
			if ( this.possibleCombinations[I].requiredUrl && this.possibleCombinations[I].requiredUrl == window.location.hostname ) { // Found a special case
                if (this.settings.debugLevel > 1) cipDebug.log('Dealing with special case for ' + window.location.hostname);
				this.possibleCombinations[I].callback( this.forms );

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

	if(typeof(action) != "string" || action == "" || action.indexOf('{') > -1) {
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

		if ( currentForm.combination.score < 100 ) {
			currentForm.combination = false;
			cipDebug.log('\t\t\t %c mcCombinations - Form Detection: %c No viable combination found!','background-color: #c3c6b4','color: #800000');
		} else if ( currentForm.combination.preExtraFunction ) {
			if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c Running PreExtraFunction for combination','background-color: #c3c6b4','color: #333333');
			currentForm.combination.preExtraFunction( currentForm.combination.fields );
		}
	}

	// Init the password generator as always
	if ( mcCombs.settings.usePasswordGenerator ) {
		var inputs = cipFields.getAllFields();
		cip.initPasswordGenerator(inputs);
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

		// Ignore our field(s)
		if( field.attr('id') == 'mooltipass-password-generator') {
			return;
		}
		
		// Check for field availability
		if( this.isAvailableField( field ) ) {
			found++;
			if (this.settings.debugLevel > 3) cipDebug.log('%c mcCombinations: %c Available Field ', 'background-color: #c3c6b4','color: #00FF00', field[0]);
			this.setUniqueId( field );

			// Store fields in FORMS
			containerForm = field.closest('form');
			if ( containerForm.length == 0 ) var currentForm = this.forms.noform; // Field isn't in a Form
			else {
				if ( !containerForm.data('mp-id') ) {
					this.setUniqueId( containerForm );
				}

				if ( !this.forms[ containerForm.data('mp-id') ] ) {
					this.forms[ containerForm.data('mp-id') ] = {
						fields: [],
						element: containerForm
					};
					containerForm.submit( mpJQ.proxy(this.onSubmit,this) );

					// Fire submit event for accounts.google.com when "Next" buttons is clicked
					// and when return key is pressed inside input.
					// It's good to move this quirk to universal method in extendedCombinations,
					// something like "handleSubmit".
					if (window.location.hostname == 'accounts.google.com') {
						containerForm.find('[role=button]').click( this.onSubmit.bind(this, { target: containerForm }) );
						containerForm.find('input').keypress(function(event) {
							if (event.which == 13) {
								this.onSubmit.call(this, { target: containerForm });
							}
						}.bind(this));
					}
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
	if (this.settings.debugLevel > 1) cipDebug.log('%c mcCombinations: %c onSubmit','background-color: #c3c6b4','color: #333333');
	this.waitingForPost = false;

	// Check if there's a difference between what we retrieved and what is being submitted
	var currentForm = this.forms[ mpJQ(event.target).data('mp-id') ];

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

	if ( !storedUsernameValue && !this.credentialsCache && submittedUsernameValue) { // In case there's a 2 pages login. Store the username in cache
		this.credentialsCache = [{ TempLogin: submittedUsernameValue, Login: false, Password: false }];
	}

	if ( storedUsernameValue != submittedUsernameValue || storedPasswordValue != submittedPasswordValue ) { // Only save when they differ
		cip.rememberCredentials( event, 'unused', submittedUsernameValue, 'unused', submittedPasswordValue);
	}
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
			element.attr("data-mp-id", elementId);
			return;
		} else {
			// create own ID if no ID is set for this field
			this.uniqueNumber += 1;
			element.attr( "data-mp-id", "mpJQ" + String( this.uniqueNumber ) );
		}
	}
}

/*
* Parses the credentials obtained
*/
mcCombinations.prototype.retrieveCredentialsCallback = function (credentials) {
	this.credentialsCache = credentials;

	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c retrieveCredentialsCallback','background-color: #c3c6b4','color: #333333', credentials);

	if (!credentials || credentials.length < 1) {
		if (this.settings.debugLevel > 1) cipDebug.log('%c mcCombinations: %c retrieveCredentialsCallback returned empty','background-color: #c3c6b4','color: #FF0000');
		return;
	}

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
		if ( !currentForm.combination || !currentForm.combination.fields ) continue;

		// Unsure about this restriction. Probably should always make a retrieve credentials call (need to think about it)
		if ( currentForm.combination ) {
			if ( credentials[0].Login && currentForm.combination.fields.username ) {
				if (this.settings.debugLevel > 3) cipDebug.log('%c mcCombinations - %c retrieveCredentialsCallback filling form - Username','background-color: #c3c6b4','color: #FF0000');
				// Fill-in Username
				if ( currentForm.combination.fields.username && typeof currentForm.combination.fields.username !== 'string' ) {
					currentForm.combination.fields.username.val('');
					currentForm.combination.fields.username.click();
					try {currentForm.combination.fields.username.sendkeys( credentials[0].Login );} catch (e) {}					
					currentForm.combination.fields.username[0].dispatchEvent(new Event('change'));
					currentForm.combination.savedFields.username.value = credentials[0].Login;	
				}
			}
			
			if ( credentials[0].Password && currentForm.combination.fields.password ) {
				if (this.settings.debugLevel > 3) cipDebug.log('%c mcCombinations - %c retrieveCredentialsCallback filling form - Password','background-color: #c3c6b4','color: #FF0000');
				// Fill-in Password
				if ( 
					typeof currentForm.combination.fields.password === 'object' &&  // It is a field and not a string
					currentForm.combination.fields.password.length > 0 // && // It exists
					// !currentForm.combination.fields.password.hasClass('mooltipass-password-do-not-update')
				) {
					currentForm.combination.fields.password.val('');
					currentForm.combination.fields.password.click();
					currentForm.combination.fields.password.sendkeys( credentials[0].Password );
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
			if ( currentForm.combination.autoSubmit && !this.settings.doNotSubmitAfterFill) {
				this.doSubmit( currentForm );

				// Stop processing forms if we're going to submit
				// TODO: Weight the importance of each form and submit the most important, not the first!
				return;
			} else if ( currentForm.combination.enterFromPassword ) { // Try to send the enter key while focused on password
				try {currentForm.combination.fields.password.focus().sendkeys( "{enter}" );} catch(e){}
			}
		}
	}
}

/*
* Submits the form!
*/
mcCombinations.prototype.doSubmit = function doSubmit( currentForm ) {
	if (this.settings.debugLevel > 4) cipDebug.log('%c mcCombinations: %c doSubmit','background-color: #c3c6b4','color: #333333');
	
	// Do not autosubmit forms with Captcha
	if ( cip.formHasCaptcha ) return;

	if ( currentForm.element ) {
		// Sites like Steam use a button outside the form:
		/*if ( mpJQ('button:submit').length == 1 ) {
			// Make sure it is the button we're looking for:
			if ( mpJQ('button:submit').find('span').length == 1 ) {
				mpJQ('button:submit').find('span').click();
				return;	
			}
		}*/

		// Try to click the submit element
		var submitButton = currentForm.element.find(':submit:visible');
		// Check if we found a button
		if ( submitButton.length > 0 )
		{
			var selectedButton = submitButton[0];
			// If there are multiple buttons
			if (submitButton.length > 1)
			{
				// Simply discard buttons that may have "search" in their ids
				for (i = 0; i < submitButton.length; i++)
				{
					if (submitButton[i].id && !submitButton[i].id.includes("search") && !submitButton[i].id.includes("Search"))
					{
						selectedButton = submitButton[i];
						break;
					}
				}
			}
		}
		if ( submitButton.length > 0 ) { 
			// Add timeout to allow form check procedures to run
			setTimeout( function() {
				selectedButton.click();
			},100);
		} else if ( mpJQ('#verify_user_btn').length > 0 ) { // Exclusive else/if for Autodesk.com (probably it could be used in more 2 steps login procedures)
			mpJQ('#verify_user_btn').click();
		} else if (window.location.hostname == 'accounts.google.com') { // Special case for google
            if (mpJQ('#identifierNext').length > 0)
            {
                mpJQ('#identifierNext').click();
            }
            if (mpJQ('#passwordNext').length > 0)
            {
                mpJQ('#passwordNext').click();
            }            
        } else if ( currentForm.element ) {
			// If no submit button is found, just submit the form
			currentForm.element.submit();
		}
	} else { // There is no FORM element. Click stuff around
		setTimeout( function() {
			mpJQ('#sign-in, .btn-submit, #verify_user_btn').click();
		},1500);
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
								usernameValue = sent.formData[ currentCombination.fields.username.attr( attrUsername ) ];
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