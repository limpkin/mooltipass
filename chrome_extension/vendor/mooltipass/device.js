/* Initialize mooltipass lib */
if (typeof mooltipass == 'undefined') {
    mooltipass = {};
}

mooltipass.device = mooltipass.device || {};

/**
 * Information about connected Mooltipass app
 * Set on mooltipass.device.onSearchForApp()
 */
mooltipass.device._app = null;

/**
 * Contains status information about the device
 * Properties: connected, unlocked, version, state
 */
mooltipass.device._status = {};

/**
 * Boolean information whether the Mooltipass app was found and is connected
 * Used to speedup periodical requests
 */
mooltipass.device.connectedToApp = false;

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
 * Boolean to know if we saw an unlocked device
 */
mooltipass.device.wasPreviouslyUnlocked = false;

/**
 * Parameters manually set for ansynchronous requests
 */
mooltipass.device._asynchronous = {
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
 * Flag var to let us know if we're using the APP or Moolticute. Starts false.
 */
mooltipass.device.usingApp = false;

/**
 * Checks for connected app and triggers search for app otherwise
 * Periodically sends PING to device which returns current status of device
 */
mooltipass.device.checkConnection = function() {
    if(!mooltipass.device.connectedToApp && !moolticute.connectedToDaemon) {
        // Search for Mooltipass App
        chrome.management.getAll(mooltipass.device.onSearchForApp);
        return;
    }

    mooltipass.device.sendPing();
    setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
};

/**
 * Searches for mooltipass app in all available chrome apps
 * Triggers ping to device if app is found and sets mooltipass.device._app
 */
mooltipass.device.onSearchForApp = function(ext) {
    var foundApp = false;
    for (var i = 0; i < ext.length; i++) {
        if (ext[i].shortName == mooltipass.device._appName) {
            if(ext[i]['enabled'] !== true) {
                continue;
            }
            mooltipass.device._app = ext[i];
            foundApp = true;
            break;
        }
    }

    if(!foundApp) {
        mooltipass.device._app = null;
    }

    if (mooltipass.device._app != null) {
        mooltipass.device.connectedToApp = true;
        mooltipass.device.sendPing();
        //console.log('found mooltipass app "' + mooltipass.device._app.shortName + '" id=' + mooltipass.device._app.id,' app: ', mooltipass.device._app);
    }
    else {
        mooltipass.device.connectedToApp = false;
        mooltipass.device._status = {};
        //console.log('No mooltipass app found');
    }

    setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
};

mooltipass.device.sendPing = function() {
    // Send ping which triggers status response from device (only to MooltiApp or ChromeApp)
    //if ( mooltipass.device._status.middleware === 'MooltiApp' || mooltipass.device._status.middleware === 'Chrome App' ) {
        if (moolticute.connectedToDaemon) {
            moolticute.sendRequest( { ping: [] } );
        } else if ( mooltipass.device._app && mooltipass.device._app.id ) {
            chrome.runtime.sendMessage(mooltipass.device._app.id, { ping: [] });
        }
    //}
}

/**
 * Returns the current status of the connection to the device
 * @access backend
 * @returns {{connectedToApp: boolean, connectedToDevice: boolean, deviceUnlocked: boolean}}
 */
mooltipass.device.getStatus = function() {
    if (moolticute.connectedToDaemon) {
        if ( mooltipass.device.usingApp ) mooltipass.device.stopUsingApp();
        return {
            'connectedToApp': moolticute.connectedToDaemon,
            'connectedToDevice': moolticute.status.connected,
            'deviceUnlocked': moolticute.status.unlocked
        }
    }

    if (! mooltipass.device.usingApp ) mooltipass.device.useApp();
    return {
        'connectedToApp': mooltipass.device._app ? true : false,
        'connectedToDevice': mooltipass.device._status.connected,
        'deviceUnlocked': mooltipass.device._status.unlocked
    };    

};

/**
 * Checks if the device is unlocked
 * @access backend
 * @returns boolean
 */
mooltipass.device.isUnlocked = function() {
    return mooltipass.device.getStatus()['deviceUnlocked'];
};


/**
 * Generate a random password based on a random string returned from device
 * @access backend
 * @param callback to send the generated password to
 * @param tab current tab object with tab.id
 * @param length of the password
 */
mooltipass.device.generatePassword = function(callback, tab, length) {
    if (background_debug_msg > 4) mpDebug.log('%c device: %c generatePassword ','background-color: #e244ff','color: #484848', arguments);

    // unset error message
    page.tabs[tab.id].errorMessage = null;

    // Only request new random string from device once a minute
    // The requested random string is used to salt Math.random() again
    var currentDate = new Date();
    var currentDayMinute = currentDate.getUTCHours() * 60 + currentDate.getUTCMinutes();
    if(!mooltipass.device._latestRandomStringRequest || mooltipass.device._latestRandomStringRequest != currentDayMinute) {    
        mooltipass.device._asynchronous.randomCallback = callback;
        mooltipass.device._asynchronous.randomParameters = {'length': length, tab: tab};
        mooltipass.device._latestRandomStringRequest = currentDayMinute;

        //console.log('mooltipass.generatePassword()', 'request random string from app');
        if (moolticute.connectedToDaemon) {
            moolticute.getRandomNumbers();
        } else {
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
    for(var i = 0; i < length; i++) {
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
mooltipass.device.addCredentials = function(callback, tab, username, password, url) {
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
mooltipass.device.updateCredentials = function(callback, tab, entryId, username, password, url) {
    if (background_debug_msg > 2) mpDebug.log('%c device: updateCredentials ', mpDebug.css('e244ff') , 'Userame:' + username + ' / Password:' + password );
    //TODO: Trigger unlock if device is connected but locked
    // Check that the Mooltipass is unlocked
    if(!event.isMooltipassUnlocked()) {
        return;
    }

    if (mooltipass.backend.isBlacklisted(url)) {
        //console.log('notify: ignoring blacklisted url',url);
        if (callback) {
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

    if (moolticute.connectedToDaemon) {
        moolticute.sendRequest( request );
    } else {
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

    mooltipass.device.onTabClosed(tabId, navigationInfo );
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
            /* Send a cancelling request if it is the tab from which we're waiting an answer */
            if (moolticute.connectedToDaemon) {
                moolticute.cancelRequest( mooltipass.device.retrieveCredentialsQueue[0].reqid, mooltipass.device.retrieveCredentialsQueue[0].domain, mooltipass.device.retrieveCredentialsQueue[0].subdomain );
            } else {
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
mooltipass.device.onTabUpdated = function(tabId, removeInfo) {
    if (background_debug_msg > 4) mpDebug.log('%c device: onTabUpdated ', mpDebug.css('e244ff') , 'Tab ID: ' + tabId );
    if ( tabId == mooltipass.device.lastRetrieveReqTabId && removeInfo.status && removeInfo.status == "loading" && removeInfo.url )
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
mooltipass.device.retrieveCredentials = function(callback, tab, url, submiturl, forceCallback, triggerUnlock) {
    if (background_debug_msg > 3) mpDebug.log('%c device: %c retrieveCredentials ','background-color: #e244ff','color: #484848', arguments);

    if (!tab.id) tab.id = 'safari';

    // unset error message
    //page.tabs[tab.id].errorMessage = null;

    // parse url and check if it is valid and not blacklisted
    var parsed_url = mooltipass.backend.extractDomainAndSubdomain(submiturl);
    if( !parsed_url.valid || parsed_url.blacklisted ) {
        if(forceCallback) {
            callback([]);
        }
        return;
    }

    //TODO: Trigger unlock if device is connected but locked
    // Check that the Mooltipass is unlocked
    if( !event.isMooltipassUnlocked() ) {
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
            if (background_debug_msg > 3) mpDebug.log("Not storing this new credential request as one is already in the buffer!");
            return;
        }
    }
    
    // If our retrieveCredentialsQueue is empty and the device is unlocked, send the request to the app. Otherwise, queue it
    mooltipass.device.retrieveCredentialsQueue.push({'tabid': tab.id, 'callback': callback, 'domain': parsed_url.domain, 'subdomain': parsed_url.subdomain, 'tabupdated': false, 'reqid': mooltipass.device.retrieveCredentialsCounter, 'tab': tab});

    mooltipass.device.retrieveCredentialsCounter++;
    mooltipass.device._asynchronous.inputCallback = callback;

    if(mooltipass.device.retrieveCredentialsQueue.length == 1 && mooltipass.device._status.unlocked == true)
    {
        // We are about to send a message: put the follow var to true to prevent message sending by status change
        mooltipass.device.wasPreviouslyUnlocked = true;
        if (moolticute.connectedToDaemon) {
            moolticute.askPassword({
                'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 
                'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 
                'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain
            });
        } else {
            chrome.runtime.sendMessage(mooltipass.device._app.id, {'getInputs' : {'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain}});
        }
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
        mooltipass.device._status = 
        {
            'connected': message.deviceStatus.connected,
            'unlocked': message.deviceStatus.unlocked,
            'version': message.deviceStatus.version,
            'state' : message.deviceStatus.state,
            'middleware' : message.deviceStatus.middleware?message.deviceStatus.middleware:'unknown'
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
                if ((mooltipass.device.wasPreviouslyUnlocked == false) && (mooltipass.device.retrieveCredentialsQueue.length > 0))
                {
                    //console.log('sending to ' + mooltipass.device._app.id);
                    chrome.runtime.sendMessage(mooltipass.device._app.id, {'getInputs' : {'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain}});
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
        if(mooltipass.device.retrieveCredentialsQueue.length > 0)
        {
            //console.log('sending to ' + mooltipass.device._app.id);
            chrome.runtime.sendMessage(mooltipass.device._app.id, {'getInputs' : {'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain}});
        }
    }
    // Returned on requesting credentials for a specific URL, but no credentials were found
    else if (message.noCredentials !== null) 
    {
        try
        {
            mooltipass.device.retrieveCredentialsQueue[0].callback([]);
        }
        catch(err)
        {
        }
        // Treat other pending requests
        mooltipass.device.retrieveCredentialsQueue.shift();
        if(mooltipass.device.retrieveCredentialsQueue.length > 0)
        {
            //console.log('sending to ' + mooltipass.device._app.id);
            chrome.runtime.sendMessage(mooltipass.device._app.id, {'getInputs' : {'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain}});
        }
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

/*
 * Starts listening for messages from the APP
 * 
*/
mooltipass.device.useApp = function() {
    if (background_debug_msg > 3) mpDebug.log('%c device: useApp ', mpDebug.css('e244ff') );
    mooltipass.device.usingApp = true;
    chrome.management.getAll(mooltipass.device.onSearchForApp);
    chrome.runtime.onMessageExternal.addListener( mooltipass.device.messageListener );
}

/*
 * Stops using the APP and goes with Moolticute
 * 
*/
mooltipass.device.stopUsingApp = function() {
    if (background_debug_msg > 3) mpDebug.log('%c device: stopUsingApp ', mpDebug.css('e244ff') );
    mooltipass.device.usingApp = false;
    mooltipass.device._app = { enabled: true  };
    if ( !isFirefox && !isSafari ) chrome.runtime.onMessageExternal.removeListener( mooltipass.device.messageListener );
    mooltipass.device.checkConnection();
}

/* Initialize device specific settings */
setTimeout( function() {
    // Try to use the app at first.
    if ( !isFirefox && !isSafari ) mooltipass.device.useApp();
    else mooltipass.device.stopUsingApp();
 },100);