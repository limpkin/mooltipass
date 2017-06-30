/* TODO: define mpDebug debug levels (no magic numbers) */

/* Initialize mooltipass lib */
if (typeof mooltipass == 'undefined') 
{
    mooltipass = {};
}
mooltipass.device = mooltipass.device || {};

/**
 * Mooltipass emulation vars
 */
mooltipass.device.emulation_mode = false
mooltipass.device.emulation_credentials = []
//mooltipass.device.emulation_credentials.push({"domain":"limpkin.fr", "login":"lapin", "password":"test"});

/**
 * Information about connected Mooltipass app
 * Set on mooltipass.device.onSearchForApp()
 */
mooltipass.device._app = null;

/** 
 * Know which browser we're running on: no 'else' to detect odd cases in console
 */
mooltipass.device.browser = "unknown";
if (navigator.userAgent.toLowerCase().indexOf('firefox') > -1)
{
    if (background_debug_msg > 4) mpDebug.log('%c Extension running in Firefox', mpDebug.css('00ffff'));
    mooltipass.device.browser = "firefox";
}
if (typeof(safari) == 'object')
{
    if (background_debug_msg > 4) mpDebug.log('%c Extension running in Safari', mpDebug.css('00ffff'));
    mooltipass.device.browser = "safari";
}
if (window.chrome && chrome.runtime && chrome.runtime.id)
{
    if (background_debug_msg > 4) mpDebug.log('%c Extension running in Chrome', mpDebug.css('00ffff'));
    mooltipass.device.browser = "chrome";
}

/**
 * Contains status information about the device
 * Properties: connected, unlocked, version, state
 */
mooltipass.device._status = 
{
    connected: mooltipass.device.emulation_mode? true:false,                // If device connected to computer
    unlocked: mooltipass.device.emulation_mode? true:false,                 // If device unlocked
    state: mooltipass.device.emulation_mode? "unlocked":"unknown",          // Device state in details
    middleware: mooltipass.device.emulation_mode? "emulator":"unknown",     // String for the middleware 
    firmware_version: "unknown",                                            // Firmware version
    middleware_version: "unknown"                                           // Middleware version
};

/**
 * Boolean information whether the Mooltipass Chrome App was found and is connected
 * Used to speedup periodical requests
 */
mooltipass.device.connectedToApp = false;

/**
 * Flag var to let us know if we're using the chrome app or external App (MooltiApp or Moolticute). Starts false.
 */
mooltipass.device.connectedToExternalApp = false;

/**
 * On initial load, the extension looks for the app to communicate with based on the exact app name
 * WARNING: If you change the app name, you have to also modify this parameter!
 */
mooltipass.device._appName = 'Mooltipass App';

/**
 * Interval of milliseconds to check connection to app and device
 */
mooltipass.device._intervalCheckConnection = 500;

/**
 * TODO: Ping logic to detect timeouts
 */

/**
 * Boolean to know if we saw an unlocked device
 */
mooltipass.device.wasPreviouslyUnlocked = false;

/**
 * Parameters manually set for ansynchronous requests
 */
mooltipass.device._asynchronous = 
{
    // Callback function for returned random string
    'randomCallback': null,
    // Additional parameters for callback function, null or {}
    'randomParameters': null,
    // Callback function for received credentials from device
    'inputCallback': null,
    // Callback function for updated credentials
    'updateCallback': null,
};

/**
 * Queue of requests to retrieve credentials, as the app will always answer each request
 */
 mooltipass.device.retrieveCredentialsQueue = [];
 mooltipass.device.retrieveCredentialsCounter = 0;

/**
 * Requesting a new random string from device only once a minute
 * Minute of latest request stored in this parameter
 * Values: null or number: hour * 60 * minute
 */
mooltipass.device._latestRandomStringRequest = null;

/**
 * In some rare cases websites run some script after loading the credentials fields, which can trigger an onTabUpdatedEvent
 * The variables below allow us to discard any ontabudpatedevent what would come a few ms after getting a credential retrieve request
 */
mooltipass.device.lastRetrieveReqTabId = null;
mooltipass.device.tabUpdatedEventPrevented = false;


