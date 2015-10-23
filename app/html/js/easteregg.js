var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.easteregg = mooltipass.ui.easteregg || {};


/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */
mooltipass.ui.easteregg.init = function () {
    $("#easteregg-button").click(function () {
        console.log("Easteregg button clicked");
    });
};