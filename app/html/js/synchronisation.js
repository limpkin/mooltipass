var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.sync = mooltipass.ui.sync || {};


mooltipass.ui.sync.disableButtons = function(disabled) {
    $('#exportToFile').prop('disabled', disabled);
    $('#importFromFile').prop('disabled', disabled);
    $('#exportToCloud').prop('disabled', disabled);
    $('#importFromCloud').prop('disabled', disabled);
    $('#scanMemory').prop('disabled', disabled);
}

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

    mooltipass.ui._.waitForDevice($(this), true);
    mooltipass.ui.sync.disableButtons(true);
    mooltipass.memmgmt.memoryBackupStart(true, mooltipass.ui.sync.callbackExportToFile);
};

mooltipass.ui.sync.callbackExportToFile = function(_status) {
    var $button = $('#exportToFile');

    mooltipass.ui.status.success($button, _status.msg);
    mooltipass.ui._.waitForDevice($button, false);
    mooltipass.ui.sync.disableButtons(false);
};


mooltipass.ui.sync.onClickImportFromFile = function(e) {
    e.preventDefault();

    mooltipass.ui._.waitForDevice($(this), true);
    mooltipass.ui.sync.disableButtons(true);
    mooltipass.memmgmt.mergeCredentialFileToMooltipassStart(mooltipass.ui.sync.callbackImportFromFile);
};

mooltipass.ui.sync.callbackImportFromFile = function(_status) {
    var $button = $('#importFromFile');

    mooltipass.ui.status.success($button, _status.msg);
    mooltipass.ui._.waitForDevice($button, false);
    mooltipass.ui.sync.disableButtons(false);
};


mooltipass.ui.sync.onClickExportToCloud = function(e) {
    e.preventDefault();

    mooltipass.ui._.waitForDevice($(this), true);
    mooltipass.ui.sync.disableButtons(true);
    mooltipass.memmgmt.memoryBackupStart(false, mooltipass.ui.sync.callbackExportToCloud);
};

mooltipass.ui.sync.callbackExportToCloud = function(_status) {
    var $button = $('#exportToCloud');

    mooltipass.ui.status.success($button, _status.msg);
    mooltipass.ui._.waitForDevice($button, false);
    mooltipass.ui.sync.disableButtons(false);
};


mooltipass.ui.sync.onClickImportFromCloud = function(e) {
    e.preventDefault();

    mooltipass.ui._.waitForDevice($(this), true);
    mooltipass.ui.sync.disableButtons(true);
    mooltipass.memmgmt.mergeSyncFSCredentialFileToMooltipassStart(mooltipass.ui.sync.callbackImportFromCloud);
};

mooltipass.ui.sync.callbackImportFromCloud = function(_status) {
    var $button = $('#importFromCloud');

    mooltipass.ui.status.success($button, _status.msg);
    mooltipass.ui._.waitForDevice($button, false);
    mooltipass.ui.sync.disableButtons(false);
};


mooltipass.ui.sync.onClickScanMemory = function(e) {
    e.preventDefault();

    mooltipass.ui._.waitForDevice($(this), true);
    mooltipass.ui.sync.disableButtons(true);
    mooltipass.memmgmt.integrityCheckStart(mooltipass.ui.sync.progressScanMemory, mooltipass.ui.sync.callbackScanMemory);
};

mooltipass.ui.sync.progressScanMemory = function(_progress) {
    //TODO: Implement progress bar
    console.log('progressScanMemory(', _progress.progress, ')');
};

mooltipass.ui.sync.callbackScanMemory = function(_status) {
    var $button = $('#scanMemory');

    mooltipass.ui.status.success($button, _status.msg);
    mooltipass.ui._.waitForDevice($button, false);
    mooltipass.ui.sync.disableButtons(false);
};

$(function(){
    mooltipass.ui.sync.init();
});