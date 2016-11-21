var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui._ = mooltipass.ui._ || {};


/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */
mooltipass.ui._.init = function() {
    // Only show app, if mp is connected
    update_device_status_classes();
    setInterval(update_device_status_classes, 150);

    // Init pages
    mooltipass.ui._.showActivePage();
    $("nav li a").click(function () {
        $("nav li.active").removeClass("active");
        $(this).parent("li").addClass("active");
        mooltipass.ui._.showActivePage();
    });

    // init tooltipster
    $(document).ready(function() {
        $('.tipster').tooltipster();
    });    

    mooltipass.ui._.initConfirmButtons();
    mooltipass.ui._.showSplashScreen();

    $('#modal-changelog').click(function() {
        $(this).hide();
    });

    mooltipass.ui._.showChangelog();
};

mooltipass.ui._.showSplashScreen = function () {
    $("#splash-screen").show();
    setTimeout(function(){
        $("#splash-screen").css('opacity', 0);
        setTimeout(function(){
            $("#splash-screen").hide();
            $("#splash-screen").css('opacity', 1);
        }, 800);
    }, 1500);        
};

mooltipass.ui._.showChangelog = function() {
    chrome.storage.local.get('changelog', function (result) {
        var currentVersion = chrome.runtime.getManifest().version;
        var storedVersion = (result.changelog) ? result.changelog.version : null;
        if(currentVersion != storedVersion) {
            if(storedVersion == null || $('#modal-changelog').data('version') == currentVersion) {
                $('#modal-changelog').show();
            }
            chrome.storage.local.set({'changelog': {'version': currentVersion}});
        }
    });
};

mooltipass.ui._.reset = function() {
    $("#modal-integrity-check").hide();
    $("#modal-load-credentials").hide();
    $("#modal-export").hide();
    $("#modal-confirm-on-device").hide();
	$('#modal-mooltipass-updating').hide();
    mooltipass.ui._.showSplashScreen();   
}

mooltipass.ui._.blockInput = function() {
    $("#modal-confirm-on-device").show();
}

mooltipass.ui._.unblockInput = function() {
    $("#modal-confirm-on-device").hide();
}

mooltipass.ui._.waitForDevice = function (button, activate) {
    var $button = $(button);
    if (activate) {
        $button.prop('disabled', true);
        $button.data('old_html', $button.html());
        $button.html('<i class="fa fa-spin fa-circle-o-notch"></i> confirm on device');
    }
    else {
        $button.prop('disabled', false);
        $button.html($button.data('old_html'));
    }
};

mooltipass.ui._.initConfirmButtons = function(){
    mooltipass.ui._.currentButtonClickCount = -1;
    mooltipass.ui._.buttonClickCount = 0;

    $("*[data-confirm]").each(function(no, el) {
        var $el = $(el);

        $el.data('content-origin', $el.html());

        $el.on('mousedown', function() {
            var $button = $(this);
            $button.html("<strong style='color:red;'>" + $button.attr("data-confirm") + "</strong>");

            mooltipass.ui._.buttonClickCount++;
            mooltipass.ui._.currentButtonClickCount = mooltipass.ui._.buttonClickCount;
            var _count = mooltipass.ui._.currentButtonClickCount;

            setTimeout(function(){
                if ((mooltipass.ui._.buttonClickCount != _count) || (mooltipass.ui._.currentButtonClickCount != _count)) {
                    return;
                }

                $button.html($button.data("content-origin"));
                executeFunctionByName($button.attr("data-execute"), window);
            }, 1000);

        });

        $el.on('mouseup', mooltipass.ui._.onMouseUpOrLeaveConfirmButton);
        $el.on('mouseleave', mooltipass.ui._.onMouseUpOrLeaveConfirmButton);
    });
};

mooltipass.ui._.onMouseUpOrLeaveConfirmButton = function() {
    // Cancel interaction
    mooltipass.ui._.currentButtonClickCount = -1;

    var $button = $(this);
    setTimeout(function(){
        $button.html($button.data("content-origin"));
    }, 200);
}

