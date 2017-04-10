var mooltipass = mooltipass || {};
mooltipass.filehandler = mooltipass.memmgmt || {};

// Temp vars
mooltipass.filehandler.tempSyncFS = null;				// Temp SyncFS
mooltipass.filehandler.tempFilename = null;				// Temp Filename
mooltipass.filehandler.tempContents = "";				// Temp Contents
mooltipass.filehandler.tempReadendCallback = null;		// Temp Callback
mooltipass.filehandler.tempWriteendCallback = null;		// Temp Callback
mooltipass.filehandler.readFileName = "";				// Read file name

mooltipass.filehandler.errorHandler = function(e) 
{
	console.log("File handler error name: " + e.name + " / message: " + e.message);
}

// Ask the user to select a file to import its contents
mooltipass.filehandler.selectAndReadContents = function(name, readEndCallBack)
{
	var options_objects = {type: 'openFile'};
	var dot_separator_index = name.indexOf(".");
	
	// Check if a separator was specified
	if(dot_separator_index != -1)
	{
		options_objects['accepts'] = new Array({'extensions': new Array(name.substr(name.indexOf(".") + 1))});
		options_objects['acceptsAllTypes'] = false;
	}
	else
	{
		options_objects['acceptsAllTypes'] = true;
	}
	
	// Check if a name was specified
	if(name != "")
	{
		options_objects['suggestedName'] = name;
	}
	
	chrome.fileSystem.chooseEntry(options_objects,	function(readOnlyEntry) 
													{
														if(chrome.runtime.lastError)
														{
															// Something went wrong during file selection
															console.log("File select error: "+ chrome.runtime.lastError.message);
															readEndCallBack(null);
														}
														else
														{
															// File chosen, create reader
															mooltipass.filehandler.readFileName = readOnlyEntry.name;
															readOnlyEntry.file(	function(file) 
																				{
																					var reader = new FileReader();
																					reader.onerror = mooltipass.filehandler.errorHandler;
																					reader.onloadend = readEndCallBack;
																					reader.readAsText(file);
																				});
														}
													});	
}

// Ask the user to select a file to import its contents
mooltipass.filehandler.selectAndReadRawContents = function(name, readEndCallBack)
{
	var options_objects = {type: 'openFile'};
	var dot_separator_index = name.indexOf(".");
	
	// Check if a separator was specified
	if(dot_separator_index != -1)
	{
		options_objects['accepts'] = new Array({'extensions': new Array(name.substr(name.indexOf(".") + 1))});
		options_objects['acceptsAllTypes'] = false;
	}
	else
	{
		options_objects['acceptsAllTypes'] = true;
	}
	
	// Check if a name was specified
	if(name != "")
	{
		options_objects['suggestedName'] = name;
	}
	
	chrome.fileSystem.chooseEntry(options_objects,	function(readOnlyEntry) 
													{
														if(chrome.runtime.lastError)
														{
															// Something went wrong during file selection
															console.log("File select error: "+ chrome.runtime.lastError.message);
															readEndCallBack(null);
														}
														else
														{
															// File chosen, create reader
															mooltipass.filehandler.readFileName = readOnlyEntry.name;
															readOnlyEntry.file(	function(file) 
																				{
																					var reader = new FileReader();
																					reader.onerror = mooltipass.filehandler.errorHandler;
																					reader.onloadend = readEndCallBack;
																					reader.readAsArrayBuffer(file);
																				});
														}
													});	
}

// Ask the user to select a file and save the provided contents in it
mooltipass.filehandler.selectAndSaveFileContents = function(name, contents, writeEndCallback) 
{
	var options_objects = {type: 'saveFile'};
	var dot_separator_index = name.indexOf(".");
	
	// Check if a separator was specified
	if(dot_separator_index != -1)
	{
		options_objects['accepts'] = new Array({'extensions': new Array(name.substr(name.indexOf(".") + 1))});
		options_objects['acceptsAllTypes'] = false;
	}
	else
	{
		options_objects['acceptsAllTypes'] = true;
	}
	
	// Check if a name was specified
	if(name != "")
	{
		options_objects['suggestedName'] = name;
	}
	
	chrome.fileSystem.chooseEntry(options_objects,	function(writableFileEntry) 
													{
														if(chrome.runtime.lastError)
														{
															// Something went wrong during file selection
															console.log("File select error: "+ chrome.runtime.lastError.message);
															writeEndCallback(false);
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
																															writeEndCallback(true);
																														}
																													};
																								writer.write(contents);
																							}, mooltipass.filehandler.errorHandler, contents, writeEndCallback);
														}
													});
}

// Try and initialize the syncable file storage
mooltipass.filehandler.getSyncableFileSystemStatus = function(callbackFunction)
{
	chrome.syncFileSystem.getServiceStatus(callbackFunction);
}

// Set a callback function to listen to syncFC status changes
mooltipass.filehandler.setSyncFSStateChangeCallback = function(callbackFunction)
{
	chrome.syncFileSystem.onServiceStatusChanged.addListener(callbackFunction);
}

