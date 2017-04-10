var mooltipass = mooltipass || {};
mooltipass.ui = mooltipass.ui || {};
mooltipass.ui.credentials = mooltipass.ui.credentials || {};

mooltipass.ui.credentials.activeDeviceInteraction = false;
mooltipass.ui.credentials.rngPwdReqInProgress = false;

// Disable throwing alerts by dataTable
$.fn.dataTableExt.sErrMode = 'throw';

var DEFAULT_PASSWORD = "••••••••";
var CREDENTIALS_TABLE = null;
var USER_CREDENTIALS = [];
var USER_CREDENTIALS_DELETE = [];
var WAITING_FOR_DEVICE_LABEL = '<i class="fa fa-spin fa-circle-o-notch"></i> confirm on device';

var MONTH_NAMES = [
    "January", "February", "March",
    "April", "May", "June", "July",
    "August", "September", "October",
    "November", "December"
];

var RECENT_DOUBLECLICK = false;
var dblclick_last_500ms = function () {
    RECENT_DOUBLECLICK = true;
    setTimeout(function () {
        RECENT_DOUBLECLICK = false;
    }, 500);
};

var stop_propagation = function (e) {
    e.stopPropagation();
};

var get_credentials_from_row = function ($row) {
    var context = $('td.context span', $row).data('value');
    var username = $('td.username span', $row).data('value');

    return {
        "context": context,
        "username": username,
        "description" : get_credential_infos(context, username).description
    }
}

var update_data_values = function () {
    $("#credentials tbody tr span").each(function () {
        if($(this).hasClass('value')) {
            $(this).text($(this).data('value'));
        }
        else {
            var $tr = $(this).parents('tr:first');
            if($tr.find('td.context').length > 0) {
                var data = CREDENTIALS_TABLE.fnGetData($tr);
                $('td.context span', $tr).addClass('value').data('value', data.context).text(data.context);
                $('td.username span', $tr).addClass('value').data('value', data.username).text(data.username);
                $('td.password span', $tr).addClass('value').data('value', DEFAULT_PASSWORD).text(DEFAULT_PASSWORD);
            }
        }
    });
};


mooltipass.ui.credentials.isActiveDeviceInteraction = function() {
    if(mooltipass.ui.credentials.activeDeviceInteraction) {
        mooltipass.ui.status.error(null, 'Please confirm the request on the device first.');
        return true;
    }

    return false;
};

mooltipass.ui.credentials.isActiveEdit = function() {
    if($("#credentials .edit").length > 0) {
        mooltipass.ui.status.error(null, 'Other credentials are already in edit mode.');
        return true;
    }

    return false;
}

mooltipass.ui.credentials.loadCredentials = function (_status, _credentials) {
    if (!_status.success) {
        mooltipass.ui.status.error($('#credentials'), _status.msg);
        return false;
    }

    USER_CREDENTIALS = _credentials;
    USER_CREDENTIALS_DELETE = [];

    // Init credentials table
    var data = get_user_credentials_for_table(USER_CREDENTIALS);
    if (data.length > 0) {
        CREDENTIALS_TABLE.fnAddData(data, true);
    }

    update_data_values();
    mooltipass.ui.credentials.initializeTableActions();

    // Update header
    $("#credentials").DataTable().draw();

    return true;
};