/**
 * Reset Mooltipass device status 
 */
mooltipass.device.resetDeviceStatus = function()
{
    mooltipass.device._status = 
    {
        connected: mooltipass.device.emulation_mode? true:false,
        unlocked: mooltipass.device.emulation_mode? true:false,
        state: mooltipass.device.emulation_mode? "unlocked":"unknown",
        middleware: mooltipass.device.emulation_mode? "emulator":"unknown",
        firmware_version: "unknown",
        middleware_version: "unknown"
    };
}

/**
 * Called to inform our logic that the external app is connected and will 
 * be in charge of calling mooltipass.device.messageListener() with the correct message
 */
mooltipass.device.switchToExternalApp = function()
{
    if (mooltipass.device.emulation_mode) return;
    if (background_debug_msg > 4) mpDebug.log('%c Starting to use external app !', mpDebug.css('00ffff'));
    mooltipass.device.connectedToExternalApp = true;
    mooltipass.device.resetDeviceStatus();
    mooltipass.device.sendGetMiddlewareId();
    mooltipass.device.sendPing();           // Needed for MooltiApp to get middleware string
}

/**
 * Called to inform our logic that the external app is disconnected
 */
mooltipass.device.switchToInternalApp = function()
{
    if (mooltipass.device.emulation_mode) return;
    
    /* May be called several times in a row even when already disconnected */
    if (mooltipass.device.connectedToExternalApp)
    {
        if (background_debug_msg > 4) mpDebug.log('%c Stopping to use external app !', mpDebug.css('00ffff'));
        mooltipass.device.connectedToExternalApp = false;
        mooltipass.device.wasPreviouslyUnlocked = false;
        mooltipass.device.retrieveCredentialsQueue = [];
        mooltipass.device.resetDeviceStatus();
    }
}

/**
 * Checks for connected app and triggers search for chrome app otherwise
 * Periodically sends PING to device which returns current status of device
 */
mooltipass.device.checkConnection = function() 
{
    if (mooltipass.device.emulation_mode) return;
    
    if(!mooltipass.device.connectedToApp && !mooltipass.device.connectedToExternalApp) 
    {
        if (mooltipass.device.browser == "chrome") 
        {
            // Search for Mooltipass App
            chrome.management.getAll(mooltipass.device.onSearchForApp);
        } 
        else 
        {
            // Call again this function a bit later to send a ping if we get connected
            setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
        }
    }
    else
    {
        if (mooltipass.device._status.middleware != "moolticute")
        {
            // Ping only when the middleware is known (mooltiapp)
            mooltipass.device.sendPing();
        }
        setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
    }
};

/**
 * Searches for mooltipass app in all available chrome apps
 * Triggers ping to device if app is found and sets mooltipass.device._app
 */
mooltipass.device.onSearchForApp = function(ext) 
{
    var foundApp = false;
    
    // Look for the string describing our app
    for (var i = 0; i < ext.length; i++)
    {
        if (ext[i].shortName == mooltipass.device._appName) 
        {
            if(ext[i]['enabled'] !== true) 
            {
                continue;
            }
            mooltipass.device._app = ext[i];
            foundApp = true;
            break;
        }
    }

    // Didn't find our app, delete id
    if(!foundApp) 
    {
        mooltipass.device._app = null;
    }

    // We found the app, let's talk to it!
    if (mooltipass.device._app != null) 
    {
        if (background_debug_msg > 4) mpDebug.log('%c Found Mooltipass Chrome App', mpDebug.css('00ffff'), mooltipass.device._app.shortName + '" id=' + mooltipass.device._app.id,' app: ', mooltipass.device._app);
        mooltipass.device.connectedToApp = true;
    }
    else 
    {
        if (background_debug_msg > 4) mpDebug.log('%c Did not find Mooltipass Chrome App', mpDebug.css('00ffff'));
        mooltipass.device.connectedToApp = false;
    }

    // Try again later
    setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
};

/**
 * Send ping which triggers status response from device (only to MooltiApp or ChromeApp)
 */
