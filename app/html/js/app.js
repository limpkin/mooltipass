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

is_device_connected = function() {
  // DEBUG
  //return true;
  return mooltipass.device.isConnected;
}

is_device_unlocked = function() {
  // DEBUG
  //return true;
  return mooltipass.device.isUnlocked;
}

is_device_in_mmm = function() {
  // DEBUG
  //return true;
  return mooltipass.device.inMemoryManagementMode;
}

update_device_status_classes = function() {
  if (is_device_connected()) {
    $(".show-if-connected").show();
    $(".hide-if-connected").hide();
  } else {
    $(".show-if-connected").hide();
    $(".hide-if-connected").show();

    return;
  }

  if (is_device_unlocked()) {
    $(".show-if-unlocked").show();
    $(".hide-if-unlocked").hide();
  } else {
    $(".show-if-unlocked").hide();
    $(".hide-if-unlocked").show();  

    return  
  }

  if (is_device_in_mmm()) {
    $(".show-if-mmm").show();
    $(".hide-if-mmm").hide();
  } else {
    $(".show-if-mmm").hide();    
    $(".hide-if-mmm").show();
  }
}

$(function(){
  // Only show app, if mp is connected
  update_device_status_classes();
  setInterval(update_device_status_classes, 500);

  // Init pages
  show_active_page();
  $("nav li a").click(function() {
    $("nav li.active").removeClass("active");
    $(this).parent("li").addClass("active");
    show_active_page();
  });  
});