var mooltipass = mooltipass || {};
mooltipass.datamemmgmt = mooltipass.datamemmgmt || {};

// Defines
var NODE_SIZE							= 132;				// Node size
var HID_PAYLOAD_SIZE					= 62;				// HID payload
var MEDIA_BUNDLE_CHUNK_SIZE				= 33;				// How many bytes we send per chunk
var DATAMGMT_PREFERENCES_VERSION		= 0;				// Preferences version
var DATA_SENDING_CHUNK_SIZE				= 32				// Packet size when storing/getting data
		
// State machine modes		
var DATAMGMT_IDLE						= 0;				// Idle mode
var DATAMGMT_PARAM_LOAD_NAME_LIST_REQ	= 1;				// Parameter loading for node name listing
var DATAMGMT_PARAM_LOAD_NAME_LIST		= 2;				// Parameter loading for node name listing
var DATAMGMT_LOAD_NAME_SCAN				= 3;				// Node name scanning
var DATAMGMT_PARAM_LOAD_FILEADD_REQ		= 4;				// Parameter loading for node name listing
var DATAMGMT_FILEADD_REQ				= 5;				// Adding a file to the mooltipass
var DATAMGMT_PARAM_LOAD_FILEGET_REQ		= 6;				// Parameter loading for node name listing
var DATAMGMT_FILEGET_REQ				= 7;				// Adding a file to the mooltipass

// Local preferences
/* Default preferences */
mooltipass.datamemmgmt.preferences = {"version": DATAMGMT_PREFERENCES_VERSION, "cpz_to_ctrhash": [], "stored_files": []};

// Mooltipass memory params
mooltipass.datamemmgmt.nbMb = null;									// Mooltipass memory size

// State machines & temp variables
mooltipass.datamemmgmt.currentMode = DATAMGMT_IDLE;					// Current mode
mooltipass.datamemmgmt.nodeNames = [];								// Data node names
mooltipass.datamemmgmt.dataStartingParent = null;					// Mooltipass current data starting parrent
mooltipass.datamemmgmt.packetToSendBuffer = [];						// Packets to send buffer
mooltipass.datamemmgmt.curDataServiceNodes = [];					// Data service nodes
mooltipass.datamemmgmt.curDataNodes = [];							// Data nodes
mooltipass.datamemmgmt.clonedCurDataServiceNodes = [];				// Data service nodes (cloned)
mooltipass.datamemmgmt.clonedCurDataNodes = []; 					// Data nodes (cloned)
mooltipass.datamemmgmt.currentNode = new Uint8Array(NODE_SIZE);		// Current node we're receiving
mooltipass.datamemmgmt.nodePacketId = 0;							// Packet number for the node we're receiving
mooltipass.datamemmgmt.curNodeAddressRequested = [];				// The address of the current node we're requesting
mooltipass.datamemmgmt.nextParentNodeTSAddress = [];				// Next parent node to scan address
mooltipass.datamemmgmt.byteCounter = 0;								// File read / write byte counter
mooltipass.datamemmgmt.fileRead = null;								// File read contents
mooltipass.datamemmgmt.fileReadName = "";							// File read name
mooltipass.datamemmgmt.fileSize = 0;								// File size
mooltipass.datamemmgmt.fileRealSize = 0;							// File real size
mooltipass.datamemmgmt.CPZCTRHashTable = [];						// All cards CPZ for current user
mooltipass.datamemmgmt.userCTRHash = "";							// Current user CTR hash
 
// Export file written callback
mooltipass.datamemmgmt.fileWrittenCallback = function()
{
	console.log("File written!");
}