mooltipass.device.sendPing = function() 
{
    /* TODO: when middleware is moolticute, do not send pings */
    if (mooltipass.device.connectedToExternalApp) 
    {
        if (background_debug_msg > 4) mpDebug.log('%c Sending ping to external app ', mpDebug.css('ffeef9'));
        moolticute.sendRequest( { ping: [] } );
    } 
    else if (mooltipass.device.connectedToApp && mooltipass.device._app.id) 
    {
        if (background_debug_msg > 4) mpDebug.log('%c Sending ping to chrome app ', mpDebug.css('ffeef9'));
        chrome.runtime.sendMessage(mooltipass.device._app.id, { ping: [] });
    }
        
    // Send ping which triggers status response from device (only to MooltiApp or ChromeApp)
    //if ( mooltipass.device._status.middleware === 'MooltiApp' || mooltipass.device._status.middleware === 'Chrome App' )
};

/**
 * Get the middleware ID
 */
mooltipass.device.sendGetMiddlewareId = function()
{
    /* Mooltiapp / Chrome App: middleware ID sent back using a ping packet */
    if (mooltipass.device.connectedToExternalApp) 
    {
        if (background_debug_msg > 4) mpDebug.log('%c Sending getmiddlewareid to external app ', mpDebug.css('ffeef9'));
        moolticute.sendRequest( { msg:'get_application_id' } );
    } 
};

/**
 * Send a credential request packet from the current queue
 */
