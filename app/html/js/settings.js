$(function(){
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