mooltipass.datamemmgmt.mergeFileListWithPreferences = function()
{	
	var changes_made = false;
	
	// Loop through our stored file names and check if some were deleted
	for(var i = mooltipass.datamemmgmt.preferences.stored_files.length - 1; i >= 0; i--)
	{
		if(mooltipass.datamemmgmt.preferences.stored_files[i][0] == mooltipass.datamemmgmt.userCTRHash)
		{
			// try to find the file in the list we have
			var file_found = false;
			for(var j = 0; j < mooltipass.datamemmgmt.clonedCurDataServiceNodes.length; j++)
			{
				if(mooltipass.datamemmgmt.clonedCurDataServiceNodes[j].name == mooltipass.datamemmgmt.preferences.stored_files[i][1])
				{
					file_found = true;
					break;
				}
			}
			// didn't find the file, delete it from our preferences
			if(file_found == false)
			{
				console.log("File " + mooltipass.datamemmgmt.preferences.stored_files[i][1] + " was deleted");
				mooltipass.datamemmgmt.preferences.stored_files.splice(i, 1);
				changes_made = true;
			}
		}
	}
	
	// Loop through our found nodes and find new ones
	for(var i = 0; i < mooltipass.datamemmgmt.clonedCurDataServiceNodes.length; i++)
	{
		// try to find the file in the list we have
		var file_found = false;
		for(var j = 0; j < mooltipass.datamemmgmt.preferences.stored_files.length; j++)
		{
			if(mooltipass.datamemmgmt.clonedCurDataServiceNodes[i].name == mooltipass.datamemmgmt.preferences.stored_files[j][1] && mooltipass.datamemmgmt.userCTRHash == mooltipass.datamemmgmt.preferences.stored_files[j][0])
			{
				file_found = true;
			}
		}
		// didn't find the file, delete it from our preferences
		if(file_found == false)
		{
			changes_made = true;
			console.log("File " + mooltipass.datamemmgmt.clonedCurDataServiceNodes[i].name + " was added");
			mooltipass.datamemmgmt.preferences.stored_files.push([mooltipass.datamemmgmt.userCTRHash, mooltipass.datamemmgmt.clonedCurDataServiceNodes[i].name]);
		}
	}	
	
	if(changes_made == true)
	{
		mooltipass.prefstorage.setStoredPreferences({"datamemmgmtPrefsStored": true, "datamemmgmtPrefs": mooltipass.datamemmgmt.preferences});		
	}
}

mooltipass.datamemmgmt.mergeCPZCTRHashTableWithPreferences = function()
{
	var changes_made = false;
	
	for(var i = 0; i < mooltipass.datamemmgmt.CPZCTRHashTable.length; i++)
	{
		var cpz_found = false;
		for(var j = 0; j < mooltipass.datamemmgmt.preferences.cpz_to_ctrhash.length; j++)
		{
			if(mooltipass.datamemmgmt.CPZCTRHashTable[i][0] == mooltipass.datamemmgmt.preferences.cpz_to_ctrhash[j][0])
			{
				cpz_found = true;
				break;
			}
		}
		if(cpz_found == false)
		{
			//console.log("Adding CPZ/Hash to preferences");
			//console.log(mooltipass.datamemmgmt.CPZCTRHashTable[i][0]);
			//console.log(mooltipass.datamemmgmt.CPZCTRHashTable[i][1]);
			changes_made = true;
			mooltipass.datamemmgmt.preferences.cpz_to_ctrhash.push(mooltipass.datamemmgmt.CPZCTRHashTable[i]);
		}
	}
	
	if(changes_made == true)
	{
		mooltipass.prefstorage.setStoredPreferences({"datamemmgmtPrefsStored": true, "datamemmgmtPrefs": mooltipass.datamemmgmt.preferences});		
	}
}

// Load memory management preferences callback
mooltipass.datamemmgmt.preferencesCallback = function(items)
{	
	//console.log(items);
	if(chrome.runtime.lastError)
	{
		// Something went wrong during file selection
		console.log("preferencesCallback error: "+ chrome.runtime.lastError.message);
	}
	else
	{
		if(items.datamemmgmtPrefsStored == null)
		{
			// Empty file, save new preferences
			console.log("Data preferences storage: No preferences stored!");
			mooltipass.prefstorage.setStoredPreferences({"datamemmgmtPrefsStored": true, "datamemmgmtPrefs": mooltipass.datamemmgmt.preferences});
		}
		else
		{
			console.log("Data preferences storage: loaded preferences");
			
			// Check if the preferences we got are of the latest version
			if(items.datamemmgmtPrefs.version != DATAMGMT_PREFERENCES_VERSION)
			{
				console.log("Loaded data preferences are from an older version");
				// Check what fields are missing in what is stored
				for(var key in mooltipass.datamemmgmt.preferences)
				{
					if(key in items.datamemmgmtPrefs)
					{
						console.log("Same key: " + key);
						mooltipass.datamemmgmt.preferences[key] = items.datamemmgmtPrefs[key];
					}
					else
					{
						console.log("New key: " + key);
					}
				}
				// Save preferences
				mooltipass.datamemmgmt.preferences.version = DATAMGMT_PREFERENCES_VERSION;
				mooltipass.prefstorage.setStoredPreferences({"datamemmgmtPrefsStored": true, "datamemmgmtPrefs": mooltipass.datamemmgmt.preferences});
			}
			else
			{
				mooltipass.datamemmgmt.preferences = items.datamemmgmtPrefs;				
			}
			//console.log(mooltipass.datamemmgmt.preferences);
		}
	}
}

