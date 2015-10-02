var mooltipass = mooltipass || {};
mooltipass.memmgmt_hid = mooltipass.memmgmt_hid || {};

mooltipass.memmgmt_hid.responseCallback = null;		// Response callback
mooltipass.memmgmt_hid.timeoutCallback = null;		// Timeout callback
mooltipass.memmgmt_hid.nbSendRetries = 3;			// Nb send retries
mooltipass.memmgmt_hid.receiveTempHash = 0;			// Temp hash for receive

mooltipass.memmgmt_hid.request = 
{
	'packet': [], /* created with mooltipass.device.createPacket() */
	'milliseconds': 1000,
	'retries': 3
};

mooltipass.memmgmt_hid._sendMsg = function() 
{
	// Set number of retries
	mooltipass.memmgmt_hid.request['retries'] = mooltipass.memmgmt_hid.nbSendRetries;
	
	if(mooltipass.memmgmt_hid.request.milliseconds && mooltipass.memmgmt_hid.request.retries) 
	{
		var hash = Math.random() + Math.random();
		mooltipass.memmgmt_hid.request.hash = hash;
		setTimeout(	function() 
					{
						mooltipass.memmgmt_hid._retrySendMsg(hash);
					}, mooltipass.memmgmt_hid.request.milliseconds);
	}

	chrome.hid.send(mooltipass.device.connectionId, 0, mooltipass.memmgmt_hid.request.packet, mooltipass.memmgmt_hid._onSendMsg);
};

mooltipass.memmgmt_hid._onSendMsg = function() 
{
	if (chrome.runtime.lastError) 
	{
		if (mooltipass.device.isConnected) 
		{
			// Trigger retry
			mooltipass.memmgmt_hid._retrySendMsg(mooltipass.memmgmt_hid.request.hash);
		}

		// TODO: Leave Memory Management mode and restart queue OR what do you need
		return;
	}

	chrome.hid.receive(mooltipass.device.connectionId, mooltipass.memmgmt_hid._onDataReceived);
};

mooltipass.memmgmt_hid.receiveMsg = function()
{
	if(mooltipass.memmgmt_hid.request.milliseconds) 
	{
		var hash = Math.random() + Math.random();
		mooltipass.memmgmt_hid.receiveTempHash = hash;
		setTimeout(	function() 
					{
						mooltipass.memmgmt_hid.receiveMsgTimeout(hash);
					}, mooltipass.memmgmt_hid.request.milliseconds);
	}
	chrome.hid.receive(mooltipass.device.connectionId, mooltipass.memmgmt_hid._onDataReceived);
}

mooltipass.memmgmt_hid.receiveMsgTimeout = function(hash)
{
	// Successfully processed command, no retries needed
	if(hash != mooltipass.memmgmt_hid.receiveTempHash) 
	{
		return;
	}
	
	console.log('receive timeout: same hash');
	
	// If callbackFunction is set, call it in case of retries is reached
	if(mooltipass.memmgmt_hid.timeoutCallback) 
	{
		mooltipass.memmgmt_hid.timeoutCallback();
	}
}

mooltipass.memmgmt_hid._retrySendMsg = function(hash) 
{
	//console.log('mooltipass.memmgmt_hid._retrySendMsg()');

	// No requests set
	if(!mooltipass.memmgmt_hid.request) 
	{
		return;
	}

	//console.log('	 queue not empty');

	// Successfully processed command, no retries needed
	if(hash != mooltipass.memmgmt_hid.request.hash) 
	{
		return;
	}
	// Change hash in case of multiple calls through chrome.runtime.lastError
	mooltipass.memmgmt_hid.request.hash = Math.random() + Math.random();

	console.log('	 same hash');

	// No timeout object found (shouldn't happen)
	if(!mooltipass.memmgmt_hid.request.milliseconds || !mooltipass.memmgmt_hid.request.retries) 
	{
		console.error('milliseconds or retires not set in request:', mooltipass.memmgmt_hid.request);
		return;
	}

	console.log('	', mooltipass.memmgmt_hid.request.retries, 'retries');

	// Retry request until retries is 0
	if(mooltipass.memmgmt_hid.request.retries > 0) 
	{
		mooltipass.memmgmt_hid.request.retries -= 1;
		mooltipass.memmgmt_hid._sendMsg();
		return;
	}

	console.log('	 call callback function');

	// If callbackFunction is set, call it in case of retries is reached
	if(mooltipass.memmgmt_hid.timeoutCallback) 
	{
		mooltipass.memmgmt_hid.timeoutCallback();
	}
};

mooltipass.memmgmt_hid._onDataReceived = function(reportId, data) 
{
	if (typeof reportId === 'undefined' || typeof data === 'undefined') 
	{
		console.log('undefined response');
		if (chrome.runtime.lastError) 
		{
			var error = chrome.runtime.lastError;
			if (error.message != 'Transfer failed.') 
			{
				console.log('Error in onDataReceived:', error.message);
			}
		}

		// Trigger retry
		mooltipass.memmgmt_hid._retrySendMsg(mooltipass.memmgmt_hid.request.hash);
		return;
	}

	// Change hash to avoid retry after timeout
	mooltipass.memmgmt_hid.request.hash = Math.random() + Math.random();
	mooltipass.memmgmt_hid.receiveTempHash = Math.random() + Math.random();

	// Extract data
	var bytes = new Uint8Array(data);
	//console.log(bytes);

	// Invoke callback function
	if(mooltipass.memmgmt_hid.responseCallback) 
	{
		mooltipass.memmgmt_hid.responseCallback(bytes);
	}
};