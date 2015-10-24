var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.message = mooltipass.ui.message || {};

mooltipass.ui.message.SUCCESS = "success";
mooltipass.ui.message.ALERT = "alert";
mooltipass.ui.message.WARNING = mooltipass.ui.message.ALERT;
mooltipass.ui.message.ERROR = "error";

mooltipass.ui.message._currentMessage = 0;

mooltipass.ui.message.show = function(message_type, message) {
    $("#notifications .message").html(message);
    $("#notifications img").attr("src", "../images/message_" + message_type + ".png");
    $("#notifications").attr("data-type", message_type);

    setTimeout(function(){
        $("#notifications").css("bottom", 0);
    }, 20);

    mooltipass.ui.message._currentMessage++;
    var i = mooltipass.ui.message._currentMessage;
    setTimeout(function(){
        if (i == mooltipass.ui.message._currentMessage) {
            $("#notifications").css("bottom", -($("#notifications").outerHeight()));
        }
    }, 3000);  
}