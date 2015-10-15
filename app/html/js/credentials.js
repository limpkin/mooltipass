var _cred = {};

DEFAULT_PASSWORD = "••••••••";
USER_CREDENTIALS = [];
WAITING_FOR_DEVICE_LABEL = '<i class="fa fa-spin fa-circle-o-notch"></i> waiting for device';

var MONTH_NAMES = [
  "January", "February", "March",
  "April", "May", "June", "July",
  "August", "September", "October",
  "November", "December"
];

RECENT_DOUBLECLICK = false;
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
    var app = $row.find(".app span").attr("data-value");
    var user = $row.find(".user span").attr("data-value");  

    return {
      "app" : app,
      "user" : user
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
  $table = $("#credentials").DataTable({
    data : get_user_credentials_for_table(),
    scrollY : 250,
    dom : '<t>',
    columns: [
      { data: "favourite" },
      {
        data: {
          "display": 'app.display',
          "_" : 'app.plain'
        }
      },
      {
        data: {
          "display": 'user.display',
          "_" : 'user.plain'
        }
      },
      { data: "password" },
      { data: "actions" }
    ]
  });

  update_data_values();

  return true;
};

_cred.onClickMMMEnter = function() {
  $(this).hide();
  $('#mmm-save, #mmm-discard').show();

  mooltipass.device.interface.send({
    'command': 'startMemoryManagementMode',
    'callbackFunction': _cred.loadCredentials
  });
};


