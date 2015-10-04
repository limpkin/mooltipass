DEFAULT_PASSWORD = "••••••••";
USER_CREDENTIALS = [];

$(function(){
  // Init credentials table
  $table = $("#credentials").DataTable({
    data : get_user_credentials_for_table(),
    scrollY : 200,
    dom : '<t>'
  });

  // Search for credentials
  $("#search-input").on("keyup change", function(){
    $("#credentials").DataTable().search($(this).val()).draw()
  });
  $("#search-input").on("keyup", function(e){
    if (e.keyCode == 27) $(this).val("").trigger("change");
  });

  // Table actions
  //  Show password
  $(".fa-eye").on('click', function(){
    var app = $(this).parents("tr").find(".app").html();
    var user = $(this).parents("tr").find(".user").html();
    password = get_password(app, user);
    $(this).parents("tr").find(".password").html(password);
    $(this).parents("tr").find(".fa-eye").hide();
    $(this).parents("tr").find(".fa-eye-slash").show();
  });
  //  Hide password
  $(".fa-eye-slash").on('click', function(){
    $(this).parents("tr").find(".password").html(DEFAULT_PASSWORD);
    $(this).parents("tr").find(".fa-eye-slash").hide();
    $(this).parents("tr").find(".fa-eye").show();
  });
  //  Add to / remove from favourites
  $("tbody .fa-star-o, tbody .fa-star").on('click', function(){
    var app = $(this).parents("tr").find(".app").html();
    var user = $(this).parents("tr").find(".user").html(); 
    for (_credential in USER_CREDENTIALS) {
      if ((USER_CREDENTIALS[_credential].app == app) && (USER_CREDENTIALS[_credential].user == user)) {  
        USER_CREDENTIALS[_credential].is_favourite = !USER_CREDENTIALS[_credential].is_favourite;
      }
    }     
    $(this).toggleClass("fa-star-o fa-star");
  });
  //  Delete credentials
  $(".fa-times").on('click', function() {
    var app = $(this).parents("tr").find(".app").html();
    var user = $(this).parents("tr").find(".user").html(); 
    for (_credential in USER_CREDENTIALS) {
      var credential = USER_CREDENTIALS[_credential];
      if ((credential.app == app) && (credential.user == user)) {  
        USER_CREDENTIALS.splice(_credential, 1);
      }
    }
    $(this).parents("tr").remove(); 
  });
  //  Edit credentials
  var edit_credentials = function (e) {  
    var $app = $(this).parents("tr").find(".app");
    var app = $app.html();
    var $user = $(this).parents("tr").find(".user");
    var user = $user.html();
    var $password = $(this).parents("tr").find(".password");
    var password = get_password(app, user);

    $app.html("<input class='inline change-credentials' data-old='" + app + "' value='" + app + "'/>");
    $user.html("<input class='inline change-credentials' data-old='" + user + "' value='" + user + "'/>");
    $password.html("<input class='inline change-credentials' data-old='" + password + "' value='" + password + "'/>");
    $(".inline.change-credentials").on('keydown', save_credential_changes);

    $(this).parents("tr").find(".fa-pencil").hide();
    $(this).parents("tr").find(".fa-eye").hide();
    $(this).parents("tr").find(".fa-eye-slash").hide();
    $(this).parents("tr").find(".fa-times").hide();
    $(this).parents("tr").find(".fa-floppy-o").show();    

    if (e.type = 'dblclick') {
      $(this).find('input').focus();
    } else {
      $app.find('input').focus();
    }    
  }
  $(".fa-pencil").on('click', edit_credentials);
  $("tbody tr td").on('dblclick', edit_credentials);
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
    $parent.find(".fa-times").show();
    $parent.find(".fa-floppy-o").hide();       

    $parent.find(".app").html(new_app);
    $parent.find(".user").html(new_user);
    $parent.find(".password").html(DEFAULT_PASSWORD);     
  }
  $(".fa-floppy-o").on("click", save_credential_changes);
  $(".inline.change-credentials").on('keydown', save_credential_changes);

});

