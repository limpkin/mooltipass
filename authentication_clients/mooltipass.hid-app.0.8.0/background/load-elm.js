/* This file loads the Elm application and sets up communication with the
   gui through chrome.runtime. */
var emptyFromDeviceMessage = { setHidConnected : null
                             , receiveCommand  : null
                             , appendToLog     : null
                             };
var emptyFromExtensionMessage  = { ping      : null
                                 , getInputs : null
                                 , update    : null
                                 };
var guiOpen = false;
var extensionId = null;

var elm = Elm.worker(
    Elm.Background,
    { fromGUI       : emptyFromGuiMessage
    , fromDevice    : emptyFromDeviceMessage
    , fromExtension : emptyFromExtensionMessage
    }
);

chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) {
    if (request.toBackground !== undefined) {
        elm.ports.fromGUI.send(request.toBackground);
    }
});

elm.ports.toGUI.subscribe(function(message) {
    chrome.runtime.sendMessage({toGUI: message});
});

elm.ports.toDevice.subscribe(function(message) {
    if (message.connect != null) {
        device.connect();
    } else if (message.sendCommand != null) {
        sendMsg(message.sendCommand);
    }
});

elm.ports.toExtension.subscribe(function(message) {
    if (extensionId != null) {
        if (message.connectState != null) {
            chrome.runtime.sendMessage(extensionId,
                { type: message.connectState.state
                , version: message.connectState.version
                }
            );
        } else if (message.credentials != null) {
            chrome.runtime.sendMessage(extensionId,
                { type : "credentials"
                , inputs : { login : {value : message.credentials.login}
                           , password : {value: message.credentials.password}
                           }
                }
            );
       } else if (message.noCredentials != null) {
            chrome.runtime.sendMessage(extensionId,
                { type : "noCredentials"}
            );
       } else if (message.updateComplete != null) {
            chrome.runtime.sendMessage(extensionId,
                { type : "updateComplete"}
            );
       }
    }

});

deviceSendToElm = function (message) {
    var messageWithNulls = {};
    //replace undefined with null so it becomes 'Nothing' in Elm
    for (var prop in emptyFromDeviceMessage) {
        if (message.hasOwnProperty(prop)) {
            messageWithNulls[prop] = message[prop];
        } else {
            messageWithNulls[prop] = emptyFromDeviceMessage[prop];
        }
    }
    elm.ports.fromDevice.send(messageWithNulls);
};

extensionSendToElm = function (message) {
    var messageWithNulls = {};
    //replace undefined with null so it becomes 'Nothing' in Elm
    for (var prop in emptyFromExtensionMessage) {
        if (message.hasOwnProperty(prop)) {
            messageWithNulls[prop] = message[prop];
        } else {
            messageWithNulls[prop] = emptyFromExtensionMessage[prop];
        }
    }
    elm.ports.fromExtension.send(messageWithNulls);
}

function launch()
{
    chrome.app.window.create('gui/index.html',
            //id takes care of making sure only one is running
            { id:"mooltipass"
            , minWidth: 550
            , minHeight: 600
            }
    );
}

chrome.runtime.onInstalled.addListener(launch);
chrome.runtime.onStartup.addListener(launch);
chrome.app.runtime.onLaunched.addListener(launch);

toContext = function (url) {
    // URL regex to extract base domain for context
    var reContext = /^\https?\:\/\/([\w\-\+]+\.)*([\w\-\_]+\.[\w\-\_]+)/;
    return reContext.exec(url)[2];
}
chrome.runtime.onMessageExternal.addListener(function(request, sender, sendResponse)
{
    extensionId = sender.id
    switch (request.type) {
        case 'ping':
            extensionSendToElm({ping : []});
            break;
        case 'inputs':
            var context = toContext(request.url);
            extensionSendToElm({getInputs:{context:context}});
            break;
        case 'update':
            var context = toContext(request.url);
            extensionSendToElm({update:
                { context  : context
                , login    : request.inputs.login.value
                , password : request.inputs.password.value
                }
            });
            break;
    }
});
