var httpAuth = httpAuth || {};

httpAuth.callback = null
httpAuth.contentReady = false

httpAuth.onSubmit = function(credentials) {
  httpAuth.callback({
      authCredentials: {
          username: credentials.login,
          password: credentials.password
      }
  });
};

httpAuth.onCancel = function() {
  httpAuth.callback({ cancel: true });
};

httpAuth.handleRequest = function(details, callback) {
    // Cancel requests which are initiated not from tabs.
    if (!page.tabs[details.tabId]) {
      callback({ cancel: true })
      // Firefox expects this object on return.
      return { cancel: true }
    }
    
    // For the first HTTP Auth request we are opening http-auth.html with auth popup
    // and then redirecting user to the requested url again.
    if (!httpAuth.contentReady) {
      chrome.tabs.update(details.tabId, { url: chrome.extension.getURL('http-auth.html') })
      httpAuth.contentReady = true
      
      // Waiting when content scripts are loaded.
      setTimeout(function() {
        chrome.tabs.update(details.tabId, { url: details.url })
        
        // Resetting contentReady for future http auth requests.
        setTimeout(function() {
          httpAuth.contentReady = false
        }, 1000)
        
        messaging({
          action: "show_http_auth",
          args: [{
            url: details.isProxy
                 ? 'proxy://' + details.challenger.host + ':' + details.challenger.port
                 : details.url
          }]
        }, details.tabId);
      }, 100)
      
      callback({ cancel: true })
      // Firefox expects this object on return.
      return { cancel: true }
    }
    
    httpAuth.callback = callback
    if (isFirefox) {
      return new Promise(function(resolve) {
        httpAuth.callback = function(credentials) {
          resolve(credentials);
        }
      })
    }
}
