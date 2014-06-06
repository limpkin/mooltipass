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
});
