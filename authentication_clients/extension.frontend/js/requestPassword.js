function showDeviceInteractionNotification() {
  chrome.notifications.create({
    "type" : "basic",
    "iconUrl" : "images/mooltipass-128.png",
    "title" : "Password confirmation required",
    "message" : "Please confirm your credentials on the Mooltipass device."
  });
}


function checkPassword(tabId, changeInfo, tab) {
  if (!mooltipass.device.isConnected()) return;
  if (changeInfo.status != 'complete') return;
  if (!mooltipass.device.containsCredentials(tab.url)) return;
  
  // We cannot directly access the source code of the tab, hence we
  // send a request to the tab which returns the source code.
  // //////////////////////////////////////////////////////////////////////// //
  // WARNING: DO NOT EVALUATE THIS CODE, IT MIGHT MAKE THE SCRIPT VULNERABLE  //
  //    TO CROSS SITE SCRIPTING, SINCE THE WEBSITE COULD OVERWRITE OUR        //
  //    getSource METHOD.                                                     //
  // //////////////////////////////////////////////////////////////////////// //
  chrome.tabs.sendRequest(tabId, {action: "getSource"}, function(source) {
    if (!mooltipass.website.hasLoginForm(source)) return;
    mooltipass.device.requestCredentials(tab.url);
    showDeviceInteractionNotification();
  });  
}

chrome.tabs.onUpdated.addListener(checkPassword)