// Request fail handler
mooltipass.datamemmgmt.requestFailHander = function(message, nextMode)
{
	if(mooltipass.datamemmgmt.currentMode == DATAMGMT_LOAD_NAME_SCAN)
	{
		// During load name scan or mem mgmt exit
		console.log(message);
	}
	else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_NAME_LIST_REQ)
	{
		// During parameter loading or request to enter mem management mode
		console.log(message);
	}
	else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_FILEADD_REQ)
	{
		console.log(message);		
	}
	else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_FILEADD_REQ)
	{
		console.log(message);		
	}
	else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_FILEGET_REQ)
	{
		console.log(message);		
	}
	else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_FILEGET_REQ)
	{
		console.log(message);		
	}
	
	// Change mode if specified
	if(nextMode != null)
	{
		mooltipass.datamemmgmt.currentMode = nextMode;
	}
	
	// Give back the hand to the main app
	mooltipass.device.processQueue();
}
 
// Data received from USB callback
mooltipass.datamemmgmt.dataReceivedCallback = function(packet)
{
	// If it is a leave memory management mode packet, process the queue and exit
	if(packet[1] == mooltipass.device.commands['endMemoryManagementMode'])
	{		 
		// Did we succeed?
		if(packet[2] == 1)
		{
			if(mooltipass.datamemmgmt.currentMode == DATAMGMT_LOAD_NAME_SCAN)
			{
				if(mooltipass.datamemmgmt.nodeNames.length == 0)
				{
					console.log("No data files in memory!");
				}
				else
				{
					mooltipass.datamemmgmt.mergeFileListWithPreferences();
					//console.log(mooltipass.datamemmgmt.nodeNames);
				}				
			}
			mooltipass.datamemmgmt.currentMode = DATAMGMT_IDLE;
			console.log("Memory management mode exit");
			mooltipass.device.processQueue();
		}
		else
		{
			mooltipass.datamemmgmt.requestFailHander("Couldn't exit memory management mode", null);
		}
	}
	
	// Check for this packet
	if(packet[1] == 0xC4)
	{
		console("Please retry packet !!!")
	}
	
	if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_NAME_LIST_REQ || mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_FILEADD_REQ || mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_FILEGET_REQ)
	{
		if(packet[1] == mooltipass.device.commands['getMooltipassParameter'])
		{
			// We received the user interaction timeout, use it for our packets timeout
			console.log("Mooltipass interaction timeout is " + packet[2] + " seconds");
			mooltipass.memmgmt_hid.request.milliseconds = (packet[2]) * 2000;
			
			if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_NAME_LIST_REQ || mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_FILEADD_REQ || mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_FILEGET_REQ)
			{
				// Query the Mooltipass status		
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassStatus'], null);
				mooltipass.memmgmt_hid._sendMsg();
			}
			else
			{
			}			 
		}
		else if(packet[1] == mooltipass.device.commands['getMooltipassStatus'])
		{
			if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_NAME_LIST_REQ)
			{
				console.log("Mooltipass current status is: " + mooltipass.memmgmt.mooltipass_status[packet[2]]);
				if(mooltipass.memmgmt.mooltipass_status[packet[2]] == 'Unlocked')
				{
					// If the mooltipass is in the good state... start memory management mode
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['startMemoryManagementMode'], null);
					mooltipass.memmgmt_hid._sendMsg();	
				}
				else
				{
					mooltipass.datamemmgmt.requestFailHander("Incorrect Mooltipass State!!!", DATAMGMT_IDLE);
				}
			}
			else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_FILEADD_REQ || mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_FILEGET_REQ)
			{
				console.log("Mooltipass current status is: " + mooltipass.memmgmt.mooltipass_status[packet[2]]);
				if(mooltipass.memmgmt.mooltipass_status[packet[2]] == 'Unlocked')
				{
					// If the mooltipass is in the good state... set data context
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setDataContext'], mooltipass.util.strToArray(mooltipass.datamemmgmt.fileReadName.substring(0, 120)));
					mooltipass.memmgmt_hid._sendMsg();

					if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_FILEGET_REQ)
					{
						mooltipass.datamemmgmt.currentMode = DATAMGMT_FILEGET_REQ;
					}
					else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_FILEADD_REQ)
					{
						mooltipass.datamemmgmt.currentMode = DATAMGMT_FILEADD_REQ;
					}
				}
				else
				{
					mooltipass.datamemmgmt.requestFailHander("Incorrect Mooltipass State!!!", DATAMGMT_IDLE);
				}
			}
		}		
		else if(packet[1] == mooltipass.device.commands['startMemoryManagementMode'])
		{			 
			// Did we succeed?
			if(packet[2] == 1)
			{				 
				// Load memory params
				if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_NAME_LIST_REQ)
				{					
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getVersion'], null);
					mooltipass.datamemmgmt.currentMode = DATAMGMT_PARAM_LOAD_NAME_LIST;
					mooltipass.memmgmt_hid.nbSendRetries = 3;
					mooltipass.memmgmt_hid._sendMsg();
				}
				console.log("Memory management mode entered");
			}
			else
			{
				mooltipass.datamemmgmt.requestFailHander("Couldn't enter memory management mode", DATAMGMT_IDLE);
			}
		}	
	}
	else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_FILEADD_REQ)
	{
		if(packet[1] == mooltipass.device.commands['setDataContext'])
		{
			if(packet[2] == 0x01)
			{
				// Data context set, start sending data
				console.log("Data context " + mooltipass.datamemmgmt.fileReadName + " set");	
				var packet_to_send = new Uint8Array(1 + DATA_SENDING_CHUNK_SIZE);
				mooltipass.memmgmt_hid.request['milliseconds'] = 4000;

				if(mooltipass.datamemmgmt.byteCounter + DATA_SENDING_CHUNK_SIZE == mooltipass.datamemmgmt.fileRead.length)
				{
					// Signal last packet
					packet_to_send.set([33], 0);
					packet_to_send.set(mooltipass.datamemmgmt.fileRead.subarray(mooltipass.datamemmgmt.byteCounter, mooltipass.datamemmgmt.byteCounter + DATA_SENDING_CHUNK_SIZE), 1);
					mooltipass.datamemmgmt.byteCounter += DATA_SENDING_CHUNK_SIZE;
					
					// We finished sending data
					console.log("Sending last packet");
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['write32BytesInCurrentContext'], packet_to_send);
					mooltipass.memmgmt_hid._sendMsg();
				}
				else
				{
					// Prepare packet
					packet_to_send.set([0], 0);
					packet_to_send.set(mooltipass.datamemmgmt.fileRead.subarray(mooltipass.datamemmgmt.byteCounter, mooltipass.datamemmgmt.byteCounter + DATA_SENDING_CHUNK_SIZE), 1);
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['write32BytesInCurrentContext'], packet_to_send);
					mooltipass.datamemmgmt.byteCounter += DATA_SENDING_CHUNK_SIZE;
					mooltipass.memmgmt_hid._sendMsg();
				}
			}
			else if(packet[2] == 0x00)
			{
				console.log("Mooltipass doesn't know data context " + mooltipass.datamemmgmt.fileReadName);
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['addDataContext'], mooltipass.util.strToArray(mooltipass.datamemmgmt.fileReadName.substring(0, 120)));
				mooltipass.memmgmt_hid._sendMsg();	
			}
			else
			{
				mooltipass.datamemmgmt.requestFailHander("User not logged in", DATAMGMT_IDLE);
			}
		}
		else if(packet[1] == mooltipass.device.commands['addDataContext'])
		{
			if(packet[2] == 0x01)
			{
				// Data context added, set it now
				console.log("Data context " + mooltipass.datamemmgmt.fileReadName + " added");
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setDataContext'], mooltipass.util.strToArray(mooltipass.datamemmgmt.fileReadName.substring(0, 120)));
				mooltipass.memmgmt_hid._sendMsg();	
			}
			else
			{
				mooltipass.datamemmgmt.requestFailHander("User denied context add", DATAMGMT_IDLE);
			}
		}
		else if(packet[1] == mooltipass.device.commands['write32BytesInCurrentContext'])
		{
			mooltipass.memmgmt_hid.nbSendRetries = 3;
			
			if(packet[2] == 0x01)
			{
				// We managed to write data
				var packet_to_send = new Uint8Array(1 + DATA_SENDING_CHUNK_SIZE);
				
				if(mooltipass.datamemmgmt.byteCounter == mooltipass.datamemmgmt.fileRead.length)
				{
					console.log("Data stored!");
					mooltipass.datamemmgmt.currentMode = MGMT_IDLE;
				}
				else if(mooltipass.datamemmgmt.byteCounter + DATA_SENDING_CHUNK_SIZE == mooltipass.datamemmgmt.fileRead.length)
				{
					// Signal last packet
					packet_to_send.set([33], 0);
					packet_to_send.set(mooltipass.datamemmgmt.fileRead.subarray(mooltipass.datamemmgmt.byteCounter, mooltipass.datamemmgmt.byteCounter + DATA_SENDING_CHUNK_SIZE), 1);
					mooltipass.datamemmgmt.byteCounter += DATA_SENDING_CHUNK_SIZE;
					
					// We finished sending data
					console.log("Sending last packet");
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['write32BytesInCurrentContext'], packet_to_send);
					mooltipass.memmgmt_hid._sendMsg();
				}
				else
				{
					// Prepare packet
					packet_to_send.set([0], 0);
					packet_to_send.set(mooltipass.datamemmgmt.fileRead.subarray(mooltipass.datamemmgmt.byteCounter, mooltipass.datamemmgmt.byteCounter + DATA_SENDING_CHUNK_SIZE), 1);
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['write32BytesInCurrentContext'], packet_to_send);
					mooltipass.datamemmgmt.byteCounter += DATA_SENDING_CHUNK_SIZE;
					mooltipass.memmgmt_hid._sendMsg();
				}
			}
			else
			{
				mooltipass.datamemmgmt.requestFailHander("Couldn't add data to data context", DATAMGMT_IDLE);
			}			
		}			
	}
	else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_FILEGET_REQ)
	{
		if(packet[1] == mooltipass.device.commands['setDataContext'])
		{
			if(packet[2] == 0x01)
			{
				// Data context set, start sending data
				console.log("Data context " + mooltipass.datamemmgmt.fileReadName + " set");
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['read32BytesInCurrentContext'], null);
				mooltipass.memmgmt_hid.request['milliseconds'] = 4000;
				mooltipass.memmgmt_hid._sendMsg();
			}
			else if(packet[2] == 0x00)
			{
				mooltipass.datamemmgmt.requestFailHander("Mooltipass doesn't know data context " + mooltipass.datamemmgmt.fileReadName + " !", DATAMGMT_IDLE);
			}
			else
			{
				mooltipass.datamemmgmt.requestFailHander("User not logged in", DATAMGMT_IDLE);
			}
		}
		else if(packet[1] == mooltipass.device.commands['read32BytesInCurrentContext'])
		{
			mooltipass.memmgmt_hid.nbSendRetries = 3;
			
			if(packet[0] != 0x01)
			{
				// We managed to read data
				if(mooltipass.datamemmgmt.byteCounter == 0)
				{
					console.log("Receiving data...")
					// First block of data, set receiving buffer
					mooltipass.datamemmgmt.fileRealSize = (packet[2]*256 + packet[3]);
					mooltipass.datamemmgmt.fileSize = Math.ceil((packet[2]*256 + packet[3])/32)*32;
					mooltipass.datamemmgmt.fileRead = new Uint8Array(mooltipass.datamemmgmt.fileSize);
				}
				
				// Store data
				mooltipass.datamemmgmt.fileRead.set(packet.subarray(2, 2 + DATA_SENDING_CHUNK_SIZE), mooltipass.datamemmgmt.byteCounter);
				mooltipass.datamemmgmt.byteCounter += DATA_SENDING_CHUNK_SIZE;
				
				// Request the rest
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['read32BytesInCurrentContext'], null);
				mooltipass.memmgmt_hid._sendMsg();
			}
			else
			{
				if(mooltipass.datamemmgmt.byteCounter == 0)
				{
					mooltipass.datamemmgmt.requestFailHander("User denied data request !", DATAMGMT_IDLE);
				}	
				else				
				{
					console.log("Data received!");
					mooltipass.datamemmgmt.currentMode = DATAMGMT_IDLE;
					var data_to_write = mooltipass.datamemmgmt.fileRead.subarray(2, 2 + mooltipass.datamemmgmt.fileRealSize);
					mooltipass.filehandler.selectAndSaveFileContents(mooltipass.datamemmgmt.fileReadName, new Blob([data_to_write], {type: 'application/octet-stream'}), mooltipass.datamemmgmt.fileWrittenCallback);
				}
			}			
		}			
	}
	else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_NAME_LIST)
	{
		// Parameter loading
		if(packet[1] == mooltipass.device.commands['getVersion'])
		{
			mooltipass.datamemmgmt.nbMb = packet[2];
			mooltipass.datamemmgmt.CPZCTRHashTable = [];
			console.log("Mooltipass is " + mooltipass.datamemmgmt.nbMb + "Mb");			
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getCPZandCTR'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['exportCPZandCTR'])
		{
			//console.log("CPZ packet export packet received: " + packet.subarray(2, 2 + 8)); 
			
			// Generate CTR hash, store CPZ <> CTR hash entry
			mooltipass.datamemmgmt.userCTRHash = mooltipass.util.sha256hash(mooltipass.util.arrayToHexStr(packet.subarray(2+8, 2+8+16)));
			mooltipass.datamemmgmt.CPZCTRHashTable.push([mooltipass.util.arrayToHexStr(packet.subarray(2, 2 + 8)), mooltipass.datamemmgmt.userCTRHash]);
			
			// Arm receive
			mooltipass.memmgmt_hid.receiveMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getCPZandCTR'])
		{
			if(packet[2] == 0)
			{				
				mooltipass.datamemmgmt.requestFailHander("Error during CPZ/CTR request (card removed?)", MGMT_IDLE);
			}
			else
			{
				// Inform that all CPZ packets were sent
				console.log("All CPZ packets are received");		

				// Merge CPZ / hash values with our preferences
				mooltipass.datamemmgmt.mergeCPZCTRHashTableWithPreferences();
				
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getStartingDataParentAddress'], null);
				mooltipass.memmgmt_hid._sendMsg();				
			}
		}
		else if(packet[1] == mooltipass.device.commands['getStartingDataParentAddress'])
		{
			if(packet[0] == 1)
			{
				mooltipass.datamemmgmt.requestFailHander("Error during get data starting parent request (card removed?)", DATAMGMT_IDLE);
			}
			else
			{
				mooltipass.datamemmgmt.dataStartingParent = [packet[2], packet[3]];
				console.log("Data starting parent is " + mooltipass.datamemmgmt.dataStartingParent);

				/* Reset global vars */
				mooltipass.datamemmgmt.packetToSendBuffer = [];
				mooltipass.datamemmgmt.curDataServiceNodes = [];		
				mooltipass.datamemmgmt.curDataNodes = [];			
				mooltipass.datamemmgmt.clonedCurDataServiceNodes = [];		
				mooltipass.datamemmgmt.clonedCurDataNodes = []; 
				mooltipass.datamemmgmt.currentNode = new Uint8Array(NODE_SIZE);
				mooltipass.datamemmgmt.nodePacketId = 0;	
				
				if(mooltipass.datamemmgmt.currentMode == DATAMGMT_PARAM_LOAD_NAME_LIST)
				{
					if(mooltipass.memmgmt.isSameAddress(mooltipass.datamemmgmt.dataStartingParent, [0,0]) == true)
					{				
						// Starting data node is empty... nothing to list!
						mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
						mooltipass.datamemmgmt.currentMode = DATAMGMT_LOAD_NAME_SCAN;
						mooltipass.memmgmt_hid._sendMsg();
					}
					else
					{
						// Follow the nodes, starting with the parent one
						mooltipass.datamemmgmt.currentMode = DATAMGMT_LOAD_NAME_SCAN;
						mooltipass.datamemmgmt.curNodeAddressRequested = mooltipass.datamemmgmt.dataStartingParent;
						mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.datamemmgmt.dataStartingParent);
						mooltipass.memmgmt_hid._sendMsg();						
					}
				}			
			}
		}
	}
	else if(mooltipass.datamemmgmt.currentMode == DATAMGMT_LOAD_NAME_SCAN)
	{
		// check if we actually could read the node (permission problems....)
		if(packet[0] > 1)
		{
			// extend current node
			mooltipass.datamemmgmt.currentNode.set(packet.subarray(2, 2 + packet[0]), mooltipass.datamemmgmt.nodePacketId*HID_PAYLOAD_SIZE);
			 
			// check if is the last packet for a given node
			if(++mooltipass.datamemmgmt.nodePacketId == 3)
			{
				//console.log("Node received: " + mooltipass.datamemmgmt.currentNode);
				mooltipass.datamemmgmt.nodePacketId = 0;
				 
				// Parse node
				if(mooltipass.memmgmt.isNodeValid(mooltipass.datamemmgmt.currentNode))
				{
					// Get node type
					var nodeType = mooltipass.memmgmt.getNodeType(mooltipass.datamemmgmt.currentNode);
					 
					if(nodeType == 'dataparent')
					{
						// Store names, addresses, nodes
						mooltipass.datamemmgmt.curDataServiceNodes.push({'address': mooltipass.datamemmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getServiceName(mooltipass.datamemmgmt.currentNode), 'data': new Uint8Array(mooltipass.datamemmgmt.currentNode)});
						mooltipass.datamemmgmt.clonedCurDataServiceNodes.push({'address': mooltipass.datamemmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getServiceName(mooltipass.datamemmgmt.currentNode), 'data': new Uint8Array(mooltipass.datamemmgmt.currentNode)});
						console.log("Received data service " + mooltipass.datamemmgmt.curDataServiceNodes[mooltipass.datamemmgmt.curDataServiceNodes.length - 1].name + " at address " + mooltipass.datamemmgmt.curDataServiceNodes[mooltipass.datamemmgmt.curDataServiceNodes.length - 1].address);
						
						// Store next parent address
						mooltipass.datamemmgmt.nextParentNodeTSAddress = mooltipass.memmgmt.getNextAddress(mooltipass.datamemmgmt.currentNode);
						var first_child_address = mooltipass.memmgmt.getFirstChildAddress(mooltipass.datamemmgmt.currentNode);
						
						// Depending on the mode we're in
						if(mooltipass.datamemmgmt.currentMode == DATAMGMT_LOAD_NAME_SCAN)
						{
							// Store the name and ask for the next node
							mooltipass.datamemmgmt.nodeNames.push(mooltipass.memmgmt.getServiceName(mooltipass.datamemmgmt.currentNode));
							
							if(mooltipass.memmgmt.isSameAddress(mooltipass.datamemmgmt.nextParentNodeTSAddress, [0,0]))
							{
								// Finished scanning
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
								mooltipass.memmgmt_hid._sendMsg();
							}
							else
							{
								// Request next parent
								mooltipass.datamemmgmt.curNodeAddressRequested = mooltipass.datamemmgmt.nextParentNodeTSAddress;
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.datamemmgmt.nextParentNodeTSAddress);
								mooltipass.memmgmt_hid._sendMsg();										
							}
						}
					}
					else if(nodeType == 'data')
					{
						// Store names, addresses, nodes
						mooltipass.datamemmgmt.curDataNodes.push({'address': mooltipass.datamemmgmt.curNodeAddressRequested, 'data': new Uint8Array(mooltipass.datamemmgmt.currentNode), 'pointed': false});
						mooltipass.datamemmgmt.clonedCurDataNodes.push({'address': mooltipass.datamemmgmt.curNodeAddressRequested, 'data': new Uint8Array(mooltipass.datamemmgmt.currentNode), 'pointed': false});
					}
				}
				else
				{
					// Well this isn't a good situation... we read an empty node
					mooltipass.datamemmgmt.requestFailHander("Empty node read!!!", null);
					console.log(mooltipass.datamemmgmt.currentNode);
					// TODO: inform the user the memory is corrupted
				}
				 
				// Reset current node
				mooltipass.datamemmgmt.currentNode = new Uint8Array(NODE_SIZE);
			}
			else
			{
				// Else, receive other packet the Mooltipass should send
				mooltipass.memmgmt_hid.receiveMsg();
			}
		}
		else
		{
			// Well this isn't a good situation... we read a node that we weren't allowed to read
			mooltipass.datamemmgmt.requestFailHander("Error during listing credentials (card removed/memory corrupted?)", DATAMGMT_IDLE);
		}
	}
}