mooltipass.device.sendCredentialRequestMessageFromQueue = function()
{
    if ((mooltipass.device.retrieveCredentialsQueue.length > 0) && (mooltipass.device._status.unlocked == true))
    {
        if (background_debug_msg > 3) mpDebug.log("%c Asking credentials for %s", mpDebug.css('00ff00'), mooltipass.device.retrieveCredentialsQueue[0].domain);
        
        // We are about to send a message: put the follow var to true to prevent message sending by status change
        mooltipass.device.wasPreviouslyUnlocked = true;
        
        // Send the message
        if (mooltipass.device.emulation_mode)
        {
            // Put everything in a timeout to avoid duplicate requests
            setTimeout(function()
            {
                for (var i = 0; i < mooltipass.device.emulation_credentials.length; i++)
                {
                    if (mooltipass.device.emulation_credentials[i]["domain"] == mooltipass.device.retrieveCredentialsQueue[0].domain)
                    {
                        if (background_debug_msg > 3) mpDebug.log("%c Emulation mode: found credential in buffer:", mpDebug.css('00ff00'), mooltipass.device.emulation_credentials[i]);
                        setTimeout(function() 
                        {
                            try
                            {
                                mooltipass.device.retrieveCredentialsQueue[0].callback([
                                    {
                                        Login: mooltipass.device.emulation_credentials[i]["login"],
                                        Name: '<name>',
                                        Uuid: '<Uuid>',
                                        Password: mooltipass.device.emulation_credentials[i]["password"],
                                        StringFields: []
                                    }
                                ], mooltipass.device.retrieveCredentialsQueue[0].tabid == 'safari'?mooltipass.device.retrieveCredentialsQueue[0].tab:mooltipass.device.retrieveCredentialsQueue[0].tabid );
                            }
                            catch(err)
                            {
                                //console.log( err );
                            }
                            // Treat other pending requests
                            mooltipass.device.retrieveCredentialsQueue.shift();
                            mooltipass.device.sendCredentialRequestMessageFromQueue();
                        }, 2000);
                        return;
                    }
                }
                if (background_debug_msg > 3) mpDebug.log("%c Emulation mode: nothing in buffer!", mpDebug.css('00ff00'));
                mooltipass.device.retrieveCredentialsQueue.shift();
                mooltipass.device.sendCredentialRequestMessageFromQueue();
            }, 300);
        }
        else if (mooltipass.device.connectedToExternalApp) 
        {
            moolticute.askPassword(
            {
                'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 
                'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 
                'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain
            });
        } 
        else 
        {
            chrome.runtime.sendMessage(mooltipass.device._app.id, {'getInputs' : {'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain}});
        }
    }     
};

/**
 * Returns the current status of the connection to the device
 * @access backend
 * @returns {{connectedToApp: boolean, connectedToDevice: boolean, deviceUnlocked: boolean}}
 */
mooltipass.device.getStatus = function() 
{
    return {
        'connectedToApp': mooltipass.device.emulation_mode? true:mooltipass.device.connectedToExternalApp || mooltipass.device.connectedToApp,
        'connectedToDevice': mooltipass.device._status.connected,
        'deviceUnlocked': mooltipass.device._status.unlocked,
        'usingChromeApp': mooltipass.device.connectedToApp,
        'usingExternalApp': mooltipass.device.connectedToExternalApp,
        'emulationMode':mooltipass.device.emulation_mode,
        'middleware': mooltipass.device.middleware,
        'firmware_version': mooltipass.device.firmware_version,
        'middleware_version': mooltipass.device.middleware_version
    }; 
};

/**
 * Checks if the device is unlocked
 * @access backend
 * @returns boolean
 */
mooltipass.device.isUnlocked = function() 
{
    return mooltipass.device._status.unlocked;
};

/**
 * Checks if we are connected to an external app
 * @access backend
 * @returns boolean
 */
mooltipass.device.isConnectedToExternalApp = function() 
{
    return mooltipass.device.connectedToExternalApp;
};

/**
 * Generate a random password based on a random string returned from device
 * @access backend
 * @param callback to send the generated password to
 * @param tab current tab object with tab.id
 * @param length of the password
 */
mooltipass.device.generatePassword = function(callback, tab, length) 
{
    if (background_debug_msg > 4) mpDebug.log('%c device: %c generatePassword ','background-color: #e244ff','color: #484848', arguments);

    // unset error message
    page.tabs[tab.id].errorMessage = null;

    // Only request new random string from device once a minute
    // The requested random string is used to salt Math.random() again
    var currentDate = new Date();
    var currentDayMinute = currentDate.getUTCHours() * 60 + currentDate.getUTCMinutes();
    
    if((!mooltipass.device._latestRandomStringRequest || mooltipass.device._latestRandomStringRequest != currentDayMinute) && !mooltipass.device.emulation_mode)
    {    
        mooltipass.device._asynchronous.randomCallback = callback;
        mooltipass.device._asynchronous.randomParameters = {'length': length, tab: tab};
        mooltipass.device._latestRandomStringRequest = currentDayMinute;

        //console.log('mooltipass.generatePassword()', 'request random string from app');
        if (mooltipass.device.connectedToExternalApp) 
        {
            moolticute.getRandomNumbers();
        } 
        else 
        {
            var request = { getRandom : [] };
            chrome.runtime.sendMessage(mooltipass.device._app.id, request);
        }
        return;
    }

    //console.log('mooltipass.generatePassword()', 'use current seed for another password');
    callback( {'seeds': mooltipass.device.generateRandomNumbers(length), 'settings': page.settings}, tab );
};

/**
 * Based on a salted Math.random() generate random numbers
 * @access backend
 * @param length number of random numbers to generate
 * @returns {Array} array of Numbers
 */
mooltipass.device.generateRandomNumbers = function(length) {
    
    var seeds = [];
    for(var i = 0; i < length; i++) 
    {
        seeds.push(Math.random());
    }

    return seeds;
};


/**
 * Add credentials to device
 * @access backend
 * @param callback function to be triggered on response from device
 * @param tab which triggered the storing request
 * @param username
 * @param password
 * @param url
 */
mooltipass.device.addCredentials = function(callback, tab, username, password, url) 
{
    mooltipass.device.updateCredentials(callback, tab, null, username, password, url);
};


/**
 * Update or add credentials to device
 * IMPORTANT: needs to block until a response is received.
 *
 * @access backend
 * @param callback function to be triggered on response from device
 * @param tab which triggered the storing request
 * @param entryId not used
 * @param username
 * @param password
 * @param url
 */
mooltipass.device.updateCredentials = function(callback, tab, entryId, username, password, url) 
{
    if (background_debug_msg > 2) mpDebug.log('%c device: updateCredentials ', mpDebug.css('e244ff') , 'Userame:' + username + ' / Password:' + password );

    // Check that the Mooltipass is unlocked, returns false if it is anything but unlocked
    if(!event.isMooltipassUnlocked()) 
    {
        return;
    }

    // Check for blacklisted
    if (mooltipass.backend.isBlacklisted(url)) 
    {
        if (background_debug_msg > 4) mpDebug.log('%c Blacklisted website:', mpDebug.css('ff0000'), url);
        if (callback) 
        {
            callback('failure');
        }
        return;
    }

    // unset error message
    page.tabs[tab.id].errorMessage = null;

    if ( typeof username === 'object') username = username[0];
    if ( typeof password === 'object') password = password[0];

    request = {update: {context: url, login: username, password: password}};

    // Cancel possible pending request
    mooltipass.device.onTabClosed(tab.id, null);
    mooltipass.device._asynchronous.updateCallback = callback;

    if (mooltipass.device.emulation_mode)
    {
        if (background_debug_msg > 3) mpDebug.log("%c Emulation mode: storing credential in buffer for domain %s : %s & %s", mpDebug.css('00ff00'), url, username, password);
        mooltipass.device.emulation_credentials.push({"domain":url, "login":username, "password":password});
    }
    else if (mooltipass.device.connectedToExternalApp) 
    {
        moolticute.sendRequest( request );
    } 
    else 
    {
        chrome.runtime.sendMessage(mooltipass.device._app.id, request);    
    }   
};

/* 
 * Function called when a tab navigates away
 */
 mooltipass.device.onNavigatedAway = function(tabId, navigationInfo)
 {
    //console.log("Navigated away before retrieving credentials for tab: " + tabId);
    mooltipass.device.lastRetrieveReqTabId = null;

    mooltipass.device.onTabClosed(tabId, navigationInfo);
    mooltipass.device.retrieveCredentialsQueue.splice(0,1);
 }

/*
 * Function called when a tab is closed
 */
mooltipass.device.onTabClosed = function(tabId, removeInfo)
{
    //console.log("Tab closed: " + tabId + " remove info: ", removeInfo);
    
    // Return if queue empty
    if(mooltipass.device.retrieveCredentialsQueue.length == 0)
    {
        return;
    }
    
    /* Check if we have a pending credential request from that tab */    
    if (mooltipass.device.retrieveCredentialsQueue[0].tabid == tabId)
    {
        /* If the device is locked, the first request is actually pending to be sent to the app */
        if (mooltipass.device._status.unlocked == false)
        {
            mooltipass.device.retrieveCredentialsQueue.splice(0,1);
        }
        else
        {
            if (background_debug_msg > 4) mpDebug.log('%c device: onTabClosed ', mpDebug.css('ffff00') , 'Sending cancel request for ' + mooltipass.device.retrieveCredentialsQueue[0].domain + ', reqid: ' +  mooltipass.device.retrieveCredentialsQueue[0].reqid);
            /* Send a cancelling request if it is the tab from which we're waiting an answer */
            if (mooltipass.device.emulation_mode)
            {
            }
            else if (mooltipass.device.connectedToExternalApp) 
            {
                moolticute.cancelRequest( mooltipass.device.retrieveCredentialsQueue[0].reqid, mooltipass.device.retrieveCredentialsQueue[0].domain, mooltipass.device.retrieveCredentialsQueue[0].subdomain );
            } 
            else 
            {
                chrome.runtime.sendMessage(mooltipass.device._app.id, {'cancelGetInputs' : {'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain}});    
            }
        }
    }
    else
    {        
        for (var i = 1; i < mooltipass.device.retrieveCredentialsQueue.length; i++)
        {
            if (mooltipass.device.retrieveCredentialsQueue[i].tabid == tabId)
            {
                mooltipass.device.retrieveCredentialsQueue.splice(i,1);
                return;
            }
        }
    }
}

/*
 * Function called when a tab is updated
 */
mooltipass.device.onTabUpdated = function(tabId, removeInfo) 
{
    if (background_debug_msg > 4) mpDebug.log('%c device: onTabUpdated ', mpDebug.css('e244ff') , 'Tab ID: ' + tabId );
    if ( tabId == mooltipass.device.lastRetrieveReqTabId && removeInfo.status && removeInfo.status == "loading" && removeInfo.url)
    {
        mooltipass.device.onNavigatedAway(tabId, removeInfo);
    }
}

/**
 * Request credentials for given URL
 * @access backend
 * @param callback function to be triggered on response from device
 * @param tab which triggered the request
 * @param url
 * @param submiturl
 * @param forceCallback
 * @param triggerUnlock
 */
mooltipass.device.retrieveCredentials = function(callback, tab, url, submiturl, forceCallback, triggerUnlock) 
{
    if (background_debug_msg > 3) mpDebug.log('%c device: %c retrieveCredentials ','background-color: #e244ff','color: #484848', arguments);

    if (!tab.hasOwnProperty('id')) tab.id = 'safari';

    // unset error message
    //page.tabs[tab.id].errorMessage = null;

    // parse url and check if it is valid and not blacklisted
    var parsed_url = mooltipass.backend.extractDomainAndSubdomain(submiturl);
    if(!parsed_url.valid)
    {
        if (background_debug_msg > 4) mpDebug.log('%c Invalid URL:', mpDebug.css('ff0000'), submiturl);
        if(forceCallback) 
        {
            callback([]);
        }
        return;
    }
    else if(parsed_url.blacklisted) 
    {
        if (background_debug_msg > 4) mpDebug.log('%c Blacklisted website:', mpDebug.css('ff0000'), submiturl);
        if(forceCallback) 
        {
            callback([]);
        }
        return;
    }

    // Check that the Mooltipass is unlocked
    if(!event.isMooltipassUnlocked()) 
    {
        // Don't return if the device is locked, queue the request
        /*if(forceCallback) {
            callback([]);
        }
        return;
        */
    }

    // Store the tab id, prevent a possible very close tabupdatevent action
    mooltipass.device.lastRetrieveReqTabId = tab.id;
    mooltipass.device.tabUpdatedEventPrevented = true;

    // Check if we don't already have a request from this tab
    for (var i = 0; i < mooltipass.device.retrieveCredentialsQueue.length; i++)
    {
        if (mooltipass.device.retrieveCredentialsQueue[i].tabid == tab.id && mooltipass.device.retrieveCredentialsQueue[i].tabupdated == false)
        {
            if (background_debug_msg > 3) mpDebug.log("%c Not storing this new credential request as one is already in the buffer!", mpDebug.css('ff0000'));
            return;
        }
    }
    
    // If our retrieveCredentialsQueue is empty and the device is unlocked, send the request to the app. Otherwise, queue it
    mooltipass.device.retrieveCredentialsQueue.push({'tabid': tab.id, 'callback': callback, 'domain': parsed_url.domain, 'subdomain': parsed_url.subdomain, 'tabupdated': false, 'reqid': mooltipass.device.retrieveCredentialsCounter, 'tab': tab});

    mooltipass.device.retrieveCredentialsCounter++;
    mooltipass.device._asynchronous.inputCallback = callback;

    if(mooltipass.device.retrieveCredentialsQueue.length == 1 && mooltipass.device._status.unlocked == true)
    {
        mooltipass.device.sendCredentialRequestMessageFromQueue();
    }
    else
    {
        if(!mooltipass.device._status.unlocked)
        {
            if (background_debug_msg > 3) mpDebug.log("Mooltipass locked, waiting for unlock", mooltipass.device._status);
        }
        else
        {
            if (background_debug_msg > 3) mpDebug.log("Requests still in the queue, waiting for reply from the app");
        }        
    }
};

/****************************************************************************************************************/

mooltipass.device.messageListener = function(message, sender, sendResponse) {
    if (background_debug_msg > 5) mpDebug.log('%c device: message from device: ', mpDebug.css('e244ff'), message);
    else if (background_debug_msg > 4 && !message.deviceStatus) mpDebug.log('%c device: message from device: ', mpDebug.css('e244ff'), message);

    if ( typeof( message.deviceStatus ) === "undefined" ) message.deviceStatus = null;
    if ( typeof( message.random ) === "undefined" ) message.random = null;
    if ( typeof( message.credentials ) === "undefined" ) message.credentials = null;
    if ( typeof( message.noCredentials ) === "undefined" ) message.noCredentials = null;
    if ( typeof( message.updateComplete ) === "undefined" ) message.updateComplete = null;
    
    
    //console.log('messageListener:', message );
    // Returned on a PING, contains the status of the device
    if (message.deviceStatus !== null) 
    {
        if (background_debug_msg > 5) mpDebug.log('%c device: status from device: ', mpDebug.css('e244ff'), message.deviceStatus);
        mooltipass.device._status = 
        {
            'connected': message.deviceStatus.connected,
            'unlocked': message.deviceStatus.unlocked,
            'state' : message.deviceStatus.state,
            'middleware' : message.deviceStatus.middleware?message.deviceStatus.middleware:'unknown',
            'firmware_version': message.deviceStatus.version?message.deviceStatus.version:'unknown',
            'middleware_version': message.deviceStatus.middleware_version?message.deviceStatus.middleware_version:'unknown'
        };
        if (!message.deviceStatus.connected)
        {
                mooltipass.device.retrieveCredentialsQueue = [];            
        }
        else
        {
            if (!message.deviceStatus.unlocked)
            {
                if (mooltipass.device.wasPreviouslyUnlocked == true)
                {
                    // Cancel pending requests
                    mooltipass.device.retrieveCredentialsQueue = [];
                }
                mooltipass.device.wasPreviouslyUnlocked = false;
            }
            else
            {
                // In case we have pending messages in the queue
                if (mooltipass.device.wasPreviouslyUnlocked == false)
                {
                    mooltipass.device.sendCredentialRequestMessageFromQueue();
                }                
                mooltipass.device.wasPreviouslyUnlocked = true;
            }            
        }
        //console.log(mooltipass.device._status)
    }
    // Returned on request for a random number
    else if (message.random !== null) {
        Math.seedrandom(message.random);
        if(mooltipass.device._asynchronous.randomCallback) {
            mooltipass.device._asynchronous.randomCallback({
                'seeds': mooltipass.device.generateRandomNumbers(mooltipass.device._asynchronous.randomParameters.length),
                'settings': page.settings,
            }, mooltipass.device._asynchronous.randomParameters.tab );
        }
    }
    // Returned on successfully requesting credentials for a specific URL
    else if (message.credentials !== null) 
    {
        if (background_debug_msg > 4) mpDebug.log('%c device: received credentials from app', mpDebug.css('00ff00'));
        try
        {
            mooltipass.device.retrieveCredentialsQueue[0].callback([
                {
                    Login: message.credentials.login,
                    Name: '<name>',
                    Uuid: '<Uuid>',
                    Password: message.credentials.password,
                    StringFields: []
                }
            ], mooltipass.device.retrieveCredentialsQueue[0].tabid == 'safari'?mooltipass.device.retrieveCredentialsQueue[0].tab:mooltipass.device.retrieveCredentialsQueue[0].tabid );
        }
        catch(err)
        {
            //console.log( err );
        }
        // Treat other pending requests
        mooltipass.device.retrieveCredentialsQueue.shift();
        mooltipass.device.sendCredentialRequestMessageFromQueue();
    }
    // Returned on requesting credentials for a specific URL, but no credentials were found
    else if (message.noCredentials !== null) 
    {
        if (background_debug_msg > 4) mpDebug.log('%c device: received empty credentials from app', mpDebug.css('00ff00'));
        try
        {
            mooltipass.device.retrieveCredentialsQueue[0].callback([]);
        }
        catch(err)
        {
        }
        // Treat other pending requests
        mooltipass.device.retrieveCredentialsQueue.shift();
        mooltipass.device.sendCredentialRequestMessageFromQueue();
    }
    // Returned on a completed update of credentials on the device
    else if (message.updateComplete !== null) {
        if (mooltipass.device._asynchronous.updateCallback) {
            try {
                mooltipass.device._asynchronous.updateCallback('success');
            } catch (e) {
                //console.log("Error: " + e);
            }
            mooltipass.device._asynchronous.updateCallback = null;
        }
    }
};

/* Register message listener for chrome */
if (mooltipass.device.browser == "chrome")
{
    chrome.runtime.onMessageExternal.addListener(mooltipass.device.messageListener);
}

/* First call to check connection() */
setTimeout(function() {mooltipass.device.checkConnection();}, 1000);