mooltipass.ui.credentials.initializeTableActions = function () {
    // Table actions

    //  Show password
    $("tbody .fa-eye").off('click').on('click', function (e) {
        if(mooltipass.ui.credentials.isActiveDeviceInteraction()) {
            return;
        }

        mooltipass.ui.credentials.activeDeviceInteraction = true;

        var $parent = $(this).parents("tr");
        var credentials = get_credentials_from_row($parent);
        var context = credentials.context;
        var username = credentials.username;

        $parent.find(".password span").html(WAITING_FOR_DEVICE_LABEL);
        get_password(context, username, function (_success, password) {
            mooltipass.ui.credentials.activeDeviceInteraction = false;

            if (_success) {
                $parent.find(".password span").data('value', password).text(password);
                $parent.find(".fa-eye").hide();
                $parent.find(".fa-eye-slash").show();
            }
            else {
                mooltipass.ui.status.error($parent, 'Could not get password from device');
                $parent.find(".password span").data('value', DEFAULT_PASSWORD).text(DEFAULT_PASSWORD);
            }
        });

        e.stopPropagation();
    });

    //  Hide password
    $("tbody .fa-eye-slash").off('click').on('click', function (e) {
        $(this).parents("tr").find(".password span").data('value', DEFAULT_PASSWORD).text(DEFAULT_PASSWORD);
        $(this).parents("tr").find(".fa-eye-slash").hide();
        $(this).parents("tr").find(".fa-eye").show();

        e.stopPropagation();
    });

    //  Add to / remove from favourites
    $("tbody .fa-star-o, tbody .fa-star").off('click').on('click', function (e) {
        var $parent = $(this).parents("tr");
        var credentials = get_credentials_from_row($parent);
        var context = credentials.context;
        var username = credentials.username;

        for (var _key in USER_CREDENTIALS) {
            if ((USER_CREDENTIALS[_key].context == context) && (USER_CREDENTIALS[_key].username == username)) {
                USER_CREDENTIALS[_key].favorite = !USER_CREDENTIALS[_key].favorite;
                USER_CREDENTIALS[_key]._changed = true;
            }
        }
        $(this).toggleClass("fa-star-o fa-star");
        
        $('#button-placeholder').hide();
        $('#unsaved-changes-warning').fadeIn();

        e.stopPropagation();
    });
    //  Delete credentials
    $(".fa-trash-o").off('click').on('click', function (e) {
        if(mooltipass.ui.credentials.isActiveEdit()) {
            return false;
        }

        var $parent = $(this).parents("tr");
        var credentials = get_credentials_from_row($parent);
        var context = credentials.context;
        var username = credentials.username;

        for (var _credential in USER_CREDENTIALS) {
            var credential = USER_CREDENTIALS[_credential];
            if ((credential.context == context) && (credential.username == username)) {
                var _delete = USER_CREDENTIALS.splice(_credential, 1);
                if (_delete[0].address) {
                    // Only existing credentials have an address, new credentials can be removed without adding them to USER_CREDENTIALS_DELETE
                    USER_CREDENTIALS_DELETE.push({
                        'address': _delete[0].address,
                        'parent_address': _delete[0].parent_address,
                    });
                }
            }
        }
        if ($parent.hasClass("active"))
            $(".credential-details").remove();

        // Don't redraw to not cause scroll.
        CREDENTIALS_TABLE.fnDeleteRow($parent[0], null, false);

        // Delete the row manually
        $($parent[0]).remove();

        $('#button-placeholder').hide();
        $('#unsaved-changes-warning').fadeIn();

        e.stopPropagation();
        return false;
    });

    //  Edit credentials
    var edit_credentials = function (e) {
        if(mooltipass.ui.credentials.isActiveDeviceInteraction()) {
            return;
        }

        if(mooltipass.ui.credentials.isActiveEdit()) {
            return;
        }

        mooltipass.ui.credentials.activeDeviceInteraction = true;

        var $parent = $(this).parents("tr");
        if ($parent.hasClass("credential-details"))
            $parent = $parent.prev();
        var $this = $(this);

        // Return if already in edit mode
        if ($parent.find("input").length > 0) return;

        if($("#credentials .edit").length > 0) {
            mooltipass.ui.status.error('Other credentials are already in edit mode.');
            return;
        }

        var credentials = get_credentials_from_row($parent);
        var context = credentials.context;
        var username = credentials.username;
        var description = credentials.description;

        var $app = $parent.find(".context span");
        var $user = $parent.find(".username span");
        var $password = $parent.find(".password span");

        $password.html(WAITING_FOR_DEVICE_LABEL);
        get_password(context, username, function (_success, password) {
            mooltipass.ui.credentials.activeDeviceInteraction = false;
            if (_success) {
                if (!($parent.hasClass("active"))) {
                    $(".active").removeClass("active");
                    $parent.addClass("active");
                    update_details_view();
                }

                var $description = $parent.next().find("p.description span");

                $parent.addClass('edit');

                $("td.editable", $parent).off('dblclick');

                $app.empty().append(
                    $('<input>')
                        .addClass('inline').addClass('change-credentials')
                        .attr('name', 'context')
                        .data('old', context).data('maxlength', 57)
                        .val(context)
                );
                $user.empty().append(
                    $('<input>')
                        .addClass('inline').addClass('change-credentials')
                        .attr('name', 'username')
                        .data('old', username).data('maxlength', 61)
                        .val(username)
                );
                $password.empty().append(
                    $('<input>')
                        .addClass('inline').addClass('change-credentials')
                        .attr('name', 'password')
                        .data('old', password).data('maxlength', 31)
                        .val(password)
                );
                $description.empty().append(
                    $('<input>')
                        .addClass('inline').addClass('change-credentials')
                        .attr('name', 'description')
                        .data('old', description).data('maxlength', 23)
                        .val(description)
                );


                $(".inline.change-credentials").on('keydown', save_credential_changes);
                $(".inline.change-credentials").on('keydown', discard_credential_changes);
                $(".inline.change-credentials").on('click', stop_propagation);

                $("#credentials input.inline").on('keyup', mooltipass.ui.credentials.onKeyUpInputMax);

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
            }
            else {
                //Message already created on get_password()
                //mooltipass.ui.status.error($parent, 'Could not get password from device');
                $password.html(DEFAULT_PASSWORD);
            }
        });

        e.stopPropagation();
    }
    $(".fa-pencil").off('click').on('click', edit_credentials);
    $("tbody tr td.editable").off('dblclick').on('dblclick', edit_credentials).on('dblclick', dblclick_last_500ms);

    //  Save credentials
    var save_credential_changes = function (e) {
        if ((e.type == "keydown") && (e.keyCode != 13)) return;

        var $parent = $(this).parents("tr");

        if($('#credentials input.inline.error').length > 0) {
            mooltipass.ui.credentials.showMaximumLengthErrorMessage($('#credentials input.inline.error'));
            return false;
        }

        if ($parent.hasClass("credential-details")) {
            $parent = $parent.prev();
        }

        var old_context = $parent.find(".context input").data("old");
        var new_context = $parent.find(".context input").val();
        var old_username = $parent.find(".username input").data("old");
        var new_username = $parent.find(".username input").val();
        var old_password = $parent.find(".password input").data("old");
        var new_password = $parent.find(".password input").val();
        var old_description = $parent.next().find("p.description input").data("old");
        var new_description = $.trim($parent.next().find("p.description input").val());

        var credential_item = null;

        // Changed at least one field
        if (old_context != new_context || old_username != new_username || old_password != new_password || old_description != new_description) {
            if(old_username != new_username || old_context != new_context) {
                for (var _key in USER_CREDENTIALS) {
                    var credential = USER_CREDENTIALS[_key];
                    if(credential.context == new_context && credential.username == new_username) {
                        mooltipass.ui.status.error(null, 'Credentials with this username and app already exists!');
                        return;
                    }
                }
            }

            for (var _key in USER_CREDENTIALS) {
                var credential = USER_CREDENTIALS[_key];

                if ((credential.context == old_context) && (credential.username == old_username)) {
                    USER_CREDENTIALS[_key].context = new_context;
                    USER_CREDENTIALS[_key].username = new_username;
                    USER_CREDENTIALS[_key].password = new_password;
                    USER_CREDENTIALS[_key].description = new_description;
                    USER_CREDENTIALS[_key]._has_password_changed = USER_CREDENTIALS[_key].password_original != new_password;
                    USER_CREDENTIALS[_key]._changed = true;

                    if (USER_CREDENTIALS[_key]._has_password_changed) {
                        USER_CREDENTIALS[_key].date_modified = new Date();
                    }

                    credential_item = USER_CREDENTIALS[_key];

                    break;
                }
            }
        }

        $parent.removeClass("edit");

        $parent.find(".fa-pencil").show();
        $parent.find(".fa-eye").show();
        $parent.find(".fa-trash-o").show();
        $parent.find(".fa-floppy-o").hide();
        $parent.find(".fa-times").hide();

        new_description = new_description || '- empty -';

        //$parent.find(".context span").html(new_context).attr("data-value", new_context);
        $parent.find(".context span").data('value', new_context).text(new_context);
        //$parent.find(".username span").html(new_username).attr("data-value", new_username);
        $parent.find(".username span").data('value', new_username).text(new_username);
        //$parent.find(".password span").html(DEFAULT_PASSWORD);
        $parent.find(".password span").text($parent.find(".password span").data('value'));
        //$parent.next().find("p.description").html(new_description).attr("data-value", new_description);
        $parent.next().find("p.description span").data('value', new_description).text(new_description);

        $("td.editable", $parent).off('dblclick').on('dblclick', edit_credentials).on('dblclick', dblclick_last_500ms);

        // Only update table if changes were applied
        if(credential_item != null) {
            $('#credentials .credential-details').remove();

            var _item = get_user_credentials_for_table([credential_item])[0];
            CREDENTIALS_TABLE.fnUpdate(_item, $parent[0], undefined, true);

            update_data_values();
            mooltipass.ui.credentials.initializeTableActions();

            update_details_view();

            $('#button-placeholder').hide();
            $('#unsaved-changes-warning').fadeIn();
        }

        e.stopPropagation();
    }
    $(".fa-floppy-o").off('click').on("click", save_credential_changes);

    //  Discard credentials
    var discard_credential_changes = function (e) {
        // ESC to discard
        if ((e.type == "keydown") && (e.keyCode != 27)) {
            return;
        }

        var $parent = $(this).parents("tr");

        if ($parent.hasClass("credential-details")) {
            $parent = $parent.prev();
        }

        $parent.removeClass("edit");

        $parent.find(".fa-pencil").show();
        $parent.find(".fa-eye").show();
        $parent.find(".fa-trash-o").show();
        $parent.find(".fa-floppy-o").hide();
        $parent.find(".fa-times").hide();

        update_data_values();

        e.stopPropagation();
    }
    $(".fa-times").off('click').on("click", discard_credential_changes);

    // Returns a TR with credentials details
    var generate_details_view = function( credentials ) {
        var context = credentials.context;
        var username = credentials.username;

        var credential_details = get_credential_infos(context, username);

        var description = credential_details.description || '- empty -';

        var now = new Date();
        var date = new Date(credential_details.date_lastused);
        var last_used = MONTH_NAMES[date.getMonth()] + " " + date.getDate();
        if (now - date > 365 * 24 * 60 * 60 * 1000) {
            last_used += ", " + date.getFullYear();
        }
        if (isNaN(date.getFullYear()) || date.getFullYear() < 2013) {
            last_used = '- never -';
        }

        var date = new Date(credential_details.date_modified);
        var last_modified = MONTH_NAMES[date.getMonth()] + " " + date.getDate();
        if (now - date > 365 * 24 * 60 * 60 * 1000) {
            last_modified += ", " + date.getFullYear();
        }
        if (isNaN(date.getFullYear()) || date.getFullYear() < 2013) {
            last_modified = '- never -';
        }

        var $tr = $('<tr class="credential-details"><td colspan=2></td><td class="labels">\
            <p>Last used</p>\
            <p>Last modified</p>\
            <p>Description</p>\
            </td><td colspan=2>\
            <p class="last_used"></p>\
            <p class="last_modified"></p>\
            <p class="description"><span class="value"></span></p>\
            </td></tr>');

        $('.last_used', $tr).text(last_used);
        $('.last_modified', $tr).text(last_modified);
        $('.description span', $tr).data('value', description).text(description);

        $(".description span", $tr).off('dblclick')
            .on('dblclick', edit_credentials)
            .on('dblclick', dblclick_last_500ms);

        return $tr;
    }

    //  View details (description / last used / last modified)
    var update_details_view = function () {
        if(mooltipass.ui.credentials.isActiveEdit()) {
            return;
        }

        $(".credential-details").remove();
        if ($(".active").length > 0) {
            var credentials = get_credentials_from_row($(".active"));
            var $tr = generate_details_view( credentials );
            $tr.addClass('active');
            $(".active").after($tr);
        }
    };

    var display_all_details = function(e) {
        // Cleanup previous actions
        $(".credential-details").remove();
        $(".active").removeClass("active").removeClass("edit");

        // Add details for every row
        $('#credentials tbody tr').each( function() { 
            var credentials = get_credentials_from_row( $(this) );
            var $tr = generate_details_view( credentials );
            $(this).after($tr)
        } );
    }

    var display_details = function (e) {
        if(mooltipass.ui.credentials.isActiveEdit()) {
            return;
        }

        $this = $(this);
        setTimeout(function () {
            if (RECENT_DOUBLECLICK) return;
            $(".credential-details").remove();
            if ($this.hasClass("active")) {
                $(".active").removeClass("active").removeClass("edit");
            }
            else {
                $(".active").removeClass("active").removeClass("edit");
                $this.addClass("active");
            }
            update_details_view();
        }, 300);

        e.stopPropagation();
    };
    $("tbody tr").off('click').on('click', display_details);
    $(".expandContractLink").off('click').on('click', display_all_details);
};

