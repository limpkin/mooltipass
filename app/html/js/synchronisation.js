var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.sync = mooltipass.ui.sync || {};

mooltipass.ui.sync.init = function() {
    $("input[data-key='sync-file-auto']").on("click", function(){
        var is_active = $(this).prop("checked");

        // Todo: Store is_active
        console.log("Sync file automatically is set to ", is_active)
    });

    $("input[data-key='sync-cloud-auto']").on("click", function(){
        var is_active = $(this).prop("checked");

        // Todo: Store is_active
        console.log("Sync cloud automatically is set to ", is_active)
    });    


    // Todo: Use real default values
    $("input[data-key='sync-file-auto']").prop("checked", false);
    $("input[data-key='sync-cloud-auto']").prop("checked", true);
}

$(function(){
    mooltipass.ui.sync.init();
});