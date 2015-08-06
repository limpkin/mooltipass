if (mpJQ) {
    var $ = mpJQ.noConflict(true);
}

$(function () {
    options.initGeneralSettings();
    options.initAbout();
    options.initBlacklist();
    options.initCredentialList();
    options.initPasswordGeneratorSettings();
});


var options = options || {};

options.settings = typeof(localStorage.settings) == 'undefined' ? {} : JSON.parse(localStorage.settings);
options.keyRing = typeof(localStorage.keyRing) == 'undefined' ? {} : JSON.parse(localStorage.keyRing);

options.initGeneralSettings = function () {
    $(".settings input[type=checkbox]").each(function () {
        $(this).attr("checked", options.settings[$(this).attr("name")]);
    });

    $(".settings input[type=checkbox]").change(function () {
        options.settings[$(this).attr("name")] = $(this).is(':checked');
        localStorage.settings = JSON.stringify(options.settings);

        chrome.extension.sendMessage({
            action: 'load_settings'
        });
    });

    $(".settings input[type=radio]").each(function () {
        if ($(this).val() == options.settings[$(this).attr("name")]) {
            $(this).attr("checked", options.settings[$(this).attr("name")]);
        }
    });

    $(".settings input[type=radio]").change(function () {
        options.settings[$(this).attr("name")] = $(this).val();
        localStorage.settings = JSON.stringify(options.settings);

        chrome.extension.sendMessage({
            action: 'load_settings'
        });
    });
};

options.initAbout = function () {
    $("#contributor-list").each(function () {
        $.getJSON("https://api.github.com/repos/limpkin/mooltipass/contributors", function (contributors) {
            $("#contributor-list").html("");
            for (_contributor in contributors) {
                contributor = contributors[_contributor];
                e = $("<a class='pure-u-1-3 contributor' href='" + contributor.html_url + "'><img src='" + contributor.avatar_url + "' />" + contributor.login + "</a>");
                $("#contributor-list").append(e);
            }
        });
    });
}

options.isEmpty = function (dict) {
    var keys = [];
    for (var key in dict) {
        if (key != null) keys.push(key);
    }
    return (keys.length == 0)
}

options.initBlacklist = function () {
    $("#blacklisted-urls").each(function () {

        // get blacklist from storage, or create an empty one if none exists
        options.blacklist = typeof(localStorage.mpBlacklist) == 'undefined' ? {} : JSON.parse(localStorage.mpBlacklist);

        if (options.isEmpty(options.blacklist)) {
            $("#no-blacklisted-urls").show();
            return;
        }
        ;
        $("#no-blacklisted-urls").hide();

        $(this).html("");
        for (var url in options.blacklist) {
            $element = $("<tr data-url='" + url + "'><td><span class='value'>" + url + "</span><span class='remove'><i class='fa fa-times'></i></span></td>");
            $element.appendTo($(this));
        }

        $("#blacklisted-urls .remove").click(function () {
            var url = $(this).closest('tr').attr('data-url');
            delete options.blacklist[url];
            localStorage.mpBlacklist = JSON.stringify(options.blacklist);
            chrome.extension.sendMessage({action: 'load_settings'});

            $(this).closest('tr').remove();

            if (options.isEmpty(options.blacklist)) {
                $("#no-blacklisted-urls").show()
            }
        });


        $('#tab-blacklist tr.clone:first button.delete:first').click(function (e) {
            var url = $(this).closest('tr').data('url');
            var id = $(this).closest('tr').attr('id');
            $('#tab-blacklist #' + id).remove();
            delete options.blacklist[url];
            localStorage.mpBlacklist = JSON.stringify(options.blacklist);
            chrome.extension.sendMessage({action: 'load_settings'});
        });

        var trClone = $("#tab-blacklist table tr.clone:first").clone(true);
        trClone.removeClass("clone");

        var index = 1;
        for (var url in options.blacklist) {
            var tr = trClone.clone(true);
            tr.data('url', url);
            tr.attr('id', 'tr-scf' + index);
            tr.children('td:first').text(url);
            $('#tab-blacklist table tbody:first').append(tr);
            index++;
        }

        if ($('#tab-blacklist table tbody:first tr').length > 2) {
            $('#tab-blacklist table tbody:first tr.empty:first').hide();
        }
        else {
            $('#tab-blacklist table tbody:first tr.empty:first').show();
        }
    });
}

options.initCredentialList = function () {
    $("#credential-urls").each(function () {

        // get blacklist from storage, or create an empty one if none exists

        if (options.isEmpty(options.settings["defined-credential-fields"])) {
            $("#no-credential-urls").show();
            return;
        }
        ;
        $("#no-credential-urls").hide();

        $(this).html("");
        for (var url in options.settings["defined-credential-fields"]) {
            $element = $("<tr data-url='" + url + "'><td><span class='value'>" + url + "</span><span class='remove'><i class='fa fa-times'></i></span></td>");
            $element.appendTo($(this));
        }

        $("#credential-urls .remove").click(function () {
            var url = $(this).closest('tr').attr('data-url');

            delete options.settings["defined-credential-fields"][url];
            localStorage.settings = JSON.stringify(options.settings);
            chrome.extension.sendMessage({action: 'load_settings'});

            $(this).closest('tr').remove();

            if (options.isEmpty(options.settings["defined-credential-fields"])) {
                $("#no-credential-urls").show()
            }
        });
    });
}

options.initPasswordGeneratorSettings = function () {
    $("*[name='usePasswordGenerator']").click(function () {
        if (this.checked) {
            $("#password-generator-settings").fadeIn(200);
        } else {
            $("#password-generator-settings").fadeOut(200);
        }
    });
    $("*[name='usePasswordGenerator']").each(function () {
        if (this.checked) {
            $("#password-generator-settings").fadeIn(0);
        } else {
            $("#password-generator-settings").fadeOut(0);
        }
    });


    $("*[name='usePasswordGeneratorLength']").val(options.settings['usePasswordGeneratorLength']);
    $("*[name='usePasswordGeneratorLength']").change(function () {
        $(this).val(parseInt($(this).val()));

        min = parseInt($(this).attr("min"));
        max = parseInt($(this).attr("max"));
        val = parseInt($(this).val());

        if (val > max) $(this).val(max);
        if (val < min) $(this).val(min);

        options.settings[$(this).attr("name")] = $(this).val();
        localStorage.settings = JSON.stringify(options.settings);

        chrome.extension.sendMessage({
            action: 'load_settings'
        });
    });

    $('#password-generator-settings .checkbox.password-generator-characters input').change(function () {
        options.disableLastCharCheckboxOnPasswordGenerator();
    });

    if (!options.disableLastCharCheckboxOnPasswordGenerator()) {
        $('#password-generator-settings .checkbox.password-generator-characters input:first').prop('checked', true);
        options.disableLastCharCheckboxOnPasswordGenerator();
    }
};

options.disableLastCharCheckboxOnPasswordGenerator = function () {
    var checked = [];
    $('#password-generator-settings .checkbox.password-generator-characters input').each(function (no, el) {
        if ($(el).is(':checked')) {
            checked.push(el);
        }
    });

    if (checked.length == 1) {
        $(checked[0]).prop('disabled', true);
    }
    else {
        $.each(checked, function (no, item) {
            $(item).prop('disabled', false);
        });
    }

    return checked.length > 0;
}