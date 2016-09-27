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
 * Checks for connected app and triggers search for app otherwise
 * Periodically sends PING to device which returns current status of device
 */
mooltipass.device.checkConnection = function() {
    // When using moolticute, don't lookup for app
    if (page.settings.useMoolticute) {
        return;
    }

    if(!mooltipass.device.connectedToApp) {
        // Search for Mooltipass App
        chrome.management.getAll(mooltipass.device.onSearchForApp);
        return;
    }

    chrome.runtime.sendMessage(mooltipass.device._app.id, { ping: [] });
    setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
};

/**
 * Searches for mooltipass app in all available chrome apps
 * Triggers ping to device if app is found and sets mooltipass.device._app
 */
mooltipass.device.onSearchForApp = function(ext) {
    // 
    if (page.settings.useMoolticute) {
        return;
    }

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
        // Send ping which triggers status response from device
        chrome.runtime.sendMessage(mooltipass.device._app.id, { ping: [] });

        console.log('found mooltipass app "' + mooltipass.device._app.shortName + '" id=' + mooltipass.device._app.id,' app: ', mooltipass.device._app);
    }
    else {
        mooltipass.device.connectedToApp = false;
        mooltipass.device._status = {};
        console.log('No mooltipass app found');
    }

    setTimeout(mooltipass.device.checkConnection, mooltipass.device._intervalCheckConnection);
};

/**
 * Returns the current status of the connection to the device
 * @access backend
 * @returns {{connectedToApp: boolean, connectedToDevice: boolean, deviceUnlocked: boolean}}
 */
