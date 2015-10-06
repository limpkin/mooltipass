var mooltipass = mooltipass || {};
mooltipass.prefstorage = mooltipass.prefstorage || {};

// Get stored preferences
mooltipass.prefstorage.getStoredPreferences = function(callbackFunction)
{
	chrome.storage.sync.get(null, callbackFunction);	
}

// Stored preferences for a given key
mooltipass.prefstorage.setStoredPreferences = function(items)
{
	// Save it using the Chrome extension storage API.
	chrome.storage.sync.set(items, function() 
	{
		if (chrome.runtime.error)
		{
			console.log("Store preferences error: " + chrome.runtime.lastError.message);
		}
		else
		{
			console.log("Preferences saved to local storage");			
		}
	});
}

// Clear preference storage
mooltipass.prefstorage.clearStoredPreferences = function()
{
	chrome.storage.sync.clear();
}