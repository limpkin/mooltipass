/* Initialize mooltipass lib */
if (typeof mooltipass === 'undefined') mooltipass = {};
mooltipass.device = mooltipass.device || {};

/* library functions for mooltipass.device ********************** */

// [MOCKUP]
mooltipass.device.hasStoredCredentials = function(url, callback) {
  // Return true if Mooltipass has already stored credentials for the
  // given url, or it's top-level-domain.
  // e.g.   stored credentials  | 

  // Stored credentials for 
  callback((url == "http://www.reddit.com/"));
}

// [MOCKUP]
mooltipass.device.isConnected = function(callback) {
  // Calls callback with true of false depending of a established
  // connection to the device
  callback(true);
}

// [MOCKUP]
// Is called, when a user confirms his credentials for a certain 
// website.
// mooltipass.device.onConfirmCredentials.dispatch(url, username, password)
mooltipass.device.onConfirmCredentials = new chrome.Event();