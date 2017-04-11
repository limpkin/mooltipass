# McCombinations Documentation

mcCombinations is the heart of the extension. It takes the fields presents in the page and 
tries to come out with the most accurate procedure after analyzing the fields.

In a broad view, what it does is:

	* Split all the fields into forms
	* Detect a combination for each form
	* Holds the most pertinent form/combination

The most common structure would be a form with a submit button, the first field being the login
and the second a password. From there, everything is a special case and it is handled by mcCombinations


## Work method

As mcCombinations tries to perform the whole detection and submit preparation of forms, it doesn't return what it finds, but actually stores and prepares itself for communication with the device. So, everything is stored inside the "forms" variable inside mcCombs

To check detected forms in a page, just type in the console:

- mcCombs.forms

(You'll see a "noform", which is a special case for orphaned input fields)

forms is an object with the ID of the form and combination information inside it:

combination: the combination itself (so... preExtraFunction can modify it)
element: A reference to the FORM element that's associated
fields: A reference to every field in the combination that has relationship with it.




## Combination Matching

Combinations are selected in order from top to bottom, so the process is abandoned as soon as a viable combination is found.

A combination is viable if: All the required fields have been found, and the score reached 100. 




## Combination structure

Each combination has a specific structure in JSON format, and properties are as below:

	combinationId: [String] Just an indicative name for the combination, no spaces nor special chars please, eg: 'simpleLoginForm001'
	combinationName: [String] Descriptive info about the combination
	requiredFields: [Object] Fields that must be present in the form for this combination to pass as positive
	requiredFields.selector: [String] DOM selector for the requirement. eg: 'input[type=password]'
	requiredFields.submitPropertyName: [String] While by default we submit the NAME property as the descriptor, some sites use other property, specified in this field
	requiredFields.mapsTo: [ENUM:username/password] Set if this required field maps to either username or password
	requiredFields.closeToPrevious: [BOOL] If the found element matching the requiredFields.selector also has to be immediate in DOM order to the previous requiredField (Ignores any other HTML element besides input fields)
	scorePerMatch: [INT 0-100] On each found match, add this to the combination score.
	score: [INT 0-100] Starting score for combination.
	autoSubmit: [BOOL] Indicates if the combination will submit the form after filling it or not
	maxfields: [INT] Maximum number of fields the FORM element can contain in order to consider this combinationName
	preExtraFunction: [FUNCTION] Called when the combination is found and pass as approved, BEFORE credentials retrieve callback 
	extraFunction: [FUNCTION] Called when the combination is found and pass as approved, AFTER credentials retrieve callback 
	isPasswordOnly: [BOOL] Indicates if the combination expects to have only password fields
	usePasswordGenerator: [BOOL] Use password generator in the password fields or not
	enterFromPassword: [BOOL] Send an enter key after filling in the password
	requiredUrl: [String] Only works for the specificied URL
	callback: [Function] Skips the normal combination processor and executes this function instead. Meant for very specific cases.



## Example

It is easy to explaing this method by analyzing one of the first combinations: 'canFieldBased'. As the combinationName says: 'Login Form with can-field properties' some websites opperate with a CMS that generate input fields with 'can-field' as a property.

The required fields are indicates it is looking for two fields: input[can-field=accountName] and input[can-field=password] with mapping to username and password respectively.

As we expect two fields, each match scores 50 points (in hope to reach and not surpass 100)

Later, we know that CMS doesn't render standard submit fields, and that it requires some time since the last time something was typed until the form is submitted (probably to avoid robots) So in the extra function we add a timeout plus an indication to issue a click event to the submit button.

