var updateNotificationBar = function() {
  chrome.notifications.getPermissionLevel(function(response) {
    if (response == 'denied') {
      $("#notifications-disabled").show();
      $("#notifications-enabled").hide();
    } else {
      $("#notifications-disabled").hide();
      $("#notifications-enabled").show();
    }
  });        
}

var _updateNotificationBar = function() {
    updateNotificationBar();
    setTimeout(_updateNotificationBar, 200);  
}

$(function(){
  _updateNotificationBar();
});