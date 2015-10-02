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

  // Load contributor list
  $("#contributor-list").each(function () {
    $.getJSON("https://api.github.com/repos/limpkin/mooltipass/contributors", function (contributors) {
      $("#contributor-list").html("");
      for (_contributor in contributors) {
        var contributor = contributors[_contributor];

        var p = $('<a class="column small-4 contributor" id="contributor-' + _contributor.toString() + '">');
        $("#contributor-list").append(p);

        var xhr = new XMLHttpRequest();
        xhr.open('GET', contributor.avatar_url, true);
        xhr.contributor = contributor;
        xhr.contributor_id = _contributor.toString();
        xhr.responseType = 'blob';
        xhr.onload = function(e) {
          var img = document.createElement('img');
          img.src = window.URL.createObjectURL(this.response);
          var p = $('#contributor-' + this.contributor_id);
          p.attr("href", this.contributor.html_url);
          p.append(img);
          p.append(this.contributor.login);
        };
        xhr.send();        
      }
    });
  });

  // Load settings
  $("#page-settings select, #page-settings input").each(function(){
    var parameter = $(this).attr("name");
    mooltipass.device.interface.send({
        'command': 'getMooltipassParameter',
        'parameter': parameter,
        'callbackFunction': function(_response) {
            console.log(parameter + ':' + _response);
            // TODO #as: show parameter in ui
        },
        'callbackParameters': null
    });    
  });

  // Save settings on change
  $("#page-settings select, #page-settings input").on('change keyup', function(){
    key = $(this).attr("name");
    if ($(this).attr('type') == 'checkbox') {
      value = this.checked;
    } else {
      value = $(this).val();
    }

    // TODO #as: send parameters to device
    console.log(key + ": " + value);

  });
});