// Detect if we're dealing with Firefox or Chrome
var isFirefox = navigator.userAgent.toLowerCase().indexOf('firefox') > -1;

// contains already called method names
var _called = {};
var background_debug_msg = (window.chrome && chrome.runtime && !('update_url' in chrome.runtime.getManifest()))? 55 : false;

var mpDebug = {
    css: function( backgroundColor ) {
        return 'background-color: #' + backgroundColor + '; padding: 3px 10px;';
    }
};

if (background_debug_msg) {
    mpDebug.log = function( message ) {
        this.log( message );
    }
    mpDebug.log = console.log.bind(window.console);
    mpDebug.warn = console.warn.bind(window.console);
    mpDebug.trace = console.trace.bind(window.console);
    mpDebug.error = console.error.bind(window.console);
} else {
    mpDebug.log = function() {}
    mpDebug.log = function() {}
    mpDebug.warn = function() {}
    mpDebug.trace = function() {}
    mpDebug.error = function() {}
}

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
    if (isSafari) {
        var theFunction = function(newIcon) {
            var iconUri = safari.extension.baseURI.replace(/\/$/, "") + newIcon.path;
            safari.extension.toolbarItems[0].image = iconUri;
        };
    } else if ( isFirefox ) {
        var theFunction = browser.browserAction.setIcon;
    } else {
        var theFunction = chrome.browserAction.setIcon;
    }

    theFunction({
        tabId: page.currentTabId,
        path: "/images/icon_" + icon_name + "_19.png"
    });
}

mooltipass.backend.updateStatusIcon = function() {
    var status = mooltipass.device.getStatus();
    if (status['deviceUnlocked']) {
        iconName = "normal";
    } else {
        iconName = "cross";
    }

    if ( !isSafari ) {
        if ( typeof chrome.notifications.getPermissionLevel == 'function') {
            chrome.notifications.getPermissionLevel(function(response) {
                if (response == 'denied') {
                    iconName += "_warning";
                }
            });  
        }
    }

    mooltipass.backend.setStatusIcon(iconName);
}
mooltipass.backend._updateStatusIcon = function() {
    mooltipass.backend.updateStatusIcon();
    setTimeout(mooltipass.backend._updateStatusIcon, 500);
}

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
    if (background_debug_msg > 4) mpDebug.log('%c backend: %c isBlacklisted ','background-color: #ffc107','color: #000', url, url in mooltipass.backend._blacklist);
    return url in mooltipass.backend._blacklist;
}

/**
 * Adds an URL to the blacklist
 * @access backend
 * @param url
 */
mooltipass.backend.blacklistUrl = function(url) {
    if (background_debug_msg > 4) mpDebug.log('%c backend: %c got blacklist req. for ','background-color: #ffc107','color: #000', url);

    if(url.indexOf('://') > -1) {
        var parsed_url = mooltipass.backend.extractDomainAndSubdomain(url);
        var subdomain;
        var domain;

        // See if our script detected a valid domain & subdomain
        if(!parsed_url.valid)
        {
            if (background_debug_msg > 4) mpDebug.log('%c backend: %c Invalid URL for blacklisting given','background-color: #ffc107','color: #000', url);
            return;
        }

        domain = parsed_url.domain;
        subdomain = parsed_url.subdomain;

        url = domain;
        if(subdomain != null) {
            url = subdomain + '.' + domain;
        }
    }

    //console.log( mooltipass.backend._blacklist );

    mooltipass.backend._blacklist[url] = true;
    localStorage.mpBlacklist = JSON.stringify(mooltipass.backend._blacklist);
};

mooltipass.backend.handlerBlacklistUrl = function(callback, tab, url) {
    if (background_debug_msg > 4) mpDebug.log('%c backend: %c handlerBlacklistUrl','background-color: #ffc107','color: #000', url);
    mooltipass.backend.blacklistUrl(url);
    callback(true);
    if (background_debug_msg > 4) mpDebug.log('%c backend: %c updated blacklist store ','background-color: #ffc107','color: #000', url);
}

/**
 * Removes an URL from the blacklist
 * @access backend
 * @param url
 */
mooltipass.backend.unblacklistUrl = function(url) {
    if (background_debug_msg > 4) mpDebug.log('%c backend: %c got blacklist removal req. for','background-color: #ffc107','color: #000', url);

    if(url.indexOf('://') > -1) {
        var parsed_url = mooltipass.backend.extractDomainAndSubdomain(url);
        var subdomain;
        var domain;

        // See if our script detected a valid domain & subdomain
        if(!parsed_url.valid)
        {
            if (background_debug_msg > 4) mpDebug.log('%c backend: %c Invalid URL for blacklisting given:','background-color: #ffc107','color: #000', url);
            return;
        }

        domain = parsed_url.domain;
        subdomain = parsed_url.subdomain;

        url = domain;
        if(subdomain != null) {
            url = subdomain;
        }
    }

    delete mooltipass.backend._blacklist[url];
    localStorage.mpBlacklist = JSON.stringify(mooltipass.backend._blacklist);
    if (background_debug_msg > 4) mpDebug.log('%c backend: %c updated blacklist store ','background-color: #ffc107','color: #000', url);
};