mooltipass.datamemmgmt.fileReadCallback = function(e)
{
	console.log("File read event...");
	 
	if(e != null && e.type == "loadend" && mooltipass.datamemmgmt.currentMode == MGMT_IDLE)
	{	
		// Init vars
		mooltipass.datamemmgmt.byteCounter = 0;		
		mooltipass.datamemmgmt.fileRead = new Uint8Array(e.target.result);
		mooltipass.datamemmgmt.fileReadName = mooltipass.filehandler.readFileName;
		
		if(mooltipass.datamemmgmt.fileRead.length < 5000)
		{
			console.log("File " + mooltipass.datamemmgmt.fileReadName + ", " + mooltipass.datamemmgmt.fileRead.length + " bytes long");
			
			// Change state & update data to be stored (multiple of 32 bytes)
			mooltipass.datamemmgmt.currentMode = DATAMGMT_PARAM_LOAD_FILEADD_REQ;
			var array_to_store_size = Math.ceil((mooltipass.datamemmgmt.fileRead.length + 2)/32)*32;
			var temp_array = new Uint8Array(array_to_store_size);
			temp_array.set([Math.floor(mooltipass.datamemmgmt.fileRead.length/256), (mooltipass.datamemmgmt.fileRead.length%256)], 0);
			temp_array.set(new Uint8Array(mooltipass.datamemmgmt.fileRead), 2);			
			mooltipass.datamemmgmt.fileRead = temp_array;
			
			// Set the timeouts & callbacks
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
			mooltipass.memmgmt_hid.responseCallback = mooltipass.datamemmgmt.dataReceivedCallback;
			mooltipass.memmgmt_hid.request['milliseconds'] = 2000;
			mooltipass.memmgmt_hid.nbSendRetries = 0;
			mooltipass.memmgmt_hid._sendMsg();		
		}
		else
		{
			console.log("File " + mooltipass.datamemmgmt.fileReadName + " is too long: " + mooltipass.datamemmgmt.fileRead.length + " bytes");
		}
	}	
}

