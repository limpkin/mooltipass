chrome.app.runtime.onLaunched.addListener(function() 
{
    chrome.app.window.create('mooltipass.html', 
        {
            'bounds': 
            {
                'width': 400,
                'height': 500
            }
    });
    chrome.runtime.onMessage.addListener(function(request, sender, sendResponse) 
    {
        console.log(sender.tab ?  'from a content script:' + sender.tab.url : 'from the extension');
        if (request.type == 'inputs') 
        {
            console.log('inputs:');
            for (var i=0; i<request.inputs.length; i++) 
            {
                console.log('    "'+request.inputs[i].id);
            }
            sendResponse({type: 'response', msg: 'Hi From Client'});
        }
    });
});