mooltipass.ui.credentials.onKeyUpInputMax = function(e) {
    if (e.keyCode == 13) return;

    if($(this).val().length > $(this).data('maxlength')) {
        $(this).addClass('error');
    }
    else {
        $(this).removeClass('error');
    }
};

mooltipass.ui.credentials.showMaximumLengthErrorMessage = function(elements) {
    var $el = $(elements[0]);
    var $msg = 'The %s value is too long! Allowed maximum length: ' + $el.data('maxlength') + '&nbsp;&nbsp;&nbsp;(used: ' + $el.val().length + ')';
    switch($el.attr('name')) {
        case 'context':
        case 'app':
            $msg = $msg.replace('%s', 'app');
            break;
        case 'username':
            $msg = $msg.replace('%s', 'username');
            break;
        case 'password':
            $msg = $msg.replace('%s', 'password');
            break;
        case 'description':
            $msg = $msg.replace('%s', 'description');
            break;
    }

    mooltipass.ui.status.error(null, $msg);
};


mooltipass.ui.credentials.callbackMMMEnter = function (_status, _credentials) {
    if (_status.success) {
        $('#mmm-save, #mmm-discard').show();

        mooltipass.device.singleCommunicationModeEntered = true;
        update_device_status_classes();

        $('#button-placeholder').show();
        $('#unsaved-changes-warning').removeClass("hide").hide();

        mooltipass.ui.credentials.loadCredentials(_status, _credentials);
    }
    else {
        // Could not enter MemoryManagementMode
        mooltipass.device.endSingleCommunicationMode(_status.skipEndingSingleCommunicationMode);
        mooltipass.ui.status.error($('#mmm-enter'), _status.msg);
    }

    // Set back ui button
    // mooltipass.ui._.waitForDevice($('#mmm-enter'), false);
    $("#modal-confirm-on-device").hide();
    $("#modal-load-credentials").hide();
}