mooltipass.datamemmgmt.getFileFromMooltipass = function(name)
{
	if(mooltipass.datamemmgmt.currentMode == MGMT_IDLE)
	{
		// Save name & set current mode
		mooltipass.datamemmgmt.byteCounter = 0;
		mooltipass.datamemmgmt.fileReadName = name;
		mooltipass.datamemmgmt.currentMode = DATAMGMT_PARAM_LOAD_FILEGET_REQ;
		// Set the timeouts & callbacks then send a media import start packet
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.datamemmgmt.dataReceivedCallback;
		mooltipass.memmgmt_hid.request['milliseconds'] = 2000;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();		
	}
}

mooltipass.datamemmgmt.addFileToMooltipass = function()
{
	mooltipass.filehandler.selectAndReadRawContents("", mooltipass.datamemmgmt.fileReadCallback);
}
	
mooltipass.datamemmgmt.listDataNodeNames = function()
{
	if(mooltipass.datamemmgmt.currentMode == DATAMGMT_IDLE)
	{
		mooltipass.datamemmgmt.nodeNames = [];
		mooltipass.datamemmgmt.currentMode = DATAMGMT_PARAM_LOAD_NAME_LIST_REQ;
		// First step is to query to user interaction timeout to set the correct packet timeout retry!
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.datamemmgmt.dataReceivedCallback;
		mooltipass.memmgmt_hid.request['milliseconds'] = 2000;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();
	}	
}



