$(function(){
  $('#mmm-save, #mmm-discard').hide();

  // Search for credentials
  $("#search-input").on("keyup change", function(){
    $("#credentials").DataTable().search($(this).val()).draw()
  });
  $("#search-input").on("keyup", function(e){
    if (e.keyCode == 27) $(this).val("").trigger("change");
  });

  // Table actions
  //  Show password
  $(".fa-eye").on('click', function(e){
    var $parent = $(this).parents("tr");
    credentials = get_credentials_from_row($parent);
    var app = credentials.app;
    var user = credentials.user;

    $parent.find(".password span").html(WAITING_FOR_DEVICE_LABEL);
    get_password(app, user, function(password) {
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
    credentials = get_credentials_from_row($parent);
    var app = credentials.app;
    var user = credentials.user;

    for (_credential in USER_CREDENTIALS) {
      if ((USER_CREDENTIALS[_credential].app == app) && (USER_CREDENTIALS[_credential].user == user)) {  
        USER_CREDENTIALS[_credential].is_favourite = !USER_CREDENTIALS[_credential].is_favourite;
      }
    }     
    $(this).toggleClass("fa-star-o fa-star");

    e.stopPropagation();
  });
  //  Delete credentials
  $(".fa-trash-o").on('click', function(e) {
    var $parent = $(this).parents("tr");
    credentials = get_credentials_from_row($parent);
    var app = credentials.app;
    var user = credentials.user;

    for (_credential in USER_CREDENTIALS) {
      var credential = USER_CREDENTIALS[_credential];
      if ((credential.app == app) && (credential.user == user)) {  
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

    credentials = get_credentials_from_row($parent);
    var app = credentials.app;
    var user = credentials.user;    

    var $app = $parent.find(".app span");
    var $user = $parent.find(".user span");
    var $password = $parent.find(".password span");

    $password.html(WAITING_FOR_DEVICE_LABEL);
    get_password(app, user, function(password) {
      $app.html("<input class='inline change-credentials' data-old='" + app + "' value='" + app + "'/>");
      $user.html("<input class='inline change-credentials' data-old='" + user + "' value='" + user + "'/>");
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

    $parent = $(this).parents("tr");

    var old_app = $parent.find(".app input").attr("data-old");
    var new_app = $parent.find(".app input").val();
    var old_user = $parent.find(".user input").attr("data-old");
    var new_user = $parent.find(".user input").val();
    var old_password = $parent.find(".password input").attr("data-old");
    var new_password = $parent.find(".password input").val();  

    for (_credential in USER_CREDENTIALS) {
      var credential = USER_CREDENTIALS[_credential];
      if ((credential.app == old_app) && (credential.user == old_user)) {  
        USER_CREDENTIALS[_credential].app = new_app;
        USER_CREDENTIALS[_credential].password = new_password;
        USER_CREDENTIALS[_credential].user = new_user;
      }
    }     

    $parent.find(".fa-pencil").show();
    $parent.find(".fa-eye").show();
    $parent.find(".fa-trash-o").show();
    $parent.find(".fa-floppy-o").hide();       
    $parent.find(".fa-times").hide();       

    $parent.find(".app span").html(new_app).attr("data-value", new_app);
    $parent.find(".user span").html(new_user).attr("data-value", new_user);
    $parent.find(".password span").html(DEFAULT_PASSWORD); 

    e.stopPropagation();    
  }
  $(".fa-floppy-o").on("click", save_credential_changes);
  //  Discard credentials
  var discard_credential_changes = function(e) {
    if ((e.type == "keydown") && (e.keyCode != 27)) return;

    update_data_values();
    $parent = $(this).parents("tr");

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
      var app = credentials.app;
      var user = credentials.user;

      var credential_details = get_credential_infos(app, user);

      var now = new Date();
      var date = new Date(credential_details.last_used);
      last_used = MONTH_NAMES[date.getMonth()] + " " + date.getDay();
      if (now - date > 365*24*60*60*1000) last_used += ", " + date.getFullYear();

      var date = new Date(credential_details.last_modified);
      last_modified = MONTH_NAMES[date.getMonth()] + " " + date.getDay();
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


});

var get_user_credentials_for_table = function() {
  //var credentials = JSON.parse(JSON.stringify(USER_CREDENTIALS));
  for (_credential in credentials) {
    credentials[_credential].password = "<span data-value='" + DEFAULT_PASSWORD + "''></span>";
    credentials[_credential].app = {
      "display" : "<span data-value='" + credentials[_credential].app + "'></span>",
      "plain" : credentials[_credential].app
    };
    credentials[_credential].user = {
      "display" : "<span data-value='" + credentials[_credential].user + "'></span>",
      "plain" : credentials[_credential].user
    };
    credentials[_credential].actions = '<nobr><i class="fa fa-eye" title="Show password"></i>\
      <i class="fa fa-eye-slash" style="display:none;" title="Hide password"></i>\
      <i class="fa fa-pencil" title="Edit credentials"></i>\
      <i class="fa fa-trash-o" title="Delete credentials"></i>\
      <i class="fa fa-times" style="display:none;" title="Discard changes"></i>\
      <i class="fa fa-floppy-o" style="display:none;" title="Save credentials"></i></nobr>';

    var now = new Date();
    var date = new Date(credentials[_credential].last_used);
    credentials[_credential].last_used = MONTH_NAMES[date.getMonth()] + " " + date.getDay();
    if (now - date > 365*24*60*60*1000) {
      credentials[_credential].last_used += ", " + (date.getYear() % 100);
    }
    var date = new Date(credentials[_credential].last_modified);
    credentials[_credential].last_modified = MONTH_NAMES[date.getMonth()] + " " + date.getDay();
    if (now - date > 365*24*60*60*1000) {
      credentials[_credential].last_modified += ", " + (date.getYear() % 100);
    }

    if (credentials[_credential].is_favourite) {
      credentials[_credential].favourite = '<i class="fa fa-star" title="Remove from favourites"></i>';
    } else {
      credentials[_credential].favourite = '<i class="fa fa-star-o" title="Add to favourites"></i>';
    }
  }
  return credentials
}

var get_credential_infos = function(app, user) {
  for (_credential in USER_CREDENTIALS) {
    var credential = USER_CREDENTIALS[_credential];
    if ((credential.app == app) && (credential.user == user)) {  
      return credential
    }
  }   
}

var get_password = function(app, user, callback) {
  for (_credential in USER_CREDENTIALS) {
    var credential = USER_CREDENTIALS[_credential];
    if ((credential.app == app) && (credential.user == user)) {  
      if ("password" in credential) {
        callback(credential.password);
        return
      }
    }
  } 

  mooltipass.app.get_password(app, user, function(password) {
    // Add password to local user credential data
    for (_credential in USER_CREDENTIALS) {
      var credential = USER_CREDENTIALS[_credential];
      if ((credential.app == app) && (credential.user == user)) {  
        USER_CREDENTIALS[_credential].password = password;
      }
    } 

    // Call original callback
    callback(password);
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
  "app": "youtube.com", 
  "user" : "tautbaps",
  "description" : "tautbaps_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "baidu.com", 
  "user" : "bugsnevis",
  "description" : "bugsnevis_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "yahoo.com", 
  "user" : "pumlumonyork",
  "description" : "pumlumonyork_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "amazon.com", 
  "user" : "lumpno",
  "description" : "lumpno_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "wikipedia.org", 
  "user" : "projectjumbled",
  "description" : "projectjumbled_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "qq.com", 
  "user" : "wingsredundant",
  "description" : "wingsredundant_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2013, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "twitter.com", 
  "user" : "bindchemistry",
  "description" : "bindchemistry_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.co.in", 
  "user" : "translatorgalilean",
  "description" : "translatorgalilean_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "taobao.com", 
  "user" : "irkedexciting",
  "description" : "irkedexciting_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "live.com", 
  "user" : "spokendates",
  "description" : "spokendates_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "sina.com.cn", 
  "user" : "jonesfreeze",
  "description" : "jonesfreeze_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "linkedin.com", 
  "user" : "cravegalvanising",
  "description" : "cravegalvanising_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "yahoo.co.jp", 
  "user" : "glazecooked",
  "description" : "glazecooked_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "weibo.com", 
  "user" : "genesassynt",
  "description" : "genesassynt_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "ebay.com", 
  "user" : "slangblob",
  "description" : "slangblob_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.co.jp", 
  "user" : "porphyrymetacarpus",
  "description" : "porphyrymetacarpus_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "yandex.ru", 
  "user" : "geocachingoften",
  "description" : "geocachingoften_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "blogspot.com", 
  "user" : "simplicityphp",
  "description" : "simplicityphp_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "vk.com", 
  "user" : "uncommonvisiting",
  "description" : "uncommonvisiting_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "hao123.com", 
  "user" : "beingpiles",
  "description" : "beingpiles_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "t.co", 
  "user" : "zestyhercules",
  "description" : "zestyhercules_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.de", 
  "user" : "whooshlanguage",
  "description" : "whooshlanguage_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "bing.com", 
  "user" : "legohyundai",
  "description" : "legohyundai_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "msn.com", 
  "user" : "ovalnecessary",
  "description" : "ovalnecessary_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "instagram.com", 
  "user" : "dbasegroan",
  "description" : "dbasegroan_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "amazon.co.jp", 
  "user" : "pepperthin",
  "description" : "pepperthin_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.co.uk", 
  "user" : "sorrylens",
  "description" : "sorrylens_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "reddit.com", 
  "user" : "warblingdartboard",
  "description" : "warblingdartboard_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "tmall.com", 
  "user" : "navierbending",
  "description" : "navierbending_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "aliexpress.com", 
  "user" : "immodestpukao",
  "description" : "immodestpukao_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "microsoft.com", 
  "user" : "deanmacedonian",
  "description" : "deanmacedonian_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.fr", 
  "user" : "fraidadoption",
  "description" : "fraidadoption_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.com.br", 
  "user" : "bloodwhoop",
  "description" : "bloodwhoop_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "ask.com", 
  "user" : "crumbleclipclop",
  "description" : "crumbleclipclop_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "pinterest.com", 
  "user" : "respectwimpled",
  "description" : "respectwimpled_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Mail.Ru", 
  "user" : "terbiumaustralis",
  "description" : "terbiumaustralis_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "wordpress.com", 
  "user" : "butterbowls",
  "description" : "butterbowls_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "tumblr.com", 
  "user" : "dacapopepper",
  "description" : "dacapopepper_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "imgur.com", 
  "user" : "glutinoussheep",
  "description" : "glutinoussheep_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Onclickads.net", 
  "user" : "dialectburger",
  "description" : "dialectburger_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "PayPal.com", 
  "user" : "hoodprotest",
  "description" : "hoodprotest_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "sohu.com", 
  "user" : "softballmimicry",
  "description" : "softballmimicry_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.ru", 
  "user" : "costaricanburns",
  "description" : "costaricanburns_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "xvideos.com", 
  "user" : "conjunctionpeer",
  "description" : "conjunctionpeer_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "imdb.com", 
  "user" : "alkanekame",
  "description" : "alkanekame_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "apple.com", 
  "user" : "squalidyawn",
  "description" : "squalidyawn_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.it", 
  "user" : "rieslingmopping",
  "description" : "rieslingmopping_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.es", 
  "user" : "stuffingshrug",
  "description" : "stuffingshrug_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "fc2.com", 
  "user" : "selfishunwritten",
  "description" : "selfishunwritten_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Googleadservices.com", 
  "user" : "cribpolitical",
  "description" : "cribpolitical_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "amazon.de", 
  "user" : "highlightprovided",
  "description" : "highlightprovided_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "netflix.com", 
  "user" : "partridgereservoir",
  "description" : "partridgereservoir_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "stackoverflow.com", 
  "user" : "studyingkey",
  "description" : "studyingkey_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "360.cn", 
  "user" : "kuipertwang",
  "description" : "kuipertwang_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "craigslist.org", 
  "user" : "farmeraxiomatic",
  "description" : "farmeraxiomatic_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Tianya.cn", 
  "user" : "motherpretend",
  "description" : "motherpretend_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.ca", 
  "user" : "tendoncycle",
  "description" : "tendoncycle_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "ok.ru", 
  "user" : "coteriedirect",
  "description" : "coteriedirect_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "diply.com", 
  "user" : "primesobtuse",
  "description" : "primesobtuse_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Google.com.mx", 
  "user" : "stateassure",
  "description" : "stateassure_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "alibaba.com", 
  "user" : "physicalvenerable",
  "description" : "physicalvenerable_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "pornhub.com", 
  "user" : "kiloparsecoccipital",
  "description" : "kiloparsecoccipital_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.com.hk", 
  "user" : "ashurtful",
  "description" : "ashurtful_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Naver.com", 
  "user" : "valencepreseli",
  "description" : "valencepreseli_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "adcash.com", 
  "user" : "poloaustralis",
  "description" : "poloaustralis_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "amazon.co.uk", 
  "user" : "guillemotlope",
  "description" : "guillemotlope_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "rakuten.co.jp", 
  "user" : "galileansamosa",
  "description" : "galileansamosa_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "xhamster.com", 
  "user" : "technologyducks",
  "description" : "technologyducks_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Outbrain.com", 
  "user" : "braveimaginable",
  "description" : "braveimaginable_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Booking.com", 
  "user" : "mixtapechoux",
  "description" : "mixtapechoux_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Kat.cr", 
  "user" : "contentscovet",
  "description" : "contentscovet_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "soso.com", 
  "user" : "earlobesneaky",
  "description" : "earlobesneaky_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "adobe.com", 
  "user" : "risetranslate",
  "description" : "risetranslate_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Nicovideo.jp", 
  "user" : "teenagewhicker",
  "description" : "teenagewhicker_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "cnn.com", 
  "user" : "wightdamocloids",
  "description" : "wightdamocloids_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "flipkart.com", 
  "user" : "dissmarzipan",
  "description" : "dissmarzipan_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Chinadaily.com.cn", 
  "user" : "absorbingcrest",
  "description" : "absorbingcrest_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "go.com", 
  "user" : "chartcurium",
  "description" : "chartcurium_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "ebay.de", 
  "user" : "anorexiacooking",
  "description" : "anorexiacooking_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.com.au", 
  "user" : "stoveprefer",
  "description" : "stoveprefer_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.co.id", 
  "user" : "etherealcagey",
  "description" : "etherealcagey_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Xinhuanet.com", 
  "user" : "biathlonflanked",
  "description" : "biathlonflanked_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "google.pl", 
  "user" : "taughtmoons",
  "description" : "taughtmoons_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "bbc.co.uk", 
  "user" : "testifyattack",
  "description" : "testifyattack_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "pixnet.net", 
  "user" : "forceporcus",
  "description" : "forceporcus_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "People.com.cn", 
  "user" : "diplomasdysprosium",
  "description" : "diplomasdysprosium_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "googleusercontent.com", 
  "user" : "jamanalytical",
  "description" : "jamanalytical_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Github.com", 
  "user" : "bivyfiles",
  "description" : "bivyfiles_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Cntv.cn", 
  "user" : "divideplayer",
  "description" : "divideplayer_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 08, 01),
  "is_favourite" : false
},
{ 
  "app": "dailymotion.com", 
  "user" : "igloorough",
  "description" : "igloorough_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "gmw.cn", 
  "user" : "daltonslimeball",
  "description" : "daltonslimeball_d",
  "last_used" : new Date(2015, 09, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Amazon.in", 
  "user" : "strictlyeach",
  "description" : "strictlyeach_d",
  "last_used" : new Date(2015, 07, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "dropbox.com", 
  "user" : "cutsdiggers",
  "description" : "cutsdiggers_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 06, 01),
  "is_favourite" : false
},
{ 
  "app": "Google.co.kr", 
  "user" : "innocentfusty",
  "description" : "innocentfusty_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Wikia.com", 
  "user" : "whackmug",
  "description" : "whackmug_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "Dailymail.co.uk", 
  "user" : "withinreferred",
  "description" : "withinreferred_d",
  "last_used" : new Date(2015, 08, 01),
  "last_modified" : new Date(2015, 07, 01),
  "is_favourite" : false
},
{ 
  "app": "163.com", 
  "user" : "fundsvug",
  "description" : "fundsvug_d",
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

USER_CREDENTIALS = fakedata;