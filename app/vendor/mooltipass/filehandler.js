var mooltipass = mooltipass || {};
mooltipass.filehandler = mooltipass.memmgmt || {};

mooltipass.filehandler.errorHandler = function(e) 
{
	var msg = '';

	switch (e.code) 
	{
		case FileError.QUOTA_EXCEEDED_ERR:
			msg = 'QUOTA_EXCEEDED_ERR'+e;
			break;
		case FileError.NOT_FOUND_ERR:
			msg = 'NOT_FOUND_ERR'+e;
			break;
		case FileError.SECURITY_ERR:
			msg = 'SECURITY_ERR z.B. Speicherplatz wurde abgelehnt.'+e;
			break;
		case FileError.INVALID_MODIFICATION_ERR:
			msg = 'INVALID_MODIFICATION_ERR'+e;
			break;
		case FileError.INVALID_STATE_ERR:
			msg = 'INVALID_STATE_ERR'+e;
			break;
		default:
			msg = 'Unknown Error'+e;
			break;
	}

	console.log('Writer Error: ' + msg);
}

// Ask the user to select a file and save the provided contents in it
mooltipass.filehandler.selectAndSaveFileContents = function(name, contents, writeEndCallback) 
{
	chrome.fileSystem.chooseEntry({type: 'saveFile', suggestedName: name, accepts: new Array({'extensions': new Array("bin")}), acceptsAllTypes: false},	function(writableFileEntry) 
																																							{
																																								if(chrome.runtime.lastError)
																																								{
																																									// Something went wrong during file selection
																																									console.log("File select error: "+ chrome.runtime.lastError.message);
																																								}
																																								else
																																								{
																																									// File chosen, create writer
																																									writableFileEntry.createWriter(	function(writer) 
																																																	{
																																																		var truncated = false;
																																																		// Setup error and writeend call backs, start write
																																																		writer.onerror = mooltipass.filehandler.errorHandler;
																																																		writer.onwriteend =	function(e)
																																																							{
																																																								if(!truncated)
																																																								{
																																																									truncated = true;
																																																									this.truncate(this.position);
																																																									writeEndCallback();
																																																								}
																																																							};
																																																		writer.write(contents);
																																																	}, mooltipass.filehandler.errorHandler);
																																								}
																																							});
}



