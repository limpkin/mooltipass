var httpAuth = httpAuth || {}

httpAuth.credentials = null
httpAuth.url = null
httpAuth.tabId = null

httpAuth.onSubmit = function(credentials) {
  httpAuth.credentials = credentials
  chrome.tabs.update(httpAuth.tabId, { url: httpAuth.url })
}

httpAuth.handleRequest = function(details, callback) {
    // Cancel requests which are initiated not from tabs.
    if (!page.tabs[details.tabId]) {
      if (!isFirefox) {
        callback({ cancel: true })
        return
      } else {
        // Firefox expects this object on return.
        return { cancel: true }
      }
    }
    
    // Enter credentials that we saved last time.
    if (httpAuth.credentials) {
      var credentials = httpAuth.credentials
      httpAuth.credentials = null
      
      if (!isFirefox) {
        callback({
          authCredentials: {
            username: credentials.login,
            password: credentials.password
          }
        })
        return
      } else {
        return {
          authCredentials: {
            username: credentials.login,
            password: credentials.password
          }
        }
      }
    }
    
    // For the first HTTP Auth request we are opening http-auth.html with auth popup
    // and redirecting user to the requested url after form is submitted.
    chrome.tabs.update(details.tabId, { url: chrome.extension.getURL('http-auth.html') })
    
    httpAuth.url = details.url
    httpAuth.tabId = details.tabId
    
    // Waiting when content scripts are loaded.
    setTimeout(function() {
      messaging({
        action: "show_http_auth",
        args: [{
          isProxy: details.isProxy,
          proxy: isFirefox ? details.ip : details.challenger.host + ':' + details.challenger.port,
          url:  details.url
        }]
      }, details.tabId);
    }, 500)
    
    if (!isFirefox) {
      callback({ cancel: true })
      return
    } else {
      // Firefox expects this object on return.
      return { cancel: true }
    }
}
