var _console_log = console.log;

console.log = function() {
  text = arguments[0];
  i = 1;
  while (i < arguments.length) {
    text += " " + arguments[i].toString();
    i++;
  }
  $("#log").val($("#log").val() + text + "\n");
  $("#log").animate({
        scrollTop:$("#log")[0].scrollHeight - $("#log").height()
  }, 100);
  return _console_log.apply(console, arguments);
}

show_active_page = function() {
  $(".page").hide();
  newPage = $("nav li.active a").attr("mp-open-page");
  $("#page-" + newPage).show();  
}

is_device_connected = function() {
  // DEBUG
  return true;
  return mooltipass.device.isConnected;
}

show_missing_connection_page = function() {
  if (is_device_connected()) {
    $(".hide-if-connected").hide();
    $(".show-if-connected").show();
  } else {
    $(".hide-if-connected").show();
    $(".show-if-connected").hide();      
  }  
}

$(function(){
  // Only show app, if mp is connected
  setInterval(show_missing_connection_page, 500);

  // Init pages
  show_active_page();
  $("nav li a").click(function() {
    $("nav li.active").removeClass("active");
    $(this).parent("li").addClass("active");
    show_active_page();
  });
});