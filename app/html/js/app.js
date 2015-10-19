var _console_log = console.log;
var _console_warn = console.warn;

console.log = function() {
  text = arguments[0];
  i = 1;
  while (i < arguments.length) {
    if(arguments[i] === undefined) {
      text += " undefined"
    }
    else {
      text += " " + arguments[i].toString();
    }
    i++;
  }
  $("#log").val($("#log").val() + text + "\n");
  $("#log").animate({
        scrollTop:$("#log")[0].scrollHeight - $("#log").height()
  }, 100);
  return _console_log.apply(console, arguments);
}

console.warn = function() {
  text = arguments[0];
  i = 1;
  while (i < arguments.length) {
    if(arguments[i] === undefined) {
      text += " undefined"
    }
    else {
      text += " " + arguments[i].toString();
    }
    i++;
  }
  text = "[WARNING] " + text;
  $("#log").val($("#log").val() + text + "\n");
  $("#log").animate({
        scrollTop:$("#log")[0].scrollHeight - $("#log").height()
  }, 100);
  return _console_warn.apply(console, arguments);
}

show_active_page = function() {
  $(".page").hide();
  newPage = $("nav li.active a").attr("mp-open-page");
  $("#page-" + newPage).show();  
}

is_device_in_mmm = function() {
  // DEBUG
  // return false;
  return mooltipass.device.inMemoryManagementMode;
}

is_device_connected = function() {
  // DEBUG
  // return true;
  return mooltipass.device.isConnected;
}

update_device_status_classes = function() {
  if (is_device_connected()) {
    $(".hide-if-connected").hide();
    $(".show-if-connected").show();

    if (is_device_in_mmm()) {
      $(".hide-if-mmm").hide();      
      $(".show-if-mmm").show();        
    } else {
      $(".hide-if-mmm").show();      
      $(".show-if-mmm").hide();              
    } 

  } else {
    $(".hide-if-connected").show();
    $(".show-if-connected").hide();
  }     
}

$(function(){
  // Only show app, if mp is connected
  setInterval(update_device_status_classes, 500);

  // Init pages
  show_active_page();
  $("nav li a").click(function() {
    $("nav li.active").removeClass("active");
    $(this).parent("li").addClass("active");
    show_active_page();
  });  
});