/*
var _console_log = console.log;
var _console_warn = console.warn;

console.log = function () {
    var text = arguments[0];
    var i = 1;
    while (i < arguments.length) {
        if (arguments[i] === undefined) {
            text += " undefined"
        }
        else if (arguments[i] == null) {
            text += " null";
        }
        else {
            text += " " + arguments[i].toString();
        }
        i++;
    }
    $("#log").val($("#log").val() + text + "\n");
    $("#log").animate({
        scrollTop: $("#log")[0].scrollHeight - $("#log").height()
    }, 100);
    return _console_log.apply(console, arguments);
}

console.warn = function () {
    var text = arguments[0];
    var i = 1;
    while (i < arguments.length) {
        if (arguments[i] === undefined) {
            text += " undefined"
        }
        else if (arguments[i] == null) {
            text += " null";
        }
        else {
            text += " " + arguments[i].toString();
        }
        i++;
    }
    text = "[WARNING] " + text;
    $("#log").val($("#log").val() + text + "\n");
    $("#log").animate({
        scrollTop: $("#log")[0].scrollHeight - $("#log").height()
    }, 100);
    return _console_warn.apply(console, arguments);
}
*/

mooltipass.ui._.showActivePage = function () {
    $(".page").hide();
    newPage = $("nav li.active a").attr("mp-open-page");
    $("#page-" + newPage).show();
}

mooltipass.ui._.isDeviceConnected = function () {
    return mooltipass.device.isConnected;
}

mooltipass.ui._.isCardUnknown = function() {
    return mooltipass.device.isUnknownCard;
}

mooltipass.ui._.isDeviceUnlocked = function () {
    return mooltipass.device.isUnlocked;
}

mooltipass.ui._.getDeviceVersion = function() {
    return mooltipass.device.version;
}

mooltipass.ui._.hasCard = function () {
    return !mooltipass.device.hasNoCard;
}

mooltipass.ui._.isDeviceInMMM = function () {
    return mooltipass.device.singleCommunicationMode
        && mooltipass.device.singleCommunicationModeEntered
        && mooltipass.device.singleCommunicationReason == 'memorymanagementmode';
}

mooltipass.ui._.isDeviceInSM = function () {
    return mooltipass.device.singleCommunicationMode
        && mooltipass.device.singleCommunicationModeEntered
        && mooltipass.device.singleCommunicationReason == 'synchronisationmode';
}

function executeFunctionByName(functionName, context) {
  var args = [].slice.call(arguments).splice(2);
  var namespaces = functionName.split(".");
  var func = namespaces.pop();
  for(var i = 0; i < namespaces.length; i++) {
    context = context[namespaces[i]];
  }
  return context[func].apply(this, args);
}

update_device_status_classes = function () {
    if (mooltipass.ui._.isCardUnknown()) {
        $(".show-if-card-unknown").show();
        $(".hide-if-card-unknown").hide();
        if($("#page-developers #resetCardCheckbox").prop("disabled") && $("#page-developers #resetCardCheckbox").data('active') != 1) {
            $("#page-developers #resetCardCheckbox").prop("disabled", false);
            $("#page-developers button.resetCard").prop("disabled", true);
        }
    } else {
        $(".show-if-card-unknown").hide();
        $(".hide-if-card-unknown").show();
        $("#page-developers #resetCardCheckbox").prop("disabled", true);
    }


    if (mooltipass.ui._.isDeviceConnected()) {
        $(".show-if-connected").show();
        $(".hide-if-connected").hide();
    } else {
        $(".show-if-connected").hide();
        $(".hide-if-connected").show();

        return
    }    

    if (mooltipass.ui._.hasCard()) {
        $(".show-if-card").show();
        $(".hide-if-card").hide();
    } else {
        $(".show-if-card").hide();
        $(".hide-if-card").show();

        return
    }

    if (mooltipass.ui._.isDeviceUnlocked()) {
        $(".show-if-unlocked").show();
        $(".hide-if-unlocked").hide();
    } else {
        $(".show-if-unlocked").hide();
        $(".hide-if-unlocked").show();

        return
    }

    // Each div can only have *-if-mmm OR *-if-sm, but not both
    if (mooltipass.ui._.isDeviceInMMM()) {
        $(".show-if-mmm").show();
        $(".hide-if-mmm").hide();
    } else {
        $(".show-if-mmm").hide();
        $(".hide-if-mmm").show();
    }

    if (mooltipass.ui._.isDeviceInSM()) { 
        $(".show-if-sm").show(); 
        $(".hide-if-sm").hide();
    } else {
        $(".show-if-sm").hide(); 
        $(".hide-if-sm").show();   
    }
};






