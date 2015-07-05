function checkMooltipassConnection() 
{
  if (!mooltipass.device.isConnected()) {
    chrome.notifications.create({
      "type" : "basic",
      "iconUrl" : "images/mooltipass-128.png",
      "title" : "No Mooltipass device found!",
      "message" : "Connect the Mooltipass device and insert your Security Card."
    });
  }
}

chrome.runtime.onInstalled.addListener(checkMooltipassConnection);
chrome.runtime.onStartup.addListener(checkMooltipassConnection);