mooltipass.ui.credentials.callbackMMMEnterProgress = function(_progress) {
    $("#modal-load-credentials").show();
    $("#modal-confirm-on-device").hide();

    $("#modal-load-credentials span.meter").css("width", _progress.progress + "%");
};

mooltipass.ui.credentials.onClickMMMEnter = function () {
    // Inform user about device interaction
    $("#modal-confirm-on-device").show();
    // mooltipass.ui._.waitForDevice($button, true);

    // Request mmm activation from device
    mooltipass.device.interface.send({
        'command': 'startSingleCommunicationMode',
        'callbackFunction': mooltipass.ui.credentials.callbackMMMEnter,
        'reason': 'memorymanagementmode',
        'callbackFunctionStart': function() {
            mooltipass.memmgmt.memmgmtStart(mooltipass.ui.credentials.callbackMMMEnter, mooltipass.ui.credentials.callbackMMMEnterProgress);
        }
    });
};

mooltipass.ui.credentials.onClickMMMDiscard = function() {
    if(mooltipass.ui.credentials.isActiveDeviceInteraction()) {
        return;
    }
    
    $(".add-credential input").val("");

    mooltipass.memmgmt.memmgmtStop(function (_status) {
        if (_status.success) {
            mooltipass.device.endSingleCommunicationMode();
            update_device_status_classes();
            USER_CREDENTIALS = [];
            var $table = $('#credentials').dataTable();
            $table.fnClearTable();
            $('#mmm-save, #mmm-discard').hide();
            $('#mmm-enter').show();
            mooltipass.ui.status.success($('#mmm-enter'), _status.msg);
        }
        else {
            mooltipass.ui.status.error($('#mmm-discard'), _status.msg);
        }
    });
};

