chrome.tabs.onUpdated.addListener( function (tabId, changeInfo, tab) {
	alert("tabId");
  if (changeInfo.status == 'complete') {

    

  }
})