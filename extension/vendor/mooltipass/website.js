/**
*
* This file is a mockup for the required mooltipass functions
* of the extension. It is used as a content script, and injected
* into all loaded websites, and popups.
*
**/

var mooltipass = mooltipass || {};
mooltipass.website = mooltipass.website || {};

mooltipass.website.generatePassword = function(length) {
  // Return a random password with given length

  // MOCKUP does not support random length
  return Math.random().toString(36).substring(7)
}