var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.sync = mooltipass.ui.sync || {};

mooltipass.ui.sync.init = function() {
    $("input[data-key='sync-cloud-auto']").on("click", function(){
        var is_active = $(this).prop("checked");

        // Todo: Store is_active
        console.log("Sync cloud automatically is set to ", is_active)
    });    


    // Todo: Use real default values
    $("input[data-key='sync-cloud-auto']").prop("checked", true);

    $('#exportToFile').click(mooltipass.ui.sync.onClickExportToFile);
    $('#importFromFile').click(mooltipass.ui.sync.onClickImportFromFile);

    $('#exportToCloud').click(mooltipass.ui.sync.onClickExportToCloud);
    $('#importFromCloud').click(mooltipass.ui.sync.onClickImportFromCloud);

    $('#scanMemory').click(mooltipass.ui.sync.onClickScanMemory);
};

mooltipass.ui.sync.onClickExportToFile = function(e) {
    e.preventDefault();
    mooltipass.memmgmt.memoryBackupStart(true);
};

mooltipass.ui.sync.onClickImportFromFile = function(e) {
    mooltipass.memmgmt.mergeCredentialFileToMooltipassStart();
};

mooltipass.ui.sync.onClickExportToCloud = function(e) {
    e.preventDefault();
    mooltipass.memmgmt.memoryBackupStart(false);
};

mooltipass.ui.sync.onClickImportFromCloud = function(e) {
    e.preventDefault();
    mooltipass.memmgmt.mergeSyncFSCredentialFileToMooltipassStart();
};

mooltipass.ui.sync.onClickScanMemory = function(e) {
    e.preventDefault();
    mooltipass.memmgmt.integrityCheckStart(mooltipass.ui.sync.progressScanMemory, mooltipass.ui.sync.callbackScanMemory);
};

mooltipass.ui.sync.progressScanMemory = function(_progress) {
    //TODO: Implement progress bar
    console.log('progressScanMemory(', _progress.progress, ')');
};

mooltipass.ui.sync.callbackScanMemory = function(_status) {
    if(_status.success) {
        mooltipass.ui.status.success($('#scanMemory'), _status.msg);
    }
    else {
        mooltipass.ui.status.success($('#scanMemory'), _status.msg);
    }
};

$(function(){
    mooltipass.ui.sync.init();
});