// Request SyncFS
mooltipass.filehandler.requestSyncFS = function(callbackFunction)
{
	chrome.syncFileSystem.requestFileSystem(callbackFunction);
}

// Callback for valid syncFS getFile, for read purposes
mooltipass.filehandler.getFileCreateTrueFalseCallbackRead = function(fileEntry)
{
	if(chrome.runtime.lastError)
	{
		// Something went wrong during file selection
		console.log("SyncFS file select error: "+ chrome.runtime.lastError.message);
	}
	else
	{
		// File chosen, create reader
		fileEntry.file(		function(file) 
							{
								var reader = new FileReader();
								reader.onerror = mooltipass.filehandler.errorHandler;
								reader.onloadend = mooltipass.filehandler.tempReadendCallback;
								reader.readAsText(file);
							});
	}
}

// Callback for valid syncFS getFile, for write purposes
mooltipass.filehandler.getFileCreateTrueFalseCallbackWrite = function(fileEntry)
{
	if(chrome.runtime.lastError)
	{
		// Something went wrong during file selection
		console.log("SyncFS file select error: "+ chrome.runtime.lastError.message);
	}
	else
	{
		// File chosen, create writer
		fileEntry.createWriter(	function(writer) 
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
																mooltipass.filehandler.tempWriteendCallback();
															}
														};
									writer.write(mooltipass.filehandler.tempContents);
								}, mooltipass.filehandler.errorHandler);
	}
}

// Callback for get file with create false
mooltipass.filehandler.getFileCreateFalseErrorCallback = function(e)
{
	if(e.name == "NotFoundError")
	{
		mooltipass.filehandler.tempSyncFS.root.getFile(mooltipass.filehandler.tempFilename, {create:true}, mooltipass.filehandler.getFileCreateTrueFalseCallbackRead, mooltipass.filehandler.errorHandler);		
	}
	else
	{
		console.log("Unsupported error for getFile with create false: " + e.name + " / message: " + e.message);
	}
}

// Callback for get file with create false
mooltipass.filehandler.writeFileCreateFalseErrorCallback = function(e)
{
	if(e.name == "NotFoundError")
	{
		mooltipass.filehandler.tempSyncFS.root.getFile(mooltipass.filehandler.tempFilename, {create:true}, mooltipass.filehandler.getFileCreateTrueFalseCallbackWrite, mooltipass.filehandler.errorHandler);
	}
	else
	{
		console.log("Unsupported error for writeFile with create false: " + e.name + " / message: " + e.message);
	}
}

// Request file in SyncFS
mooltipass.filehandler.requestOrCreateFileFromSyncFS = function(filesystem, filename, fileReadCallback)
{
	mooltipass.filehandler.tempFilename = filename;
	mooltipass.filehandler.tempSyncFS = filesystem;
	mooltipass.filehandler.tempReadendCallback = fileReadCallback;
	mooltipass.filehandler.tempSyncFS.root.getFile(mooltipass.filehandler.tempFilename, {create:false}, mooltipass.filehandler.getFileCreateTrueFalseCallbackRead, mooltipass.filehandler.getFileCreateFalseErrorCallback);
}

// Request file in SyncFS
mooltipass.filehandler.requestFileFromSyncFS = function(filesystem, filename, fileReadCallback)
{
	mooltipass.filehandler.tempFilename = filename;
	mooltipass.filehandler.tempSyncFS = filesystem;
	mooltipass.filehandler.tempReadendCallback = fileReadCallback;
	mooltipass.filehandler.tempSyncFS.root.getFile(mooltipass.filehandler.tempFilename, {create:false}, mooltipass.filehandler.getFileCreateTrueFalseCallbackRead, mooltipass.filehandler.errorHandler);
}

// Write a file in SyncFS
mooltipass.filehandler.writeFileToSyncFS = function(filesystem, filename, contents, fileWrittenCallback)
{
	mooltipass.filehandler.tempContents = contents;
	mooltipass.filehandler.tempFilename = filename;
	mooltipass.filehandler.tempSyncFS = filesystem;
	mooltipass.filehandler.tempWriteendCallback = fileWrittenCallback;
	mooltipass.filehandler.tempSyncFS.root.getFile(mooltipass.filehandler.tempFilename, {create:false}, mooltipass.filehandler.getFileCreateTrueFalseCallbackWrite, mooltipass.filehandler.errorHandler);
}

// Write a file in SyncFS
mooltipass.filehandler.writeCreateFileToSyncFS = function(filesystem, filename, contents, fileWrittenCallback)
{
	mooltipass.filehandler.tempContents = contents;
	mooltipass.filehandler.tempFilename = filename;
	mooltipass.filehandler.tempSyncFS = filesystem;
	mooltipass.filehandler.tempWriteendCallback = fileWrittenCallback;
	mooltipass.filehandler.tempSyncFS.root.getFile(mooltipass.filehandler.tempFilename, {create:false}, mooltipass.filehandler.getFileCreateTrueFalseCallbackWrite, mooltipass.filehandler.writeFileCreateFalseErrorCallback);
}