mooltipass.device.getStatus = function() {
    if (page.settings.useMoolticute) {
        return {
            'connectedToApp': moolticute.connectedToDaemon,
            'connectedToDevice': moolticute.status.connected,
            'deviceUnlocked': moolticute.status.unlocked
        }
    }

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
    console.log('mooltipass.generatePassword()', 'length =', length);

    // unset error message
    page.tabs[tab.id].errorMessage = null;

    // Only request new random string from device once a minute
    // The requested random string is used to salt Math.random() again
    var currentDate = new Date();
    var currentDayMinute = currentDate.getUTCHours() * 60 + currentDate.getUTCMinutes();
    if(!mooltipass.device._latestRandomStringRequest || mooltipass.device._latestRandomStringRequest != currentDayMinute) {
        mooltipass.device._asynchronous.randomCallback = callback;
        mooltipass.device._asynchronous.randomParameters = {'length': length};
        mooltipass.device._latestRandomStringRequest = currentDayMinute;

        console.log('mooltipass.generatePassword()', 'request random string from app');
        if (page.settings.useMoolticute) {
            moolticute.getRandomNumbers(function(data) {
                Math.seedrandom(data);
                if(mooltipass.device._asynchronous.randomCallback) {
                    mooltipass.device._asynchronous.randomCallback({
                        'seeds': mooltipass.device.generateRandomNumbers(mooltipass.device._asynchronous.randomParameters.length),
                        'settings': page.settings,
                    });
                }
            });
        } else {
            var request = { getRandom : [] };
            chrome.runtime.sendMessage(mooltipass.device._app.id, request);
        }
        return;
    }

    console.log('mooltipass.generatePassword()', 'use current seed for another password');
    callback({'seeds': mooltipass.device.generateRandomNumbers(length), 'settings': page.settings});
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
    //TODO: Trigger unlock if device is connected but locked
    // Check that the Mooltipass is unlocked
    if(!event.isMooltipassUnlocked()) {
        return;
    }

    if (mooltipass.backend.isBlacklisted(url)) {
        console.log('notify: ignoring blacklisted url',url);
        if (callback) {
            callback('failure');
        }
        return;
    }

    // unset error message
    page.tabs[tab.id].errorMessage = null;

    request = {update: {context: url, login: username, password: password}};

    // Cancel possible pending request
    mooltipass.device.onTabClosed(tab.id, null);
    mooltipass.device._asynchronous.updateCallback = callback;

    if (page.settings.useMoolticute) {
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
    console.log("Navigated away before retrieving credentials for tab: " + tabId);
    mooltipass.device.lastRetrieveReqTabId = null;

    mooltipass.device.onTabClosed(tabId, navigationInfo );
    mooltipass.device.retrieveCredentialsQueue.splice(0,1);
 }

/*
 * Function called when a tab is closed
 */
mooltipass.device.onTabClosed = function(tabId, removeInfo)
{
    console.log("Tab closed: " + tabId + " remove info: ", removeInfo);
    
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
            if (page.settings.useMoolticute) {
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
mooltipass.device.onTabUpdated = function(tabId, removeInfo)
{
    console.log('On Tab Updated', tabId,  mooltipass.device.lastRetrieveReqTabId, removeInfo );
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
    page.debug("mp.retrieveCredentials(callback, {1}, {2}, {3}, {4})", tab.id, url, submiturl, forceCallback);

    // unset error message
    page.tabs[tab.id].errorMessage = null;

    //TODO: Trigger unlock if device is connected but locked
    // Check that the Mooltipass is unlocked
    if(!event.isMooltipassUnlocked()) {
        // Don't return if the device is locked, queue the request
        /*if(forceCallback) {
            callback([]);
        }
        return;*/
    }

    // parse url and check if it is valid
    var parsed_url = mooltipass.backend.extractDomainAndSubdomain(submiturl);
    if(!parsed_url.valid) {
        if(forceCallback) {
            callback([]);
        }
        return;
    }

    if(parsed_url.domain && mooltipass.backend.isBlacklisted(parsed_url.domain)) {
        return;
    }

    if(parsed_url.subdomain && mooltipass.backend.isBlacklisted(parsed_url.subdomain)) {
        return;
    }

    // Store the tab id, prevent a possible very close tabupdatevent action
    mooltipass.device.lastRetrieveReqTabId = tab.id;
    mooltipass.device.tabUpdatedEventPrevented = true;

    // Check if we don't already have a request from this tab
    for (var i = 0; i < mooltipass.device.retrieveCredentialsQueue.length; i++)
    {
        if (mooltipass.device.retrieveCredentialsQueue[i].tabid == tab.id && mooltipass.device.retrieveCredentialsQueue[i].tabupdated == false)
        {
            console.log("Not storing this new credential request as one is already in the buffer!");
            return;
        }
    }
    
    // If our retrieveCredentialsQueue is empty and the device is unlocked, send the request to the app. Otherwise, queue it
    mooltipass.device.retrieveCredentialsQueue.push({'tabid': tab.id, 'callback': callback, 'domain': parsed_url.domain, 'subdomain': parsed_url.subdomain, 'tabupdated': false, 'reqid': mooltipass.device.retrieveCredentialsCounter});
    mooltipass.device.retrieveCredentialsCounter++;
    mooltipass.device._asynchronous.inputCallback = callback;
    if (page.settings.useMoolticute) {
        var srv = '';
        if (parsed_url.domain && parsed_url.subdomain) {
            srv = parsed_url.subdomain + '.' + parsed_url.domain;
        }
        else if (parsed_url.domain) {
            srv = parsed_url.domain;
        }

        moolticute.askPassword(srv, '', function(data) {
            if (data.failed) {
                callback([]);
            }
            else {
                callback([{
                    Login: data.login,
                    Name: '<name>',
                    Uuid: '<Uuid>',
                    Password: data.password,
                    StringFields: []
                }]);
            }
        });
    } else {
        if(mooltipass.device.retrieveCredentialsQueue.length == 1 && mooltipass.device._status.unlocked == true)
        {
            chrome.runtime.sendMessage(mooltipass.device._app.id, {'getInputs' : {'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain}});        
            console.log('sending to ' + mooltipass.device._app.id);
        }
        else
        {
            if(!mooltipass.device._status.unlocked)
            {
                console.log("Mooltipass locked, waiting for unlock");
            }
            else
            {
                console.log("Requests still in the queue, waiting for reply from the app");
            }        
        }    
    }
};

/****************************************************************************************************************/

/* Initialize device specific settings */

/**
 * Initially start searching for the Mooltipass app
 * This also triggers the status request to the device
 */
if (page.settings.useMoolticute) mooltipass.device._app = { enabled: true  };
else chrome.management.getAll(mooltipass.device.onSearchForApp);

/**
 * Process messages from the Mooltipass app
 */
if (!page.settings.useMoolticute) 
chrome.runtime.onMessageExternal.addListener(function(message, sender, sendResponse) {
    // Returned on a PING, contains the status of the device
    if (message.deviceStatus !== null) 
    {
        mooltipass.device._status = 
        {
            'connected': message.deviceStatus.connected,
            'unlocked': message.deviceStatus.unlocked,
            'version': message.deviceStatus.version,
            'state' : message.deviceStatus.state
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
                    chrome.runtime.sendMessage(mooltipass.device._app.id, {'getInputs' : {'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain}});        
                    console.log('sending to ' + mooltipass.device._app.id);
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
            });
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
            ]);
        }
        catch(err)
        {
        }
        // Treat other pending requests
        mooltipass.device.retrieveCredentialsQueue.shift();
        if(mooltipass.device.retrieveCredentialsQueue.length > 0)
        {
            chrome.runtime.sendMessage(mooltipass.device._app.id, {'getInputs' : {'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain}});       
            console.log('sending to ' + mooltipass.device._app.id);
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
            chrome.runtime.sendMessage(mooltipass.device._app.id, {'getInputs' : {'reqid': mooltipass.device.retrieveCredentialsQueue[0].reqid, 'domain': mooltipass.device.retrieveCredentialsQueue[0].domain, 'subdomain': mooltipass.device.retrieveCredentialsQueue[0].subdomain}});   
            console.log('sending to ' + mooltipass.device._app.id);
        }
    }
    // Returned on a completed update of credentials on the device
    else if (message.updateComplete !== null) {
        if (mooltipass.device._asynchronous.updateCallback) {
            try {
                mooltipass.device._asynchronous.updateCallback('success');
            } catch (e) {
                console.log("Error: " + e);
            }
            mooltipass.device._asynchronous.updateCallback = null;
        }
    }
});


moolticute.on('statusChange', function(type, data) {
    console.log('moolticute statusChange event received');
    mooltipass.device._status = {
        'connected': moolticute.status.connected,
        'unlocked': moolticute.status.unlocked,
        'version': moolticute.status.version,
        'state' : moolticute.status.state
    };
    mooltipass.connectedToApp = moolticute.connectedToDaemon;
});