mooltipass.backend.handlerUnBlacklistUrl = function(callback, tab, url) {
    mooltipass.backend.unblacklistUrl(url);
    callback(true);
}

/**
 * Extract domain and subdomain from a given URL and checks whether the URL is valid at all
 * @access backend
 * @param url
 * @returns {{valid: {boolean}, domain: {string|null}, subdomain: {string|null}}}
 */
mooltipass.backend.extractDomainAndSubdomain = function ( url ) {
    if (background_debug_msg > 4) mpDebug.log('%c backend: %c extractDomainAndSubdomain ','background-color: #ffc107','color: #000', url);

    var toReturn = { url: url, valid: false, domain: null, subdomain: null, blacklisted: false, port: null };
    
    // Don't know why this is here, leaving it just in case
    toReturn.url = toReturn.url.replace('www.', 'wWw.');

    // URL trimming
    // Remove possible www.
    toReturn.url = toReturn.url.replace(/:\/\/www./ig, '://');
    toReturn.url = toReturn.url.replace(/^www\./ig, '');
    // Remove everything before ://
    //    also ensure that only the first :// is used
    //    (negative example: https://id.atlassian.com/login?continue=https://my.atlassian.com&application=mac)
    toReturn.url = toReturn.url.replace(/^[^:]+:\/\//ig, "");
    // Remove everything after first /
    var n = toReturn.url.indexOf('/');
    toReturn.url = toReturn.url.substring(0, n != -1 ? n : url.length);
    // Remove everything after first : and save as port.
    var n = toReturn.url.indexOf(':');
    toReturn.port = toReturn.url.substring(n != -1 ? n + 1 : toReturn.url.length);
    toReturn.url = toReturn.url.substring(0, n != -1 ? n : toReturn.url.length);
    // Remove possible starting '.', (residual from www[number] urls)
    if((toReturn.url.length > 0) && (toReturn.url.charAt(0) == '.')) {
        toReturn.url = toReturn.url.substring(1);
    }

    if(psl.isValid(toReturn.url)) {
        // Managed to extract a domain using the public suffix list
        toReturn.valid = true;
        var parsed = psl.parse( String(toReturn.url) );
        toReturn.domain = parsed.domain + (toReturn.port ? ':' + toReturn.port : '');
        toReturn.subdomain = parsed.subdomain;
    } else {
        // Check if it is an ip address or localhost.
        var ipV4Pattern = /^\s*(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\s*$/;
        var ipV6Pattern = /^\s*((([0-9A-Fa-f]{1,4}:){7}([0-9A-Fa-f]{1,4}|:))|(([0-9A-Fa-f]{1,4}:){6}(:[0-9A-Fa-f]{1,4}|((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){5}(((:[0-9A-Fa-f]{1,4}){1,2})|:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3})|:))|(([0-9A-Fa-f]{1,4}:){4}(((:[0-9A-Fa-f]{1,4}){1,3})|((:[0-9A-Fa-f]{1,4})?:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){3}(((:[0-9A-Fa-f]{1,4}){1,4})|((:[0-9A-Fa-f]{1,4}){0,2}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){2}(((:[0-9A-Fa-f]{1,4}){1,5})|((:[0-9A-Fa-f]{1,4}){0,3}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(([0-9A-Fa-f]{1,4}:){1}(((:[0-9A-Fa-f]{1,4}){1,6})|((:[0-9A-Fa-f]{1,4}){0,4}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:))|(:(((:[0-9A-Fa-f]{1,4}){1,7})|((:[0-9A-Fa-f]{1,4}){0,5}:((25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)){3}))|:)))(%.+)?\s*$/;
        var ipV4Array = toReturn.url.match(ipV4Pattern);
        var ipV6Array = toReturn.url.match(ipV6Pattern);
        if(ipV4Array != null || ipV6Array != null || toReturn.url == 'localhost') {
            toReturn.valid = true;
            toReturn.domain = toReturn.url + (toReturn.port ? ':' + toReturn.port : '');
            toReturn.subdomain = null;
        }
    }

    if ( mooltipass.backend.isBlacklisted( toReturn.domain ) || mooltipass.backend.isBlacklisted( toReturn.subdomain ) ) {
        toReturn.blacklisted = true;
    }

    if (background_debug_msg > 3) mpDebug.log('%c backend: %c extractDomainAndSubdomain results for: ' + url ,'background-color: #ffc107','color: #000', toReturn);
    return toReturn;
}

setTimeout( function() {
    mooltipass.backend._updateStatusIcon();
},500);
