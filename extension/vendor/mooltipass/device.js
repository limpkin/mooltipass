/**
*
* This file is a mockup for the required mooltipass functions
* of the extension. It is loaded as a background script and
* not accessible via the website.
*
**/

var mooltipass = mooltipass || {};
mooltipass.device = mooltipass.device || {};

mooltipass.device.containsCredentials = function(url) {
  // Return true if Mooltipass has already stored credentials for the
  // given url, or it's top-level-domain.
  // e.g.   stored credentials  | 

  // Stored credentials for 
  return (url == "http://www.reddit.com/")
}

mooltipass.device.isConnected = function() {
  // Return true if a device with a security card is connected.
  return true
}

mooltipass.device.requestCredentials = function(url) {
  // Requests credentials from user on Mooltipass device
  return;
}

// Is called, when a user confirms his credentials for a certain 
// website.
// mooltipass.device.onConfirmCredentials.dispatch(url, username, password)
mooltipass.device.onConfirmCredentials = new chrome.Event();

mooltipass.device.getRandomPassword = function(length) {
  return Math.random().toString(36).substring(7)
}