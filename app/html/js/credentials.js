var _cred = {};

var DEFAULT_PASSWORD = "••••••••";
var USER_CREDENTIALS = [];
var WAITING_FOR_DEVICE_LABEL = '<i class="fa fa-spin fa-circle-o-notch"></i> waiting for device';

var MONTH_NAMES = [
  "January", "February", "March",
  "April", "May", "June", "July",
  "August", "September", "October",
  "November", "December"
];

var RECENT_DOUBLECLICK = false;
var dblclick_last_500ms = function() {
  RECENT_DOUBLECLICK = true;
  setTimeout(function(){
    RECENT_DOUBLECLICK = false;
  }, 500);
}

var stop_propagation = function(e){
  e.stopPropagation();
}

var get_credentials_from_row = function($row) {
    var context = $row.find(".context span").attr("data-value");
    var username = $row.find(".username span").attr("data-value");

    return {
      "context" : context,
      "username" : username
    }
} 

var update_data_values = function() {
  $("*[data-value]").each(function(){
    $(this).html($(this).attr("data-value"));
  });
}


_cred.loadCredentials = function(_status, _credentials) {
  if(!_status.success) {
    // TODO: Could not retrieve credentials from device
    return false;
  }

  USER_CREDENTIALS = _credentials;

  // Init credentials table
  var $table = $("#credentials").DataTable({
    data : get_user_credentials_for_table(),
    scrollY : 250,
    dom : '<t>',
    columns: [
      { data: "favorite" },
      {
        data: {
          "display": 'context.display',
          "_" : 'context.plain'
        }
      },
      {
        data: {
          "display": 'username.display',
          "_" : 'username.plain'
        }
      },
      { data: "password" },
      { data: "actions" }
    ]
  });

  update_data_values();
  _cred.initializeTableActions();

  return true;
};