mooltipass.ui.credentials.onClickMMMSave = function () {
    if(mooltipass.ui.credentials.isActiveDeviceInteraction()) {
        return;
    }
    
    $(".add-credential input").val("");

    var deletes = USER_CREDENTIALS_DELETE;
    var updates = [];
    var adds = [];

    for (var _key in USER_CREDENTIALS) {
        if ('_changed' in USER_CREDENTIALS[_key]) {
            var item = USER_CREDENTIALS[_key];

            delete item._changed;
            delete item.password_original;

            if ('address' in item) {
                if (!item._has_password_changed) {
                    delete item.password;
                }
                delete item._has_password_changed;
                updates.push(item);
            }
            else {
                delete item._has_password_changed;
                adds.push(item);
            }
        }
    }

    mooltipass.memmgmt.memmgmtSave(function (_status) {
        if (_status.success) {
            mooltipass.device.endSingleCommunicationMode();
            update_device_status_classes();
            USER_CREDENTIALS = [];
            var $table = $('#credentials').dataTable();
            $table.fnClearTable();
            $('#mmm-save, #mmm-discard').hide();
            $('#mmm-enter').show();

            mooltipass.ui.status.success($('#mmm-enter'), _status.msg);
        }
        else {
            mooltipass.ui.status.error($('#mmm-enter'), _status.msg);
        }
    }, deletes, updates, adds);
};

mooltipass.ui.credentials.onClickImportFromCSV_DUMMY = function(_callbackFunction) {
    // TODO #MS
    // TODO Remove this DUMMY method
    // TODO and replace in the next function:
    // TODO     mooltipass.ui.credentials.onClickImportFromCSV_DUMMY
    // TODO with your function name!
    console.warn('NOT IMPLEMENTED YET');
};
mooltipass.ui.credentials.onClickImportFromCSV = function(e) {
    e.preventDefault();

    // TODO #MS
    // TODO replace mooltipass.ui.credentials.onClickImportFromCSV_DUMMY with your function call: your_function(callbackFunction)
    mooltipass.ui.credentials.onClickImportFromCSV_DUMMY(
        function (_status) {
            if (_status.success) {
                mooltipass.device.endSingleCommunicationMode();
                update_device_status_classes();
                USER_CREDENTIALS = [];
                var $table = $('#credentials').dataTable();
                $table.fnClearTable();
                mooltipass.ui.status.success($('#import-from-csv'), _status.msg);
            }
            else {
                mooltipass.ui.status.error($('#import-from-csv'), _status.msg);
            }
        }
    );
}

/**
 * Function dedicated to the quick add submit event
 *
 */
mooltipass.ui.credentials.quickAddSubmit = function()
{
    var $inputs;
    var i;

    // Check if form is valid
    $inputs = $(".quickcredentialadd input[required]");
    i = 0;
    var is_valid = true;
    while (i < $inputs.length) {
        var $input = $($inputs[i]);
        if (($input.attr("required") == 'required') && ($input.val() == '')) {
            $input.parents("label").addClass("alert").addClass("alert-required");
            is_valid = false;
        } else {
            $input.parents("label").removeClass("alert-required");
        }
        i++;
    }

    //Check if value length is < max-length
    $inputs = $(".quickcredentialadd input[data-maxlength]");
    i = 0;
    while (i < $inputs.length) {
        var $input = $($inputs[i]);
        if (parseInt($input.data("maxlength")) < $input.val().length) {
            $input.parents("label").addClass("alert").addClass("alert-maxlength");
            is_valid = false;
        } else {
            $input.parents("label").removeClass("alert-maxlength");
        }
        i++;
    }

    //Remove class .alert from labels if neither alert-required nor alert-maxlength is set
    $inputs = $(".quickcredentialadd input");
    i = 0;
    while (i < $inputs.length) {
        var $input = $($inputs[i]);

        var $label = $input.parents("label");

        if(!$label.hasClass('alert-required') && !$label.hasClass('alert-maxlength')) {
            $label.removeClass('alert');
        }

        i++;
    }

    if (!is_valid) return;
    
    // Start the storage process!
    mooltipass.device.interface.send(
    {   'command':'updateCredentials', 
        'context':$(".quickcredentialadd input[name='quick-app']").val().trim(), 
        'username': $(".quickcredentialadd input[name='quick-user']").val(),
        'password': $(".quickcredentialadd input[name='quick-password']").val(),
        'callbackFunction': mooltipass.ui.credentials.quickAddCallback
    });

    // Empty form fields again
    $(".quickcredentialadd input:not('.pwgen-slider,.pwgen-length')").val("");
    $(".quickcredentialadd input:visible:first").focus();
};

mooltipass.ui.credentials.quickAddCallback = function(_status)
{
    if (_status.success) 
    {
        mooltipass.ui.status.success($('#mmm-enter'), "Credential Storage Successfull");
    }
    else 
    {
        mooltipass.ui.status.error($('#mmm-enter'), "Couldn't Store New Credential");
    }
    
    $("#modal-confirm-on-device").hide();
    $("#modal-load-credentials").hide();
};