var get_user_credentials_for_table = function() {
  var credentials = JSON.parse(JSON.stringify(USER_CREDENTIALS));
  for (_credential in credentials) {
    credentials[_credential].password = DEFAULT_PASSWORD;
    credentials[_credential].actions = '<i class="fa fa-eye" title="Show password"></i>\
      <i class="fa fa-eye-slash" style="display:none;" title="Hide password"></i>\
      <i class="fa fa-pencil" title="Edit credentials"></i>\
      <i class="fa fa-floppy-o" style="display:none;" title="Save credentials"></i>\
      <i class="fa fa-times" title="Delete credentials"></i>';

    if (credentials[_credential].is_favourite) {
      credentials[_credential].favourite = '<i class="fa fa-star" title="Remove from favourites"></i>';
    } else {
      credentials[_credential].favourite = '<i class="fa fa-star-o" title="Add to favourites"></i>';
    }
  }
  return credentials
}

var get_password = function(app, user) {
  for (_credential in USER_CREDENTIALS) {
    var credential = USER_CREDENTIALS[_credential];
    if ((credential.app == app) && (credential.user == user)) {
      return credential.password;
    }
  }
  return ""
}

fakedata = [{ 
  "app": "google.com", 
  "user" : "htmlchat",
  "password" : "htmlc123at",
  "is_favourite" : true
},
{ 
  "app": "facebook.com", 
  "user" : "braceradish",
  "password" : "bracerad123sh",
  "is_favourite" : false
},
{ 
  "app": "youtube.com", 
  "user" : "tautbaps",
  "password" : "tautb123ps",
  "is_favourite" : false
},
{ 
  "app": "baidu.com", 
  "user" : "bugsnevis",
  "password" : "bugsne123is",
  "is_favourite" : false
},
{ 
  "app": "yahoo.com", 
  "user" : "pumlumonyork",
  "password" : "pumlumony123rk",
  "is_favourite" : false
},
{ 
  "app": "amazon.com", 
  "user" : "lumpno",
  "password" : "lum123no",
  "is_favourite" : false
},
{ 
  "app": "wikipedia.org", 
  "user" : "projectjumbled",
  "password" : "projectjumb123ed",
  "is_favourite" : false
},
{ 
  "app": "qq.com", 
  "user" : "wingsredundant",
  "password" : "wingsredund123nt",
  "is_favourite" : false
},
{ 
  "app": "twitter.com", 
  "user" : "bindchemistry",
  "password" : "bindchemis123ry",
  "is_favourite" : false
},
{ 
  "app": "google.co.in", 
  "user" : "translatorgalilean",
  "password" : "translatorgalil123an",
  "is_favourite" : false
},
{ 
  "app": "taobao.com", 
  "user" : "irkedexciting",
  "password" : "irkedexcit123ng",
  "is_favourite" : false
},
{ 
  "app": "live.com", 
  "user" : "spokendates",
  "password" : "spokenda123es",
  "is_favourite" : false
},
{ 
  "app": "sina.com.cn", 
  "user" : "jonesfreeze",
  "password" : "jonesfre123ze",
  "is_favourite" : false
},
{ 
  "app": "linkedin.com", 
  "user" : "cravegalvanising",
  "password" : "cravegalvanis123ng",
  "is_favourite" : false
},
{ 
  "app": "yahoo.co.jp", 
  "user" : "glazecooked",
  "password" : "glazecoo123ed",
  "is_favourite" : false
},
{ 
  "app": "weibo.com", 
  "user" : "genesassynt",
  "password" : "genesass123nt",
  "is_favourite" : false
},
{ 
  "app": "ebay.com", 
  "user" : "slangblob",
  "password" : "slangb123ob",
  "is_favourite" : false
},
{ 
  "app": "google.co.jp", 
  "user" : "porphyrymetacarpus",
  "password" : "porphyrymetacar123us",
  "is_favourite" : false
},
{ 
  "app": "yandex.ru", 
  "user" : "geocachingoften",
  "password" : "geocachingof123en",
  "is_favourite" : false
},
{ 
  "app": "blogspot.com", 
  "user" : "simplicityphp",
  "password" : "simplicity123hp",
  "is_favourite" : false
},
{ 
  "app": "vk.com", 
  "user" : "uncommonvisiting",
  "password" : "uncommonvisit123ng",
  "is_favourite" : false
},
{ 
  "app": "hao123.com", 
  "user" : "beingpiles",
  "password" : "beingpi123es",
  "is_favourite" : false
},
{ 
  "app": "t.co", 
  "user" : "zestyhercules",
  "password" : "zestyhercu123es",
  "is_favourite" : false
},
{ 
  "app": "google.de", 
  "user" : "whooshlanguage",
  "password" : "whooshlangu123ge",
  "is_favourite" : false
},
{ 
  "app": "bing.com", 
  "user" : "legohyundai",
  "password" : "legohyun123ai",
  "is_favourite" : false
},
{ 
  "app": "msn.com", 
  "user" : "ovalnecessary",
  "password" : "ovalnecess123ry",
  "is_favourite" : false
},
{ 
  "app": "instagram.com", 
  "user" : "dbasegroan",
  "password" : "dbasegr123an",
  "is_favourite" : false
},
{ 
  "app": "amazon.co.jp", 
  "user" : "pepperthin",
  "password" : "peppert123in",
  "is_favourite" : false
},
{ 
  "app": "google.co.uk", 
  "user" : "sorrylens",
  "password" : "sorryl123ns",
  "is_favourite" : false
},
{ 
  "app": "reddit.com", 
  "user" : "warblingdartboard",
  "password" : "warblingdartbo123rd",
  "is_favourite" : false
},
{ 
  "app": "tmall.com", 
  "user" : "navierbending",
  "password" : "navierbend123ng",
  "is_favourite" : false
},
{ 
  "app": "aliexpress.com", 
  "user" : "immodestpukao",
  "password" : "immodestpu123ao",
  "is_favourite" : false
},
{ 
  "app": "microsoft.com", 
  "user" : "deanmacedonian",
  "password" : "deanmacedon123an",
  "is_favourite" : false
},
{ 
  "app": "google.fr", 
  "user" : "fraidadoption",
  "password" : "fraidadopt123on",
  "is_favourite" : false
},
{ 
  "app": "google.com.br", 
  "user" : "bloodwhoop",
  "password" : "bloodwh123op",
  "is_favourite" : false
},
{ 
  "app": "ask.com", 
  "user" : "crumbleclipclop",
  "password" : "crumbleclipc123op",
  "is_favourite" : false
},
{ 
  "app": "pinterest.com", 
  "user" : "respectwimpled",
  "password" : "respectwimp123ed",
  "is_favourite" : false
},
{ 
  "app": "Mail.Ru", 
  "user" : "terbiumaustralis",
  "password" : "terbiumaustra123is",
  "is_favourite" : false
},
{ 
  "app": "wordpress.com", 
  "user" : "butterbowls",
  "password" : "butterbo123ls",
  "is_favourite" : false
},
{ 
  "app": "tumblr.com", 
  "user" : "dacapopepper",
  "password" : "dacapopep123er",
  "is_favourite" : false
},
{ 
  "app": "imgur.com", 
  "user" : "glutinoussheep",
  "password" : "glutinoussh123ep",
  "is_favourite" : false
},
{ 
  "app": "Onclickads.net", 
  "user" : "dialectburger",
  "password" : "dialectbur123er",
  "is_favourite" : false
},
{ 
  "app": "PayPal.com", 
  "user" : "hoodprotest",
  "password" : "hoodprot123st",
  "is_favourite" : false
},
{ 
  "app": "sohu.com", 
  "user" : "softballmimicry",
  "password" : "softballmimi123ry",
  "is_favourite" : false
},
{ 
  "app": "google.ru", 
  "user" : "costaricanburns",
  "password" : "costaricanbu123ns",
  "is_favourite" : false
},
{ 
  "app": "xvideos.com", 
  "user" : "conjunctionpeer",
  "password" : "conjunctionp123er",
  "is_favourite" : false
},
{ 
  "app": "imdb.com", 
  "user" : "alkanekame",
  "password" : "alkanek123me",
  "is_favourite" : false
},
{ 
  "app": "apple.com", 
  "user" : "squalidyawn",
  "password" : "squalidy123wn",
  "is_favourite" : false
},
{ 
  "app": "google.it", 
  "user" : "rieslingmopping",
  "password" : "rieslingmopp123ng",
  "is_favourite" : false
},
{ 
  "app": "google.es", 
  "user" : "stuffingshrug",
  "password" : "stuffingsh123ug",
  "is_favourite" : false
},
{ 
  "app": "fc2.com", 
  "user" : "selfishunwritten",
  "password" : "selfishunwrit123en",
  "is_favourite" : false
},
{ 
  "app": "Googleadservices.com", 
  "user" : "cribpolitical",
  "password" : "cribpoliti123al",
  "is_favourite" : false
},
{ 
  "app": "amazon.de", 
  "user" : "highlightprovided",
  "password" : "highlightprovi123ed",
  "is_favourite" : false
},
{ 
  "app": "netflix.com", 
  "user" : "partridgereservoir",
  "password" : "partridgereserv123ir",
  "is_favourite" : false
},
{ 
  "app": "stackoverflow.com", 
  "user" : "studyingkey",
  "password" : "studying123ey",
  "is_favourite" : false
},
{ 
  "app": "360.cn", 
  "user" : "kuipertwang",
  "password" : "kuipertw123ng",
  "is_favourite" : false
},
{ 
  "app": "craigslist.org", 
  "user" : "farmeraxiomatic",
  "password" : "farmeraxioma123ic",
  "is_favourite" : false
},
{ 
  "app": "Tianya.cn", 
  "user" : "motherpretend",
  "password" : "motherpret123nd",
  "is_favourite" : false
},
{ 
  "app": "google.ca", 
  "user" : "tendoncycle",
  "password" : "tendoncy123le",
  "is_favourite" : false
},
{ 
  "app": "ok.ru", 
  "user" : "coteriedirect",
  "password" : "coteriedir123ct",
  "is_favourite" : false
},
{ 
  "app": "diply.com", 
  "user" : "primesobtuse",
  "password" : "primesobt123se",
  "is_favourite" : false
},
{ 
  "app": "Google.com.mx", 
  "user" : "stateassure",
  "password" : "stateass123re",
  "is_favourite" : false
},
{ 
  "app": "alibaba.com", 
  "user" : "physicalvenerable",
  "password" : "physicalvenera123le",
  "is_favourite" : false
},
{ 
  "app": "pornhub.com", 
  "user" : "kiloparsecoccipital",
  "password" : "kiloparsecoccipi123al",
  "is_favourite" : false
},
{ 
  "app": "google.com.hk", 
  "user" : "ashurtful",
  "password" : "ashurt123ul",
  "is_favourite" : false
},
{ 
  "app": "Naver.com", 
  "user" : "valencepreseli",
  "password" : "valencepres123li",
  "is_favourite" : false
},
{ 
  "app": "adcash.com", 
  "user" : "poloaustralis",
  "password" : "poloaustra123is",
  "is_favourite" : false
},
{ 
  "app": "amazon.co.uk", 
  "user" : "guillemotlope",
  "password" : "guillemotl123pe",
  "is_favourite" : false
},
{ 
  "app": "rakuten.co.jp", 
  "user" : "galileansamosa",
  "password" : "galileansam123sa",
  "is_favourite" : false
},
{ 
  "app": "xhamster.com", 
  "user" : "technologyducks",
  "password" : "technologydu123ks",
  "is_favourite" : false
},
{ 
  "app": "Outbrain.com", 
  "user" : "braveimaginable",
  "password" : "braveimagina123le",
  "is_favourite" : false
},
{ 
  "app": "Booking.com", 
  "user" : "mixtapechoux",
  "password" : "mixtapech123ux",
  "is_favourite" : false
},
{ 
  "app": "Kat.cr", 
  "user" : "contentscovet",
  "password" : "contentsco123et",
  "is_favourite" : false
},
{ 
  "app": "soso.com", 
  "user" : "earlobesneaky",
  "password" : "earlobesne123ky",
  "is_favourite" : false
},
{ 
  "app": "adobe.com", 
  "user" : "risetranslate",
  "password" : "risetransl123te",
  "is_favourite" : false
},
{ 
  "app": "Nicovideo.jp", 
  "user" : "teenagewhicker",
  "password" : "teenagewhic123er",
  "is_favourite" : false
},
{ 
  "app": "cnn.com", 
  "user" : "wightdamocloids",
  "password" : "wightdamoclo123ds",
  "is_favourite" : false
},
{ 
  "app": "flipkart.com", 
  "user" : "dissmarzipan",
  "password" : "dissmarzi123an",
  "is_favourite" : false
},
{ 
  "app": "Chinadaily.com.cn", 
  "user" : "absorbingcrest",
  "password" : "absorbingcr123st",
  "is_favourite" : false
},
{ 
  "app": "go.com", 
  "user" : "chartcurium",
  "password" : "chartcur123um",
  "is_favourite" : false
},
{ 
  "app": "ebay.de", 
  "user" : "anorexiacooking",
  "password" : "anorexiacook123ng",
  "is_favourite" : false
},
{ 
  "app": "google.com.au", 
  "user" : "stoveprefer",
  "password" : "stovepre123er",
  "is_favourite" : false
},
{ 
  "app": "google.co.id", 
  "user" : "etherealcagey",
  "password" : "etherealca123ey",
  "is_favourite" : false
},
{ 
  "app": "Xinhuanet.com", 
  "user" : "biathlonflanked",
  "password" : "biathlonflan123ed",
  "is_favourite" : false
},
{ 
  "app": "google.pl", 
  "user" : "taughtmoons",
  "password" : "taughtmo123ns",
  "is_favourite" : false
},
{ 
  "app": "bbc.co.uk", 
  "user" : "testifyattack",
  "password" : "testifyatt123ck",
  "is_favourite" : false
},
{ 
  "app": "pixnet.net", 
  "user" : "forceporcus",
  "password" : "forcepor123us",
  "is_favourite" : false
},
{ 
  "app": "People.com.cn", 
  "user" : "diplomasdysprosium",
  "password" : "diplomasdyspros123um",
  "is_favourite" : false
},
{ 
  "app": "googleusercontent.com", 
  "user" : "jamanalytical",
  "password" : "jamanalyti123al",
  "is_favourite" : false
},
{ 
  "app": "Github.com", 
  "user" : "bivyfiles",
  "password" : "bivyfi123es",
  "is_favourite" : false
},
{ 
  "app": "Cntv.cn", 
  "user" : "divideplayer",
  "password" : "dividepla123er",
  "is_favourite" : false
},
{ 
  "app": "dailymotion.com", 
  "user" : "igloorough",
  "password" : "iglooro123gh",
  "is_favourite" : false
},
{ 
  "app": "gmw.cn", 
  "user" : "daltonslimeball",
  "password" : "daltonslimeb123ll",
  "is_favourite" : false
},
{ 
  "app": "Amazon.in", 
  "user" : "strictlyeach",
  "password" : "strictlye123ch",
  "is_favourite" : false
},
{ 
  "app": "dropbox.com", 
  "user" : "cutsdiggers",
  "password" : "cutsdigg123rs",
  "is_favourite" : false
},
{ 
  "app": "Google.co.kr", 
  "user" : "innocentfusty",
  "password" : "innocentfu123ty",
  "is_favourite" : false
},
{ 
  "app": "Wikia.com", 
  "user" : "whackmug",
  "password" : "whack123ug",
  "is_favourite" : false
},
{ 
  "app": "Dailymail.co.uk", 
  "user" : "withinreferred",
  "password" : "withinrefer123ed",
  "is_favourite" : false
},
{ 
  "app": "163.com", 
  "user" : "fundsvug",
  "password" : "funds123ug",
  "is_favourite" : false
},
{ 
  "app": "blogger.com", 
  "user" : "henchmantle",
  "password" : "henchman123le",
  "is_favourite" : false
}];

USER_CREDENTIALS = fakedata;