_cred.initializeTableActions = function() {
  // Table actions

  //  Show password
  $(".fa-eye").on('click', function(e){
    var $parent = $(this).parents("tr");
    var credentials = get_credentials_from_row($parent);
    var context = credentials.context;
    var username = credentials.username;

    $parent.find(".password span").html(WAITING_FOR_DEVICE_LABEL);
    get_password(context, username, function(password) {
      $parent.find(".password span").html(password);
      $parent.find(".fa-eye").hide();
      $parent.find(".fa-eye-slash").show();
    });

    e.stopPropagation();
  });

  //  Hide password
  $(".fa-eye-slash").on('click', function(e){
    $(this).parents("tr").find(".password span").html(DEFAULT_PASSWORD);
    $(this).parents("tr").find(".fa-eye-slash").hide();
    $(this).parents("tr").find(".fa-eye").show();

    e.stopPropagation();
  });

  //  Add to / remove from favourites
  $("tbody .fa-star-o, tbody .fa-star").on('click', function(e){
    var $parent = $(this).parents("tr");
    var credentials = get_credentials_from_row($parent);
    var context = credentials.context;
    var username = credentials.username;

    for (var _credential in USER_CREDENTIALS) {
      if ((USER_CREDENTIALS[_credential].context == context) && (USER_CREDENTIALS[_credential].username == username)) {
        USER_CREDENTIALS[_credential].favorite = !USER_CREDENTIALS[_credential].favorite;
      }
    }
    $(this).toggleClass("fa-star-o fa-star");

    e.stopPropagation();
  });
  //  Delete credentials
  $(".fa-trash-o").on('click', function(e) {
    var $parent = $(this).parents("tr");
    var credentials = get_credentials_from_row($parent);
    var context = credentials.context;
    var username = credentials.username;

    for (var _credential in USER_CREDENTIALS) {
      var credential = USER_CREDENTIALS[_credential];
      if ((credential.context == context) && (credential.username == username)) {
        USER_CREDENTIALS.splice(_credential, 1);
      }
    }
    if ($parent.hasClass("active")) $(".credential-details").remove();
    $parent.remove();

    e.stopPropagation();
  });

  //  Edit credentials
  var edit_credentials = function (e) {
    var $parent = $(this).parents("tr");
    var $this = $(this);

    // Return if already in edit mode
    if ($parent.find("input").length > 0) return;

    var credentials = get_credentials_from_row($parent);
    var context = credentials.context;
    var username = credentials.username;

    var $app = $parent.find(".context span");
    var $user = $parent.find(".username span");
    var $password = $parent.find(".password span");

    $password.html(WAITING_FOR_DEVICE_LABEL);
    get_password(context, username, function(password) {
      $app.html("<input class='inline change-credentials' data-old='" + context + "' value='" + context + "'/>");
      $user.html("<input class='inline change-credentials' data-old='" + username + "' value='" + username + "'/>");
      $password.html("<input class='inline change-credentials' data-old='" + password + "' value='" + password + "'/>");
      $(".inline.change-credentials").on('keydown', save_credential_changes);
      $(".inline.change-credentials").on('keydown', discard_credential_changes);
      $(".inline.change-credentials").on('click', stop_propagation);

      $parent.find(".fa-pencil").hide();
      $parent.find(".fa-eye").hide();
      $parent.find(".fa-eye-slash").hide();
      $parent.find(".fa-trash-o").hide();
      $parent.find(".fa-floppy-o").show();
      $parent.find(".fa-times").show();

      if (e.type = 'dblclick') {
        $this.find('input').focus();
      } else {
        $app.find('input').focus();
      }
    });

    e.stopPropagation();
  }
  $(".fa-pencil").on('click', edit_credentials);
  $("tbody tr td.editable").on('dblclick', edit_credentials);
  $("tbody tr td.editable").on('dblclick', dblclick_last_500ms);

  //  Save credentials
  var save_credential_changes = function(e) {
    if ((e.type == "keydown") && (e.keyCode != 13)) return;

    var $parent = $(this).parents("tr");

    var old_context = $parent.find(".context input").attr("data-old");
    var new_context = $parent.find(".context input").val();
    var old_username = $parent.find(".username input").attr("data-old");
    var new_username = $parent.find(".username input").val();
    var old_password = $parent.find(".password input").attr("data-old");
    var new_password = $parent.find(".password input").val();

    for (var _credential in USER_CREDENTIALS) {
      var credential = USER_CREDENTIALS[_credential];
      if ((credential.context == old_context) && (credential.username == old_username)) {
        USER_CREDENTIALS[_credential].context = new_context;
        USER_CREDENTIALS[_credential].password = new_password;
        USER_CREDENTIALS[_credential].username = new_username;
      }
    }

    $parent.find(".fa-pencil").show();
    $parent.find(".fa-eye").show();
    $parent.find(".fa-trash-o").show();
    $parent.find(".fa-floppy-o").hide();
    $parent.find(".fa-times").hide();

    $parent.find(".context span").html(new_context).attr("data-value", new_context);
    $parent.find(".username span").html(new_username).attr("data-value", new_username);
    $parent.find(".password span").html(DEFAULT_PASSWORD);

    e.stopPropagation();
  }
  $(".fa-floppy-o").on("click", save_credential_changes);

  //  Discard credentials
  var discard_credential_changes = function(e) {
    if ((e.type == "keydown") && (e.keyCode != 27)) return;

    update_data_values();
    var $parent = $(this).parents("tr");

    $parent.find(".fa-pencil").show();
    $parent.find(".fa-eye").show();
    $parent.find(".fa-trash-o").show();
    $parent.find(".fa-floppy-o").hide();
    $parent.find(".fa-times").hide();

    e.stopPropagation();
  }
  $(".fa-times").on("click", discard_credential_changes);

  //  View details (description / last used / last modified)
  var update_details_view = function() {
    $(".credential-details").remove();
    if ($(".active").length > 0) {
      var credentials = get_credentials_from_row($(".active"));
      var context = credentials.context;
      var username = credentials.username;

      var credential_details = get_credential_infos(context, username);

      var now = new Date();
      var date = new Date(credential_details.date_lastused);
      var last_used = MONTH_NAMES[date.getMonth()] + " " + date.getDay();
      if (now - date > 365*24*60*60*1000) last_used += ", " + date.getFullYear();

      var date = new Date(credential_details.date_modified);
      var last_modified = MONTH_NAMES[date.getMonth()] + " " + date.getDay();
      if (now - date > 365*24*60*60*1000) last_modified += ", " + date.getFullYear();

      $(".active").after('<tr class="active credential-details"><td colspan=2></td><td class="labels">\
        <p>Last used</p>\
        <p>Last modified</p>\
        <p>Description</p>\
      </td><td colspan=2>\
      <p>' + last_used + '</p>\
      <p>' + last_modified + '</p>\
      <p>lorem ipsum<p>\
      </td></tr>');

      //$("#credentials_wrapper").insertAfter("<div>hallo welt</div>");

    }
  }
  var display_details = function(e) {
    $this = $(this);
    setTimeout(function(){
      if (RECENT_DOUBLECLICK) return;
      $(".credential-details").remove();
      if ($this.hasClass("active")) {
        $(".active").removeClass("active");
      } else {
        $(".active").removeClass("active");
        $this.addClass("active");
      }
      update_details_view();
    }, 300);

    e.stopPropagation();
  }
  $("tbody tr").on('click', display_details);
}