/**
 * Generate a random password based on a random string returned from device
 * @access backend
 * @param _callback to send the generated password to
 * @param _length of the password
 */
mooltipass.ui.credentials.initializePasswordGenerator = function(_callback, _length, _settings) {
    // Only request new random string from device once a minute
    // The requested random string is used to salt Math.random() again
    var currentDate = new Date();
    var currentDayMinute = currentDate.getUTCHours() * 60 + currentDate.getUTCMinutes();
    if(!mooltipass.ui.credentials._latestRandomStringRequest || mooltipass.ui.credentials._latestRandomStringRequest != currentDayMinute) {
        if(mooltipass.ui.credentials.rngPwdReqInProgress == false && mooltipass.device.singleCommunicationMode == false)
        {
            mooltipass.device.interface.send({
                'command':'getRandomNumber',
                'callbackFunction': function(_responseObject) {
                    Math.seedrandom(_responseObject.value, { entropy: true });
                    mooltipass.ui.credentials.passwordGeneratorCallback(_callback, _length, _settings, mooltipass.ui.credentials.generateRandomNumbers(_length));
                }
            });
            mooltipass.ui.credentials.rngPwdReqInProgress = true;
            mooltipass.ui.credentials._latestRandomStringRequest = currentDayMinute;
            return;
        }
    }

    mooltipass.ui.credentials.passwordGeneratorCallback(_callback, _length, _settings, mooltipass.ui.credentials.generateRandomNumbers(_length));
};

/**
 * Based on a salted Math.random() generate random numbers
 * @access backend
 * @param length number of random numbers to generate
 * @returns {Array} array of Numbers
 */
mooltipass.ui.credentials.generateRandomNumbers = function(_length) {
    var seeds = [];
    for(var i = 0; i < _length; i++) {
        seeds.push(Math.random());
    }

    return seeds;
};

mooltipass.ui.credentials.passwordGeneratorCallback = function(callback, length, settings, seeds) {
    // Return a random password with given length
    var charactersLowercase = 'abcdefghijklmnopqrstuvwxyz';
    var charactersUppercase = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';
    var charactersNumbers = '1234567890';
    var charactersSpecial = '!$%*()_+{}-[]:"|;\'?,./';

    var hash = "";
    var possible = "";

    if(settings["lowercase"]) {
        possible += charactersLowercase;
    }
    if(settings["uppercase"]) {
        possible += charactersUppercase;
    }
    if(settings["numbers"]) {
        possible += charactersNumbers;
    }
    if(settings["specialchars"]) {
        possible += charactersSpecial;
    }

    for( var i=0; i < length; i++ ) {
        hash += possible.charAt(Math.floor(seeds[i] * possible.length));
    }

    callback(hash);
}

/**
 * Initialize function
 * triggered by mooltipass.app.init()
 */
