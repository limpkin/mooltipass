var _console_log = console.log;

console.log = function() {
  text = arguments[0];
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

$(function(){
  // Only show app, if mp is connected
  // _mp.isConnected  
  setInterval(function(){
    if (mooltipass.device.isConnected) {
      $(".hide-if-connected").hide();
      $(".show-if-connected").show();
    } else {
      $(".hide-if-connected").show();
      $(".show-if-connected").hide();      
    }
  }, 500);

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

        var xhr = new XMLHttpRequest();
        xhr.open('GET', contributor.avatar_url, true);
        xhr.contributor = contributor;
        xhr.responseType = 'blob';
        xhr.onload = function(e) {
          var img = document.createElement('img');
          img.src = window.URL.createObjectURL(this.response);
          var p = $('<a class="column small-4 contributor">');
          p.attr("href", this.contributor.html_url);
          p.append(img);
          p.append(this.contributor.login);
          $("#contributor-list").append(p);
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
            console.log(parameter + ':', _response);
        },
        'callbackParameters': undefined
    });    
  });

});