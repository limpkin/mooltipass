if(cIPJQ) {
	var $ = cIPJQ.noConflict(true);
}

$(function() {
	options.initMenu();
	options.initGeneralSettings();
    options.initMooltipassSettings();
    options.initBlacklist();
	options.initSpecifiedCredentialFields();
	options.initAbout();
});


var options = options || {};

options.settings = typeof(localStorage.settings)=='undefined' ? {} : JSON.parse(localStorage.settings);
options.keyRing = typeof(localStorage.keyRing)=='undefined' ? {} : JSON.parse(localStorage.keyRing);

options.initMenu = function() {
	$(".navbar:first ul.nav:first li a").click(function(e) {
		e.preventDefault();
		$(".navbar:first ul.nav:first li").removeClass("active");
		$(this).parent("li").addClass("active");
		$("div.tab").hide();
		$("div.tab#tab-" + $(this).attr("href").substring(1)).fadeIn();
	});

	$("div.tab:first").show();
}

options.initGeneralSettings = function() {
	$("#tab-general-settings input[type=checkbox]").each(function() {
		$(this).attr("checked", options.settings[$(this).attr("name")]);
	});

	$("#tab-general-settings input[type=checkbox]").change(function() {
		options.settings[$(this).attr("name")] = $(this).is(':checked');
		localStorage.settings = JSON.stringify(options.settings);

        chrome.extension.sendMessage({
            action: 'load_settings'
        });
	});

	$("#tab-general-settings input[type=radio]").each(function() {
		if($(this).val() == options.settings[$(this).attr("name")]) {
			$(this).attr("checked", options.settings[$(this).attr("name")]);
		}
	});

	$("#tab-general-settings input[type=radio]").change(function() {
		options.settings[$(this).attr("name")] = $(this).val();
		localStorage.settings = JSON.stringify(options.settings);

        chrome.extension.sendMessage({
            action: 'load_settings'
        });
	});

	chrome.extension.sendMessage({
		action: "get_keepasshttp_versions"
	}, options.showKeePassHttpVersions);

	$("#tab-general-settings button.checkUpdateKeePassHttp:first").click(function(e) {
		e.preventDefault();
		$(this).attr("disabled", true);
		chrome.extension.sendMessage({
			action: "check_update_keepasshttp"
		}, options.showKeePassHttpVersions);
	});
};

options.initMooltipassSettings = function() {
	$("#tab-mooltipass-settings input[type=checkbox]").each(function() {
		$(this).attr("checked", options.settings[$(this).attr("name")]);
	});

	$("#tab-mooltipass-settings input[type=checkbox]").change(function() {
		options.settings[$(this).attr("name")] = $(this).is(':checked');
		localStorage.settings = JSON.stringify(options.settings);

        chrome.extension.sendMessage({
            action: 'load_settings'
        });
	});

	$("#tab-mooltipass-settings input[type=radio]").each(function() {
		if($(this).val() == options.settings[$(this).attr("name")]) {
			$(this).attr("checked", options.settings[$(this).attr("name")]);
		}
	});

	$("#tab-mooltipass-settings input[type=radio]").change(function() {
		options.settings[$(this).attr("name")] = $(this).val();
		localStorage.settings = JSON.stringify(options.settings);

        chrome.extension.sendMessage({
            action: 'load_settings'
        });
	});
}

options.initBlacklist = function() {
    $('#save').button();
    $('#save').click(options.saveBlacklist);

    var blacklist = typeof(localStorage.blacklist)=='undefined' ? [] : JSON.parse(localStorage.blacklist);
    $('#mpBlacklist').val(blacklist.join('\n'));
}

options.saveBlacklist = function() {
    var mpBlacklist = $('#mpBlacklist').val().split('\n');
    mpBlacklist.sort();
    localStorage.blacklist = JSON.stringify(mpBlacklist);
    $('#status').html('Mooltipass Blacklist Updated.');
    setTimeout(function() { $('#status').html(''); }, 750);
}

options.showKeePassHttpVersions = function(response) {
	if(response.current <= 0) {
		response.current = "unknown";
	}
	if(response.latest <= 0) {
		response.latest = "unknown";
	}
	$("#tab-general-settings .kphVersion:first em.yourVersion:first").text(response.current);
	$("#tab-general-settings .kphVersion:first em.latestVersion:first").text(response.latest);

	$("#tab-about em.versionKPH").text(response.current);

	$("#tab-general-settings button.checkUpdateKeePassHttp:first").attr("disabled", false);
}

options.initSpecifiedCredentialFields = function() {
	$("#dialogDeleteSpecifiedCredentialFields").modal({keyboard: true, show: false, backdrop: true});
	$("#tab-specified-fields tr.clone:first button.delete:first").click(function(e) {
		e.preventDefault();
		$("#dialogDeleteSpecifiedCredentialFields").data("url", $(this).closest("tr").data("url"));
		$("#dialogDeleteSpecifiedCredentialFields").data("tr-id", $(this).closest("tr").attr("id"));
		$("#dialogDeleteSpecifiedCredentialFields .modal-body:first strong:first").text($(this).closest("tr").children("td:first").text());
		$("#dialogDeleteSpecifiedCredentialFields").modal("show");
	});

	$("#dialogDeleteSpecifiedCredentialFields .modal-footer:first button.yes:first").click(function(e) {
		$("#dialogDeleteSpecifiedCredentialFields").modal("hide");

		var $url = $("#dialogDeleteSpecifiedCredentialFields").data("url");
		var $trId = $("#dialogDeleteSpecifiedCredentialFields").data("tr-id");
		$("#tab-specified-fields #" + $trId).remove();

		delete options.settings["defined-credential-fields"][$url];
		localStorage.settings = JSON.stringify(options.settings);

        chrome.extension.sendMessage({
            action: 'load_settings'
        });

		if($("#tab-specified-fields table tbody:first tr").length > 2) {
			$("#tab-specified-fields table tbody:first tr.empty:first").hide();
		}
		else {
			$("#tab-specified-fields table tbody:first tr.empty:first").show();
		}
	});

	var $trClone = $("#tab-specified-fields table tr.clone:first").clone(true);
	$trClone.removeClass("clone");
	var counter = 1;
	for(var url in options.settings["defined-credential-fields"]) {
		var $tr = $trClone.clone(true);
		$tr.data("url", url);
		$tr.attr("id", "tr-scf" + counter);
		counter += 1;

		$tr.children("td:first").text(url);
		$("#tab-specified-fields table tbody:first").append($tr);
	}

	if($("#tab-specified-fields table tbody:first tr").length > 2) {
		$("#tab-specified-fields table tbody:first tr.empty:first").hide();
	}
	else {
		$("#tab-specified-fields table tbody:first tr.empty:first").show();
	}
}

options.initAbout = function() {
	$("#tab-about em.versionCIP").text(chrome.app.getDetails().version);
}
