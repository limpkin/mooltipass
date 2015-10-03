pressedKeys = [];

var is_key_pressed = function(key) {
  if (key in pressedKeys) return pressedKeys[key];
  return false
}
var is_keyset_pressed = function(keys) {
  var result = true;
  for (_key in keys) {
    var key = parseInt(keys[_key]);
    result = result && is_key_pressed(key);
  }
  return result;
}

$(document.body).on("keydown keyup", function(e){
  pressedKeys[e.keyCode] = (e.type == 'keydown');
});

$(document.body).on("keydown", function(e){
  $('*[mp-show-for-hotkey]').each(function(){
    keys = $(this).attr('mp-show-for-hotkey').split(",");
    if (is_keyset_pressed(keys)) {
      $(this).css("display", "inherit");
    }    
  });

  $("*[mp-focus-on-hotkey]").each(function() {
    var keysets = $(this).attr("mp-focus-on-hotkey").split(";");
    for (_keyset in keysets) {
      var keyset = keysets[_keyset].split(",");
      if (is_keyset_pressed(keyset)) {
        $(this).focus();
      }  
    }
  });
});