mooltipass.ui.credentials.init = function () {
    $('#quick-add-store').click(mooltipass.ui.credentials.quickAddSubmit);
    $('#mmm-enter').click(mooltipass.ui.credentials.onClickMMMEnter);
    $('#mmm-save').click(mooltipass.ui.credentials.onClickMMMSave);
    $('#mmm-save, #mmm-discard').hide();

    $("#import-from-csv").click(mooltipass.ui.credentials.onClickImportFromCSV);
    // TODO #MS
    // TODO Remove next line after implementing functionality
    $("#import-from-csv").hide();
    
    mooltipass.ui.credentials.rngPwdReqInProgress = false;


    CREDENTIALS_TABLE = $("#credentials").dataTable({
        scrollY: 210,
        dom: '<t>',
        columns: [
            {
                type: "html",
                data: "favorite"
            },
            {
                type: "string",
                data: {
                    "display": "span",
                    "_": "context"
                }
            },
            {
                type: "string",
                data: {
                    "display": "span",
                    "_": "username"
                }
            },
            {
                type: "html",
                data: "password",
                "render": function ( data, type, full, meta ) {
                    return '<span></span>';
                }
            },
            {
                type: "html",
                data: "actions"
            }
        ],
        oLanguage: {
            sEmptyTable: "No credentials stored in your mooltipass. Add your first one below."
        }        
    });

    // Search for credentials
    $("#search-input").on("keyup change", function () {
        $("#credentials").DataTable().search($(this).val()).draw()
    });
    $("#search-input").on("keyup", function (e) {
        if (e.keyCode == 27) $(this).val("").trigger("change");
    });

    // Init add credentials interactions
    $(".add-credential input").on("focus", function () {
        var el = $(this);
        for(var i = 0; i < 5; i++) {
            if(el.next('.comment').length) {
                el.next('.comment').css("opacity", 1);
                break;
            }
            el = el.next()
        }
    })
    .on("focusout", function () {
        var el = $(this);
        for(var i = 0; i < 5; i++) {
            if(el.next('.comment').length) {
                el.next('.comment').css("opacity", 0);
                break;
            }
            el = el.next()
        }
    }).on("keydown", function (e) {
        // Manage TABbing to next field
        if (e.keyCode == 9) {
            if ((!is_key_pressed(16)) && ($(this).attr("required") == 'required') && ($(this).val().trim() == '')) {
                $(this).parents("label").addClass("alert").addClass('alert-required');
            } else {
                $(this).parents("label").removeClass('alert-required');
                if(!$(this).parents("label").hasClass('alert-maxlength')) {
                    $(this).parents("label").removeClass('alert');
                }
            }
        }
        // Manage submit of new credentials
        if ((e.keyCode == 13) && ($(this).attr("data-submit") == '')) {
            submit_add_credentials();
            e.preventDefault();
        }
    }).on("keyup", function (e) {
        var $label = $(this).parents("label");
        var is_valid = true;

        if($(this).data('maxlength') && parseInt($(this).data('maxlength')) < $(this).val().length) {
            $label.addClass("alert").addClass('alert-maxlength');
            is_valid = false;
        }
        else {
            $label.removeClass('alert-maxlength')
        }

        // Remove any error label if input is not empty
        if (($(this).attr("required") == 'required') && ($(this).val() != '')) {
            $label.removeClass('alert-required');
        }
        else {
            is_valid = false;
        }

        if(is_valid) {
            $label.removeClass('alert');
        }
    });

    $(".password-eye").click(function() {
        if($(this).data("state") == "hidden") {
            $(this).prev("input").attr("type", "text");
            $(this).find("i.fa").hide();
            $(this).find("i.fa-eye-slash").show();
            $(this).data("state", "visible");
        }
        else {
            $(this).prev("input").attr("type", "password");
            $(this).find("i.fa").hide();
            $(this).find("i.fa-eye").show();
            $(this).data("state", "hidden");
        }
    });

    $(".password-eye i.fa-eye-slash").hide();
    
    $("#add-credentials-button").click(function(e) {
        e.preventDefault();
        submit_add_credentials();
    })

    $(".pwgen-slider").bind("input", function() {
        $(this).parent().next(".column").children("input:first").val($(this).val());
    });

    $("input.pwgen-length").change(function() {
        $(this).parent().prev(".column").children("input:first").val($(this).val());
    });

    $("button.pwgen-button").click(function(e) {
        e.preventDefault();

        var $settings = $("#" + $(this).data("settingsId"));
        var $length = $("#" + $(this).data("lengthId"));

        var length = $length.find(".pwgen-length:first").val();
        var settings = {
            "uppercase": $settings.find(".pwgen-uppercase:first").prop("checked"),
            "lowercase": $settings.find(".pwgen-lowercase:first").prop("checked"),
            "numbers": $settings.find(".pwgen-numbers:first").prop("checked"),
            "specialchars": $settings.find(".pwgen-specialchars:first").prop("checked"),

        };
        var $field = $("#" + $(this).data("fillId"));
        mooltipass.ui.credentials.initializePasswordGenerator(function(value) {
            $field.val(value);
        }, length, settings);
    });
        
    // Init add credentials interactions
    $(".quickcredentialadd input").on("focus", function () {
        var el = $(this);
        for(var i = 0; i < 5; i++) {
            if(el.next('.comment').length) {
                el.next('.comment').css("opacity", 1);
                break;
            }
            el = el.next()
        }
    })
    .on("focusout", function () {
        var el = $(this);
        for(var i = 0; i < 5; i++) {
            if(el.next('.comment').length) {
                el.next('.comment').css("opacity", 0);
                break;
            }
            el = el.next()
        }
    }).on("keydown", function (e) {
        // Manage TABbing to next field
        if (e.keyCode == 9) {
            if ((!is_key_pressed(16)) && ($(this).attr("required") == 'required') && ($(this).val().trim() == '')) {
                $(this).parents("label").addClass("alert").addClass('alert-required');
            } else {
                $(this).parents("label").removeClass('alert-required');
                if(!$(this).parents("label").hasClass('alert-maxlength')) {
                    $(this).parents("label").removeClass('alert');
                }
            }
        }
        // Manage submit of new credentials
        if ((e.keyCode == 13) && ($(this).attr("data-submit") == '')) {
            e.preventDefault();
            mooltipass.ui.credentials.quickAddSubmit();
        }
    }).on("keyup", function (e) {
        var $label = $(this).parents("label");
        var is_valid = true;

        if($(this).data('maxlength') && parseInt($(this).data('maxlength')) < $(this).val().length) {
            $label.addClass("alert").addClass('alert-maxlength');
            is_valid = false;
        }
        else {
            $label.removeClass('alert-maxlength')
        }

        // Remove any error label if input is not empty
        if (($(this).attr("required") == 'required') && ($(this).val() != '')) {
            $label.removeClass('alert-required');
        }
        else {
            is_valid = false;
        }

        if(is_valid) {
            $label.removeClass('alert');
        }
    });
};