_cred.onClickMMMEnter = function() {
  $(this).hide();
  $('#mmm-save, #mmm-discard').show();

  mooltipass.device.interface.send({
    'command': 'startMemoryManagementMode',
    'callbackFunction': _cred.loadCredentials
  });
};

_cred.onClickMMMDiscard = function(e) {
  console.warn('call method to leave MemoryManagementMode');
  // TODO: call this method on callback function
  USER_CREDENTIALS = [];
  var $table = $('#credentials').DataTable();
  $table.clear().draw();
  $('#mmm-save, #mmm-discard').hide();
  $('#mmm-enter').show();
}


$(function(){
  $('#mmm-enter').click(_cred.onClickMMMEnter);
  $('#mmm-discard').click(_cred.onClickMMMDiscard);
  $('#mmm-save, #mmm-discard').hide();

  // Search for credentials
  $("#search-input").on("keyup change", function(){
    $("#credentials").DataTable().search($(this).val()).draw()
  });
  $("#search-input").on("keyup", function(e){
    if (e.keyCode == 27) $(this).val("").trigger("change");
  });


});

var get_user_credentials_for_table = function() {
  //var credentials = JSON.parse(JSON.stringify(USER_CREDENTIALS));
  var credentials = [];
  for (var _credential in USER_CREDENTIALS) {
    credentials[_credential] = {};
    credentials[_credential].address = USER_CREDENTIALS[_credential].address;
    credentials[_credential].parent_address = USER_CREDENTIALS[_credential].parent_address;
    credentials[_credential].password = "<span data-value='" + DEFAULT_PASSWORD + "''></span>";
    credentials[_credential].context = {
      "display" : "<span data-value='" + USER_CREDENTIALS[_credential].context + "'></span>",
      "plain" : USER_CREDENTIALS[_credential].context
    };
    credentials[_credential].username = {
      "display" : "<span data-value='" + USER_CREDENTIALS[_credential].username + "'></span>",
      "plain" : USER_CREDENTIALS[_credential].username
    };
    credentials[_credential].actions = '<nobr><i class="fa fa-eye" title="Show password"></i>\
      <i class="fa fa-eye-slash" style="display:none;" title="Hide password"></i>\
      <i class="fa fa-pencil" title="Edit credentials"></i>\
      <i class="fa fa-trash-o" title="Delete credentials"></i>\
      <i class="fa fa-times" style="display:none;" title="Discard changes"></i>\
      <i class="fa fa-floppy-o" style="display:none;" title="Save credentials"></i></nobr>';

    var now = new Date();
    var date = new Date(USER_CREDENTIALS[_credential].date_lastused);
    credentials[_credential].date_lastused = MONTH_NAMES[date.getMonth()] + " " + date.getDay();
    if (now - date > 365*24*60*60*1000) {
      credentials[_credential].date_lastused += ", " + (date.getYear() % 100);
    }
    var date = new Date(USER_CREDENTIALS[_credential].date_modified);
    credentials[_credential].date_modified = MONTH_NAMES[date.getMonth()] + " " + date.getDay();
    if (now - date > 365*24*60*60*1000) {
      credentials[_credential].date_modified += ", " + (date.getYear() % 100);
    }

    if (USER_CREDENTIALS[_credential].favorite) {
      credentials[_credential].favorite = '<i class="fa fa-star" title="Remove from favourites"></i>';
    } else {
      credentials[_credential].favorite = '<i class="fa fa-star-o" title="Add to favourites"></i>';
    }
  }

  return credentials;
}

var get_credential_infos = function(_context, _username) {
  for (var _credential in USER_CREDENTIALS) {
    var credential = USER_CREDENTIALS[_credential];
    if ((credential.context == _context) && (credential.username == _username)) {
      return credential
    }
  }   
}

var get_password = function(_context, _username, _callback) {
  for (var _key in USER_CREDENTIALS) {
    var credential = USER_CREDENTIALS[_key];
    if ((credential.context == _context) && (credential.username == _username)) {
      if ("password" in credential) {
        _callback(credential.password);
        return
      }
    }
  } 

  mooltipass.app.get_password(_context, _username, function(password) {
    // Add password to local user credential data
    for (var _key in USER_CREDENTIALS) {
      var credential = USER_CREDENTIALS[_key];
      if ((credential.context == _context) && (credential.username == _username)) {
        USER_CREDENTIALS[_key].password = password;
      }
    } 

    // Call original callback
    _callback(password);
  });
}

fakedata = [{ 
  "app": "google.com", 
  "user" : "htmlchat",
  "description" : "htmlchat_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : true
},
{ 
  "app": "facebook.com", 
  "user" : "braceradish",
  "description" : "braceradish_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "blogger.com", 
  "user" : "henchmantle",
  "description" : "henchmantle_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
}];

//USER_CREDENTIALS = fakedata;