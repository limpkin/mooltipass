var httpAuth = httpAuth || {};

httpAuth.callback = null;

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
    
    httpAuth.callback = callback
    messaging({
      action: "show_http_auth",
      args: [{
        isProxy: details.isProxy,
        proxyURL: 'proxy://' + details.challenger.host + ':' + details.challenger.port
      }]
    }, details.tabId);
    
    if (isFirefox) {
      return new Promise(function(resolve) {
        httpAuth.callback = function(credentials) {
          resolve(credentials);
        }
      })
    }
}