var submit_add_credentials = function() {
    var $inputs;
    var i;

    // Check if form is valid
    $inputs = $(".add-credential input[required]");
    i = 0;
    var is_valid = true;
    while (i < $inputs.length) {
        var $input = $($inputs[i]);
        if (($input.attr("required") == 'required') && ($input.val() == '')) {
            $input.parents("label").addClass("alert").addClass("alert-required");
            is_valid = false;
        } else {
            $input.parents("label").removeClass("alert-required");
        }
        i++;
    }

    //Check if value length is < max-length
    $inputs = $(".add-credential input[data-maxlength]");
    i = 0;
    while (i < $inputs.length) {
        var $input = $($inputs[i]);
        if (parseInt($input.data("maxlength")) < $input.val().length) {
            $input.parents("label").addClass("alert").addClass("alert-maxlength");
            is_valid = false;
        } else {
            $input.parents("label").removeClass("alert-maxlength");
        }
        i++;
    }

    //Remove class .alert from labels if neither alert-required nor alert-maxlength is set
    $inputs = $(".add-credential input");
    i = 0;
    while (i < $inputs.length) {
        var $input = $($inputs[i]);

        var $label = $input.parents("label");

        if(!$label.hasClass('alert-required') && !$label.hasClass('alert-maxlength')) {
            $label.removeClass('alert');
        }

        i++;
    }

    if (!is_valid) return;

    if(mooltipass.ui.credentials.isActiveEdit()) {
        return;
    }

    // If submission is valid, add to USER_CREDENTIALS
    var credential = {
        "favorite": false,
        "context": $(".add-credential input[name='app']").val().trim(),
        "username": $(".add-credential input[name='user']").val(),
        "password": $(".add-credential input[name='password']").val(),
        "description": $(".add-credential input[name='description']").val().trim(),
        "date_modified": new Date(),
        "_changed": true,
    };

    for(i = 0; i < USER_CREDENTIALS.length; i++) {
        if(USER_CREDENTIALS[i].context == credential.context && USER_CREDENTIALS[i].username == credential.username) {
            mooltipass.ui.status.error(null, 'Credentials with this username and app already exists!');
            return;
        }
    }

    USER_CREDENTIALS.push(credential);

    // Empty form fields again
    $(".add-credential input").val("");

    $(".add-credential input:visible:first").focus();

    // Init credentials table
    var data = get_user_credentials_for_table([credential]);
    if (data.length > 0) {
        CREDENTIALS_TABLE.fnAddData(data, true);
    }

    $('#button-placeholder').hide();
    $('#unsaved-changes-warning').fadeIn();

    update_data_values();
    mooltipass.ui.credentials.initializeTableActions();
};

var get_user_credentials_for_table = function (_user_credentials) {
//var credentials = JSON.parse(JSON.stringify(USER_CREDENTIALS));
    var credentials = [];
    for (var _key in _user_credentials) {
        credentials[_key] = {};
        credentials[_key].address = _user_credentials[_key].address;
        credentials[_key].parent_address = _user_credentials[_key].parent_address;
        credentials[_key].password = DEFAULT_PASSWORD;
        credentials[_key].context = _user_credentials[_key].context;
        credentials[_key].username = _user_credentials[_key].username;

        credentials[_key].span = "<span></span>";

        credentials[_key].actions = '<nobr><i class="fa fa-eye" title="Show password"></i>\
<i class="fa fa-eye-slash" style="display:none;" title="Hide password"></i>\
<i class="fa fa-pencil" title="Edit credentials"></i>\
<i class="fa fa-trash-o" title="Delete credentials"></i>\
<i class="fa fa-times" style="display:none;" title="Discard changes"></i>\
<i class="fa fa-floppy-o" style="display:none;" title="Save credentials"></i></nobr>';

        var now = new Date();
        var date = new Date(_user_credentials[_key].date_lastused);
        credentials[_key].date_lastused = MONTH_NAMES[date.getMonth()] + " " + date.getDay();
        if (now - date > 365 * 24 * 60 * 60 * 1000) {
            credentials[_key].date_lastused += ", " + (date.getYear() % 100);
        }
        var date = new Date(_user_credentials[_key].date_modified);
        credentials[_key].date_modified = MONTH_NAMES[date.getMonth()] + " " + date.getDay();
        if (now - date > 365 * 24 * 60 * 60 * 1000) {
            credentials[_key].date_modified += ", " + (date.getYear() % 100);
        }

        if (_user_credentials[_key].favorite) {
            credentials[_key].favorite = '<i class="fa fa-star" title="Remove from favourites"></i>';
        } else {
            credentials[_key].favorite = '<i class="fa fa-star-o" title="Add to favourites"></i>';
        }

        credentials[_key].description = _user_credentials[_key].description;
    }

    return credentials;
}

var get_credential_infos = function (_context, _username) {
    for (var _credential in USER_CREDENTIALS) {
        var credential = USER_CREDENTIALS[_credential];
        if ((credential.context == _context) && (credential.username == _username)) {
            return credential;
        }
    }
}

var get_password = function (_context, _username, _callback) {
    for (var _key in USER_CREDENTIALS) {
        var credential = USER_CREDENTIALS[_key];
        if ((credential.context == _context) && (credential.username == _username)) {
            if ("password" in credential) {
                _callback(true, credential.password);
                return
            }
        }
    }

    mooltipass.app.getPassword(_context, _username, function (_context2, _username2, _status, _password) {
        if (_status.success) {
            // Add password to local user credential data
            for (var _key in USER_CREDENTIALS) {
                var credential = USER_CREDENTIALS[_key];
                if ((credential.context == _context2) && (credential.username == _username2)) {
                    USER_CREDENTIALS[_key].password = _password;
                    USER_CREDENTIALS[_key].password_original = _password;
                }
            }
        }
        else {
            mooltipass.ui.status.error($('#credentials'), _status.msg);
        }

        // Call original callback
        _callback(_status.success, _password);
    });
};