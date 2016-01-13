/* Initialize mooltipass lib */
if (typeof mooltipass == 'undefined') {
    mooltipass = {};
}
mooltipass.backend = mooltipass.backend || {};


/**
 * Stored blacklisted websites
 * Information are saved in the local storage of this extension in Chrome
 */
mooltipass.backend._blacklist = typeof(localStorage.mpBlacklist)=='undefined' ? {} : JSON.parse(localStorage.mpBlacklist);

/**
 * Disable notificatons about unlocked device
 * TODO: add this parameter to the settings dialog and let the user decide
 */
mooltipass.backend.disableNonUnlockedNotifications = false;



mooltipass.backend.setStatusIcon = function(icon_name) {
    chrome.browserAction.setIcon({
        tabId: page.currentTabId,
        path: "/images/icon_" + icon_name + "_19.png"
    });    
}

mooltipass.backend.updateStatusIcon = function() {
    if (mooltipass.device.getStatus()['deviceUnlocked']) {
        iconName = "normal";
    } else {
        iconName = "cross";
    }
    chrome.notifications.getPermissionLevel(function(response) {
        if (response == 'denied') {
            iconName += "_warning";
        }
        mooltipass.backend.setStatusIcon(iconName);
    });
}
mooltipass.backend._updateStatusIcon = function() {
    mooltipass.backend.updateStatusIcon();
    setTimeout(mooltipass.backend._updateStatusIcon, 500);
}
mooltipass.backend._updateStatusIcon();


/**
 * Load the backend settings of this extension
 * @access: backend
 */
mooltipass.backend.loadSettings = function() {
    mooltipass.backend._blacklist = typeof(localStorage.mpBlacklist)=='undefined' ? {} : JSON.parse(localStorage.mpBlacklist);
}

/**
 * Checks whether a given URL is blacklisted
 * @access backend
 * @param url
 * @returns {boolean}
 */
mooltipass.backend.isBlacklisted = function(url) {
    return url in mooltipass.backend._blacklist;
}

/**
 * Adds an URL to the blacklist
 * @access backend
 * @param url
 */
mooltipass.backend.blacklistUrl = function(url) {
    console.log('got blacklist req. for', url);
    mooltipass.backend._blacklist[url] = true;
    localStorage.mpBlacklist = JSON.stringify(mooltipass.backend._blacklist);
    console.log('updated blacklist store');
};

mooltipass.backend.handlerBlacklistUrl = function(callback, tab, url) {
    console.log('backlist:', url);
    mooltipass.backend.blacklistUrl(url);
    callback(true);
}

/**
 * Extract domain and subdomain from a given URL and checks whether the URL is valid at all
 * @access backend
 * @param url
 * @returns {{valid: {boolean}, domain: {string|null}, subdomain: {string|null}}}
 */
mooltipass.backend.extractDomainAndSubdomain = function (url) {
    var url_valid;
    var domain = null;
    var subdomain = null;

    url = url.replace('www.', 'wWw.');
    console.log("Parsing ", url);

    // URL trimming
    // Remove possible www.
    url = url.replace(/:\/\/www./ig, '://');
    url = url.replace(/^www\./ig, '');
    // Remove everything before ://
    //    also ensure that only the first :// is used
    //    (negative example: https://id.atlassian.com/login?continue=https://my.atlassian.com&application=mac)
    url = url.replace(/^[^:]+:\/\//ig, "");
    // Remove everything after first /
    var n = url.indexOf('/');
    url = url.substring(0, n != -1 ? n : url.length);
    // Remove everything after first :
    var n = url.indexOf(':');
    url = url.substring(0, n != -1 ? n : url.length);
    console.log("Trimmed URL: ", url)

    if(psl.isValid(url))
    {
        // Managed to extract a domain using the public suffix list
        console.log("valid URL detected")

        url_valid = true;
        var parsed = psl.parse(String(url))
        domain = parsed.domain;
        subdomain = parsed.subdomain;

        console.log("Extracted domain: ", domain);
        console.log("Extracted subdomain: ", subdomain);
    }
    else
    {
        // Check if it is an ip address
        var ipV4Pattern = /^\s*(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\s*$/;
        var ipV6Pattern = /^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$/;
        var ipV4Array = url.match(ipV4Pattern);
        var ipV6Array = url.match(ipV6Pattern);
        if(ipV4Array != null)
        {
            url_valid = true;
            domain = url;
            subdomain = null;
            console.log("ip v4 address detected")
        }
        else if(ipV6Array != null)
        {
            url_valid = true;
            domain = url;
            subdomain = null;
            console.log("ip v6 address detected")
        }
        else
        {
            url_valid = false;
            console.log("invalid URL detected")
        }
    }

    return {valid: url_valid, domain: domain, subdomain: subdomain}
}