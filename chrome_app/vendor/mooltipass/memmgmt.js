var mooltipass = mooltipass || {};
mooltipass.memmgmt = mooltipass.memmgmt || {};

// Next error code available 702

// Defines
var NODE_SIZE							= 132;			// Node size
var HID_PAYLOAD_SIZE					= 62;			// HID payload
var MEDIA_BUNDLE_CHUNK_SIZE				= 33;			// How many bytes we send per chunk
var MGMT_PREFERENCES_VERSION			= 0;			// Preferences version
var MAX_CONTEXT_LENGTH					= 61;			// Context maximum length
var MAX_PASSWORD_LENGTH					= 31;			// Password maximum length
var MAX_DESCRIPTION_LENGTH				= 23;			// Description maximum length
	
// State machine modes	
var MGMT_IDLE							= 0;			// Idle mode
var MGMT_PARAM_LOAD						= 1;			// Parameter loading
var MGMT_PARAM_LOAD_REQ					= 2;			// Parameter loading
var MGMT_PARAM_LOAD_INT_CHECK			= 3;			// Parameter loading for integrity check
var MGMT_PARAM_LOAD_INT_CHECK_REQ		= 4;			// Parameter loading for integrity check, request
var MGMT_INT_CHECK_SCAN					= 5;			// Scanning through the memory for integrity check
var MGMT_INT_CHECK_PACKET_SENDING		= 6;			// Sending the correction packets
var MGMT_NORMAL_SCAN					= 7;			// Normal credential scan, following the nodes
var MGMT_BUNDLE_UPLOAD_REQ				= 8;			// Media bundle upload req
var MGMT_BUNDLE_UPLOAD					= 9;			// Media bundle upload
var MGMT_NORMAL_SCAN_DONE				= 10;			// Normal credentials can done
var MGMT_PASSWORD_REQ					= 11;			// Asking a password to the MP
var MGMT_DBFILE_MERGE_REQ				= 12;			// Asking to merge the credential file to the mooltipass
var MGMT_PARAM_LOAD_DBFILE_MERGE_REQ	= 13;			// Parameter load when merging credential file
var MGMT_PARAM_LOAD_DBFILE_MERGE		= 14;			// Parameter load when merging credential file
var MGMT_DBFILE_MERGE_NORMAL_SCAN		= 15;			// Normal memory scan when wanting to merge DBs
var MGMT_PARAM_LOAD_MEM_BACKUP			= 16;			// Parameter loading for memory backup
var MGMT_PARAM_LOAD_MEM_BACKUP_REQ		= 17;			// Parameter loading for memory backup, request
var MGMT_MEM_BACKUP_NORMAL_SCAN			= 18;			// Normal memory scan when wanting to backup memory
var MGMT_DB_FILE_MERGE_GET_FREE_ADDR	= 19;			// DB File merge: request free addresses to write
var MGMT_DB_FILE_MERGE_PACKET_SENDING	= 20;			// DB File merge: packet sending
var MGMT_PARAM_LOAD_FAIL				= 21;			// Something wrong happened during parameter loading
var MGMT_NORMAL_SCAN_FAIL				= 22;			// Something wrong happened during normal scan
var MGMT_USER_CHANGES_PACKET_SENDING	= 23;			// We are currently sending packets due to user changes on the memory contents
var MGMT_NORMAL_SCAN_DONE_NO_CHANGES	= 24;			// Normal scan done, no changes on the memory
var MGMT_NORMAL_SCAN_DONE_GET_FREE_ADDR	= 25;			// Normal scan done, asking for free addresses
var MGMT_NORMAL_SCAN_DONE_PASSWD_CHANGE	= 26;			// Changing passwords
var MGMT_ERROR_CUR_EXITTING_MMM			= 27;			// Following an error, we're exiting MMM
var MGMT_FORCE_EXIT_MMM					= 28;			// Force MMM exit

// Debug log
mooltipass.memmgmt.debugLog = false;					// Debug log in the console
 
// Mooltipass memory params
mooltipass.memmgmt.nbMb = null;							// Mooltipass memory size
mooltipass.memmgmt.ctrValue = [];						// Mooltipass CTR value
mooltipass.memmgmt.CPZCTRValues = [];					// Mooltipass CPZ CTR values
mooltipass.memmgmt.startingParent = null;				// Mooltipass current starting parent
mooltipass.memmgmt.dataStartingParent = null;			// Mooltipass current data starting parrent
mooltipass.memmgmt.favoriteAddresses = [];				// Mooltipass current favorite addresses
mooltipass.memmgmt.curServiceNodes = [];				// Mooltipass current service nodes
mooltipass.memmgmt.curLoginNodes = [];					// Mooltipass current login nodes
mooltipass.memmgmt.curDataServiceNodes = [];			// Mooltipass current data service nodes
mooltipass.memmgmt.curDataNodes = [];					// Mooltipass current data nodes
mooltipass.memmgmt.clonedStartingParent = null;			// Mooltipass current starting parent
mooltipass.memmgmt.clonedFavoriteAddresses = [];		// Mooltipass current favorite addresses
mooltipass.memmgmt.clonedCurServiceNodes = [];			// Mooltipass current service nodes
mooltipass.memmgmt.clonedCurLoginNodes = [];			// Mooltipass current login nodes
mooltipass.memmgmt.clonedCurDataServiceNodes = [];		// Mooltipass current data service nodes
mooltipass.memmgmt.clonedCurDataNodes = [];				// Mooltipass current data nodes
mooltipass.memmgmt.importedCtrValue = [];				// Mooltipass CTR value
mooltipass.memmgmt.importedCPZCTRValues = [];			// Mooltipass CPZ CTR values
mooltipass.memmgmt.importedStartingParent = null;		// Mooltipass current starting parent
mooltipass.memmgmt.importedDataStartingParent = null;	// Mooltipass current data starting parrent
mooltipass.memmgmt.importedFavoriteAddresses = [];		// Mooltipass current favorite addresses
mooltipass.memmgmt.importedCurServiceNodes = [];		// Mooltipass current service nodes
mooltipass.memmgmt.importedCurLoginNodes = [];			// Mooltipass current login nodes
mooltipass.memmgmt.importedCurDataServiceNodes = [];	// Mooltipass current data service nodes
mooltipass.memmgmt.importedCurDataNodes = [];			// Mooltipass current data nodes
mooltipass.memmgmt.credentialArrayForGui = [];			// Credential array for GUI

// Local preferences
/* Default preferences */
mooltipass.memmgmt.preferences = {"version": MGMT_PREFERENCES_VERSION, "backup_files": []};

// State machines & temp variables
mooltipass.memmgmt.version = null;                          // Mooltipass version
mooltipass.memmgmt.syncFS = null;							// SyncFS
mooltipass.memmgmt.syncFSOK = false;						// SyncFS state
mooltipass.memmgmt.currentMode = MGMT_IDLE;					// Current mode
mooltipass.memmgmt.currentFavorite = 0;						// Current favorite read/write
mooltipass.memmgmt.pageIt = 0;								// Page iterator
mooltipass.memmgmt.nodeIt = 0;								// Node iterator
mooltipass.memmgmt.scanPercentage = 0;						// Scanning percentage
mooltipass.memmgmt.nodePacketId = 0;						// Packet number for node sending/receiving
mooltipass.memmgmt.currentNode = [];						// Current node we're sending/receiving
mooltipass.memmgmt.packetToSendBuffer = [];					// Packets we need to send at the end of the checks etc...
mooltipass.memmgmt.origPacketToSendBufferLength = 0;		// Number of items inside the packet to send buffer when populated
mooltipass.memmgmt.packetToSendCompletionPercentage = 0;	// Percentage of packets to send sent
mooltipass.memmgmt.nextParentNodeTSAddress = [];			// Next parent node to scan address
mooltipass.memmgmt.curNodeAddressRequested = [];			// The address of the current node we're requesting
mooltipass.memmgmt.getPasswordCallback = null;				// Get password callback
mooltipass.memmgmt.getPasswordLogin = "";					// Login for the get password call
mooltipass.memmgmt.getPasswordServiceAddress = [];			// Service address for the get password call
mooltipass.memmgmt.getPasswordLoginAddress = [];			// Login address for the get password call
mooltipass.memmgmt.totalAddressesRequired = null;			// Number of addresses we need
mooltipass.memmgmt.totalAddressesReceived = null;			// Number of addresses we received
mooltipass.memmgmt.freeAddressesBuffer = [];				// The addresses we received
mooltipass.memmgmt.lastFreeAddressReceived = null;			// Last free address we received
mooltipass.memmgmt.tempCallbackErrorString = null;			// Temp string used for callback
mooltipass.memmgmt.memmgmtDeleteData = [];					// Delete data when clicking save
mooltipass.memmgmt.memmgmtUpdateData = [];					// Update data when clicking save
mooltipass.memmgmt.memmgmtAddData = [];						// Add data when clicking save
mooltipass.memmgmt.changePasswordReqs = [];					// Change password reqs
mooltipass.memmgmt.isCardKnownByMp = false;					// Check if the mooltipass knows the inserted the card
mooltipass.memmgmt.backupToFileReq = true;					// User is requesting a backup to file
mooltipass.memmgmt.backupFromFileReq = true;				// User is requesting a backup from file
mooltipass.memmgmt.statusCallback = null;					// Status callback different operations
mooltipass.memmgmt.progressCallback = null;					// Progress callback for integrity check
mooltipass.memmgmt.syncFSFileName = "";						// SyncFS file name for current user
mooltipass.memmgmt.CPZTable = [];							// A list of all the CPZ we know with the associated file name
mooltipass.memmgmt.syncFSParsedFileIndex = 0;				// Index of the syncFS file we are parsing
mooltipass.memmgmt.currentCardCPZ = [];						// Current card CPZ
mooltipass.memmgmt.lastLetter = '0';						// Current letter we're at
mooltipass.memmgmt.mergeFileTypeCsv = false;				// File type of the credential file we're merging
mooltipass.memmgmt.mediaBundleUploadPercentage = 0;			// Media upload progress percentage
mooltipass.memmgmt.currentLoginForRequestedPassword = "";	// The login for which we want its password

// State machines & temp variables related to media bundle upload
mooltipass.memmgmt.tempPassword = [];						// Temp password to unlock upload functionality
mooltipass.memmgmt.byteCounter = 0;							// Current byte counter
mooltipass.memmgmt.mediaBundle = [];						// Media bundle contents
mooltipass.memmgmt.mediaImportEndPacketSent = false;		// Media Import End packet sent

// Variables used for statistics
mooltipass.memmgmt.statsLastDataReceivedTime = new Date().getTime();// Time at which our last packet was received
mooltipass.memmgmt.statsTotalBytesReceived = 0;						// Number of bytes received since the time stored
 
 
// Node types
mooltipass.memmgmt.node_types = 
{
	0 : 'parent',
	1 : 'child',
	2 : 'dataparent',
	3 : 'data'
}
mooltipass.memmgmt.node_types_rev = 
{
	'parent' : 0,
	'child': 1,
	'dataparent' : 2,
	'data' : 3
}
mooltipass.memmgmt.mooltipass_status = 
{
	0 : 'No card inserted',
	1 : 'Card inserted & locked',
	2 : 'Unlocking screen',
	5 : 'Unlocked',
	9 : 'Unknown card'
} 

// Function for debug log
mooltipass.memmgmt.consoleLog = function(string)
{
	if (mooltipass.memmgmt.debugLog)
	{
		console.log(string);
	}
}
 
// Get node type
mooltipass.memmgmt.getNodeType = function(node)
{
	//mooltipass.memmgmt.consoleLog("Node Type is " + mooltipass.memmgmt.node_types[(node[1] >> 6) & 0x03]);
	return mooltipass.memmgmt.node_types[(node[1] >> 6) & 0x03];
}

// Set node type
mooltipass.memmgmt.setNodeType = function(node, type)
{
	node.set([(mooltipass.memmgmt.node_types_rev[type] << 6) & 0xC0], 1);
}
 
// Get user ID
mooltipass.memmgmt.getUserId = function(node)
{
	mooltipass.memmgmt.consoleLog("User Id is " + (node[1] & 0x1F));
	return (node[1] & 0x1F);
}
 
// Get previous address
mooltipass.memmgmt.getPrevAddress = function(node)
{
	return [node[2], node[3]];
}
 
// Change previous address
mooltipass.memmgmt.changePrevAddress = function(node, address)
{
	node.set(address, 2);
}
 
// Get next address
mooltipass.memmgmt.getNextAddress = function(node)
{
	return [node[4], node[5]];
}
 
// Change next address
mooltipass.memmgmt.changeNextAddress = function(node, address)
{
	node.set(address, 4);
}
 
// Get first child address
mooltipass.memmgmt.getFirstChildAddress = function(node)
{
	return [node[6], node[7]];
}
 
// Change first child address
mooltipass.memmgmt.changeFirstChildAddress = function(node, address)
{
	node.set(address, 6);
}
 
// Get next data node address
mooltipass.memmgmt.getNextDataNodeAddress = function(node)
{
	return [node[2], node[3]];
}
 
// Get service name
mooltipass.memmgmt.getServiceName = function(node)
{
	//mooltipass.memmgmt.consoleLog("Parent Node: Service name is " + mooltipass.util.arrayToStr(node.subarray(8, node.length)));
	return mooltipass.util.arrayToStr(node.subarray(8, node.length)); 
}

// Set service name
mooltipass.memmgmt.setServiceName = function(node, name)
{
	if(name.length < 121)
	{
		node.set(mooltipass.util.strToArray(name), 8);
	}	
	else
	{
		mooltipass.memmgmt.consoleLog("Service name too long: " + name);
	}
}
 
// Get description
mooltipass.memmgmt.getDescription = function(node)
{
	//mooltipass.memmgmt.consoleLog("Child Node: Description is " + mooltipass.util.arrayToStr(node.subarray(6, node.length)));
	return mooltipass.util.arrayToStr(node.subarray(6, node.length)); 
}

// Set description
mooltipass.memmgmt.setDescription = function(node, description)
{
	if(description.length < 24)
	{
		node.set(mooltipass.util.strToArray(description), 6);
	}	
}
 
// Get login
mooltipass.memmgmt.getLogin = function(node)
{
	//mooltipass.memmgmt.consoleLog("Child Node: Login is " + mooltipass.util.arrayToStr(node.subarray(37, node.length)));
	return mooltipass.util.arrayToStr(node.subarray(37, node.length)); 
}

// Set login
mooltipass.memmgmt.setLogin = function(node, login)
{
	if(login.length < 63)
	{
		node.set(mooltipass.util.strToArray(login), 37);
	}
}
 
// Get date created
mooltipass.memmgmt.getDateCreated = function(node)
{
	var year = ((node[30] >> 1) & 0x7F) + 2010;
	var month = ((node[30] & 0x01) << 3) | ((node[31] >> 5) & 0x07);
	var day = (node[31] & 0x1F);
	//mooltipass.memmgmt.consoleLog("Date created is " + day + "/" + month + "/" + year);
	return new Date(year, month, day, 0, 0, 0, 0);
}

// Set date created
mooltipass.memmgmt.setDateCreated = function(node, date)
{
	var array = [0,0];
	array[0] = ((date.getFullYear() - 2010) << 1) & 0xFE;
	if(date.getMonth() >= 8)
	{
		array[0] |= 0x01;
	}
	array[1] = ((date.getMonth()%8) << 5) & 0xE0;
	array[1] |= date.getDate();
	node.set(array, 30);
}
 
// Get date last used
mooltipass.memmgmt.getDateLastUsed = function(node)
{
	var year = ((node[32] >> 1) & 0x7F) + 2010;
	var month = ((node[32] & 0x01) << 3) | ((node[33] >> 5) & 0x07);
	var day = (node[33] & 0x1F);
	//mooltipass.memmgmt.consoleLog("Date last used is " + day + "/" + month + "/" + year);
	return new Date(year, month, day, 0, 0, 0, 0);
}

// Set date last used
mooltipass.memmgmt.setDateLastUsed = function(node, date)
{
	var array = [0,0];
	array[0] = ((date.getFullYear() - 2010) << 1) & 0xFE;
	if(date.getMonth() >= 8)
	{
		array[0] |= 0x01;
	}
	array[1] = ((date.getMonth()%8) << 5) & 0xE0;
	array[1] |= date.getDate();
	node.set(array, 32);
}
 
// Find if node is valid
mooltipass.memmgmt.isNodeValid = function(node)
{
	if((node[1] & 0x20) == 0)
	{
		//mooltipass.memmgmt.consoleLog("Node valid");
		return true;
	}
	else
	{
		//mooltipass.memmgmt.consoleLog("Node not valid");
		return false;
	}
}
 
// Get number of pages in the memory
mooltipass.memmgmt.getNumberOfPages = function(nbMb)
{
	if(nbMb >= 16)
	{
		return 256 * nbMb;
	}
	else
	{
		return 512 * nbMb;
	}
}
 
// Get the number of nodes per page
mooltipass.memmgmt.getNodesPerPage = function(nbMb)
{
	if(nbMb >= 16)
	{
		return 4;
	}
	else
	{
		return 2;
	}
}

mooltipass.memmgmt.getStartPage = function(nbMb)
{
	if(nbMb == 1)
	{
		return 128;
	}
	else if(nbMb == 2)
	{
		return 128;
	}
	else if(nbMb == 4)
	{
		return 256;
	}
	else if(nbMb == 8)
	{
		return 256;
	}
	else if(nbMb == 16)
	{
		return 256;
	}
	else if(nbMb == 32)
	{
		return 128;
	}
}
 
// Compare addresses
mooltipass.memmgmt.isSameAddress = function(addressA, addressB)
{
	if((addressA[0] == addressB[0]) && (addressA[1] == addressB[1]))
	{
		return true;
	}
	else
	{
		return false;
	}
}
 
// Compare addresses
mooltipass.memmgmt.isSameFavoriteAddress = function(addressA, addressB)
{
	if((addressA[0] == addressB[0]) && (addressA[1] == addressB[1]) && (addressA[2] == addressB[2]) && (addressA[3] == addressB[3]))
	{
		return true;
	}
	else
	{
		return false;
	}
}
 
// Find index of a given node in an array based on the address
mooltipass.memmgmt.findIdByAddress = function(arrayToSearch, address)
{
	for(var i = 0; i < arrayToSearch.length; i++)
	{
		if(mooltipass.memmgmt.isSameAddress(address, arrayToSearch[i].address))
		{
			return i;
		}
	}
	return null;
}

// Find index of a given node in an array based on the address
mooltipass.memmgmt.findIdByNextAddress = function(arrayToSearch, address)
{
	for(var i = 0; i < arrayToSearch.length; i++)
	{		
		if(mooltipass.memmgmt.isSameAddress(address, mooltipass.memmgmt.getNextAddress(arrayToSearch[i].data)))
		{
			return i;
		}
	}
	return null;
}

// Find index of a given node in an array based on the address
mooltipass.memmgmt.findIdByPrevAddress = function(arrayToSearch, address)
{
	for(var i = 0; i < arrayToSearch.length; i++)
	{		
		if(mooltipass.memmgmt.isSameAddress(address, mooltipass.memmgmt.getPrevAddress(arrayToSearch[i].data)))
		{
			return i;
		}
	}
	return null;
}

// Find index of a given node in an array based on the address
mooltipass.memmgmt.findIdByFirstChildAddress = function(arrayToSearch, address)
{
	for(var i = 0; i < arrayToSearch.length; i++)
	{		
		if(mooltipass.memmgmt.isSameAddress(address, mooltipass.memmgmt.getFirstChildAddress(arrayToSearch[i].data)))
		{
			return i;
		}
	}
	return null;
}

// Find index of a given node in an array based on the name
mooltipass.memmgmt.findIdByName = function(arrayToSearch, name)
{
	for(var i = 0; i < arrayToSearch.length; i++)
	{
		if(arrayToSearch[i].name == name)
		{
			return i;
		}
	}
	return null;
}

// Know if parent / child node is a favorite
mooltipass.memmgmt.isParentChildAFavorite = function(parentAddress, childAddress)
{
	for(var i = 0; i < mooltipass.memmgmt.clonedFavoriteAddresses.length; i++)
	{
		if(mooltipass.memmgmt.isSameFavoriteAddress(mooltipass.memmgmt.clonedFavoriteAddresses[i], [parentAddress[0], parentAddress[1], childAddress[0], childAddress[1]]))
		{
			return true;
		}
	}
	return false;
}

// Know if parent / child node is a favorite
mooltipass.memmgmt.isParentChildAFavoriteIndex = function(parentAddress, childAddress)
{
	for(var i = 0; i < mooltipass.memmgmt.clonedFavoriteAddresses.length; i++)
	{
		if(mooltipass.memmgmt.isSameFavoriteAddress(mooltipass.memmgmt.clonedFavoriteAddresses[i], [parentAddress[0], parentAddress[1], childAddress[0], childAddress[1]]))
		{
			return i;
		}
	}
	return null;
}

// Delete a favorite
mooltipass.memmgmt.deleteParentChildFavorite = function(parentAddress, childAddress)
{
	for(var i = 0; i < mooltipass.memmgmt.clonedFavoriteAddresses.length; i++)
	{
		if(mooltipass.memmgmt.isSameFavoriteAddress(mooltipass.memmgmt.clonedFavoriteAddresses[i], [parentAddress[0], parentAddress[1], childAddress[0], childAddress[1]]))
		{
			mooltipass.memmgmt.clonedFavoriteAddresses[i].set([0,0,0,0], 0);
		}
	}
}

// Add a favorite
mooltipass.memmgmt.addParentChildFavorite = function(parentAddress, childAddress)
{
	for(var i = 0; i < mooltipass.memmgmt.clonedFavoriteAddresses.length; i++)
	{
		if(mooltipass.memmgmt.isSameFavoriteAddress(mooltipass.memmgmt.clonedFavoriteAddresses[i], [0,0,0,0]))
		{
			mooltipass.memmgmt.clonedFavoriteAddresses[i].set([parentAddress[0], parentAddress[1], childAddress[0], childAddress[1]], 0);
			return;
		}
	}
}
 
// Compare node objects
mooltipass.memmgmt.compareNodeObjects = function(nodeA, nodeB)
{
	if(mooltipass.memmgmt.isSameAddress(nodeA.address, nodeB.address) == false)
	{
		return false;
	}
	if(nodeA.name != nodeB.name)
	{
		return false;
	}
	for(var i = 0; i < nodeA.data.length; i++)
	{
		if(nodeA.data[i] != nodeB.data[i])
		{
			return false;
		}
	}
	return true;
}

// Compare node data
mooltipass.memmgmt.compareNodeData = function(nodeA, nodeB)
{
	for(var i = 0; i < nodeA.data.length; i++)
	{
		if(nodeA.data[i] != nodeB.data[i])
		{
			return false;
		}
	}
	return true;	
}

// Compare parent node core data
mooltipass.memmgmt.compareParentNodeCoreData = function(nodeA, nodeB)
{
	// The only difference is that we are not checking the flags / prev / next / first child fields
	for(var i = 8; i < nodeA.data.length; i++)
	{
		if(nodeA.data[i] != nodeB.data[i])
		{
			return false;
		}
	}
	return true;	
}

// Compare child node core data
mooltipass.memmgmt.compareChildNodeCoreData = function(nodeA, nodeB)
{
	// The only difference is that we are not checking the flags / prev / next / first child fields
	for(var i = 6; i < nodeA.data.length; i++)
	{
		if(nodeA.data[i] != nodeB.data[i])
		{
			return false;
		}
	}
	return true;	
}
 
// Find if CPZ currently is in our CTR CPZ vector
mooltipass.memmgmt.isCPZValueKnownInCPZCTRVector = function(CPZValue, vector)
{
	for(var i = 0; i < vector.length; i++)
	{
		var cpz_found = true;
		for(var j = 0; j < 8; j++)
		{
			if(vector[i][j] != CPZValue[j])
			{
				cpz_found = false;
				break;
			}
		}
		if(cpz_found)
		{
			return true;
		}
	}
	return false;
} 

// Find if CPZ currently is in our CTR CPZ vector, return index
mooltipass.memmgmt.isCPZValueKnownInCPZCTRVectorIndex = function(CPZValue, vector)
{
	for(var i = 0; i < vector.length; i++)
	{
		var cpz_found = true;
		for(var j = 0; j < 8; j++)
		{
			if(vector[i][j] != CPZValue[j])
			{
				cpz_found = false;
				break;
			}
		}
		if(cpz_found)
		{
			return i;
		}
	}
	return null;
} 
 
// Add write node command to current packet bufferDepth
mooltipass.memmgmt.addWriteNodePacketToSendBuffer = function(address, node)
{	
	for(var i = 0; i < 3; i++)
	{
		// Set correct payload size
		var payload_size = 62;
		if(i == 2)
		{
			payload_size = 17;
		}
		var payload_to_send = new Uint8Array(payload_size);
		 
		// Write address, packet number, node data
		payload_to_send.set(address, 0);
		payload_to_send.set([i], 2);
		payload_to_send.set(node.subarray(i*59, (i*59)+(payload_size-3)), 3);
		//mooltipass.memmgmt.consoleLog(payload_to_send);
		mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['writeNodeInFlash'], payload_to_send));
	}
}
 
// Add write node command to current packet buffer for empty node
mooltipass.memmgmt.addEmptyNodePacketToSendBuffer = function(address)
{
	var empty_node = new Uint8Array(NODE_SIZE);
	for(var i = 0; i < NODE_SIZE; i++)
	{
		empty_node[i] = 0xFF;
	}
	mooltipass.memmgmt.addWriteNodePacketToSendBuffer(address, empty_node);
}
 
// Integrity check procedure
mooltipass.memmgmt.integrityCheck = function()
{
	// Sort name lists by alphabetical order
	mooltipass.memmgmt.curServiceNodes.sort(function(a,b){if(a.name < b.name) return -1; if(a.name > b.name) return 1; return 0;});
	mooltipass.memmgmt.clonedCurServiceNodes.sort(function(a,b){if(a.name < b.name) return -1; if(a.name > b.name) return 1; return 0;});
	mooltipass.memmgmt.curDataServiceNodes.sort(function(a,b){if(a.name < b.name) return -1; if(a.name > b.name) return 1; return 0;});
	mooltipass.memmgmt.clonedCurDataServiceNodes.sort(function(a,b){if(a.name < b.name) return -1; if(a.name > b.name) return 1; return 0;});
	 
	// Check service starting parent
	if((mooltipass.memmgmt.curServiceNodes.length > 0) && !mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.curServiceNodes[0].address, mooltipass.memmgmt.startingParent))
	{
		// Starting parent different, try to see if we know the currently set starting parent
		var current_starting_node = mooltipass.memmgmt.curServiceNodes.filter(function(obj){return mooltipass.memmgmt.isSameAddress(obj.address, mooltipass.memmgmt.startingParent)});
		 
		if(current_starting_node[0])
		{			
			// Set correct starting parent
			mooltipass.memmgmt.startingParent = mooltipass.memmgmt.curServiceNodes[0].address;
			console.log("Wrong starting parent: " + current_starting_node[0].name + " at address " + current_starting_node[0].address + "(" + mooltipass.memmgmt.curServiceNodes[0].name + ") should be " + mooltipass.memmgmt.curServiceNodes[0].name + " at address " + mooltipass.memmgmt.curServiceNodes[0].address);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingParentAddress'], mooltipass.memmgmt.curServiceNodes[0].address));		
		}
		else
		{
			console.log("Current starting node set to invalid value");
			mooltipass.memmgmt.startingParent = mooltipass.memmgmt.curServiceNodes[0].address;
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingParentAddress'], mooltipass.memmgmt.curServiceNodes[0].address));
		}
	}
	else
	{
		mooltipass.memmgmt.consoleLog("Starting parent address ok");
	}	
	 
	// Check data service starting parent
	/*if((mooltipass.memmgmt.curDataServiceNodes.length > 0) && !mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.curDataServiceNodes[0].address, mooltipass.memmgmt.dataStartingParent))
	{
		// Starting parent different, try to see if we know the currently set starting parent
		var current_starting_node = mooltipass.memmgmt.curDataServiceNodes.filter(function(obj){return mooltipass.memmgmt.isSameAddress(obj.address, mooltipass.memmgmt.dataStartingParent)});
		 
		if(current_starting_node[0])
		{			
			console.log("Wrong data starting parent: " + current_starting_node[0].name + " at address " + current_starting_node[0].address + ", should be " + mooltipass.memmgmt.curDataServiceNodes[0].name + " at address " + mooltipass.memmgmt.curServiceNodes[0].address);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingDataParentAddress'], mooltipass.memmgmt.curDataServiceNodes[0].address));
		}
		else
		{
			console.log("Current data starting node set to invalid value");
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingDataParentAddress'], mooltipass.memmgmt.curDataServiceNodes[0].address));
		}
	}
	else
	{
		mooltipass.memmgmt.consoleLog("Starting data parent address ok");
	}*/
	
	// Because of our wonderful previous developer, we can have parent nodes having the same name (urgh...). In this case we find the node that is pointed to and delete the other
	for(var i = mooltipass.memmgmt.clonedCurServiceNodes.length - 1; i >= 0; i--)
	{
		if((i != 0) && mooltipass.memmgmt.clonedCurServiceNodes[i].name == mooltipass.memmgmt.clonedCurServiceNodes[i-1].name)
		{
			console.log("Found duplicate parent node: " + mooltipass.memmgmt.clonedCurServiceNodes[i].name)
			
			// Find the one that is pointed to
			var current_node_pointed_to = false;
			for(var j = 0; j < mooltipass.memmgmt.clonedCurServiceNodes.length; j++)
			{
				if(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.clonedCurServiceNodes[i].address, mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[j])))
				{
					current_node_pointed_to = true;
				}
			}
			
			// Delete the correct node
			if(current_node_pointed_to && !mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.startingParent, mooltipass.memmgmt.clonedCurServiceNodes[i-1].address))
			{
				// Delete the other node
				console.log("Deleting node at address " + mooltipass.memmgmt.clonedCurServiceNodes[i-1].address);
				mooltipass.memmgmt.clonedCurServiceNodes.splice(i-1, 1);
				i++;
			}
			else
			{
				console.log("Deleting node at address " + mooltipass.memmgmt.clonedCurServiceNodes[i].address);
				mooltipass.memmgmt.clonedCurServiceNodes.splice(i, 1);
			}
		}
	}
	 
	// See if the sorted parent nodes actually are in the right order. Here the first node is verified to be the correct one
	var current_previous_node_addr = [];
	var correct_previous_node_addr = [];
	var current_next_node_addr = [];
	var correct_next_node_addr = [];
	var cur_parent_node_index = 0;
	mooltipass.memmgmt.consoleLog("Parent nodes check")
	while(cur_parent_node_index != mooltipass.memmgmt.clonedCurServiceNodes.length)
	{
		// Compute normal previous node address
		if(cur_parent_node_index == 0)
		{
			// If we are dealing with the first node, previous address should be NODE_ADDR_NULL
			correct_previous_node_addr = [0, 0];
		}
		else
		{
			// If not, it should be the address of the previous node
			correct_previous_node_addr = mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index-1].address;
		}
		// Check if the previous node address is correctly set
		current_previous_node_addr = mooltipass.memmgmt.getPrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].data);
		if(!mooltipass.memmgmt.isSameAddress(correct_previous_node_addr, current_previous_node_addr))
		{
			console.log("Previous address for parent " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + ": " + current_previous_node_addr + " instead of " + correct_previous_node_addr);
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].data, correct_previous_node_addr);
		}
		else
		{
			mooltipass.memmgmt.consoleLog("Previous address for parent " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + " correct");
		}
		 
		// Compute normal next node address
		current_next_node_addr = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].data);
		if(cur_parent_node_index == mooltipass.memmgmt.clonedCurServiceNodes.length-1)
		{
			// If we are dealing with the last node, next address should be NODE_ADDR_NULL
			correct_next_node_addr = [0, 0];
			 
			// Check if the previous node address is correctly set
			if(!mooltipass.memmgmt.isSameAddress(correct_next_node_addr, current_next_node_addr))
			{
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].data, correct_next_node_addr);
			}			
		}
		else
		{
			// If not, it should be the address of the previous node
			correct_next_node_addr = mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index+1].address;
			 
			// Check if the previous node address is correctly set
			if(!mooltipass.memmgmt.isSameAddress(correct_next_node_addr, current_next_node_addr))
			{				
				// Previous parent different, try to see if we know the currently set previous parent
				var current_next_node = mooltipass.memmgmt.clonedCurServiceNodes.filter(function(obj){return mooltipass.memmgmt.isSameAddress(obj.address, current_next_node_addr)});
				 
				if(current_next_node[0])
				{
					console.log("Next address for parent " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + " at address " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].address + " : " + current_next_node_addr + " (" + current_next_node[0].name + ") instead of " + correct_next_node_addr + " (" + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index+1].name + ")");
					// Change next address
					mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].data, correct_next_node_addr);
				}
				else
				{
					// Address doesn't exist in our recovered nodes...
					console.log("Next address for parent " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + " at address " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].address + " : " + current_next_node_addr + " (invalid) instead of " + correct_next_node_addr);
					mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].data, correct_next_node_addr);
				}
			}
			else
			{
				mooltipass.memmgmt.consoleLog("Correct next address for parent " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + " at address " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].address + " : " + correct_next_node_addr + " (" + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index+1].name + ")");
			}
		}
		 
		cur_parent_node_index++;
	}
	 
	// Check the child nodes for the parent nodes...
	var cur_parent_node_index = 0;
	var cur_child_node_address = 0;
	mooltipass.memmgmt.consoleLog("Child nodes check");
	while(cur_parent_node_index != mooltipass.memmgmt.clonedCurServiceNodes.length)
	{
		// Get child node address
		cur_child_node_address = mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].data)
		 
		// Try to find the first child node address in our database
		var current_child_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, cur_child_node_address);
		 
		// If we found a match, or if the address is set to nothing
		if(current_child_node_index != null)
		{			
			// Address found, tag it as pointed
			mooltipass.memmgmt.consoleLog("Parent node " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + " first child is " + mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].name + " at address " + cur_child_node_address);
			mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].pointed = true;
			 
			// Now check the next nodes
			var correct_prev_child_node_address = [0,0];
			var next_child_node_address;
			var next_child_node_index;
			var temp_bool = true;
			while(temp_bool)
			{
				// Get previous child node address
				var prev_child_node_address = mooltipass.memmgmt.getPrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].data);
				// Get next child node address
				next_child_node_address = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].data);
				// Check if we know its address
				next_child_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, next_child_node_address);
				 
				// Prev child node address
				if(mooltipass.memmgmt.isSameAddress(correct_prev_child_node_address, prev_child_node_address))
				{
					mooltipass.memmgmt.consoleLog("Previous child address is OK: " + prev_child_node_address);
				}
				else
				{
					console.log("Problem with previous child address: " + prev_child_node_address + " instead of " + correct_prev_child_node_address);
					mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].data, correct_prev_child_node_address);
				}
				 
				// Next child node address
				if(next_child_node_index == current_child_node_index)
				{
					// We have a loop
					console.log("Child node " + mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].name + " has an invalid next child address of itself " + next_child_node_address + ", setting it to 0");
					mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].data, [0,0]);
					temp_bool = false;
				}
				else if(next_child_node_index != null)
				{
					// Known next address, continue looping
					mooltipass.memmgmt.consoleLog("Next child node is " + mooltipass.memmgmt.clonedCurLoginNodes[next_child_node_index].name + " at address " + next_child_node_address);
					mooltipass.memmgmt.clonedCurLoginNodes[next_child_node_index].pointed = true;
					correct_prev_child_node_address = mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].address;
					current_child_node_index = next_child_node_index;
				}
				else if(mooltipass.memmgmt.isSameAddress([0, 0], next_child_node_address))
				{
					// No next child node, we're done
					temp_bool = false
				}
				else
				{
					// Invalid address
					console.log("Child node " + mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].name + " has an invalid next child address of " + next_child_node_address + ", setting it to 0");
					mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].data, [0,0]);
					temp_bool = false;
				}
			}			
		}
		else if(mooltipass.memmgmt.isSameAddress([0, 0], cur_child_node_address))
		{
			// No child nodes... which can happen
			console.log("Parent node " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + " doesn't have credentials");
		}
		else
		{
			// Invalid address
			console.log("Parent node " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + " has an invalid child address of " + cur_child_node_address + " setting it to 0");
			mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].data, [0, 0]);
		}		
		 
		cur_parent_node_index++;
	}
	 
	// Check for possible orphan child nodes
	mooltipass.memmgmt.consoleLog("Checking for orphan child nodes...");
	for(var i = mooltipass.memmgmt.clonedCurLoginNodes.length - 1; i >= 0; i--)
	{
		// Check if it was pointed
		if(mooltipass.memmgmt.clonedCurLoginNodes[i].pointed == false)
		{
			console.log("Found orphan node: " + mooltipass.memmgmt.clonedCurLoginNodes[i].name + " at address " + mooltipass.memmgmt.clonedCurLoginNodes[i].address + " , deleting it...");
			mooltipass.memmgmt.clonedCurLoginNodes.splice(i,1);
		}		
	}
													 
	// Now that we possibility have removed nodes, check that the favorite addresses are correct
	for(var i = 0; i < mooltipass.memmgmt.favoriteAddresses.length; i++)
	{
		// Extract addresses
		var cur_favorite_address_parent = mooltipass.memmgmt.favoriteAddresses[i].subarray(0, 0 + 2);
		var cur_favorite_address_child = mooltipass.memmgmt.favoriteAddresses[i].subarray(2, 2 + 2);
		 
		// Only compare if both addresses are different than 0
		if(mooltipass.memmgmt.isSameAddress(cur_favorite_address_child, [0,0]) && mooltipass.memmgmt.isSameAddress(cur_favorite_address_parent,[0,0]))
		{
			mooltipass.memmgmt.consoleLog("Favorite " + i + " is empty");
		}
		else
		{
			var parent_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, cur_favorite_address_parent);		
			var child_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, cur_favorite_address_child);		
			 
			// Check if both addresses are correct
			if(parent_node_index != null && child_node_index != null)
			{
				mooltipass.memmgmt.consoleLog("Favorite " + i + " is valid: " + mooltipass.memmgmt.clonedCurLoginNodes[child_node_index].name + " on " + mooltipass.memmgmt.clonedCurServiceNodes[parent_node_index].name);
			}
			else
			{
				// Reset favorite
				console.log("Favorite " + i + " is incorrect");
				mooltipass.memmgmt.clonedFavoriteAddresses[i] = new Uint8Array([0,0,0,0]);
			}
		}
	}
	 
	// Compare what is currently in memory and what we correct
	for(var i = 0; i < mooltipass.memmgmt.curServiceNodes.length; i++)
	{
		// Try to find the node at the same address (we never change addresses, just change data or delete nodes)
		var same_address_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.curServiceNodes[i].address);
		 
		if(same_address_node_index != null)
		{
			// We found a node at this address
			if(mooltipass.memmgmt.compareNodeObjects(mooltipass.memmgmt.curServiceNodes[i], mooltipass.memmgmt.clonedCurServiceNodes[same_address_node_index]) == false)
			{
				// Nodes differ
				console.log("Parent node " + mooltipass.memmgmt.curServiceNodes[i].name + " differs... updating it");
				mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.curServiceNodes[i].address, mooltipass.memmgmt.clonedCurServiceNodes[same_address_node_index].data);
			}
			else
			{
				mooltipass.memmgmt.consoleLog("Parent node " + mooltipass.memmgmt.curServiceNodes[i].name + " OK");
			}
		}
		else
		{
			// Node was deleted
			console.log("Deleting parent node " + mooltipass.memmgmt.curServiceNodes[i].name);
			mooltipass.memmgmt.addEmptyNodePacketToSendBuffer(mooltipass.memmgmt.curServiceNodes[i].address);
		}
	}
	for(var i = 0; i < mooltipass.memmgmt.curLoginNodes.length; i++)
	{
		// Try to find the node at the same address (we never change addresses, just change data or delete nodes)
		var same_address_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.curLoginNodes[i].address);
		 
		if(same_address_node_index != null)
		{
			// We found a node at this address
			if(mooltipass.memmgmt.compareNodeObjects(mooltipass.memmgmt.curLoginNodes[i], mooltipass.memmgmt.clonedCurLoginNodes[same_address_node_index]) == false)
			{
				// Nodes differ
				console.log("Child node " + mooltipass.memmgmt.curLoginNodes[i].name + " differs... updating it");
				mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.curLoginNodes[i].address, mooltipass.memmgmt.clonedCurLoginNodes[same_address_node_index].data);
			}
			else
			{				
				mooltipass.memmgmt.consoleLog("Child node " + mooltipass.memmgmt.curLoginNodes[i].name + " OK");
			}
		}
		else
		{
			// Node was deleted
			console.log("Deleting child node " + mooltipass.memmgmt.curLoginNodes[i].name);
			mooltipass.memmgmt.addEmptyNodePacketToSendBuffer(mooltipass.memmgmt.curLoginNodes[i].address);
		}
	}	
	for(var i = 0; i < mooltipass.memmgmt.favoriteAddresses.length; i++)
	{
		// Did the favorite change?
		if(!mooltipass.memmgmt.isSameFavoriteAddress(mooltipass.memmgmt.favoriteAddresses[i], mooltipass.memmgmt.clonedFavoriteAddresses[i]))
		{
			// Send a packet to update the address
			mooltipass.memmgmt.consoleLog("Updating favorite " + i);
			var favorite_packet = new Uint8Array(5);
			favorite_packet.set([i], 0);
			favorite_packet.set(mooltipass.memmgmt.clonedFavoriteAddresses[i], 1);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], favorite_packet));
		}
	}	
	 
	//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.packetToSendBuffer);
	return;
	 
	mooltipass.memmgmt.consoleLog("Services:");
	for(var i = 0; i < mooltipass.memmgmt.curServiceNodes.length; i++)
	{
		mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.curServiceNodes[i].name + " at address " + mooltipass.memmgmt.curServiceNodes[i].address);
	}
	mooltipass.memmgmt.consoleLog("Cloned Services:");
	for(var i = 0; i < mooltipass.memmgmt.clonedCurServiceNodes.length; i++)
	{
		mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " at address " + mooltipass.memmgmt.clonedCurServiceNodes[i].address);
	}
	mooltipass.memmgmt.consoleLog("Logins:");
	for(var i = 0; i < mooltipass.memmgmt.clonedCurLoginNodes.length; i++)
	{
		mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurLoginNodes[i].name + " pointed: " + mooltipass.memmgmt.clonedCurLoginNodes[i].pointed + " address: " + mooltipass.memmgmt.clonedCurLoginNodes[i].address);
	}
	mooltipass.memmgmt.consoleLog("Data services:");
	for(var i = 0; i < mooltipass.memmgmt.curDataServiceNodes.length; i++)
	{
		mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.curDataServiceNodes[i].name);
	}
}
 
// Function called with a read progress event, parse read file
mooltipass.memmgmt.processReadProgressEvent = function(e)
{
	mooltipass.memmgmt.consoleLog("Processing progress read event...");
	 
	if(e.type == "loadend")
	{
		if(e.target.result == "")
		{
			if(mooltipass.memmgmt.currentMode != MGMT_IDLE)
			{
				mooltipass.memmgmt.requestFailHander("Read: Empty file", null, 666);
			}
			return;
		}
		 
		var imported_data;
		
		// Depending on the file we are trying to merge
		if(mooltipass.memmgmt.mergeFileTypeCsv)
		{
			Papa.BAD_DELIMITERS.push(".")
			imported_data = Papa.parse(e.target.result);
			mooltipass.memmgmt.consoleLog(imported_data);
			
			// Check data format
			if(imported_data === undefined || imported_data.errors.length != 0)
			{
				if(mooltipass.memmgmt.currentMode != MGMT_IDLE)
				{
					mooltipass.memmgmt.requestFailHander("Wrong data format!", null, 698);
				}
				return;
			}
			if((imported_data.data[0].length != 3) && (imported_data.data[0].length != 4))
			{
				if(mooltipass.memmgmt.currentMode != MGMT_IDLE)
				{
					mooltipass.memmgmt.requestFailHander("CSV file should have 3 or 4 rows: website, login and password (description)", null, 699);
				}
				return;
			}
			
			// Create the added data
			mooltipass.memmgmt.memmgmtAddData = [];			
			mooltipass.memmgmt.memmgmtDeleteData = [];
			mooltipass.memmgmt.memmgmtUpdateData = [];
			for(var i = 0; i < imported_data.data.length; i++)
			{
				if(imported_data.data[i].length == 3)
				{
					// OUTDATED from 09/12/2016: Use the public suffix list to check for valid URLs
					/*var parsing_result = mooltipass.util.extractDomainAndSubdomain(imported_data.data[i][0].toLowerCase());
					if(parsing_result.valid)
					{
						var chosen_url;
						if(parsing_result.subdomain == null)
						{
							chosen_url = parsing_result.domain;
						}
						else
						{
							chosen_url = parsing_result.subdomain + "." + parsing_result.domain;
						}
						mooltipass.memmgmt.memmgmtAddData.push({"context": chosen_url.substring(0, MAX_CONTEXT_LENGTH), "username": imported_data.data[i][1].substring(0, MAX_CONTEXT_LENGTH), "password": imported_data.data[i][2].substring(0, MAX_PASSWORD_LENGTH), "description": "Imported by CSV"});
					}*/
					mooltipass.memmgmt.memmgmtAddData.push({"context": imported_data.data[i][0].toLowerCase().substring(0, MAX_CONTEXT_LENGTH), "username": imported_data.data[i][1].substring(0, MAX_CONTEXT_LENGTH), "password": imported_data.data[i][2].substring(0, MAX_PASSWORD_LENGTH), "description": "Imported by CSV"});
				}	
				else if(imported_data.data[i].length == 4)
				{
					// OUTDATED from 09/12/2016: Use the public suffix list to check for valid URLs
					/*var parsing_result = mooltipass.util.extractDomainAndSubdomain(imported_data.data[i][0].toLowerCase());
					if(parsing_result.valid)
					{
						var chosen_url;
						if(parsing_result.subdomain == null)
						{
							chosen_url = parsing_result.domain;
						}
						else
						{
							chosen_url = parsing_result.subdomain + "." + parsing_result.domain;
						}
						mooltipass.memmgmt.memmgmtAddData.push({"context": chosen_url.substring(0, MAX_CONTEXT_LENGTH), "username": imported_data.data[i][1].substring(0, MAX_CONTEXT_LENGTH), "password": imported_data.data[i][2].substring(0, MAX_PASSWORD_LENGTH), "description": "Imported by CSV"});
					}*/
					mooltipass.memmgmt.memmgmtAddData.push({"context": imported_data.data[i][0].toLowerCase().substring(0, MAX_CONTEXT_LENGTH), "username": imported_data.data[i][1].substring(0, MAX_CONTEXT_LENGTH), "password": imported_data.data[i][2].substring(0, MAX_PASSWORD_LENGTH), "description": imported_data.data[i][3].toLowerCase().substring(0, MAX_DESCRIPTION_LENGTH)});
				}
			}
			mooltipass.memmgmt.totalAddressesRequired = mooltipass.memmgmt.memmgmtAddData.length*2;
			
			// First step is to query to user interaction timeout to set the correct packet timeout retry
			mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_DBFILE_MERGE_REQ;
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
			mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
			mooltipass.memmgmt_hid.timeoutCallback = mooltipass.memmgmt.dataSendTimeOutCallback;
			mooltipass.memmgmt_hid.nbSendRetries = 0;
			mooltipass.memmgmt_hid._sendMsg();		
		}
		else
		{		
			try
			{
				imported_data = JSON.parse(e.target.result);
			}
			catch(e)
			{
				mooltipass.memmgmt.requestFailHander("Not a valid backup file", null, 690);
			}
			 
			// Check data format
			if(imported_data === undefined || !(imported_data.length == 10 || imported_data.length == 14))
			{
				if(mooltipass.memmgmt.currentMode != MGMT_IDLE)
				{
					mooltipass.memmgmt.requestFailHander("Wrong data format: incorrect length or undefined contents", null, 667);
				}
				return;
			}
		
			// Get values
			var tempCtrValue = imported_data[0];
			var tempCPZCTRValues = imported_data[1];
			var tempStartingParent = imported_data[2];
			var tempDataStartingParent = imported_data[3];
			var tempFavoriteAddresses = imported_data[4];
			var tempCurServiceNodes = imported_data[5];
			var tempCurLoginNodes = imported_data[6];
			var tempCurDataServiceNodes = imported_data[7];
			var tempCurDataNodes = imported_data[8];
			var checkString = imported_data[9];
			 
			// Check data format
			if(!(checkString == "mooltipass" || checkString == "moolticute"))
			{
				console.log("Wrong data format!");
				if(mooltipass.memmgmt.currentMode != MGMT_IDLE)
				{
					mooltipass.memmgmt.requestFailHander("Wrong data format!", null, 668);
				}
				return;
			}
			 
			// Copy and correctly type imported data
			mooltipass.memmgmt.importedCtrValue = new Uint8Array(Object.keys(tempCtrValue).map(function(k){return tempCtrValue[k]}));
			 
			mooltipass.memmgmt.importedCPZCTRValues = [];
			for(var i = 0; i < tempCPZCTRValues.length; i++)
			{
				mooltipass.memmgmt.importedCPZCTRValues.push(new Uint8Array(Object.keys(tempCPZCTRValues[i]).map(function(k){return tempCPZCTRValues[i][k]})));
			}
			 
			mooltipass.memmgmt.importedStartingParent = tempStartingParent;
			mooltipass.memmgmt.importedDataStartingParent = tempDataStartingParent;
			 
			mooltipass.memmgmt.importedFavoriteAddresses = [];
			for(var i = 0; i < tempFavoriteAddresses.length; i++)
			{
				mooltipass.memmgmt.importedFavoriteAddresses.push(new Uint8Array(Object.keys(tempFavoriteAddresses[i]).map(function(k){return tempFavoriteAddresses[i][k]})));
			}
			 
			mooltipass.memmgmt.importedCurServiceNodes = [];
			for(var i = 0; i < tempCurServiceNodes.length; i++)
			{
				mooltipass.memmgmt.importedCurServiceNodes.push({'address': tempCurServiceNodes[i].address, 'name': tempCurServiceNodes[i].name, 'data': new Uint8Array(Object.keys(tempCurServiceNodes[0].data).map(function(k){return tempCurServiceNodes[i].data[k]}))});
			}
			 
			mooltipass.memmgmt.importedCurLoginNodes = [];
			for(var i = 0; i < tempCurLoginNodes.length; i++)
			{
				mooltipass.memmgmt.importedCurLoginNodes.push({'address': tempCurLoginNodes[i].address, 'name': tempCurLoginNodes[i].name, 'data': new Uint8Array(Object.keys(tempCurLoginNodes[0].data).map(function(k){return tempCurLoginNodes[i].data[k]})), 'pointed': tempCurLoginNodes[i].pointed});
			}
			 
			mooltipass.memmgmt.importedCurDataServiceNodes = [];
			for(var i = 0; i < tempCurDataServiceNodes.length; i++)
			{
				mooltipass.memmgmt.importedCurDataServiceNodes.push({'address': tempCurDataServiceNodes[i].address, 'name': tempCurDataServiceNodes[i].name, 'data': new Uint8Array(Object.keys(tempCurDataServiceNodes[0].data).map(function(k){return tempCurDataServiceNodes[i].data[k]}))});
			}
			 
			mooltipass.memmgmt.importedCurDataNodes = [];
			for(var i = 0; i < tempCurDataNodes.length; i++)
			{
				mooltipass.memmgmt.importedCurDataNodes.push({'address': tempCurDataNodes[i].address, 'data': new Uint8Array(Object.keys(tempCurDataNodes[0].data).map(function(k){return tempCurDataNodes[i].data[k]}))});
			}
			 
			//mooltipass.memmgmt.consoleLog([mooltipass.memmgmt.importedCtrValue, mooltipass.memmgmt.importedCPZCTRValues, mooltipass.memmgmt.importedStartingParent, mooltipass.memmgmt.importedDataStartingParent, mooltipass.memmgmt.importedFavoriteAddresses, mooltipass.memmgmt.importedCurServiceNodes, mooltipass.memmgmt.importedCurLoginNodes, mooltipass.memmgmt.importedCurDataServiceNodes, mooltipass.memmgmt.importedCurDataNodes, checkString]);
			//mooltipass.memmgmt.consoleLog("Data imported!");	
			
			// Depending on current mode, launch next actions....
			if(mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_REQ)
			{
				// File read
				// First step is to query to user interaction timeout to set the correct packet timeout retry
				mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_DBFILE_MERGE_REQ;
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
				mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
				mooltipass.memmgmt_hid.timeoutCallback = mooltipass.memmgmt.dataSendTimeOutCallback;
				mooltipass.memmgmt_hid.nbSendRetries = 0;
				mooltipass.memmgmt_hid._sendMsg();	
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE_REQ)
			{
				// SyncFS file read
				// We parsed the data, now we can either enter memmgmt or add CPZ
				if(mooltipass.memmgmt.isCardKnownByMp)
				{
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['startMemoryManagementMode'], null);
					mooltipass.memmgmt_hid._sendMsg();							
				}
				else
				{
					// Unknown card by the MP, force store
					var cpz_ctr_index = mooltipass.memmgmt.isCPZValueKnownInCPZCTRVectorIndex(mooltipass.memmgmt.currentCardCPZ, mooltipass.memmgmt.importedCPZCTRValues);
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['addUnknownCard'], mooltipass.memmgmt.importedCPZCTRValues[cpz_ctr_index]);
					mooltipass.memmgmt_hid._sendMsg();		
				}
			}
		}
	}
	else
	{
		if(mooltipass.memmgmt.currentMode != MGMT_IDLE)
		{
			mooltipass.memmgmt.requestFailHander("Unsupported progress event type", null, 669);
		}
	}
}
 
// Export current Mooltipass memory state
mooltipass.memmgmt.exportMemoryState = function()
{
	var export_data = [mooltipass.memmgmt.ctrValue, mooltipass.memmgmt.CPZCTRValues, mooltipass.memmgmt.startingParent, mooltipass.memmgmt.dataStartingParent, mooltipass.memmgmt.favoriteAddresses, mooltipass.memmgmt.curServiceNodes, mooltipass.memmgmt.curLoginNodes, mooltipass.memmgmt.curDataServiceNodes, mooltipass.memmgmt.curDataNodes, "mooltipass"];
	//mooltipass.memmgmt.consoleLog(export_data); // {type: 'application/octet-binary'}
	
	if(mooltipass.memmgmt.backupToFileReq)
	{
		mooltipass.filehandler.selectAndSaveFileContents("memory_export.bin", new Blob([JSON.stringify(export_data)], {type: 'text/plain'}), mooltipass.memmgmt.fileWrittenCallback);
	}
	else
	{
		if(mooltipass.memmgmt.syncFSOK == true)
		{
			// Check if we know this file, if not, add it to local storage
			var file_known = false;
			for(var i = 0; i < mooltipass.memmgmt.preferences.backup_files.length; i++)
			{
				if(mooltipass.memmgmt.preferences.backup_files[i] == mooltipass.memmgmt.syncFSFileName)
				{
					file_known = true;
					mooltipass.memmgmt.consoleLog("File " + mooltipass.memmgmt.syncFSFileName + " already known in our DB");
				}
			}
			// Check if we need to store new CPZ values
			for(var i = 0; i < mooltipass.memmgmt.CPZCTRValues.length; i++)
			{
				var cpz_known = false;
				for(var j = 0; j < mooltipass.memmgmt.CPZTable.length; j++)
				{
					cpz_known = true;
					for(var k = 0; k < 8; k++)
					{
						if(mooltipass.memmgmt.CPZCTRValues[i][k] != mooltipass.memmgmt.CPZTable[j][0][k])
						{
							cpz_known = false;
							break;
						}
					}
					if(cpz_known == true)
					{
						break;
					}
				}
				if(cpz_known == false)
				{
					mooltipass.memmgmt.consoleLog("Adding unknown CPZ")
					mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.CPZCTRValues[i].subarray(0,8));
					mooltipass.memmgmt.CPZTable.push([mooltipass.memmgmt.CPZCTRValues[i].subarray(0,8), mooltipass.memmgmt.syncFSFileName]);
				}				
			}
			
			if(file_known == false)
			{
				mooltipass.memmgmt.consoleLog("Adding file " + mooltipass.memmgmt.syncFSFileName + " in our DB");
				mooltipass.memmgmt.preferences.backup_files.push(mooltipass.memmgmt.syncFSFileName);
				mooltipass.prefstorage.setStoredPreferences({"memmgmtPrefsStored": true, "memmgmtPrefs": mooltipass.memmgmt.preferences});
			}
			
			mooltipass.filehandler.writeCreateFileToSyncFS(mooltipass.memmgmt.syncFS, mooltipass.memmgmt.syncFSFileName, new Blob([JSON.stringify(export_data)], {type: 'text/plain'}), mooltipass.memmgmt.syncFSFileWrittenCallback);
		}
		else
		{
			mooltipass.memmgmt.requestFailHander("SyncFS not ready", null, 670);
		}
	}
}
 
// Callback after memory import file read
mooltipass.memmgmt.importMemoryStateCallBack = function(e)
{
	if(e == null)
	{
		mooltipass.memmgmt.requestFailHander("No file selected", null, 671);
	}
	else
	{
		mooltipass.memmgmt.consoleLog("Received data from file");
		mooltipass.memmgmt.processReadProgressEvent(e);
	}	 
}
 
// Import a Mooltipass memory state file
mooltipass.memmgmt.importMemoryState = function()
{
	if(mooltipass.memmgmt.mergeFileTypeCsv)
	{
		mooltipass.filehandler.selectAndReadContents("website_login_password.csv", mooltipass.memmgmt.importMemoryStateCallBack);
	}
	else
	{
		mooltipass.filehandler.selectAndReadContents("memory_export.bin", mooltipass.memmgmt.importMemoryStateCallBack);
	}	
}
 
// Export file written callback
mooltipass.memmgmt.fileWrittenCallback = function(file_written_bool)
{
	if(file_written_bool)
	{
		mooltipass.memmgmt.consoleLog("File written!");
		applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "File written"});
		mooltipass.memmgmt.currentMode = MGMT_IDLE;
		mooltipass.device.processQueue();
	}
	else
	{
		applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "User didn't select file"});	
		mooltipass.memmgmt.currentMode = MGMT_IDLE;	
		mooltipass.device.processQueue();
	}
}

// SyncFS export file written callback
mooltipass.memmgmt.syncFSFileWrittenCallback = function()
{
	mooltipass.memmgmt.consoleLog("File written to syncFS!");
	applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "File written to syncFS!"});
	mooltipass.memmgmt.currentMode = MGMT_IDLE;
	mooltipass.device.processQueue();
}
 
// Get syncable file system status callback
mooltipass.memmgmt.syncableFSStateCallback = function(status)
{
	if(status == "initializing")
	{
		mooltipass.memmgmt.consoleLog("SyncFS initializing");
		mooltipass.memmgmt.syncFSOK = false;
	}
	else if(status == "running")
	{
		mooltipass.memmgmt.consoleLog("SyncFS running");
		mooltipass.filehandler.requestSyncFS(mooltipass.memmgmt.syncFSGetCallback);
	}
	else if(status == "authentication_required")
	{
		mooltipass.memmgmt.consoleLog("SyncFS: authentication required");
		mooltipass.memmgmt.syncFSOK = false;
	}
	else if(status == "temporary_unavailable")
	{
		mooltipass.memmgmt.consoleLog("SyncFS temporary unavailable");
		mooltipass.memmgmt.syncFSOK = false;
	}
	else if(status == "disabled")
	{
		mooltipass.memmgmt.consoleLog("SyncFS disabled");
		mooltipass.memmgmt.syncFSOK = false;
	}
}
 
// SyncFS state change callback
mooltipass.memmgmt.syncableFSStateChangeCallback = function(detail)
{
	if(detail.state == "initializing")
	{
		console.log("SyncFS update: initializing");
		mooltipass.memmgmt.syncFSOK = false;
	}
	else if(detail.state == "running")
	{
		console.log("SyncFS update: running");
		mooltipass.filehandler.requestSyncFS(mooltipass.memmgmt.syncFSGetCallback);
	}
	else if(detail.state == "authentication_required")
	{
		console.log("SyncFS update: authentication required");
		mooltipass.memmgmt.syncFSOK = false;
	}
	else if(detail.state == "temporary_unavailable")
	{
		console.log("SyncFS update: temporary unavailable");
		mooltipass.memmgmt.syncFSOK = false;
	}
	else if(detail.state == "disabled")
	{
		console.log("SyncFS update: disabled");
		mooltipass.memmgmt.syncFSOK = false;
	}
	console.log(detail.description);
}
 
// SyncFS get callback
mooltipass.memmgmt.syncFSGetCallback = function(fs)
{
	// Check for errors
	if(chrome.runtime.lastError)
	{
		console.log("SyncFS Get error: "+ chrome.runtime.lastError.message);
	}
	else
	{
		// Received syncFS, request save file
		mooltipass.memmgmt.consoleLog("Received SyncFS");
		mooltipass.memmgmt.syncFS = fs;
		mooltipass.memmgmt.syncFSOK = true;
		
		// Start looping through the files to fetch the CPZ we know
		if(mooltipass.memmgmt.CPZTable.length == 0 && mooltipass.memmgmt.preferences.backup_files.length != 0 && mooltipass.memmgmt.syncFSOK == true)
		{
			mooltipass.memmgmt.syncFSParsedFileIndex = 0;
			mooltipass.memmgmt.populateCPZTable(null);
		}		
	}
}
 
// Callback when file from SyncFS is read
mooltipass.memmgmt.syncFSRequestFileCallback = function(e)
{
	mooltipass.memmgmt.consoleLog("Received data for syncFS file");
	mooltipass.memmgmt.processReadProgressEvent(e);
}

// Populate our known CPZ table, called by a function or by callback event for read
mooltipass.memmgmt.populateCPZTable = function(event)
{
	if(event == null)
	{
		// Called by function
		mooltipass.filehandler.requestFileFromSyncFS(mooltipass.memmgmt.syncFS, mooltipass.memmgmt.preferences.backup_files[mooltipass.memmgmt.syncFSParsedFileIndex++], mooltipass.memmgmt.populateCPZTable);
	}		
	else
	{
		// Called by event
		if(event.type == "loadend")
		{
			if(event.target.result == "")
			{
				// Empty file
				console.log("Read: Empty file");
				if(mooltipass.memmgmt.syncFSParsedFileIndex == mooltipass.memmgmt.preferences.backup_files.length)
				{
					mooltipass.memmgmt.consoleLog("CPZ table populated");
					//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.CPZTable);
				}
				else
				{
					mooltipass.filehandler.requestFileFromSyncFS(mooltipass.memmgmt.syncFS, mooltipass.memmgmt.preferences.backup_files[mooltipass.memmgmt.syncFSParsedFileIndex++], mooltipass.memmgmt.populateCPZTable);
				}
				return;
			}
			 
			var imported_data = JSON.parse(event.target.result);
			 
			// Check data format
			if(imported_data.length != 10)
			{
				console.log("Wrong data format!");
				if(mooltipass.memmgmt.syncFSParsedFileIndex == mooltipass.memmgmt.preferences.backup_files.length)
				{
					mooltipass.memmgmt.consoleLog("CPZ table populated");
					//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.CPZTable);
				}
				else
				{
					mooltipass.filehandler.requestFileFromSyncFS(mooltipass.memmgmt.syncFS, mooltipass.memmgmt.preferences.backup_files[mooltipass.memmgmt.syncFSParsedFileIndex++], mooltipass.memmgmt.populateCPZTable);
				}
				return;
			}
			 
			// Get values
			var tempCPZCTRValues = imported_data[1];
			var checkString = imported_data[9];
			 
			// Check data format
			if(checkString != "mooltipass")
			{
				console.log("Wrong data format!");
				if(mooltipass.memmgmt.syncFSParsedFileIndex == mooltipass.memmgmt.preferences.backup_files.length)
				{
					mooltipass.memmgmt.consoleLog("CPZ table populated");
					//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.CPZTable);
				}
				else
				{
					mooltipass.filehandler.requestFileFromSyncFS(mooltipass.memmgmt.syncFS, mooltipass.memmgmt.preferences.backup_files[mooltipass.memmgmt.syncFSParsedFileIndex++], mooltipass.memmgmt.populateCPZTable);
				}
				return;
			}
			 
			mooltipass.memmgmt.importedCPZCTRValues = [];
			for(var i = 0; i < tempCPZCTRValues.length; i++)
			{
				var array = new Uint8Array(Object.keys(tempCPZCTRValues[i]).map(function(k){return tempCPZCTRValues[i][k]}));
				mooltipass.memmgmt.CPZTable.push([array.subarray(0,8), mooltipass.memmgmt.preferences.backup_files[mooltipass.memmgmt.syncFSParsedFileIndex-1]]);
			}			 
			
			if(mooltipass.memmgmt.syncFSParsedFileIndex == mooltipass.memmgmt.preferences.backup_files.length)
			{
				mooltipass.memmgmt.consoleLog("CPZ table populated");
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.CPZTable);
			}
			else
			{
				mooltipass.filehandler.requestFileFromSyncFS(mooltipass.memmgmt.syncFS, mooltipass.memmgmt.preferences.backup_files[mooltipass.memmgmt.syncFSParsedFileIndex++], mooltipass.memmgmt.populateCPZTable);
			}
		}
		else
		{
			console.log("Unsupported progress event type!");
		}
	}
}
 
// GUI requesting the password of a given credential
mooltipass.memmgmt.getPasswordForCredential = function(service, login, callback)
{	
	// Check if service exists
	var service_exists = false;
	for(var i = 0; i < mooltipass.memmgmt.curServiceNodes.length; i++)
	{
		if(mooltipass.memmgmt.curServiceNodes[i].name == service)
		{
			mooltipass.memmgmt.getPasswordServiceAddress = mooltipass.memmgmt.curServiceNodes[i].address;
			service_exists = true;
			break;
		}
	}
	
	// Check if login exists
	var login_exists = false;
	for(var i = 0; i < mooltipass.memmgmt.curLoginNodes.length; i++)
	{
		if(mooltipass.memmgmt.curLoginNodes[i].name == login)
		{
			mooltipass.memmgmt.getPasswordLoginAddress = mooltipass.memmgmt.curLoginNodes[i].address;
			mooltipass.memmgmt.currentLoginForRequestedPassword = login;
			login_exists = true;
			break;
		}		
	}
	
	// Store callback and function
	mooltipass.memmgmt.getPasswordCallback = callback;
	mooltipass.memmgmt.getPasswordLogin = login;
	
	if(login_exists && service_exists && (mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE))
	{
		// Start the get password process
		mooltipass.memmgmt.currentMode = MGMT_PASSWORD_REQ;
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setContext'], mooltipass.util.strToArray(service));
		mooltipass.memmgmt_hid._sendMsg();
	}
	else
	{
		if(login_exists == false)
		{
			console.log("Unkown login: " + login);
		}
		if(service_exists == false)
		{
			console.log("Unkown service: " + service);
		}
		if(mooltipass.memmgmt.currentMode != MGMT_NORMAL_SCAN_DONE)
		{
			console.log("Wrong mode");
		}
		applyCallback(mooltipass.memmgmt.getPasswordCallback, null, {'success': false, 'code': 689, 'msg': "Unknown service or login"}, "not valid");
	}
}

// Count number of children for a given service node
mooltipass.memmgmt.getNumberOfChildrenForClonedServiceNode = function(node)
{
	// Get first child index
	var first_child_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getFirstChildAddress(node.data));
	
	if(first_child_index == null)
	{
		return 0;
	}
	else
	{
		var return_val = 1;
		var temp_index = first_child_index;
		// Loop until we reach the last child
		while(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[temp_index].data), [0,0]) == false)
		{
			temp_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[temp_index].data));
			return_val += 1;
		}
		return return_val;
	}
}

// Do all the operations required by deleting a parent node and generate delete node packet
mooltipass.memmgmt.deleteServiceNodeFromCloneArrayAndGenerateDeletePacket = function(node)
{
	mooltipass.memmgmt.consoleLog("Parent node " + node.name + " has been removed");
	mooltipass.memmgmt.addEmptyNodePacketToSendBuffer(node.address);
	
	// Next we need to update the prev/next nodes
	var prev_node_index = mooltipass.memmgmt.findIdByNextAddress(mooltipass.memmgmt.clonedCurServiceNodes, node.address);
	var next_node_index = mooltipass.memmgmt.findIdByPrevAddress(mooltipass.memmgmt.clonedCurServiceNodes, node.address);
	
	// Check if we are not dealing with the first node
	if(prev_node_index == null)
	{				
		if(next_node_index == null)
		{
			mooltipass.memmgmt.clonedStartingParent = [0,0];
			mooltipass.memmgmt.consoleLog("It seems we removed the only parent node!");
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingParentAddress'], mooltipass.memmgmt.clonedStartingParent)); 
		}
		else
		{
			mooltipass.memmgmt.clonedStartingParent = mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].address;
			mooltipass.memmgmt.consoleLog("Changing starting parent to: ", mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].name);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingParentAddress'], mooltipass.memmgmt.clonedStartingParent)); 
			mooltipass.memmgmt.consoleLog("Changing previous address of " + mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].name + " to empty address (first node)");
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].data, [0,0]);
		}				
	}
	else
	{
		if(next_node_index == null)
		{
			mooltipass.memmgmt.consoleLog("Updating next node field of " + mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].name + " to empty address (last node)");
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].data, [0,0]);
		}
		else
		{					
			mooltipass.memmgmt.consoleLog("Updating next node field of " + mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].name + " to " + mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].name);
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].data, mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].address)
			mooltipass.memmgmt.consoleLog("Changing previous address of " + mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].name + " to " + mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].name);
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].data, mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].address);
		}
	}
	
	// Delete prev and next address fields
	mooltipass.memmgmt.changePrevAddress(node.data, [0,0]);
	mooltipass.memmgmt.changeNextAddress(node.data, [0,0]);
}

// Do all the operations required by deleting a child node and generate delete node packet
mooltipass.memmgmt.deleteChildNodeFromCloneArrayAndGenerateDeletePacket = function(node)
{
	mooltipass.memmgmt.consoleLog("Child node " + node.name + " has been removed");
	mooltipass.memmgmt.addEmptyNodePacketToSendBuffer(node.address);
	
	// Next we need to update the prev/next nodes
	var prev_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getPrevAddress(node.data));
	var next_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getNextAddress(node.data));
	
	// Check if we are not dealing with the first node
	if(prev_node_index == null)
	{				
		// Find the parent for which this node is the first node (we are sure it exists!)
		var matching_parent_index = mooltipass.memmgmt.findIdByFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes, node.address);
		
		if(next_node_index == null)
		{
			mooltipass.memmgmt.consoleLog("It seems we removed the only child node!");
			mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[matching_parent_index].data, [0,0]);
		}
		else
		{
			mooltipass.memmgmt.consoleLog("Changing " + mooltipass.memmgmt.clonedCurServiceNodes[matching_parent_index].name + " first child to: ", mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].name);
			mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[matching_parent_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].address);
			mooltipass.memmgmt.consoleLog("Changing previous address of " + mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].name + " to empty address (first node)");
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].data, [0,0]);
		}				
	}
	else
	{
		if(next_node_index == null)
		{
			mooltipass.memmgmt.consoleLog("Updating next node field of " + mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].name + " to empty address (last node)");
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].data, [0,0]);
		}
		else
		{					
			mooltipass.memmgmt.consoleLog("Updating next node field of " + mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].name + " to " + mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].name);
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].address)
			mooltipass.memmgmt.consoleLog("Changing previous address of " + mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].name + " to " + mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].name);
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].data, mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].address);
		}
	}
}

// Move child node to another existing parent
mooltipass.memmgmt.changeChildNodeLoginAndMoveItToExistingParent = function(address, parent_address, new_parent_address, new_login)
{
	var new_parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, new_parent_address);
	var parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, parent_address);
	var child_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, address);
	
	if(parent_index != null && child_index != null && new_parent_index != null)
	{
		// Find indexes of previous & next nodes
		var prev_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getPrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data));
		var next_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data));
		
		// Update prev field for next node, next field for prev node
		if(prev_index != null)
		{
			if(next_index != null)
			{
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_index].address);
			}
			else
			{
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_index].data, [0,0]);
			}
		}
		if(next_index != null)
		{
			if(prev_index != null)
			{
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_index].data, mooltipass.memmgmt.clonedCurLoginNodes[prev_index].address);
			}
			else
			{
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_index].data, [0,0]);
			}
		}
			
		// If the node we're moving is the first one, change first child address for the parent node
		if(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data), mooltipass.memmgmt.clonedCurLoginNodes[child_index].address) == true)
		{
			if(next_index != null)
			{
				mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_index].address);				
			}
			else
			{
				mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data, [0,0]);	
			}
		}
		
		// At this point the node is an orphan node. We now add the node as the first child to the new parent node and call the ordering function
		var new_parent_first_child_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[new_parent_index].data));
		
		// If the node doesn't have children... then it's quite easy.
		if(new_parent_first_child_index == null)
		{
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, [0,0]);
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, [0,0]);
			mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[new_parent_index].data, mooltipass.memmgmt.clonedCurLoginNodes[child_index].address);
		}
		else
		{
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, [0,0]);
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, mooltipass.memmgmt.clonedCurLoginNodes[new_parent_first_child_index].address);
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[new_parent_first_child_index].data, mooltipass.memmgmt.clonedCurLoginNodes[child_index].address);
			mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[new_parent_index].data, mooltipass.memmgmt.clonedCurLoginNodes[child_index].address);
			mooltipass.memmgmt.changeChildNodeLoginAndUpdateNodes(mooltipass.memmgmt.clonedCurLoginNodes[child_index].address, mooltipass.memmgmt.clonedCurServiceNodes[new_parent_index].address, new_login);
		}
	}
	else
	{
		console.log("Error in changeChildNodeLoginAndMoveItToExistingParent: wrong addresses!");
	}
}

// Change the login field of a child node
mooltipass.memmgmt.changeChildNodeLoginAndUpdateNodes = function(address, parent_address, new_login)
{
	var parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, parent_address);
	var child_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, address);
	
	if(parent_index != null && child_index != null)
	{
		// Find indexes of previous & next nodes
		var prev_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getPrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data));
		var next_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data));
		
		// Update login field
		mooltipass.memmgmt.setLogin(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, new_login);
		mooltipass.memmgmt.clonedCurLoginNodes[child_index].name = new_login;
				
		if(prev_index != null && mooltipass.memmgmt.clonedCurLoginNodes[prev_index].name > new_login)
		{
			// Do we need to move it to the left?			
			var cur_node_index = prev_index;
			
			// Update prev field for next node, next field for prev node
			if(prev_index != null)
			{
				if(next_index != null)
				{
					mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_index].address);
				}
				else
				{
					mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_index].data, [0,0]);
				}
			}
			if(next_index != null)
			{
				if(prev_index != null)
				{
					mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_index].data, mooltipass.memmgmt.clonedCurLoginNodes[prev_index].address);
				}
				else
				{
					mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_index].data, [0,0]);
				}
			}
			
			// Find where to insert our node
			while(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.getPrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[cur_node_index].data), [0,0]) == false && (mooltipass.memmgmt.clonedCurLoginNodes[cur_node_index].name > new_login))
			{
				cur_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getPrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[cur_node_index].data));
			}
			
			// We found our spot, fetch prev & next nodes
			if(mooltipass.memmgmt.clonedCurLoginNodes[cur_node_index].name < new_login)
			{
				prev_index = cur_node_index;
				next_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[cur_node_index].data));				
			}
			else
			{
				next_index = cur_node_index;
				prev_index = null;				
			}
			
			// Update fields
			if(prev_index == null)
			{
				mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data, address);
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, [0,0]);
			}
			else
			{
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_index].data, mooltipass.memmgmt.clonedCurLoginNodes[child_index].address);
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, mooltipass.memmgmt.clonedCurLoginNodes[prev_index].address);
			}
			if(next_index == null)
			{
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, [0,0]);			
			}
			else
			{
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_index].data, mooltipass.memmgmt.clonedCurLoginNodes[child_index].address);
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_index].address);				
			}
		}
		else if(next_index != null && mooltipass.memmgmt.clonedCurLoginNodes[next_index].name < new_login)
		{
			// Do we need to move it to the right?
			var cur_node_index = next_index;			
			
			// Update prev field for next node, next field for prev node
			if(prev_index != null)
			{
				if(next_index != null)
				{
					mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_index].address);
				}
				else
				{
					mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_index].data, [0,0]);
				}
			}
			if(next_index != null)
			{
				if(prev_index != null)
				{
					mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_index].data, mooltipass.memmgmt.clonedCurLoginNodes[prev_index].address);
				}
				else
				{
					mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_index].data, [0,0]);
				}
			}
			
			// If the node we're moving to the right is the first one, change first child address for the parent node
			if(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data), mooltipass.memmgmt.clonedCurLoginNodes[child_index].address) == true)
			{
				mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_index].address);
			}
			
			// Find where to insert our node
			while(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[cur_node_index].data), [0,0]) == false && (mooltipass.memmgmt.clonedCurLoginNodes[cur_node_index].name < new_login))
			{
				cur_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[cur_node_index].data));
			}
			
			// We found our spot, fetch prev & next nodes
			if(mooltipass.memmgmt.clonedCurLoginNodes[cur_node_index].name > new_login)
			{
				prev_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getPrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[cur_node_index].data));
				next_index = cur_node_index;				
			}
			else
			{
				prev_index = cur_node_index;
				next_index = null;					
			}
			
			// Update fields
			if(prev_index == null)
			{
				mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data, address);
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, [0,0]);
			}
			else
			{
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_index].data, mooltipass.memmgmt.clonedCurLoginNodes[child_index].address);
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, mooltipass.memmgmt.clonedCurLoginNodes[prev_index].address);
			}
			if(next_index == null)
			{
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, [0,0]);			
			}
			else
			{
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_index].data, mooltipass.memmgmt.clonedCurLoginNodes[child_index].address);
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_index].address);				
			}
		}
		else
		{
			//console.log("No changes to be applied in the linked list")
		}
	}
	else
	{
		console.log("Error in changeChildNodeLoginAndUpdateNodes: wrong addresses!");
	}
}

// Get node indexes for a new service node to be inserted
mooltipass.memmgmt.getPrevAndNextNodeIndexesForNewServiceNode = function(name)
{
	var current_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.clonedStartingParent);
	var prev_node_index = null;
	var next_node_index = current_index;
	
	// No nodes in memory!
	if(current_index == null)
	{
		return [null, null];
	}
	
	// Loop until we find the right slot
	while(current_index != null && mooltipass.memmgmt.clonedCurServiceNodes[current_index].name < name)
	{
		// Load next address
		prev_node_index = current_index;
		current_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[current_index].data));
		next_node_index = current_index;
	}
	
	return [prev_node_index, next_node_index];
}

// Get node indexes for a new service node to be inserted
mooltipass.memmgmt.getPrevAndNextNodeIndexesForNewLoginNode = function(parentAddress, loginName)
{
	// Get first child index, set next and prev vars
	var parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, parentAddress);
	var first_child_address = mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data);
	var current_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, first_child_address);
	var prev_node_index = null;
	var next_node_index = current_index;
	
	// No nodes in memory!
	if(current_index == null)
	{
		return [null, null];
	}
	
	// Loop until we find the right slot
	while(current_index != null && mooltipass.memmgmt.clonedCurLoginNodes[current_index].name < loginName)
	{
		// Load next address
		prev_node_index = current_index;
		current_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[current_index].data));
		next_node_index = current_index;
	}
	//if(prev_node_index!=null)mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].name);else{mooltipass.memmgmt.consoleLog("null");}
	//if(next_node_index!=null)mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].name);else mooltipass.memmgmt.consoleLog("null");
	
	return [prev_node_index, next_node_index];
}
 
// Generate merge operations once we have listed all the credentials inside the Mooltipass
mooltipass.memmgmt.generateMergeOperations = function()
{
	// If we are just importing credentials stored in a csv file, simply return as update process is incremental and we have already set the correct totalAddressesRequired value
	if(mooltipass.memmgmt.mergeFileTypeCsv)
	{
		return mooltipass.memmgmt.totalAddressesRequired;
	}
	
	var virtual_address_counter = 0;				// When we add nodes, we give them an address based on this counter
	var new_children_for_existing_parents = [];		// New children for existing parent nodes
	
	mooltipass.memmgmt.consoleLog("");
	mooltipass.memmgmt.consoleLog("Merging procedure started");
	
	// Know if we need to add CPZ CTR packets (additive process)
	for(var i = 0; i < mooltipass.memmgmt.importedCPZCTRValues.length; i++)
	{
		if(!mooltipass.memmgmt.isCPZValueKnownInCPZCTRVector(mooltipass.memmgmt.importedCPZCTRValues[i].subarray(0, 8), mooltipass.memmgmt.CPZCTRValues))
		{
			// Send the unknown CPZ CTR value to the Mooltipass
			mooltipass.memmgmt.consoleLog("Unknown CPZ CTR values: " + mooltipass.memmgmt.importedCPZCTRValues[i]);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['addCPZandCTR'], mooltipass.memmgmt.importedCPZCTRValues[i]));
		}
		else
		{
			//mooltipass.memmgmt.consoleLog("Already known CPZ CTR values: " + mooltipass.memmgmt.importedCPZCTRValues[i]);
		}
	}
	
	// Check CTR value
	var cur_ctr = mooltipass.memmgmt.ctrValue[0] * 256 * 256 + mooltipass.memmgmt.ctrValue[1] * 256 + mooltipass.memmgmt.ctrValue[2];
	var imported_ctr = mooltipass.memmgmt.importedCtrValue[0] * 256 * 256 + mooltipass.memmgmt.importedCtrValue[1] * 256 + mooltipass.memmgmt.importedCtrValue[2];
	if(imported_ctr > cur_ctr)
	{
		mooltipass.memmgmt.consoleLog("CTR Value mismatch: " + mooltipass.memmgmt.ctrValue + " instead of: " + mooltipass.memmgmt.importedCtrValue);
		mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setCTR'], mooltipass.memmgmt.importedCtrValue));
	}
	else
	{
		mooltipass.memmgmt.consoleLog("CTR value OK");
	}
	
	// Find the nodes we don't have in memory or that have been changed
	for(var i = 0; i < mooltipass.memmgmt.importedCurServiceNodes.length; i++)
	{
		// Loop in the memory nodes to compare data
		var service_node_found = false;
		for(var j = 0; j < mooltipass.memmgmt.clonedCurServiceNodes.length; j++)
		{
			if(mooltipass.memmgmt.compareParentNodeCoreData(mooltipass.memmgmt.importedCurServiceNodes[i], mooltipass.memmgmt.clonedCurServiceNodes[j]))
			{
				// We found a parent node that has the same core data (doesn't mean the same prev / next node though!)
				//mooltipass.memmgmt.consoleLog("Parent node core data match: " + mooltipass.memmgmt.importedCurServiceNodes[i].name);
				mooltipass.memmgmt.clonedCurServiceNodes[j]["mergeTagged"] = true;
				service_node_found = true;
				
				// Next step is to check if the children are the same
				var cur_import_child_node_addr = mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.importedCurServiceNodes[i].data);
				var matched_parent_first_child = mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[j].data);
				while(cur_import_child_node_addr[0] != 0 || cur_import_child_node_addr[1] != 0)
				{
					// Loop through the imported login nodes to find the one that matches
					for(var k = 0; k < mooltipass.memmgmt.importedCurLoginNodes.length; k++)
					{
						if(mooltipass.memmgmt.isSameAddress(cur_import_child_node_addr, mooltipass.memmgmt.importedCurLoginNodes[k].address))
						{
							// We found the imported child, now we need to find the one that matches
							//mooltipass.memmgmt.consoleLog("Looking for child " + mooltipass.memmgmt.importedCurLoginNodes[k].name + " in the Mooltipass");
							
							// Try to find the match between the child nodes of the matched parent
							var matched_login_found = false;
							var matched_parent_next_child = matched_parent_first_child.slice(0);
							while((matched_parent_next_child[0] != 0 || matched_parent_next_child[1] != 0) && (matched_login_found == false))
							{
								// Loop through our child nodes to find the child
								for(var l = 0; l < mooltipass.memmgmt.clonedCurLoginNodes.length; l++)
								{
									if(mooltipass.memmgmt.isSameAddress(matched_parent_next_child, mooltipass.memmgmt.clonedCurLoginNodes[l].address))
									{
										// We found the child, now we can compare the login name
										if(mooltipass.memmgmt.clonedCurLoginNodes[l].name == mooltipass.memmgmt.importedCurLoginNodes[k].name)
										{
											// We have a match between imported login node & current login node
											//mooltipass.memmgmt.consoleLog("Child found in the mooltipass, comparing rest of the data");
											mooltipass.memmgmt.clonedCurLoginNodes[l]["mergeTagged"] = true;
											matched_login_found = true;
											
											if(mooltipass.memmgmt.compareChildNodeCoreData(mooltipass.memmgmt.clonedCurLoginNodes[l], mooltipass.memmgmt.importedCurLoginNodes[k]))
											{
												//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.importedCurServiceNodes[i].name + " : child core data match for child " + mooltipass.memmgmt.importedCurLoginNodes[k].name + " , nothing to do");
											}
											else
											{
												// Data mismatch, overwrite the important part
												mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.importedCurServiceNodes[i].name + " : child core data mismatch for child " + mooltipass.memmgmt.importedCurLoginNodes[k].name + " , updating...");
												mooltipass.memmgmt.clonedCurLoginNodes[l].data.set(mooltipass.memmgmt.importedCurLoginNodes[k].data.subarray(6, mooltipass.memmgmt.importedCurLoginNodes[k].data.length), 6);
											}
										}
										else
										{
											// Not a match, load next child address in var
											matched_parent_next_child = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[l].data);
											break;
										}
									}
								}
							}	

							// If we couldn't find the child node, we have to add it
							if(matched_login_found == false)
							{
								// Add it in our array to be treated later...
								mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.importedCurServiceNodes[i].name + " : adding new child " + mooltipass.memmgmt.importedCurLoginNodes[k].name + " in the mooltipass...");
								new_children_for_existing_parents.push({'parrentAddress': mooltipass.memmgmt.clonedCurServiceNodes[j].address, 'name': mooltipass.memmgmt.importedCurLoginNodes[k].name, 'data': mooltipass.memmgmt.importedCurLoginNodes[k].data});
							}
							
							// Process the next imported child node
							cur_import_child_node_addr = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.importedCurLoginNodes[k].data);
							break;
						}
					}					
				}
				// Jump to next service node
				break;
			}
		}
		// Did we find the service core data?
		if(service_node_found == false)
		{
			// New service, add it to our nodes with a 0 address and a virtual address counter that will be set later
			var temp_new_service_address = virtual_address_counter++;
			mooltipass.memmgmt.consoleLog("Adding new parent node: " + mooltipass.memmgmt.importedCurServiceNodes[i].name);
			var cur_import_child_node_addr = mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.importedCurServiceNodes[i].data);
			
			// We need to find where to fit that parent node
			var next_parent_node_index = null;
			var prev_parent_node_index = null;
			var temp_parent_node_address = mooltipass.memmgmt.clonedStartingParent;
			
			// If we don't have any credential
			if(mooltipass.memmgmt.isSameAddress(temp_parent_node_address, [0,0]))
			{
				// Don't enter next while loop
				temp_parent_node_address = null;
			}
			
			while(temp_parent_node_address != null)
			{
				// Find the child node in our buffer
				for(var k = 0; k < mooltipass.memmgmt.clonedCurServiceNodes.length; k++)
				{
					// If statement depending if we're dealing with a real address or a temporary one
					if((temp_parent_node_address.length == null && mooltipass.memmgmt.clonedCurServiceNodes[k].tempAddress == temp_parent_node_address) || (temp_parent_node_address.length == 2 && mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.clonedCurServiceNodes[k].address, temp_parent_node_address)))
					{
						// We found our node, update indexes if we need to
						//mooltipass.memmgmt.consoleLog("Browsing: " + mooltipass.memmgmt.clonedCurServiceNodes[k].name);
						if(mooltipass.memmgmt.importedCurServiceNodes[i].name < mooltipass.memmgmt.clonedCurServiceNodes[k].name)
						{
							//mooltipass.memmgmt.consoleLog("Next child set: " + mooltipass.memmgmt.clonedCurServiceNodes[k].name);		
							temp_parent_node_address = null;
							next_parent_node_index = k;
							break;
						}
						if(mooltipass.memmgmt.importedCurServiceNodes[i].name > mooltipass.memmgmt.clonedCurServiceNodes[k].name)
						{
							//mooltipass.memmgmt.consoleLog("Prev child set: " + mooltipass.memmgmt.clonedCurServiceNodes[k].name);	
							prev_parent_node_index = k;
						}
						
						// Depending on next address type
						if(mooltipass.memmgmt.clonedCurServiceNodes[k].tempNextAddress == null && mooltipass.memmgmt.isSameAddress([0,0], mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[k].data)))
						{
							// We finished browsing
							temp_parent_node_address = null;
						}
						else if(mooltipass.memmgmt.clonedCurServiceNodes[k].tempNextAddress != null)
						{
							// Next address is temporary one
							temp_parent_node_address = mooltipass.memmgmt.clonedCurServiceNodes[k].tempNextAddress;
						}
						else
						{
							// Next address is real one
							temp_parent_node_address = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[k].data);
						}
						break;
					}
				}
			}

			// Update our node and the previous node with the correct address
			var temp_prev_address = null;
			if(prev_parent_node_index == null)
			{
				// First parent node
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.importedCurServiceNodes[i].name + " will fit in front of the other parents");
				mooltipass.memmgmt.clonedStartingParent = temp_new_service_address;
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.importedCurServiceNodes[i].data, [0,0]);
			}
			else
			{
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.importedCurServiceNodes[i].name + " will fit after " + mooltipass.memmgmt.clonedCurServiceNodes[prev_parent_node_index].name);
				mooltipass.memmgmt.clonedCurServiceNodes[prev_parent_node_index]["tempNextAddress"] = temp_new_service_address;
				// Depending on the type of the previous node
				if(mooltipass.memmgmt.clonedCurServiceNodes[prev_parent_node_index].tempAddress != null)
				{
					// Temporary type
					temp_prev_address = mooltipass.memmgmt.clonedCurServiceNodes[prev_parent_node_index].tempAddress;
				}
				else
				{
					// Real type
					mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.importedCurServiceNodes[i].data, mooltipass.memmgmt.clonedCurServiceNodes[prev_parent_node_index].address)
				}
			}
			
			// Update our node and the next node with the correct address
			var temp_next_address = null;
			if(next_parent_node_index == null)
			{
				// Last node in memory
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.importedCurServiceNodes[i].data, [0,0]);
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.importedCurServiceNodes[i].name + " will fit after all the other children");
			}
			else
			{
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.importedCurServiceNodes[i].name + " will fit before " + mooltipass.memmgmt.clonedCurServiceNodes[next_parent_node_index].name);
				mooltipass.memmgmt.clonedCurServiceNodes[next_parent_node_index]["tempPrevAddress"] = temp_new_service_address;
				// Depending on the type of the next node
				if(mooltipass.memmgmt.clonedCurServiceNodes[next_parent_node_index].tempAddress != null)
				{
					// Temporary type
					temp_next_address = mooltipass.memmgmt.clonedCurServiceNodes[next_parent_node_index].tempAddress;
				}
				else
				{
					// Real type
					mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.importedCurServiceNodes[i].data, mooltipass.memmgmt.clonedCurServiceNodes[next_parent_node_index].address)
				}
			}
			
			// Depending if it has a child or not, set tempChildAddress to 0			
			if(cur_import_child_node_addr[0] == 0 && cur_import_child_node_addr[1] == 0)
			{
				mooltipass.memmgmt.clonedCurServiceNodes.push({'address': [0, 0], 'name': mooltipass.memmgmt.importedCurServiceNodes[i].name, 'data': mooltipass.memmgmt.importedCurServiceNodes[i].data, 'tempAddress': temp_new_service_address, 'tempChildAddress': null, 'tempPrevAddress': temp_prev_address, 'tempNextAddress': temp_next_address, 'mergeTagged': true});
			}
			else
			{
				mooltipass.memmgmt.clonedCurServiceNodes.push({'address': [0, 0], 'name': mooltipass.memmgmt.importedCurServiceNodes[i].name, 'data': mooltipass.memmgmt.importedCurServiceNodes[i].data, 'tempAddress': temp_new_service_address, 'tempChildAddress': virtual_address_counter, 'tempPrevAddress': temp_prev_address, 'tempNextAddress': temp_next_address, 'mergeTagged': true});
			}			
						
			// Next step is to follow the children
			var number_children_added = 0;
			while(cur_import_child_node_addr[0] != 0 || cur_import_child_node_addr[1] != 0)
			{
				// Loop through the imported login nodes to find the one that matches
				for(var k = 0; k < mooltipass.memmgmt.importedCurLoginNodes.length; k++)
				{
					if(mooltipass.memmgmt.isSameAddress(cur_import_child_node_addr, mooltipass.memmgmt.importedCurLoginNodes[k].address))
					{
						var temp_new_login_address = virtual_address_counter++;
						cur_import_child_node_addr = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.importedCurLoginNodes[k].data);
						mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.importedCurServiceNodes[i].name + " : adding child node " + mooltipass.memmgmt.importedCurLoginNodes[k].name)
						
						// Set correct prev address
						var temp_prev_address = null;
						if(number_children_added == 0)
						{
							// No previous children
							mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.importedCurLoginNodes[k].data, [0,0]);
						}
						else
						{
							temp_prev_address = virtual_address_counter - 2;
						}
						
						// Set correct next address
						var temp_next_address = null;
						if(cur_import_child_node_addr[0] == 0 && cur_import_child_node_addr[1] == 0)
						{
							// No next children
							mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.importedCurLoginNodes[k].data, [0,0]);
						}
						else
						{
							temp_next_address = virtual_address_counter;							
						}
						
						// New login node, add it to our nodes with a 0 address and a virtual address counter that will be set later
						mooltipass.memmgmt.clonedCurLoginNodes.push({'address': [0, 0], 'name': mooltipass.memmgmt.importedCurLoginNodes[k].name, 'data': mooltipass.memmgmt.importedCurLoginNodes[k].data, 'tempAddress': temp_new_login_address, 'tempPrevAddress': temp_prev_address, 'tempNextAddress': temp_next_address, 'mergeTagged': true});
												
						// Process the next imported child node
						break;
					}
				}
				number_children_added++;
			}
		}
	}
	
	// If we have to add children nodes to existing parent nodes. First sort by name then add later
	new_children_for_existing_parents.sort(function(a,b){if(a.name < b.name) return -1; if(a.name > b.name) return 1; return 0;});
	// Loop through our parent nodes and see if we have children to add
	for(var i = 0; i < mooltipass.memmgmt.clonedCurServiceNodes.length; i++)
	{
		// Loop through the possible children nodes we might have to add
		for(var j = 0; j < new_children_for_existing_parents.length; j++)
		{
			if(mooltipass.memmgmt.isSameAddress(new_children_for_existing_parents[j].parrentAddress, mooltipass.memmgmt.clonedCurServiceNodes[i].address))
			{
				// We have a match, let's try to see where to fit that node
				var prev_child_node_index = null;
				var next_child_node_index = null;
				var temp_child_node_address = mooltipass.memmgmt.clonedCurServiceNodes[i].tempChildAddress;

				// First child might be a virtual one
				if(temp_child_node_address == null)
				{
					temp_child_node_address = mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[i].data);
					if(mooltipass.memmgmt.isSameAddress([0,0], temp_child_node_address))
					{
						// No children, don't enter the while loop
						temp_child_node_address = null;
					}
				}				
				
				// Browse through all the child nodes to find the previous and next child node
				while(temp_child_node_address != null)
				{
					// Find the child node in our buffer
					for(var k = 0; k < mooltipass.memmgmt.clonedCurLoginNodes.length; k++)
					{
						// If statement depending if we're dealing with a real address or a temporary one
						if((temp_child_node_address.length == null && mooltipass.memmgmt.clonedCurLoginNodes[k].tempAddress == temp_child_node_address) || (temp_child_node_address.length == 2 && mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.clonedCurLoginNodes[k].address, temp_child_node_address)))
						{
							// We found our node, update indexes if we need to
							//mooltipass.memmgmt.consoleLog("Browsing: " + mooltipass.memmgmt.clonedCurLoginNodes[k].name);
							if(new_children_for_existing_parents[j].name < mooltipass.memmgmt.clonedCurLoginNodes[k].name)
							{
								//mooltipass.memmgmt.consoleLog("Next child set: " + mooltipass.memmgmt.clonedCurLoginNodes[k].name);		
								temp_child_node_address = null;
								next_child_node_index = k;
								break;
							}
							if(new_children_for_existing_parents[j].name > mooltipass.memmgmt.clonedCurLoginNodes[k].name)
							{
								//mooltipass.memmgmt.consoleLog("Prev child set: " + mooltipass.memmgmt.clonedCurLoginNodes[k].name);	
								prev_child_node_index = k;
							}
							
							// Depending on next address type
							if(mooltipass.memmgmt.clonedCurLoginNodes[k].tempNextAddress == null && mooltipass.memmgmt.isSameAddress([0,0], mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[k].data)))
							{
								// We finished browsing
								temp_child_node_address = null;
							}
							else if(mooltipass.memmgmt.clonedCurLoginNodes[k].tempNextAddress != null)
							{
								// Next address is temporary one
								temp_child_node_address = mooltipass.memmgmt.clonedCurLoginNodes[k].tempNextAddress;
							}
							else
							{
								// Next address is real one
								temp_child_node_address = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[k].data);
							}
							break;
						}
					}					
				}
				
				// We now have the indexes for the previous and next nodes
				var temp_new_login_address = virtual_address_counter++;
				
				// Update our node and the previous node with the correct address
				var temp_prev_address = null;
				if(prev_child_node_index == null)
				{
					// First child node, we need to update the parent and we set our prev address to 0
					//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : " + new_children_for_existing_parents[j].name + " will fit in front of the other children");
					mooltipass.memmgmt.clonedCurServiceNodes[i]["tempChildAddress"] = temp_new_login_address;
					mooltipass.memmgmt.changePrevAddress(new_children_for_existing_parents[j].data, [0,0]);
				}
				else
				{
					//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : " + new_children_for_existing_parents[j].name + " will fit after " + mooltipass.memmgmt.clonedCurLoginNodes[prev_child_node_index].name);
					mooltipass.memmgmt.clonedCurLoginNodes[prev_child_node_index]["tempNextAddress"] = temp_new_login_address;
					// Depending on the type of the previous node
					if(mooltipass.memmgmt.clonedCurLoginNodes[prev_child_node_index].tempAddress != null)
					{
						// Temporary type
						temp_prev_address = mooltipass.memmgmt.clonedCurLoginNodes[prev_child_node_index].tempAddress;
					}
					else
					{
						// Real type
						mooltipass.memmgmt.changePrevAddress(new_children_for_existing_parents[j].data, mooltipass.memmgmt.clonedCurLoginNodes[prev_child_node_index].address)
					}
				}
				
				// Update our node and the next node with the correct address
				var temp_next_address = null;
				if(next_child_node_index == null)
				{
					// Last node in memory
					mooltipass.memmgmt.changeNextAddress(new_children_for_existing_parents[j].data, [0,0]);
					//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : " + new_children_for_existing_parents[j].name + " will fit after all the other children");
				}
				else
				{
					//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : " + new_children_for_existing_parents[j].name + " will fit before " + mooltipass.memmgmt.clonedCurLoginNodes[next_child_node_index].name);
					mooltipass.memmgmt.clonedCurLoginNodes[next_child_node_index]["tempPrevAddress"] = temp_new_login_address;
					// Depending on the type of the next node
					if(mooltipass.memmgmt.clonedCurLoginNodes[next_child_node_index].tempAddress != null)
					{
						// Temporary type
						temp_next_address = mooltipass.memmgmt.clonedCurLoginNodes[next_child_node_index].tempAddress;
					}
					else
					{
						// Real type
						mooltipass.memmgmt.changeNextAddress(new_children_for_existing_parents[j].data, mooltipass.memmgmt.clonedCurLoginNodes[next_child_node_index].address)
					}
				}
				
				// Push our new node into the memory!
				mooltipass.memmgmt.clonedCurLoginNodes.push({'address': [0, 0], 'name': new_children_for_existing_parents[j].name, 'data': new_children_for_existing_parents[j].data, 'tempAddress': temp_new_login_address, 'tempPrevAddress': temp_prev_address, 'tempNextAddress': temp_next_address, 'mergeTagged': true});
			}
		}
	}
	
	// See how many available addresses we need
	mooltipass.memmgmt.consoleLog("Total number of addresses required: " + virtual_address_counter);
	return virtual_address_counter;
}
 
// Generate merge packets
mooltipass.memmgmt.generateMergePackets = function()
{
	mooltipass.memmgmt.consoleLog("");
	mooltipass.memmgmt.consoleLog("Generating merge packets");
	
	// If we are importing csv credentials we just need to call the other routine from the normal memory management mode ;)
	if(mooltipass.memmgmt.mergeFileTypeCsv)
	{
		// Generate save packets
		if(mooltipass.memmgmt.generateSavePackets() == false)
		{
			// Check that we didn't break the linked list
			applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'msg': "Errors detected when merging data, please contact support at support@themooltipass.com"});
			return false;
		}
	}
	else
	{	
		// Generate an array with all our free addresses
		var free_addresses_vector = [];
		for(var i = 0; i < mooltipass.memmgmt.freeAddressesBuffer.length; i++)
		{
			for(var j = 0; j < mooltipass.memmgmt.freeAddressesBuffer[i].length; j++)
			{
				free_addresses_vector.push([mooltipass.memmgmt.freeAddressesBuffer[i][j], mooltipass.memmgmt.freeAddressesBuffer[i][++j]]);
			}		
		}
		
		// Check if the first parent node has changed
		if(mooltipass.memmgmt.clonedStartingParent.length != 2)
		{
			mooltipass.memmgmt.clonedStartingParent = free_addresses_vector[mooltipass.memmgmt.clonedStartingParent];
			mooltipass.memmgmt.consoleLog("Changing starting parent to: " + mooltipass.memmgmt.clonedStartingParent);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingParentAddress'], mooltipass.memmgmt.clonedStartingParent)); 
		}
		
		// Here we are going to make a direct temp address to free_addresses_vector[temp_address] translation
		for(var i = 0; i < mooltipass.memmgmt.clonedCurServiceNodes.length; i++)
		{
			if(mooltipass.memmgmt.clonedCurServiceNodes[i].tempAddress != null)
			{
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempAddress]);
				mooltipass.memmgmt.clonedCurServiceNodes[i].address = free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempAddress];
			}
			if(mooltipass.memmgmt.clonedCurServiceNodes[i].tempChildAddress != null)
			{
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : child address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempChildAddress]);		
				mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[i].data, free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempChildAddress]);
			}
			if(mooltipass.memmgmt.clonedCurServiceNodes[i].tempPrevAddress != null)
			{
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : prev address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempPrevAddress]);
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[i].data, free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempPrevAddress]);
			}
			if(mooltipass.memmgmt.clonedCurServiceNodes[i].tempNextAddress != null)
			{
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : next address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempNextAddress]);	
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[i].data, free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempNextAddress]);		
			}
		}
		for(var i = 0; i < mooltipass.memmgmt.clonedCurLoginNodes.length; i++)
		{
			if(mooltipass.memmgmt.clonedCurLoginNodes[i].tempAddress != null)
			{
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurLoginNodes[i].name + " : address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempAddress]);
				mooltipass.memmgmt.clonedCurLoginNodes[i].address = free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempAddress];
			}
			if(mooltipass.memmgmt.clonedCurLoginNodes[i].tempPrevAddress != null)
			{
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurLoginNodes[i].name + " : prev address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempPrevAddress]);
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[i].data, free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempPrevAddress]);
			}
			if(mooltipass.memmgmt.clonedCurLoginNodes[i].tempNextAddress != null)
			{
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurLoginNodes[i].name + " : next address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempNextAddress]);	
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[i].data, free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempNextAddress]);		
			}
		}
		
		// Next step is to check if nodes were removed and update the addresses
		for(var i = 0; i < mooltipass.memmgmt.clonedCurServiceNodes.length; i++)
		{
			// Check if it was tagged by our merge procedure
			if(mooltipass.memmgmt.clonedCurServiceNodes[i].mergeTagged == null)
			{
				mooltipass.memmgmt.deleteServiceNodeFromCloneArrayAndGenerateDeletePacket(mooltipass.memmgmt.clonedCurServiceNodes[i]);
			}
		}
		// Next step is to check if nodes were removed and update the addresses
		for(var i = 0; i < mooltipass.memmgmt.clonedCurLoginNodes.length; i++)
		{
			// Check if it was tagged by our merge procedure
			if(mooltipass.memmgmt.clonedCurLoginNodes[i].mergeTagged == null)
			{
				mooltipass.memmgmt.deleteChildNodeFromCloneArrayAndGenerateDeletePacket(mooltipass.memmgmt.clonedCurLoginNodes[i]);
			}
		}	
		
		// Now we can do the difference between our local buffer and what is on the mooltipass
		for(var i = 0; i < mooltipass.memmgmt.clonedCurServiceNodes.length; i++)
		{
			// Check if it was tagged by our merge procedure
			if(mooltipass.memmgmt.clonedCurServiceNodes[i].mergeTagged != null)
			{
				var parent_node_address = mooltipass.memmgmt.clonedCurServiceNodes[i].address;
				var parent_node_found = false;
				
				// Try to find the same address in our memory
				for(var j = 0; j < mooltipass.memmgmt.curServiceNodes.length; j++)
				{
					if(mooltipass.memmgmt.isSameAddress(parent_node_address, mooltipass.memmgmt.curServiceNodes[j].address))
					{
						// We have a match!
						parent_node_found = true;
						
						// Check if data is unchanged
						if(!mooltipass.memmgmt.compareNodeData(mooltipass.memmgmt.clonedCurServiceNodes[i], mooltipass.memmgmt.curServiceNodes[j]))
						{
							mooltipass.memmgmt.consoleLog("Node data different for parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
							mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurServiceNodes[i].address, mooltipass.memmgmt.clonedCurServiceNodes[i].data);
						}
						else
						{
							//mooltipass.memmgmt.consoleLog("Node data identical for parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
						}
						break;
					}
				}
				
				// Check if we found the node
				if(parent_node_found == false)
				{
					mooltipass.memmgmt.consoleLog("Adding new parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
					mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurServiceNodes[i].address, mooltipass.memmgmt.clonedCurServiceNodes[i].data);
				}
			}
		}
		for(var i = 0; i < mooltipass.memmgmt.clonedCurLoginNodes.length; i++)
		{
			// Check if it was tagged by our merge procedure
			if(mooltipass.memmgmt.clonedCurLoginNodes[i].mergeTagged != null)
			{
				var child_node_address = mooltipass.memmgmt.clonedCurLoginNodes[i].address;
				var child_node_found = false;
				
				// Try to find the same address in our memory
				for(var j = 0; j < mooltipass.memmgmt.curLoginNodes.length; j++)
				{
					if(mooltipass.memmgmt.isSameAddress(child_node_address, mooltipass.memmgmt.curLoginNodes[j].address))
					{
						// We have a match!
						child_node_found = true;
						
						// Check if data is unchanged
						if(!mooltipass.memmgmt.compareNodeData(mooltipass.memmgmt.clonedCurLoginNodes[i], mooltipass.memmgmt.curLoginNodes[j]))
						{
							mooltipass.memmgmt.consoleLog("Node data different for child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
							mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurLoginNodes[i].address, mooltipass.memmgmt.clonedCurLoginNodes[i].data);
						}
						else
						{
							//mooltipass.memmgmt.consoleLog("Node data identical for child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
						}
						break;
					}
				}
				
				// Check if we found the node
				if(child_node_found == false)
				{
					mooltipass.memmgmt.consoleLog("Adding new child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
					mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurLoginNodes[i].address, mooltipass.memmgmt.clonedCurLoginNodes[i].data);
				}
			}
		}
		
		// Favorite syncing
		for(var i = 0; i < mooltipass.memmgmt.importedFavoriteAddresses.length; i++)
		{
			// Extract addresses
			var cur_favorite_address_parent = mooltipass.memmgmt.importedFavoriteAddresses[i].subarray(0, 0 + 2);
			var cur_favorite_address_child = mooltipass.memmgmt.importedFavoriteAddresses[i].subarray(2, 2 + 2);
			 
			// Only compare if both addresses are different than 0
			if(mooltipass.memmgmt.isSameFavoriteAddress(mooltipass.memmgmt.importedFavoriteAddresses[i], [0,0,0,0]))
			{
				//mooltipass.memmgmt.consoleLog("Favorite " + i + " is empty");
				// Check that the same favorite is also empty on our DB
				if(!mooltipass.memmgmt.isSameFavoriteAddress(mooltipass.memmgmt.favoriteAddresses[i], [0,0,0,0]))
				{
					mooltipass.memmgmt.consoleLog("However favorite " + i + " isn't empty on our MP... deleting it");
					mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], [i, 0, 0, 0, 0]));
				}
			}
			else
			{
				var imported_parent_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.importedCurServiceNodes, cur_favorite_address_parent);	   
				var imported_child_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.importedCurLoginNodes, cur_favorite_address_child);	   
				 
				// Check if both addresses are correct
				if(imported_parent_node_index != null && imported_child_node_index != null)
				{
					//mooltipass.memmgmt.consoleLog("Favorite " + i + " : " + mooltipass.memmgmt.importedCurLoginNodes[imported_child_node_index].name + " on " + mooltipass.memmgmt.importedCurServiceNodes[imported_parent_node_index].name);
					
					// Try to find the same parent in our DB
					var cur_parent_node_index = mooltipass.memmgmt.findIdByName(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.importedCurServiceNodes[imported_parent_node_index].name);
					
					if(cur_parent_node_index == null)
					{
						mooltipass.memmgmt.consoleLog("Favorite " + i + " : couldn't find parent node " + mooltipass.memmgmt.importedCurServiceNodes[imported_parent_node_index].name);
						mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], [i, 0, 0, 0, 0]));
					}
					else
					{
						// Next step is to find the address of the matching login node
						var child_node_address = mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].data);
						var child_node_found = false;
						while(!mooltipass.memmgmt.isSameAddress(child_node_address, [0,0]))
						{
							var child_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, child_node_address);
							
							// Check if we found the node
							if(child_node_index != null)
							{
								// Check if it has the same login
								if(mooltipass.memmgmt.importedCurLoginNodes[imported_child_node_index].name == mooltipass.memmgmt.clonedCurLoginNodes[child_node_index].name)
								{
									// Epic win!
									var actual_favorite_address = [mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].address[0], mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].address[1], mooltipass.memmgmt.clonedCurLoginNodes[child_node_index].address[0], mooltipass.memmgmt.clonedCurLoginNodes[child_node_index].address[1]];
									
									// Compare favorite addresses
									if(!mooltipass.memmgmt.isSameFavoriteAddress(actual_favorite_address, mooltipass.memmgmt.favoriteAddresses[i]))
									{
										mooltipass.memmgmt.consoleLog("Different address for favorite " + i + " : " + mooltipass.memmgmt.importedCurLoginNodes[imported_child_node_index].name + " on " + mooltipass.memmgmt.importedCurServiceNodes[imported_parent_node_index].name + " : " + mooltipass.memmgmt.favoriteAddresses[i] + " instead of " + actual_favorite_address);
										var favorite_packet = new Uint8Array(5);
										favorite_packet.set([i], 0);
										favorite_packet.set(actual_favorite_address, 1);
										mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], favorite_packet));
									}
									child_node_found = true;
									break;
								}
								else
								{
									child_node_address = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[child_node_index].data);
								}							
							}
							else
							{
								mooltipass.memmgmt.consoleLog("Favorite " + i + " : couldn't find child node at address " + child_node_address);
								mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], [i, 0, 0, 0, 0]));
								break;
							}
						}
						
						if(child_node_found == false)
						{
							// Couldn't find child node
							mooltipass.memmgmt.consoleLog("Favorite " + i + " : couldn't find child node " + mooltipass.memmgmt.importedCurLoginNodes[imported_child_node_index].name + " on " + mooltipass.memmgmt.importedCurServiceNodes[imported_parent_node_index].name);
							mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], [i, 0, 0, 0, 0]));
						}
					}
				}
				else
				{
					// Reset favorite
					mooltipass.memmgmt.consoleLog("Favorite " + i + " is incorrect");
					mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], [i, 0, 0, 0, 0]));
				}
			}
		}
	}
	
	// Store original packettosend buffer length for progress callback purposes
	mooltipass.memmgmt.origPacketToSendBufferLength = mooltipass.memmgmt.packetToSendBuffer.length;
	mooltipass.memmgmt.packetToSendCompletionPercentage = 0;

	// Check that we didn't break our linked chain
	return mooltipass.memmgmt.checkClonedParentList();
} 

// Check the cloned parent list we're about to flush to the device
mooltipass.memmgmt.checkClonedParentList = function()
{
	var prev_parent_addr = [0, 0];
	var next_parent_addr = mooltipass.memmgmt.clonedStartingParent;
	
	while(!mooltipass.memmgmt.isSameAddress(next_parent_addr, [0,0]))
	{
		// Find parent in our list
		var parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, next_parent_addr);
		
		// Check if we could find the node
		if(parent_index == null)
		{
			console.log("Couldn't find next parent node at address " + next_parent_addr);			
			return false;
		}
		else
		{
			mooltipass.memmgmt.consoleLog("Loading " + mooltipass.memmgmt.clonedCurServiceNodes[parent_index].name + "...");
		}
		
		// Check previous parent address
		if(!mooltipass.memmgmt.isSameAddress(prev_parent_addr, mooltipass.memmgmt.getPrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data)))
		{
			console.log("Invalid previous parent node address: " + mooltipass.memmgmt.getPrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data) + ", should be " + prev_parent_addr);
			return false;
		}
		
		prev_parent_addr = next_parent_addr;
		next_parent_addr = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data);
	}
	
	mooltipass.memmgmt.consoleLog("Linked list checked!");
	return true;
}

mooltipass.memmgmt.requestFailHander = function(message, nextMode, code)
{
	mooltipass.memmgmt.consoleLog("requestFailHander: " + message);	
	mooltipass.memmgmt.consoleLog("Current mode: " + mooltipass.memmgmt.currentMode);
			
	if(nextMode == MGMT_IDLE)
	{
		// Leave MMM, and the call callback
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
		mooltipass.memmgmt.currentMode = MGMT_ERROR_CUR_EXITTING_MMM;
		mooltipass.memmgmt.tempCallbackErrorString = message;
		mooltipass.memmgmt.tempCallbackErrorCode = code;
		mooltipass.memmgmt_hid._sendMsg();
	}
	else
	{
		applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': code, 'msg': message});
		mooltipass.memmgmt.currentMode = MGMT_IDLE;
		mooltipass.device.processQueue();
	}
}

// Data send timeout
mooltipass.memmgmt.dataSendTimeOutCallback = function()
{
	mooltipass.memmgmt.consoleLog("Data send timeout");
	mooltipass.memmgmt.currentMode = MGMT_IDLE;
	
	// Mooltipass mini doesn't reply the import media end packet
	if(mooltipass.memmgmt.mediaImportEndPacketSent == true)
	{
		applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'code': 701, 'updating' : true, 'msg': "Firmware Uploaded, DO NOT UNPLUG YOUR MINI!!!!!"});
	}
	else
	{
		applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': 696, 'msg': "Problem with USB comms"});		
	}
	mooltipass.memmgmt.mediaImportEndPacketSent = false;
	mooltipass.device.processQueue();
}
 
// Data received from USB callback
mooltipass.memmgmt.dataReceivedCallback = function(packet)
{
	// Receive statistics
	mooltipass.memmgmt.statsTotalBytesReceived += 64;
	var nb_ms_since_last_stat_output = new Date().getTime() - mooltipass.memmgmt.statsLastDataReceivedTime;
	if(nb_ms_since_last_stat_output > 3000)
	{
		// Don't print old data
		mooltipass.memmgmt.statsLastDataReceivedTime = new Date().getTime();
		mooltipass.memmgmt.statsTotalBytesReceived = 0;		
	}
	else if(nb_ms_since_last_stat_output > 1000)
	{
		console.log("Receive speed: " + Math.round(mooltipass.memmgmt.statsTotalBytesReceived*1000/nb_ms_since_last_stat_output) + " bytes per second");
		mooltipass.memmgmt.statsLastDataReceivedTime = new Date().getTime();
		mooltipass.memmgmt.statsTotalBytesReceived = 0;
	}
	
	// If it is a leave memory management mode packet, process the queue and exit
	if(packet[1] == mooltipass.device.commands['endMemoryManagementMode'])
	{		 
		// Did we succeed?
		if(packet[2] == 1)
		{
			//mooltipass.memmgmt.consoleLog("Memory management mode exit");
			//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.currentMode);
			if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE_NO_CHANGES || mooltipass.memmgmt.currentMode == MGMT_USER_CHANGES_PACKET_SENDING)
			{
				// Do we have passwords to change?
				if(mooltipass.memmgmt.changePasswordReqs.length > 0)
				{
					mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE_PASSWD_CHANGE;	
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setContext'], mooltipass.util.strToArray(mooltipass.memmgmt.changePasswordReqs[0].service));
					mooltipass.memmgmt_hid._sendMsg();	
					return;
				}
				else
				{
					if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE_NO_CHANGES)
					{
						applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "No Changes Were Applied"});
					}
					else if(mooltipass.memmgmt.currentMode == MGMT_USER_CHANGES_PACKET_SENDING)
					{
						applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Changes Were Applied"});				
					}	
					mooltipass.memmgmt.currentMode = MGMT_IDLE;
					mooltipass.device.processQueue();
				}
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_ERROR_CUR_EXITTING_MMM)
			{
				applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': mooltipass.memmgmt.tempCallbackErrorCode, 'msg': mooltipass.memmgmt.tempCallbackErrorString});	
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
				mooltipass.device.processQueue();
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE)
			{
				applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Credentials Management Mode Left"});	
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
				mooltipass.device.processQueue();
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_INT_CHECK_SCAN)
			{
				applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Memory OK, No Changes To Make!"});	
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
				mooltipass.device.processQueue();
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_INT_CHECK_PACKET_SENDING)
			{
				applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Memory OK, Found Problems Fixed"});				
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
				mooltipass.device.processQueue();	
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_MEM_BACKUP_NORMAL_SCAN)
			{
				// Callback is called from the file written callback
				//applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Backup process done"});
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_NORMAL_SCAN || mooltipass.memmgmt.currentMode == MGMT_DB_FILE_MERGE_GET_FREE_ADDR)
			{
				applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Merging done, no changes applied"});	
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
				mooltipass.device.processQueue();
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_DB_FILE_MERGE_PACKET_SENDING)
			{
				// If we are importing from a CSV file, we might need to change passwords...
				if(mooltipass.memmgmt.mergeFileTypeCsv)
				{					
					// Do we have passwords to change?
					if(mooltipass.memmgmt.changePasswordReqs.length > 0)
					{
						mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE_PASSWD_CHANGE;	
						mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setContext'], mooltipass.util.strToArray(mooltipass.memmgmt.changePasswordReqs[0].service));
						mooltipass.memmgmt_hid._sendMsg();	
						return;
					}
					else
					{
						applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Merging done, changes applied"});			
						mooltipass.memmgmt.currentMode = MGMT_IDLE;
						mooltipass.device.processQueue();	
					}
				}
				else
				{
					applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Merging done, changes applied"});			
					mooltipass.memmgmt.currentMode = MGMT_IDLE;
					mooltipass.device.processQueue();					
				}
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_FORCE_EXIT_MMM)
			{
				applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Force exit done, mooltipass was in MMM"});			
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
				mooltipass.device.processQueue();
			}
			return;
		}
		else
		{
			if(mooltipass.memmgmt.currentMode == MGMT_FORCE_EXIT_MMM)
			{
				applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Force exit done, mooltipass wasn't in MMM"});			
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
				mooltipass.device.processQueue();
			}
			else
			{
				mooltipass.memmgmt.requestFailHander("Memory management mode exit fail", null, 650);
			}
			return;
		}
	}
	else if(packet[1] == mooltipass.device.commands['debug'])
	{
		console.log("Debug: " + mooltipass.util.arrayToStr(packet.subarray(2, packet.length)));
	}
	else if(packet[1] == 0xC4)
	{
		mooltipass.memmgmt.consoleLog("Please retry packet !!!")
	}
	 
	if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE_PASSWD_CHANGE)
	{
		if(packet[1] == mooltipass.device.commands['setContext'])
		{
			if(packet[2] == 0)
			{
				// Fail
				console.log("Set context fail");
				
				// Remove change password request from our buffer
				mooltipass.memmgmt.changePasswordReqs.splice(0, 1);
				if(mooltipass.memmgmt.changePasswordReqs.length > 0)
				{
					// Send next packet
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setContext'], mooltipass.util.strToArray(mooltipass.memmgmt.changePasswordReqs[0].service));
					mooltipass.memmgmt_hid._sendMsg();	
				}
				else
				{
					applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Changes were applied"});
					mooltipass.memmgmt.currentMode = MGMT_IDLE;
					mooltipass.device.processQueue();
				}
			}
			else
			{
				// Set login
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setLogin'], mooltipass.util.strToArray(mooltipass.memmgmt.changePasswordReqs[0].login));			
				mooltipass.memmgmt_hid._sendMsg();
			}
		}
		else if(packet[1] == mooltipass.device.commands['setLogin'])
		{
			if(packet[2] == 0)
			{
				// Fail
				console.log("Set login fail");
				
				// Remove change password request from our buffer
				mooltipass.memmgmt.changePasswordReqs.splice(0, 1);
				if(mooltipass.memmgmt.changePasswordReqs.length > 0)
				{
					// Send next packet
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setContext'], mooltipass.util.strToArray(mooltipass.memmgmt.changePasswordReqs[0].service));
					mooltipass.memmgmt_hid._sendMsg();	
				}
				else
				{
					applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Changes were applied"});
					mooltipass.memmgmt.currentMode = MGMT_IDLE;
					mooltipass.device.processQueue();
				}
			}
			else
			{
				// Set login
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setPassword'], mooltipass.util.strToArray(mooltipass.memmgmt.changePasswordReqs[0].password));			
				mooltipass.memmgmt_hid._sendMsg();
			}
		}
		else if(packet[1] == mooltipass.device.commands['setPassword'])
		{
			if(packet[2] == 0)
			{
				// Fail
				console.log("Set password fail");
			}
				
			// Remove change password request from our buffer
			mooltipass.memmgmt.changePasswordReqs.splice(0, 1);
			if(mooltipass.memmgmt.changePasswordReqs.length > 0)
			{
				// Send next packet
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setContext'], mooltipass.util.strToArray(mooltipass.memmgmt.changePasswordReqs[0].service));
				mooltipass.memmgmt_hid._sendMsg();	
			}
			else
			{
				applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'msg': "Changes were applied"});
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
				mooltipass.device.processQueue();
			}
		}
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_PASSWORD_REQ)
	{
		if(packet[1] == mooltipass.device.commands['setContext'])
		{
			if(packet[2] == 0)
			{
				// Fail
				console.log("Set context fail");
				mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;
				applyCallback(mooltipass.memmgmt.getPasswordCallback, null, {'success': false, 'code': 673, 'msg': "Context invalid"}, "not valid");
			}
			else
			{
				// Set login
				// TO CHANGE ON MOOLTIPASS V2, to SET LOGIN DIRECTLY
				//mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setLogin'], mooltipass.util.strToArray(mooltipass.memmgmt.getPasswordLogin));
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getLogin'], mooltipass.util.strToArray(mooltipass.memmgmt.currentLoginForRequestedPassword));				
				mooltipass.memmgmt_hid._sendMsg();
			}
		}
		else if(packet[1] == mooltipass.device.commands['getLogin'])
		{
			if(packet[2] == 0 && mooltipass.memmgmt.getPasswordLogin != "")
			{
				// Fail
				console.log("Get login fail");
				mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;
				applyCallback(mooltipass.memmgmt.getPasswordCallback, null, {'success': false, 'code': 674, 'msg': "User didn't select a username"}, "not valid");
			}
			else
			{
				// Get password
				if(mooltipass.memmgmt.getPasswordLogin != mooltipass.util.arrayToStr(packet.subarray(2, 2 + packet[0])))
				{
					console.log("Wrong login selected");
					mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;
					applyCallback(mooltipass.memmgmt.getPasswordCallback, null, {'success': false, 'code': 675, 'msg': "User didn't select the correct username"}, "not valid");
				}
				else
				{
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getPassword'], null);
					mooltipass.memmgmt_hid._sendMsg();					
				}
			}
		}
		else if(packet[1] == mooltipass.device.commands['setLogin'])
		{
			if(packet[2] == 0)
			{
				// Fail
				console.log("Set login fail");
				mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;
				applyCallback(mooltipass.memmgmt.getPasswordCallback, null, {'success': false, 'code': 676, 'msg': "Login invalid"}, "not valid");
			}
			else
			{
				// Set login
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getPassword'], mooltipass.util.strToArray(mooltipass.memmgmt.getPasswordLogin));
				mooltipass.memmgmt_hid._sendMsg();
			}
		}
		else if(packet[1] == mooltipass.device.commands['getPassword'])
		{
			mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;
			/*if(packet[2] == 0)
			{
				// Fail
				console.log("Get password fail");
				applyCallback(mooltipass.memmgmt.getPasswordCallback, null, {'success': false, 'code': 677, 'msg': "Request denied"}, "not valid");
			}
			else
			{*/
				//applyCallback(mooltipass.memmgmt.getPasswordCallback, null, {'success': true, 'msg': "Request approved"}, mooltipass.util.arrayToStr(packet.subarray(2, 2 + packet[0])));
				mooltipass.memmgmt.getPasswordCallback({'success': true, 'msg': "Request approved"}, mooltipass.util.arrayToStr(packet.subarray(2, 2 + packet[0])));
			//}
		}
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_DB_FILE_MERGE_GET_FREE_ADDR || mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE_GET_FREE_ADDR)
	{
		if(packet[1] == mooltipass.device.commands['getFreeSlotAddresses'])
		{
			// Check for success status
			if(packet[0] != 1)
			{
				// Store free addresses
				if(mooltipass.memmgmt.totalAddressesReceived == 0)
				{
					mooltipass.memmgmt.lastFreeAddressReceived = packet.subarray(2 + packet[0] - 2, 2 + packet[0]);
					mooltipass.memmgmt.freeAddressesBuffer.push(packet.subarray(2, 2 + packet[0]));
					mooltipass.memmgmt.totalAddressesReceived += packet[0]/2;
				}
				else
				{
					// We use this trick because in the last packet we requested a free address after the last one we received
					mooltipass.memmgmt.lastFreeAddressReceived = packet.subarray(2 + packet[0] - 2, 2 + packet[0]);
					mooltipass.memmgmt.freeAddressesBuffer.push(packet.subarray(4, 2 + packet[0]));
					mooltipass.memmgmt.totalAddressesReceived += (packet[0]/2) - 1;				
				}			
				
				mooltipass.memmgmt.consoleLog("Received " + packet[0]/2 + " free addresses, current total: " + mooltipass.memmgmt.totalAddressesReceived);			
				// Check if we need to receive more address
				if(mooltipass.memmgmt.totalAddressesReceived >= mooltipass.memmgmt.totalAddressesRequired)
				{
					if(mooltipass.memmgmt.currentMode == MGMT_DB_FILE_MERGE_GET_FREE_ADDR)
					{
						// Generate merge packets, check return for errors
						if(mooltipass.memmgmt.generateMergePackets() == true)
						{			
							// Now check if actually have something to change
							if(mooltipass.memmgmt.packetToSendBuffer.length == 0)
							{
								mooltipass.memmgmt.consoleLog("Mooltipass already synced with our credential file!");
								// Leave mem management mode				
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
								mooltipass.memmgmt_hid._sendMsg();
							}
							else
							{
								mooltipass.memmgmt.consoleLog("Sending merging packets");
								mooltipass.memmgmt.currentMode = MGMT_DB_FILE_MERGE_PACKET_SENDING;
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
								mooltipass.memmgmt_hid._sendMsg();
							}
						}
						else
						{
							applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'msg': "Errors detected when merging data, please contact support at support@themooltipass.com"});
						}
					}	
					else if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE_GET_FREE_ADDR)
					{
						// Generate save packets
						if(mooltipass.memmgmt.generateSavePackets() == false)
						{
							// Check that we didn't break the linked list
							applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'msg': "Errors detected when merging data, please contact support at support@themooltipass.com"});
							return;
						}
				
						// Now check if actually have something to change
						if(mooltipass.memmgmt.packetToSendBuffer.length == 0)
						{
							mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE_NO_CHANGES;
							mooltipass.memmgmt.consoleLog("Save button pressed but no changes");
							// Leave mem management mode				
							mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
							mooltipass.memmgmt_hid._sendMsg();
						}
						else
						{
							mooltipass.memmgmt.consoleLog("Sending updating packets");
							mooltipass.memmgmt.currentMode = MGMT_USER_CHANGES_PACKET_SENDING;
							mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
							mooltipass.memmgmt_hid._sendMsg();
						}
					}
				}
				else
				{
					// Ask more addresses
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getFreeSlotAddresses'], mooltipass.memmgmt.lastFreeAddressReceived);
					mooltipass.memmgmt_hid._sendMsg();	
				}	
			}
			else
			{
				// Couldn't request free addresses!
				mooltipass.memmgmt.requestFailHander("Couldn't request free addresses, memory full", MGMT_IDLE, 651);
			}
		}
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_INT_CHECK_PACKET_SENDING || mooltipass.memmgmt.currentMode == MGMT_DB_FILE_MERGE_PACKET_SENDING || mooltipass.memmgmt.currentMode == MGMT_USER_CHANGES_PACKET_SENDING)
	{
		// Here we should receive acknowledgements from packets sending
		if(packet[2] == 1)
		{
			// Remove sent packet from our buffer
			mooltipass.memmgmt.packetToSendBuffer.splice(0, 1);
			 
			// Write acknowledged, check if we still have packets to send?
			if(mooltipass.memmgmt.packetToSendBuffer.length > 0)
			{
				// Send next packet
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.packetToSendBuffer[0]);
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
				mooltipass.memmgmt_hid._sendMsg();
			}
			else
			{
				// Leave mem management mode				
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
				mooltipass.memmgmt_hid._sendMsg();		
			}
		}
		else
		{
			mooltipass.memmgmt.requestFailHander("Couldn't send packet", MGMT_IDLE, 652);
			mooltipass.memmgmt.consoleLog(mooltipass.util.arrayToHexStr(new Uint16Array(mooltipass.memmgmt.packetToSendBuffer[0])));
		}
		
		// Progress bar update
		if (mooltipass.memmgmt.currentMode == MGMT_DB_FILE_MERGE_PACKET_SENDING)
		{
			var temp_completion = Math.round(((mooltipass.memmgmt.origPacketToSendBufferLength - mooltipass.memmgmt.packetToSendBuffer.length) / mooltipass.memmgmt.origPacketToSendBufferLength)*100);
			if (temp_completion != mooltipass.memmgmt.packetToSendCompletionPercentage)
			{
				mooltipass.memmgmt.packetToSendCompletionPercentage = temp_completion;
				mooltipass.memmgmt.progressCallback({'progress': 50+(temp_completion/2), 'letter': 'z'});
			}
		}
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK_REQ || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_REQ || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE_REQ || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_MEM_BACKUP_REQ)
	{
		if(packet[1] == mooltipass.device.commands['getMooltipassParameter'])
		{
			// We received the user interaction timeout, use it for our packets timeout
			mooltipass.memmgmt.consoleLog("Mooltipass interaction timeout is " + packet[2] + " seconds");
			
			// Get Mooltipass status and to go into MMM if in good mode
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassStatus'], null);
			mooltipass.memmgmt_hid.request.milliseconds = (packet[2]) * 4000;
			mooltipass.memmgmt_hid.nbSendRetries = 3;
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getMooltipassStatus'])
		{
			mooltipass.memmgmt.consoleLog("Mooltipass current status is: " + mooltipass.memmgmt.mooltipass_status[packet[2]]);
			if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE_REQ)
			{
				// In merge we need to check if we need to add the card
				if(mooltipass.memmgmt.mooltipass_status[packet[2]] == 'Unknown card')
				{
					mooltipass.memmgmt.isCardKnownByMp = false;
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getCurrentCardCPZ'], null);
					mooltipass.memmgmt_hid._sendMsg();	
				}
				else if(mooltipass.memmgmt.mooltipass_status[packet[2]] == 'Unlocked')
				{
					mooltipass.memmgmt.isCardKnownByMp = true;
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getCurrentCardCPZ'], null);
					mooltipass.memmgmt_hid._sendMsg();	
				}
				else
				{
					mooltipass.memmgmt.requestFailHander("No unlocked/unknown card in the Mooltipass", null, 653);
				}
			}
			else
			{
				// We can only proceed if the card is unlocked
				if(mooltipass.memmgmt.mooltipass_status[packet[2]] == 'Unlocked')
				{
					mooltipass.memmgmt.isCardKnownByMp = true;
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['startMemoryManagementMode'], null);
					mooltipass.memmgmt_hid._sendMsg();	
				}
				else
				{
					mooltipass.memmgmt.requestFailHander("No unlocked card in the Mooltipass", null, 654);
				}				
			}
		}
		else if(packet[1] == mooltipass.device.commands['getCurrentCardCPZ'])
		{
			if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE_REQ)
			{
				// Was it a success
				if(packet[0] == 1)
				{
					mooltipass.memmgmt.requestFailHander("No card inserted in the Mooltipass", null, 655);
				}
				else
				{
					var cpz_value_known = false;
					mooltipass.memmgmt.currentCardCPZ = packet.subarray(2, 2 + 8);
					
					// Depending if a file or a syncFS request brought us here
					if(mooltipass.memmgmt.backupFromFileReq == true)
					{
						// File: imported data is already in memory
						cpz_value_known = mooltipass.memmgmt.isCPZValueKnownInCPZCTRVector(mooltipass.memmgmt.currentCardCPZ, mooltipass.memmgmt.importedCPZCTRValues);
						
						// We just received the current card CPZ, check that we know it before proceeding to the merge, bypass the if we're just adding csv credentials
						if(cpz_value_known == true || mooltipass.memmgmt.mergeFileTypeCsv)
						{
							mooltipass.memmgmt.consoleLog("CPZ value known by file");
							if(mooltipass.memmgmt.isCardKnownByMp || mooltipass.memmgmt.mergeFileTypeCsv)
							{
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['startMemoryManagementMode'], null);
								mooltipass.memmgmt_hid._sendMsg();							
							}
							else
							{
								// Unknown card by the MP, force store
								var cpz_ctr_index = mooltipass.memmgmt.isCPZValueKnownInCPZCTRVectorIndex(mooltipass.memmgmt.currentCardCPZ, mooltipass.memmgmt.importedCPZCTRValues);
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['addUnknownCard'], mooltipass.memmgmt.importedCPZCTRValues[cpz_ctr_index]);
								mooltipass.memmgmt_hid._sendMsg();		
							}
						}
						else
						{
							mooltipass.memmgmt.requestFailHander("The file is not a data backup for this card", null, 656);
						}
					}
					else
					{
						// SyncFS: look inside our CPZ Table
						var file_name = "";
						for(var i = 0; i < mooltipass.memmgmt.CPZTable.length; i++)
						{
							var same_val = true;
							for(var j = 0; j < 8; j++)
							{
								if(mooltipass.memmgmt.CPZTable[i][0][j] != mooltipass.memmgmt.currentCardCPZ[j])
								{
									same_val = false;
								}
							}
							if(same_val == true)
							{
								file_name = mooltipass.memmgmt.CPZTable[i][1];
								cpz_value_known = true;
								break;
							}
						}
						
						// If we know the file, read it
						if(cpz_value_known == true)
						{
							mooltipass.memmgmt.consoleLog("CPZ value known, fetching " + file_name + " in our syncFS");
							mooltipass.filehandler.requestFileFromSyncFS(mooltipass.memmgmt.syncFS, file_name, mooltipass.memmgmt.syncFSRequestFileCallback);
						}
						else
						{
							mooltipass.memmgmt.requestFailHander("Inserted card isn't known by the syncFS", null, 657);
						}
					}
				}				
			}
		}
		else if(packet[1] == mooltipass.device.commands['addUnknownCard'])
		{
			if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE_REQ)
			{
				// Was it a success
				if(packet[2] == 0)
				{
					mooltipass.memmgmt.requestFailHander("PIN code not entered", null, 658);
				}
				else
				{
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['startMemoryManagementMode'], null);
					mooltipass.memmgmt_hid._sendMsg();							
				}
			}
		}
		else if(packet[1] == mooltipass.device.commands['startMemoryManagementMode'])
		{
			// Start memory management request answer
			mooltipass.memmgmt_hid.nbSendRetries = 3;
			 
			// Did we succeed?
			if(packet[2] == 1)
			{
				// Reset CPZ CTR Values
				mooltipass.memmgmt.CPZCTRValues = [];
				 
				// Load memory params
				mooltipass.memmgmt.consoleLog("Memory management mode entered");
				if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK_REQ)
				{
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getVersion'], null);
					mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_INT_CHECK;
					mooltipass.memmgmt_hid._sendMsg();	
				}
				else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_REQ)
				{
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getVersion'], null);
					mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD;
					mooltipass.memmgmt_hid._sendMsg();
				}
				else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE_REQ)
				{
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getVersion'], null);
					mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_DBFILE_MERGE;
					mooltipass.memmgmt_hid._sendMsg();
				}
				else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_MEM_BACKUP_REQ)
				{
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getVersion'], null);
					mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_MEM_BACKUP;
					mooltipass.memmgmt_hid._sendMsg();				
				}
			}
			else
			{
				mooltipass.memmgmt.requestFailHander("Couldn't enter memory management mode", null, 672);
			}
		}	
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_MEM_BACKUP)
	{
		// Parameter loading
		if(packet[1] == mooltipass.device.commands['getVersion'])
		{
			mooltipass.memmgmt.nbMb = packet[2];
			mooltipass.memmgmt.consoleLog("Mooltipass is " + mooltipass.memmgmt.nbMb + "Mb");	
			mooltipass.memmgmt.version = mooltipass.device.convertMessageArrayToString(packet.subarray(3));
			mooltipass.memmgmt.consoleLog("Mooltipass version is " + mooltipass.memmgmt.version);				
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getCTR'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getCTR'])
		{
			if(packet[0] == 1)
			{
				mooltipass.memmgmt.requestFailHander("Error during CTR request (card removed?)", MGMT_IDLE, 659);
			}
			else
			{
				mooltipass.memmgmt.ctrValue = packet.subarray(2, 2 + packet[0]);
				mooltipass.memmgmt.consoleLog("Mooltipass CTR value is " + mooltipass.memmgmt.ctrValue);			
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getCPZandCTR'], null);
				mooltipass.memmgmt_hid._sendMsg();				
			}
		}
		else if(packet[1] == mooltipass.device.commands['exportCPZandCTR'])
		{
			// CPZ CTR packet export, add it to our current buffer
			mooltipass.memmgmt.consoleLog("CPZ CTR packet export packet received: " + packet.subarray(2, 2 + packet[0])); 
			mooltipass.memmgmt.CPZCTRValues.push(packet.subarray(2, 2 + packet[0]));
			
			// Generate file name for the SyncFS (if used)
			mooltipass.memmgmt.syncFSFileName = "";
			for(var i = 0; i < 16; i ++)
			{
				mooltipass.memmgmt.syncFSFileName += packet[2 + 8 + i].toString(16);
			}			
			//mooltipass.memmgmt.consoleLog("SyncFS file name: " + mooltipass.memmgmt.syncFSFileName);
			
			// Arm receive
			mooltipass.memmgmt_hid.receiveMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getCPZandCTR'])
		{
			if(packet[2] == 0)
			{				
				mooltipass.memmgmt.requestFailHander("Error during CPZ/CTR request (card removed?)", MGMT_IDLE, 660);
			}
			else
			{
				// Inform that all CPZ CTR packets were sent
				mooltipass.memmgmt.consoleLog("All CPZ CTR packets are received");			
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getStartingParentAddress'], null);
				mooltipass.memmgmt_hid._sendMsg();				
			}
		}
		else if(packet[1] == mooltipass.device.commands['getStartingParentAddress'])
		{
			if(packet[0] == 1)
			{
				mooltipass.memmgmt.requestFailHander("Error during get starting parent request (card removed?)", MGMT_IDLE, 661);
			}
			else
			{
				mooltipass.memmgmt.startingParent = [packet[2], packet[3]];
				mooltipass.memmgmt.clonedStartingParent = [packet[2], packet[3]];
				mooltipass.memmgmt.consoleLog("Starting parent is " + mooltipass.memmgmt.startingParent); 
				
				// We don't deal with data nodes here....
				mooltipass.memmgmt.currentFavorite = 0;
				mooltipass.memmgmt.favoriteAddresses = [];
				mooltipass.memmgmt.dataStartingParent = [0, 0];
				mooltipass.memmgmt.clonedFavoriteAddresses = [];
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getFavorite'], [mooltipass.memmgmt.currentFavorite]);
				mooltipass.memmgmt_hid._sendMsg();	
				
				/* 				
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getStartingDataParentAddress'], null);
				mooltipass.memmgmt_hid._sendMsg();	  
				*/		
			}
		}
		else if(packet[1] == mooltipass.device.commands['getStartingDataParentAddress'])
		{
			if(packet[0] == 1)
			{
				mooltipass.memmgmt.requestFailHander("Error during get data starting parent request (card removed?)", MGMT_IDLE, 662);
			}
			else
			{
				mooltipass.memmgmt.currentFavorite = 0;
				mooltipass.memmgmt.favoriteAddresses = [];
				mooltipass.memmgmt.clonedFavoriteAddresses = [];
				mooltipass.memmgmt.dataStartingParent = [packet[2], packet[3]];
				mooltipass.memmgmt.consoleLog("Data starting parent is " + mooltipass.memmgmt.dataStartingParent);
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getFavorite'], [mooltipass.memmgmt.currentFavorite]);
				mooltipass.memmgmt_hid._sendMsg();				
			}
		}
		else if(packet[1] == mooltipass.device.commands['getFavorite'])
		{
			if(packet[0] == 1)
			{
				mooltipass.memmgmt.requestFailHander("Error during get favorite request (card removed?)", MGMT_IDLE, 663);
			}
			else
			{
				// Extract addresses and append them to our current ones
				mooltipass.memmgmt.favoriteAddresses.push(new Uint8Array(packet.subarray(2, 2 + 4)));	
				mooltipass.memmgmt.clonedFavoriteAddresses.push(new Uint8Array(packet.subarray(2, 2 + 4)));	
				// Check if we have done all favorites
				if(++mooltipass.memmgmt.currentFavorite == 14)
				{
					mooltipass.memmgmt.consoleLog("Favorites loaded: " + mooltipass.memmgmt.favoriteAddresses);
					mooltipass.memmgmt.packetToSendBuffer = [];
					mooltipass.memmgmt.curServiceNodes = [];				
					mooltipass.memmgmt.curLoginNodes = [];				
					mooltipass.memmgmt.curDataServiceNodes = [];		
					mooltipass.memmgmt.curDataNodes = [];	
					mooltipass.memmgmt.clonedCurServiceNodes = [];				
					mooltipass.memmgmt.clonedCurLoginNodes = [];				
					mooltipass.memmgmt.clonedCurDataServiceNodes = [];		
					mooltipass.memmgmt.clonedCurDataNodes = []; 
					mooltipass.memmgmt.credentialArrayForGui = [];
					mooltipass.memmgmt.changePasswordReqs = [];
					mooltipass.memmgmt.currentNode = new Uint8Array(NODE_SIZE);
					mooltipass.memmgmt.nodePacketId = 0;
					 
					// Depending on the current mode, check what to do next
					if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK)
					{
						// Start looping through all the nodes						
						mooltipass.memmgmt.currentMode = MGMT_INT_CHECK_SCAN;
						mooltipass.memmgmt.scanPercentage = 0;
						mooltipass.memmgmt.pageIt = 128;
						mooltipass.memmgmt.nodeIt = 0;
						// Send first scan packet
						mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF]);
						mooltipass.memmgmt_hid._sendMsg();
					}
					else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD)
					{
						if(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.startingParent, [0,0]) == true)
						{						
							mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;		
							//applyCallback(mooltipass.memmgmt.statusCallback, mooltipass.memmgmt.credentialArrayForGui, {'success': true, 'msg': "Credential listing done"});
							mooltipass.memmgmt.statusCallback({'success': true, 'msg': "Credential listing done"}, mooltipass.memmgmt.credentialArrayForGui);
						}
						else
						{
							// Follow the nodes, starting with the parent one
							mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN;
							mooltipass.memmgmt.curNodeAddressRequested = mooltipass.memmgmt.startingParent;
							mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.memmgmt.startingParent);
							mooltipass.memmgmt_hid._sendMsg();						
						}
					}
					else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE)
					{
						if(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.startingParent, [0,0]) == true)
						{
							// If the db is empty, start the merging procedure									
							mooltipass.memmgmt.totalAddressesRequired = mooltipass.memmgmt.generateMergeOperations();
							mooltipass.memmgmt.totalAddressesReceived = 0;
							mooltipass.memmgmt.freeAddressesBuffer = [];
							if(mooltipass.memmgmt.totalAddressesRequired > 0)
							{
								// We need to request free addresses
								mooltipass.memmgmt.currentMode = MGMT_DB_FILE_MERGE_GET_FREE_ADDR;	
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getFreeSlotAddresses'], [0,0]);
								mooltipass.memmgmt_hid._sendMsg();		
							}
							else
							{
								// Generate merge packets, check return for errors
								if(mooltipass.memmgmt.generateMergePackets() == true)
								{			
									// Now check if actually have something to change
									if(mooltipass.memmgmt.packetToSendBuffer.length == 0)
									{
										mooltipass.memmgmt.consoleLog("Mooltipass already synced with our credential file!");
										// Leave mem management mode				
										mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
										mooltipass.memmgmt_hid._sendMsg();
									}
									else
									{
										mooltipass.memmgmt.consoleLog("Sending merging packets");
										mooltipass.memmgmt.currentMode = MGMT_DB_FILE_MERGE_PACKET_SENDING;
										mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
										mooltipass.memmgmt_hid._sendMsg();
									}
								}
								else
								{
									applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'msg': "Errors detected when merging data, please contact support at support@themooltipass.com"});
								}
							}
						}
						else
						{
							// Follow the nodes, starting with the parent one
							mooltipass.memmgmt.currentMode = MGMT_DBFILE_MERGE_NORMAL_SCAN;
							mooltipass.memmgmt.curNodeAddressRequested = mooltipass.memmgmt.startingParent;
							mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.memmgmt.startingParent);
							mooltipass.memmgmt_hid._sendMsg();						
						}
					}
					else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_MEM_BACKUP)
					{
						// Follow the nodes, starting with the parent one
						mooltipass.memmgmt.currentMode = MGMT_MEM_BACKUP_NORMAL_SCAN;
						mooltipass.memmgmt.curNodeAddressRequested = mooltipass.memmgmt.startingParent;
						mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.memmgmt.startingParent);
						mooltipass.memmgmt_hid._sendMsg();
					}
				}
				else
				{
					// Otherwise, ask for the next one
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getFavorite'], [mooltipass.memmgmt.currentFavorite]);
					mooltipass.memmgmt_hid._sendMsg();
				}
			}			
		}
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN || mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_NORMAL_SCAN || mooltipass.memmgmt.currentMode == MGMT_MEM_BACKUP_NORMAL_SCAN)
	{
		// check if we actually could read the node (permission problems....)
		if(packet[0] > 1)
		{
			// extend current node
			mooltipass.memmgmt.currentNode.set(packet.subarray(2, 2 + packet[0]), mooltipass.memmgmt.nodePacketId*HID_PAYLOAD_SIZE);
			 
			// check if is the last packet for a given node
			if(++mooltipass.memmgmt.nodePacketId == 3)
			{
				//mooltipass.memmgmt.consoleLog("Node received: " + mooltipass.memmgmt.currentNode);
				mooltipass.memmgmt.nodePacketId = 0;
				 
				// Parse node
				if(mooltipass.memmgmt.isNodeValid(mooltipass.memmgmt.currentNode))
				{
					// Get node type
					var nodeType = mooltipass.memmgmt.getNodeType(mooltipass.memmgmt.currentNode);
					 
					if(nodeType == 'parent')
					{
						if(mooltipass.memmgmt.lastLetter != mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode).charAt(0))
						{
							// Progress callback in case first letter changed
							mooltipass.memmgmt.lastLetter = mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode).charAt(0);
							
							var tempCompletion;
							if(mooltipass.memmgmt.lastLetter < 'a')
							{
								tempCompletion = 0;
							}
							else if(mooltipass.memmgmt.lastLetter > 'z')
							{
								tempCompletion = 99;
							}
							else
							{
								tempCompletion = Math.round(((mooltipass.memmgmt.lastLetter.charCodeAt(0) - 'a'.charCodeAt(0)) / (('z'.charCodeAt(0) + 1) - 'a'.charCodeAt(0)))*100);
							}
							if (mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_NORMAL_SCAN)
							{
								mooltipass.memmgmt.progressCallback({'progress': tempCompletion/2, 'letter': mooltipass.memmgmt.lastLetter});
							}
							else
							{
								mooltipass.memmgmt.progressCallback({'progress': tempCompletion, 'letter': mooltipass.memmgmt.lastLetter});
							}
						}
						// Store names, addresses, nodes
						mooltipass.memmgmt.curServiceNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode)});
						mooltipass.memmgmt.clonedCurServiceNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode)});
						mooltipass.memmgmt.consoleLog("Received service " + mooltipass.memmgmt.curServiceNodes[mooltipass.memmgmt.curServiceNodes.length - 1].name + " at address " + mooltipass.memmgmt.curServiceNodes[mooltipass.memmgmt.curServiceNodes.length - 1].address);
						 
						// Store next parent address
						mooltipass.memmgmt.nextParentNodeTSAddress = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.currentNode);
						var first_child_address = mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.currentNode);
						 
						// If it has a child, request it...
						if(!mooltipass.memmgmt.isSameAddress(first_child_address, [0,0]))
						{
							mooltipass.memmgmt.curNodeAddressRequested = first_child_address;
							mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], first_child_address);
							mooltipass.memmgmt_hid._sendMsg();							
						}		
						else
						{
							// No child, check that we haven't finished scanning
							if(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.nextParentNodeTSAddress, [0,0]))
							{
								// Finished scanning
								if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN)
								{
									// Wait for the user to do his actions...
									mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;	
									//applyCallback(mooltipass.memmgmt.statusCallback, mooltipass.memmgmt.credentialArrayForGui, {'success': true, 'msg': "Credential listing done"});
									mooltipass.memmgmt.statusCallback({'success': true, 'msg': "Credential listing done"}, mooltipass.memmgmt.credentialArrayForGui);
								}
								else if(mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_NORMAL_SCAN)
								{
									// Start the merging procedure									
									mooltipass.memmgmt.totalAddressesRequired = mooltipass.memmgmt.generateMergeOperations();
									mooltipass.memmgmt.totalAddressesReceived = 0;
									mooltipass.memmgmt.freeAddressesBuffer = [];
									if(mooltipass.memmgmt.totalAddressesRequired > 0)
									{
										// We need to request free addresses
										mooltipass.memmgmt.currentMode = MGMT_DB_FILE_MERGE_GET_FREE_ADDR;	
										mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getFreeSlotAddresses'], [0,0]);
										mooltipass.memmgmt_hid._sendMsg();		
									}
									else
									{
										// Generate merge packets, check return for errors
										if(mooltipass.memmgmt.generateMergePackets() == true)
										{			
											// Now check if actually have something to change
											if(mooltipass.memmgmt.packetToSendBuffer.length == 0)
											{
												mooltipass.memmgmt.consoleLog("Mooltipass already synced with our credential file!");
												// Leave mem management mode				
												mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
												mooltipass.memmgmt_hid._sendMsg();
											}
											else
											{
												mooltipass.memmgmt.consoleLog("Sending merging packets");
												mooltipass.memmgmt.currentMode = MGMT_DB_FILE_MERGE_PACKET_SENDING;
												mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
												mooltipass.memmgmt_hid._sendMsg();
											}
										}
										else
										{
											applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'msg': "Errors detected when merging data, please contact support at support@themooltipass.com"});
										}
									}
								}
								else if(mooltipass.memmgmt.currentMode == MGMT_MEM_BACKUP_NORMAL_SCAN)
								{
									// Leave mem management mode		
									mooltipass.memmgmt.exportMemoryState();		 
									mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
									mooltipass.memmgmt_hid._sendMsg();									
								}
							}
							else
							{
								// Request next parent
								mooltipass.memmgmt.curNodeAddressRequested = mooltipass.memmgmt.nextParentNodeTSAddress;
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.memmgmt.nextParentNodeTSAddress);
								mooltipass.memmgmt_hid._sendMsg();										
							}
						}
					}
					else if(nodeType == 'child')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curLoginNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode), 'pointed': false});
						mooltipass.memmgmt.clonedCurLoginNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode), 'pointed': false});
						mooltipass.memmgmt.credentialArrayForGui.push({	'context': mooltipass.memmgmt.clonedCurServiceNodes[mooltipass.memmgmt.clonedCurServiceNodes.length-1].name,
																		'username': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode),
																		'address': mooltipass.memmgmt.curNodeAddressRequested,
																		'description': mooltipass.memmgmt.getDescription(mooltipass.memmgmt.currentNode),
																		'date_modified': mooltipass.memmgmt.getDateCreated(mooltipass.memmgmt.currentNode),
																		'date_lastused': mooltipass.memmgmt.getDateLastUsed(mooltipass.memmgmt.currentNode),
																		'favorite': mooltipass.memmgmt.isParentChildAFavorite(mooltipass.memmgmt.clonedCurServiceNodes[mooltipass.memmgmt.clonedCurServiceNodes.length-1].address, mooltipass.memmgmt.curNodeAddressRequested),
																		'parent_address': mooltipass.memmgmt.clonedCurServiceNodes[mooltipass.memmgmt.clonedCurServiceNodes.length-1].address
																		});
						
						mooltipass.memmgmt.consoleLog("Received login " + mooltipass.memmgmt.curLoginNodes[mooltipass.memmgmt.curLoginNodes.length - 1].name + " at address " + mooltipass.memmgmt.curLoginNodes[mooltipass.memmgmt.curLoginNodes.length - 1].address);
						 
						var next_child_address = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.currentNode);
						 
						// If it has a next child, request it...
						if(!mooltipass.memmgmt.isSameAddress(next_child_address, [0,0]))
						{
							mooltipass.memmgmt.curNodeAddressRequested = next_child_address;
							mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], next_child_address);
							mooltipass.memmgmt_hid._sendMsg();							
						}		
						else
						{
							// No next child, check that we haven't finished scanning
							if(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.nextParentNodeTSAddress, [0,0]))
							{
								// Finished scanning
								if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN)
								{
									// Wait for the user to do his actions...
									mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;		
									//applyCallback(mooltipass.memmgmt.statusCallback, mooltipass.memmgmt.credentialArrayForGui, {'success': true, 'msg': "Credential listing done"});
									mooltipass.memmgmt.statusCallback({'success': true, 'msg': "Credential listing done"}, mooltipass.memmgmt.credentialArrayForGui);
								}
								else if(mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_NORMAL_SCAN)
								{
									// Start the merging procedure									
									mooltipass.memmgmt.totalAddressesRequired = mooltipass.memmgmt.generateMergeOperations();
									mooltipass.memmgmt.totalAddressesReceived = 0;
									mooltipass.memmgmt.freeAddressesBuffer = [];
									if(mooltipass.memmgmt.totalAddressesRequired > 0)
									{
										// We need to request free addresses
										mooltipass.memmgmt.currentMode = MGMT_DB_FILE_MERGE_GET_FREE_ADDR;	
										mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getFreeSlotAddresses'], [0,0]);
										mooltipass.memmgmt_hid._sendMsg();		
									}
									else
									{
										// Generate merge packets, check return for errors
										if(mooltipass.memmgmt.generateMergePackets() == true)
										{			
											// Now check if actually have something to change
											if(mooltipass.memmgmt.packetToSendBuffer.length == 0)
											{
												mooltipass.memmgmt.consoleLog("Mooltipass already synced with our credential file!");
												// Leave mem management mode				
												mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
												mooltipass.memmgmt_hid._sendMsg();
											}
											else
											{
												mooltipass.memmgmt.consoleLog("Sending merging packets");
												mooltipass.memmgmt.currentMode = MGMT_DB_FILE_MERGE_PACKET_SENDING;
												mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
												mooltipass.memmgmt_hid._sendMsg();
											}
										}
										else
										{
											applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'msg': "Errors detected when merging data, please contact support at support@themooltipass.com"});
										}
									}
								}
								else if(mooltipass.memmgmt.currentMode == MGMT_MEM_BACKUP_NORMAL_SCAN)
								{
									// Leave mem management mode	
									mooltipass.memmgmt.exportMemoryState();
									mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
									mooltipass.memmgmt_hid._sendMsg();									
								}
								// Leave mem management mode						
								//mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
								//mooltipass.memmgmt_hid._sendMsg();
								// TO REMOVE!!!!
								//mooltipass.memmgmt.exportMemoryState();
								//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.credentialArrayForGui);
							}
							else
							{
								// Request next parent
								mooltipass.memmgmt.curNodeAddressRequested = mooltipass.memmgmt.nextParentNodeTSAddress;
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.memmgmt.nextParentNodeTSAddress);
								mooltipass.memmgmt_hid._sendMsg();										
							}
						}
					}
					else if(nodeType == 'dataparent')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curDataServiceNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode)});
						mooltipass.memmgmt.clonedCurDataServiceNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode)});
					}
					else if(nodeType == 'data')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curDataNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'data': new Uint8Array(mooltipass.memmgmt.currentNode), 'pointed': false});
						mooltipass.memmgmt.clonedCurDataNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'data': new Uint8Array(mooltipass.memmgmt.currentNode), 'pointed': false});
					}
				}
				else
				{
					// Well this isn't a good situation... we read an empty node
					mooltipass.memmgmt.requestFailHander("Empty node read in memory (card removed/memory corrupted?)", MGMT_IDLE, 664);
				}
				 
				// Reset current node
				mooltipass.memmgmt.currentNode = new Uint8Array(NODE_SIZE);
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
			mooltipass.memmgmt.consoleLog("Not allowed to read node !!!");			 
			mooltipass.memmgmt.requestFailHander("Wrong node read in memory (card removed/memory corrupted?)", MGMT_IDLE, 665);
		}
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_INT_CHECK_SCAN)
	{
		// compute completion percentage
		var tempCompletion = Math.round(((mooltipass.memmgmt.pageIt-128)/(mooltipass.memmgmt.getNumberOfPages(mooltipass.memmgmt.nbMb)-128))*100);
		if(tempCompletion != mooltipass.memmgmt.scanPercentage)
		{
			mooltipass.memmgmt.progressCallback({'progress': tempCompletion});
			mooltipass.memmgmt.scanPercentage = tempCompletion;
			//mooltipass.memmgmt.consoleLog(tempCompletion + "%");
		}
		 
		// check if we actually could read the node (permission problems....)
		if(packet[0] > 1)
		{
			// extend current node
			mooltipass.memmgmt.currentNode.set(packet.subarray(2, 2 + packet[0]), mooltipass.memmgmt.nodePacketId*HID_PAYLOAD_SIZE);
			 
			// check if is the last packet for a given node
			if(++mooltipass.memmgmt.nodePacketId == 3)
			{
				//mooltipass.memmgmt.consoleLog("Node received: " + mooltipass.memmgmt.currentNode);
				mooltipass.memmgmt.nodePacketId = 0;
				 
				// Parse node
				if(mooltipass.memmgmt.isNodeValid(mooltipass.memmgmt.currentNode))
				{
					// Get node type
					var nodeType = mooltipass.memmgmt.getNodeType(mooltipass.memmgmt.currentNode);
					 
					if(nodeType == 'parent')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curServiceNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode)});
						mooltipass.memmgmt.clonedCurServiceNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode)});
					}
					else if(nodeType == 'child')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curLoginNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode), 'pointed': false});
						mooltipass.memmgmt.clonedCurLoginNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode), 'pointed': false});
					}
					else if(nodeType == 'dataparent')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curDataServiceNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode)});
						mooltipass.memmgmt.clonedCurDataServiceNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': new Uint8Array(mooltipass.memmgmt.currentNode)});
					}
					else if(nodeType == 'data')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curDataNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'data': new Uint8Array(mooltipass.memmgmt.currentNode), 'pointed': false});
						mooltipass.memmgmt.clonedCurDataNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'data': new Uint8Array(mooltipass.memmgmt.currentNode), 'pointed': false});
					}
				}
				else
				{
					// Free slot
				}
				 
				// Check that we didn't finish the scanning, ask next node
				if(((mooltipass.memmgmt.nodeIt + 1) == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb) && (mooltipass.memmgmt.pageIt + 1) == mooltipass.memmgmt.getNumberOfPages(mooltipass.memmgmt.nbMb)) || false)//(mooltipass.memmgmt.scanPercentage == 5))
				{
					mooltipass.memmgmt.integrityCheck();
					// Check if there are changes to do or not
					if(mooltipass.memmgmt.packetToSendBuffer.length > 0)
					{
						// Changes to make, change mode, start sending baby!
						console.log("Problems with memory contents... sending correction packets");
						mooltipass.memmgmt.currentMode = MGMT_INT_CHECK_PACKET_SENDING;
						//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.packetToSendBuffer[0]);
						mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
						mooltipass.memmgmt_hid._sendMsg();
					}
					else
					{
						// No changes, exit memory management mode
						mooltipass.memmgmt.consoleLog("Memory OK, no changes to make!");
						mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
						mooltipass.memmgmt_hid._sendMsg();
					}
				}
				else
				{
					if(++mooltipass.memmgmt.nodeIt == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb))
					{
						// Changing pages
						mooltipass.memmgmt.nodeIt = 0;
						mooltipass.memmgmt.pageIt++;
					}
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF]);
					mooltipass.memmgmt_hid._sendMsg();
				}
				 
				// Reset current node
				mooltipass.memmgmt.currentNode = new Uint8Array(NODE_SIZE);
			}
			else
			{
				// Else, receive other packet the Mooltipass should send
				mooltipass.memmgmt_hid.receiveMsg();
			}
		}
		else
		{
			//mooltipass.memmgmt.consoleLog("Not allowed to read node");
			 
			// If we couldn't read the node, ask the next one
			mooltipass.memmgmt.nodePacketId = 0;
			// Check that we didn't finish the scanning, ask next node
			if((mooltipass.memmgmt.nodeIt + 1) == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb) && (mooltipass.memmgmt.pageIt + 1) == mooltipass.memmgmt.getNumberOfPages(mooltipass.memmgmt.nbMb))
			{				
				mooltipass.memmgmt.integrityCheck();
				// Check if there are changes to do or not
				if(mooltipass.memmgmt.packetToSendBuffer.length > 0)
				{
					// Changes to make, change mode, start sending baby!
					mooltipass.memmgmt.currentMode = MGMT_INT_CHECK_PACKET_SENDING;
					//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.packetToSendBuffer[0]);
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
					mooltipass.memmgmt_hid._sendMsg();
				}
				else
				{
					// No changes, exit memory management mode
					mooltipass.memmgmt.consoleLog("Memory OK, no changes to make!");
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
					mooltipass.memmgmt_hid._sendMsg();
				}
			}
			else
			{
				if(++mooltipass.memmgmt.nodeIt == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb))
				{
					// Changing pages
					mooltipass.memmgmt.nodeIt = 0;
					mooltipass.memmgmt.pageIt++;
				}				
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF]);
				mooltipass.memmgmt_hid._sendMsg();
			}
		}
	}
}

// Data received from USB callback, for media bundle related comms
mooltipass.memmgmt.mediaBundleDataReceivedCallback = function(packet)
{
	//mooltipass.memmgmt.consoleLog(packet);
	if(packet[1] == mooltipass.device.commands['startMediaImport'])
	{
		// Answer to start media import packet
		mooltipass.memmgmt.tempPassword = null;
		if(packet[2] == 0)
		{
			// Fail
			mooltipass.memmgmt.currentMode = MGMT_IDLE;
			applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': 693, 'msg': "Couldn't start media import"});
			mooltipass.device.processQueue();
		}
		else
		{
			// Epic win, send first data packet
			mooltipass.memmgmt.currentMode = MGMT_BUNDLE_UPLOAD;
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['mediaImport'], mooltipass.memmgmt.mediaBundle.subarray(0, MEDIA_BUNDLE_CHUNK_SIZE));
			mooltipass.memmgmt.byteCounter += MEDIA_BUNDLE_CHUNK_SIZE;
			mooltipass.memmgmt_hid.request['milliseconds'] = 4000;
			mooltipass.memmgmt_hid.nbSendRetries = 3;
			mooltipass.memmgmt_hid._sendMsg();
			mooltipass.memmgmt.consoleLog("Media import has started, please wait a few minutes...");
		}
	}
	else if(packet[1] == mooltipass.device.commands['mediaImport'])
	{
		// Acknowledge of previous packet send
		if(packet[2] == 0)
		{
			// Fail... we kind of are stuck here...
			mooltipass.memmgmt.currentMode = MGMT_IDLE;
			console.log("Media import failed, data byte counter: " + mooltipass.memmgmt.byteCounter);
			applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': 700, 'msg': "Media import failed"});
			mooltipass.device.processQueue();
		}
		else
		{
			// Progress bar
			var tempCompletion = Math.round((mooltipass.memmgmt.byteCounter/mooltipass.memmgmt.mediaBundle.length)*100);
			if(tempCompletion != mooltipass.memmgmt.mediaBundleUploadPercentage)
			{
				mooltipass.memmgmt.progressCallback({'progress': tempCompletion});
				mooltipass.memmgmt.mediaBundleUploadPercentage = tempCompletion;				
			}			

			// Check how many bytes we need to send
			var nb_bytes_to_send = MEDIA_BUNDLE_CHUNK_SIZE;			
			if(mooltipass.memmgmt.mediaBundle.length - mooltipass.memmgmt.byteCounter < MEDIA_BUNDLE_CHUNK_SIZE)
			{
				nb_bytes_to_send = mooltipass.memmgmt.mediaBundle.length - mooltipass.memmgmt.byteCounter;
			}
			
			if(nb_bytes_to_send == 0)
			{
				// We finished sending data
				mooltipass.memmgmt.consoleLog("Last packet sent!");
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMediaImport'], null);
				mooltipass.memmgmt.mediaImportEndPacketSent = true;
				mooltipass.memmgmt_hid._sendMsg();
			}
			else
			{
				// We still have data to send
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['mediaImport'], mooltipass.memmgmt.mediaBundle.subarray(mooltipass.memmgmt.byteCounter, mooltipass.memmgmt.byteCounter + nb_bytes_to_send));
				mooltipass.memmgmt.byteCounter += nb_bytes_to_send;	
				mooltipass.memmgmt_hid._sendMsg();		
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.mediaBundle.length - mooltipass.memmgmt.byteCounter);
			}			
		}
	}
	else if(packet[1] == mooltipass.device.commands['endMediaImport'])
	{
		mooltipass.memmgmt.mediaImportEndPacketSent = false;
		mooltipass.memmgmt.currentMode = MGMT_IDLE;
		if(packet[2] == 0)
		{
			// Fail... we kind of are stuck here...
			applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': 694, 'msg': "Couldn't end media import"});
			mooltipass.device.processQueue();
		}
		else
		{
			applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': true, 'code': 695, 'msg': "Media bundle imported successfully"});
			mooltipass.device.processQueue();		
		}
	}
}

// Media bundle read callback
mooltipass.memmgmt.mediaBundleReadCallback = function(e)
{
	mooltipass.memmgmt.consoleLog("Media bundle read event...");
	 
	if(e != null && e.type == "loadend" && mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		// Change state
		mooltipass.memmgmt.currentMode = MGMT_BUNDLE_UPLOAD_REQ;
		
		// Init vars
		mooltipass.memmgmt.byteCounter = 0;		
		mooltipass.memmgmt.mediaBundle = new Uint8Array(e.target.result);
		mooltipass.memmgmt.consoleLog("Media bundle read, " + mooltipass.memmgmt.mediaBundle.length + " bytes long");
		
		// Set the timeouts & callbacks then send a media import start packet
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['startMediaImport'], mooltipass.memmgmt.tempPassword);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.mediaBundleDataReceivedCallback;
		mooltipass.memmgmt_hid.timeoutCallback = mooltipass.memmgmt.dataSendTimeOutCallback;
		mooltipass.memmgmt_hid.request['milliseconds'] = 20000;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();
	}	
	else
	{		
		applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': 692, 'msg': "Error during file select"});
		mooltipass.device.processQueue();
	}
}

// Media bundle upload
mooltipass.memmgmt.mediaBundlerUpload = function(callback, password, progressCallback)
{
	mooltipass.memmgmt.progressCallback = progressCallback;
	mooltipass.memmgmt.mediaBundleUploadPercentage = 0;
	mooltipass.memmgmt.statusCallback = callback;
	
	// Check password length
	if((password.length != 124) && (password.length != 32))
	{
		applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': 691, 'msg': "Wrong password length!"});
		mooltipass.device.processQueue();
		return;
	}
	
	// Convert the password
	mooltipass.memmgmt.tempPassword = new Uint8Array(password.length/2);
	for(var i = 0; i < password.length; i+= 2)
	{
		mooltipass.memmgmt.tempPassword[i/2] = parseInt(password.substr(i, 2), 16);
	}
	
	// Ask the user to select the bundle
	mooltipass.filehandler.selectAndReadRawContents("mooltipass_bundle.img", mooltipass.memmgmt.mediaBundleReadCallback);
}

// Load memory management preferences callback
mooltipass.memmgmt.preferencesCallback = function(items)
{	
	//mooltipass.memmgmt.consoleLog(items);
	if(chrome.runtime.lastError)
	{
		// Something went wrong during file selection
		console.log("preferencesCallback error: "+ chrome.runtime.lastError.message);
	}
	else
	{
		if(items.memmgmtPrefsStored == null)
		{
			// Empty file, save new preferences
			mooltipass.memmgmt.consoleLog("Preferences storage: No preferences stored!");
			mooltipass.prefstorage.setStoredPreferences({"memmgmtPrefsStored": true, "memmgmtPrefs": mooltipass.memmgmt.preferences});
		}
		else
		{
			mooltipass.memmgmt.consoleLog("Preferences storage: loaded preferences");
			
			// Check if the preferences we got are of the latest version
			if(items.memmgmtPrefs.version != MGMT_PREFERENCES_VERSION)
			{
				mooltipass.memmgmt.consoleLog("Loaded preferences are from an older version");
				// Check what fields are missing in what is stored
				for(var key in mooltipass.memmgmt.preferences)
				{
					if(key in items.memmgmtPrefs)
					{
						mooltipass.memmgmt.consoleLog("Same key: " + key);
						mooltipass.memmgmt.preferences[key] = items.memmgmtPrefs[key];
					}
					else
					{
						mooltipass.memmgmt.consoleLog("New key: " + key);
					}
				}
				// Save preferences
				mooltipass.memmgmt.preferences.version = MGMT_PREFERENCES_VERSION;
				mooltipass.prefstorage.setStoredPreferences({"memmgmtPrefsStored": true, "memmgmtPrefs": mooltipass.memmgmt.preferences});
			}
			else
			{
				mooltipass.memmgmt.preferences = items.memmgmtPrefs;				
			}
			//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.preferences);
		}
	}
}
 
// Memory management mode start
mooltipass.memmgmt.memmgmtStart = function(callback, progressCallback) 
{	
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		mooltipass.memmgmt.lastLetter = '0';
		mooltipass.memmgmt.statusCallback = callback;
		mooltipass.memmgmt.progressCallback = progressCallback;
		mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_REQ;
		// First step is to query to user interaction timeout to set the correct packet timeout retry!
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
		mooltipass.memmgmt_hid.timeoutCallback = mooltipass.memmgmt.dataSendTimeOutCallback;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();
	}
	else
	{
		applyCallback(callback, null, {'success': false, 'code': 678, 'msg': "Memory management in another mode"}, null);
		mooltipass.device.processQueue();
	}
}

// Memory management mode start
mooltipass.memmgmt.memmgmtStop = function(callback)
{
	mooltipass.memmgmt.statusCallback = callback;
	
	if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE || mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		// Leave memory management mode
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
		mooltipass.memmgmt_hid._sendMsg();
	}
	else
	{
		applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': 679, 'msg': "Memory management in another mode"});
		mooltipass.device.processQueue();
	}
}

// Memory management mode state reset
mooltipass.memmgmt.memmgmtForceReset = function(callback)
{
	if(mooltipass.memmgmt.currentMode != MGMT_FORCE_EXIT_MMM)
	{
		mooltipass.memmgmt.statusCallback = callback;
		
		// Leave memory management mode
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
		mooltipass.memmgmt_hid.timeoutCallback = mooltipass.memmgmt.dataSendTimeOutCallback;
		mooltipass.memmgmt.currentMode = MGMT_FORCE_EXIT_MMM;
		mooltipass.memmgmt_hid.request.milliseconds = 20000;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();
	}
}

// Generate save packets
mooltipass.memmgmt.generateSavePackets = function()
{	
	var address_taken_counter = 0;
	
	// Generate an array with all our free addresses
	var free_addresses_vector = [];
	for(var i = 0; i < mooltipass.memmgmt.freeAddressesBuffer.length; i++)
	{
		for(var j = 0; j < mooltipass.memmgmt.freeAddressesBuffer[i].length; j++)
		{
			free_addresses_vector.push([mooltipass.memmgmt.freeAddressesBuffer[i][j], mooltipass.memmgmt.freeAddressesBuffer[i][++j]]);
		}		
	}
	
	// Tackling the new items
	mooltipass.memmgmt.consoleLog("");
	mooltipass.memmgmt.consoleLog("Treating new items...");
	for(var i = 0; i < mooltipass.memmgmt.memmgmtAddData.length; i++)
	{
		// Check for null elements
		if(mooltipass.memmgmt.memmgmtAddData[i].context == null)
		{
			mooltipass.memmgmt.memmgmtAddData[i].context = "";
		}
		if(mooltipass.memmgmt.memmgmtAddData[i].username == null)
		{
			mooltipass.memmgmt.memmgmtAddData[i].username = "";
		}
		if(mooltipass.memmgmt.memmgmtAddData[i].password == null)
		{
			mooltipass.memmgmt.memmgmtAddData[i].password = "";
		}
		if(mooltipass.memmgmt.memmgmtAddData[i].description == null)
		{
			mooltipass.memmgmt.memmgmtAddData[i].description = "";
		}
		
		// Check if the new parent node exists
		var new_parent_node_index = mooltipass.memmgmt.findIdByName(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.memmgmtAddData[i].context);
		if(new_parent_node_index == null)
		{
			// We need to add a new parent node
			var new_node = new Uint8Array(NODE_SIZE);
			for(var k = 0; k < NODE_SIZE; k++)
			{
				new_node[k] = 0x00;
			}
			
			// Find the new parent previous and next nodes
			var indexes = mooltipass.memmgmt.getPrevAndNextNodeIndexesForNewServiceNode(mooltipass.memmgmt.memmgmtAddData[i].context);
			
			// Check prev node, update addresses
			if(indexes[0] == null)
			{
				// No previous node, set starting parent
				mooltipass.memmgmt.clonedStartingParent = free_addresses_vector[address_taken_counter];
				mooltipass.memmgmt.changePrevAddress(new_node, [0,0]);
			}
			else
			{
				mooltipass.memmgmt.changePrevAddress(new_node, mooltipass.memmgmt.clonedCurServiceNodes[indexes[0]].address);
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[indexes[0]].data, free_addresses_vector[address_taken_counter]);
			}
			// Check next node, update addresses
			if(indexes[1] == null)
			{
				mooltipass.memmgmt.changeNextAddress(new_node, [0,0]);
			}
			else
			{
				mooltipass.memmgmt.changeNextAddress(new_node, mooltipass.memmgmt.clonedCurServiceNodes[indexes[1]].address);
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[indexes[1]].data, free_addresses_vector[address_taken_counter]);
			}
			
			// Set node type, service name, first child address, and add it to our list
			mooltipass.memmgmt.setNodeType(new_node, 'parent');
			mooltipass.memmgmt.setServiceName(new_node, mooltipass.memmgmt.memmgmtAddData[i].context);
			mooltipass.memmgmt.clonedCurServiceNodes.push({'address': free_addresses_vector[address_taken_counter], 'name': mooltipass.memmgmt.memmgmtAddData[i].context, 'data': new_node});
			address_taken_counter++;
		}
		
		// If we didn't need to create parent node, check that the user isn't adding the same username...
		var adding_duplicate_credential = false;
		if(new_parent_node_index != null)
		{			
			// Get first child index, set next and prev vars
			var parent_index = mooltipass.memmgmt.findIdByName(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.memmgmtAddData[i].context);
			var first_child_address = mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].data);
			var current_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, first_child_address);
			
			// Loop until we find the right name
			while(current_index != null)
			{
				// Check if login is the same
				if(mooltipass.memmgmt.clonedCurLoginNodes[current_index].name == mooltipass.memmgmt.memmgmtAddData[i].username)
				{
					mooltipass.memmgmt.consoleLog("User is trying to add the same username!");
					adding_duplicate_credential = true;
					break;
				}
				
				// Load next address
				current_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[current_index].data));
			}
		}
		
		if(adding_duplicate_credential == false)
		{
			// Parent node exists, find where to store the child node
			var parent_node_index = mooltipass.memmgmt.findIdByName(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.memmgmtAddData[i].context);
			var new_node = new Uint8Array(NODE_SIZE);
			for(var k = 0; k < NODE_SIZE; k++)
			{
				new_node[k] = 0x00;
			}
			
			// Set random CTR value
			var temp_ctr_value = new Uint8Array(3);
			window.crypto.getRandomValues(temp_ctr_value);
			new_node.set(temp_ctr_value, 34);
			
			// Find the new parent previous and next nodes
			var indexes = mooltipass.memmgmt.getPrevAndNextNodeIndexesForNewLoginNode(mooltipass.memmgmt.clonedCurServiceNodes[parent_node_index].address, mooltipass.memmgmt.memmgmtAddData[i].username);
			
			// Check prev node, update addresses
			if(indexes[0] == null)
			{
				// No previous node, set first child for parent
				mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[parent_node_index].data, free_addresses_vector[address_taken_counter]);
				mooltipass.memmgmt.changePrevAddress(new_node, [0,0]);
			}
			else
			{
				mooltipass.memmgmt.changePrevAddress(new_node, mooltipass.memmgmt.clonedCurLoginNodes[indexes[0]].address);
				mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[indexes[0]].data, free_addresses_vector[address_taken_counter]);
			}
			// Check next node, update addresses
			if(indexes[1] == null)
			{
				mooltipass.memmgmt.changeNextAddress(new_node, [0,0]);
			}
			else
			{
				mooltipass.memmgmt.changeNextAddress(new_node, mooltipass.memmgmt.clonedCurLoginNodes[indexes[1]].address);
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[indexes[1]].data, free_addresses_vector[address_taken_counter]);
			}
			
			// Set node type, service name, first child address, and add it to our list
			mooltipass.memmgmt.setNodeType(new_node, 'child');
			mooltipass.memmgmt.setLogin(new_node, mooltipass.memmgmt.memmgmtAddData[i].username);
			mooltipass.memmgmt.setDescription(new_node, mooltipass.memmgmt.memmgmtAddData[i].description);
			mooltipass.memmgmt.clonedCurLoginNodes.push({'address': free_addresses_vector[address_taken_counter], 'name': mooltipass.memmgmt.memmgmtAddData[i].username, 'data': new_node});
			
			// Change password
			mooltipass.memmgmt.changePasswordReqs.push({'service': mooltipass.memmgmt.memmgmtAddData[i].context, 'login': mooltipass.memmgmt.memmgmtAddData[i].username, 'password': mooltipass.memmgmt.memmgmtAddData[i].password});
			
			// Check if it is set as favorite
			if(mooltipass.memmgmt.memmgmtAddData[i].favorite)
			{
				mooltipass.memmgmt.addParentChildFavorite(mooltipass.memmgmt.clonedCurServiceNodes[parent_node_index].address, free_addresses_vector[address_taken_counter]);
			}
			
			address_taken_counter++;
		}		
	}
	
	// Tackling the updated items
	mooltipass.memmgmt.consoleLog("");
	mooltipass.memmgmt.consoleLog("Treating updated items...");
	for(var i = 0; i < mooltipass.memmgmt.memmgmtUpdateData.length; i++)
	{
		// Check for null elements
		if(mooltipass.memmgmt.memmgmtUpdateData[i].context == null)
		{
			mooltipass.memmgmt.memmgmtUpdateData[i].context = "";
		}
		if(mooltipass.memmgmt.memmgmtUpdateData[i].username == null)
		{
			mooltipass.memmgmt.memmgmtUpdateData[i].username = "";
		}
		if(mooltipass.memmgmt.memmgmtUpdateData[i].description == null)
		{
			mooltipass.memmgmt.memmgmtUpdateData[i].description = "";
		}
		
		// Check child & parent addresses
		var child_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.memmgmtUpdateData[i].address);
		var parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.memmgmtUpdateData[i].parent_address);
		
		if(child_index != null && parent_index != null)
		{
			// Check modified date
			if(mooltipass.memmgmt.getDateCreated(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data).getTime() != mooltipass.memmgmt.memmgmtUpdateData[i].date_modified.getTime())
			{					
				mooltipass.memmgmt.consoleLog("User changed date modified from " + mooltipass.memmgmt.getDateCreated(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data) + " to " + mooltipass.memmgmt.memmgmtUpdateData[i].date_modified);
				mooltipass.memmgmt.setDateCreated(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, mooltipass.memmgmt.memmgmtUpdateData[i].date_modified);
			}
			// Check last used date
			if(mooltipass.memmgmt.getDateLastUsed(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data).getTime() != mooltipass.memmgmt.memmgmtUpdateData[i].date_lastused.getTime())
			{			
				mooltipass.memmgmt.consoleLog("User changed date last used from " + mooltipass.memmgmt.getDateLastUsed(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data) + " to " + mooltipass.memmgmt.memmgmtUpdateData[i].date_lastused);
				mooltipass.memmgmt.setDateLastUsed(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, mooltipass.memmgmt.memmgmtUpdateData[i].date_lastused);
			}
			// Check if the description has been changed
			if(mooltipass.memmgmt.getDescription(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data) != mooltipass.memmgmt.memmgmtUpdateData[i].description)
			{					
				mooltipass.memmgmt.consoleLog("User changed child node description from " + mooltipass.memmgmt.getDescription(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data) + " to " + mooltipass.memmgmt.memmgmtUpdateData[i].description);
				mooltipass.memmgmt.setDescription(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, mooltipass.memmgmt.memmgmtUpdateData[i].description);
			}
			// Check if it was just tagged/removed as favorite
			if(mooltipass.memmgmt.memmgmtUpdateData[i].favorite == true && mooltipass.memmgmt.isParentChildAFavorite(mooltipass.memmgmt.memmgmtUpdateData[i].parent_address, mooltipass.memmgmt.memmgmtUpdateData[i].address) == false)
			{
				mooltipass.memmgmt.consoleLog("Adding favorite: " + mooltipass.memmgmt.clonedCurLoginNodes[child_index].name + " on " + mooltipass.memmgmt.clonedCurServiceNodes[parent_index].name);
				mooltipass.memmgmt.addParentChildFavorite(mooltipass.memmgmt.memmgmtUpdateData[i].parent_address, mooltipass.memmgmt.memmgmtUpdateData[i].address);
			}
			if(mooltipass.memmgmt.memmgmtUpdateData[i].favorite == false && mooltipass.memmgmt.isParentChildAFavorite(mooltipass.memmgmt.memmgmtUpdateData[i].parent_address, mooltipass.memmgmt.memmgmtUpdateData[i].address) == true)
			{
				mooltipass.memmgmt.consoleLog("Removing favorite: " + mooltipass.memmgmt.clonedCurLoginNodes[child_index].name + " on " + mooltipass.memmgmt.clonedCurServiceNodes[parent_index].name);
				mooltipass.memmgmt.deleteParentChildFavorite(mooltipass.memmgmt.memmgmtUpdateData[i].parent_address, mooltipass.memmgmt.memmgmtUpdateData[i].address);
			}
			
			// Check if the parent name has been changed
			if(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].name != mooltipass.memmgmt.memmgmtUpdateData[i].context)
			{
				mooltipass.memmgmt.consoleLog("User changed parent node name from " + mooltipass.memmgmt.clonedCurServiceNodes[parent_index].name + " to " + mooltipass.memmgmt.memmgmtUpdateData[i].context);	
				
				// Delete favorite if it exists
				mooltipass.memmgmt.deleteParentChildFavorite(mooltipass.memmgmt.memmgmtUpdateData[i].parent_address, mooltipass.memmgmt.memmgmtUpdateData[i].address);
				
				// Check if the login name has been changed
				if(mooltipass.memmgmt.clonedCurLoginNodes[child_index].name != mooltipass.memmgmt.memmgmtUpdateData[i].username)
				{
					mooltipass.memmgmt.consoleLog("User also changed child node name from " + mooltipass.memmgmt.clonedCurLoginNodes[child_index].name + " to " + mooltipass.memmgmt.memmgmtUpdateData[i].username);
				}

				// Check if the new parent node exists
				var new_parent_node_index = mooltipass.memmgmt.findIdByName(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.memmgmtUpdateData[i].context);
				if(new_parent_node_index != null)
				{
					// Parent node exists
					mooltipass.memmgmt.changeChildNodeLoginAndMoveItToExistingParent(mooltipass.memmgmt.memmgmtUpdateData[i].address, mooltipass.memmgmt.memmgmtUpdateData[i].parent_address, mooltipass.memmgmt.clonedCurServiceNodes[new_parent_node_index].address, mooltipass.memmgmt.memmgmtUpdateData[i].username);
				}
				else
				{
					// We need to add a new parent node
					var new_node = new Uint8Array(NODE_SIZE);
					for(var k = 0; k < NODE_SIZE; k++)
					{
						new_node[k] = 0x00;
					}
					
					// Find the new parent previous and next nodes
					var indexes = mooltipass.memmgmt.getPrevAndNextNodeIndexesForNewServiceNode(mooltipass.memmgmt.memmgmtUpdateData[i].context);
					
					// Check prev node, update addresses
					if(indexes[0] == null)
					{
						// No previous node, set starting parent
						mooltipass.memmgmt.clonedStartingParent = free_addresses_vector[address_taken_counter];
						mooltipass.memmgmt.changePrevAddress(new_node, [0,0]);
					}
					else
					{
						mooltipass.memmgmt.changePrevAddress(new_node, mooltipass.memmgmt.clonedCurServiceNodes[indexes[0]].address);
						mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[indexes[0]].data, free_addresses_vector[address_taken_counter]);
					}
					// Check next node, update addresses
					if(indexes[1] == null)
					{
						mooltipass.memmgmt.changeNextAddress(new_node, [0,0]);
					}
					else
					{
						mooltipass.memmgmt.changeNextAddress(new_node, mooltipass.memmgmt.clonedCurServiceNodes[indexes[1]].address);
						mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[indexes[1]].data, free_addresses_vector[address_taken_counter]);
					}
					
					// Set node type, service name, first child address, and add it to our list
					mooltipass.memmgmt.setNodeType(new_node, 'parent');
					mooltipass.memmgmt.setServiceName(new_node, mooltipass.memmgmt.memmgmtUpdateData[i].context);
					mooltipass.memmgmt.clonedCurServiceNodes.push({'address': free_addresses_vector[address_taken_counter], 'name': mooltipass.memmgmt.memmgmtUpdateData[i].context, 'data': new_node});
					new_parent_node_index = mooltipass.memmgmt.clonedCurServiceNodes.length - 1;
					
					// Call the reordering function and increment address counter
					mooltipass.memmgmt.changeChildNodeLoginAndMoveItToExistingParent(mooltipass.memmgmt.memmgmtUpdateData[i].address, mooltipass.memmgmt.memmgmtUpdateData[i].parent_address, free_addresses_vector[address_taken_counter], mooltipass.memmgmt.memmgmtUpdateData[i].username);
					address_taken_counter++;
				}
				
				// If it is a favorite
				if(mooltipass.memmgmt.memmgmtUpdateData[i].favorite)
				{
					mooltipass.memmgmt.addParentChildFavorite(mooltipass.memmgmt.clonedCurServiceNodes[new_parent_node_index].address, mooltipass.memmgmt.clonedCurLoginNodes[child_index].address);
				}
			}
			else
			{					
				// Check if the login name has been changed
				if(mooltipass.memmgmt.clonedCurLoginNodes[child_index].name != mooltipass.memmgmt.memmgmtUpdateData[i].username)
				{
					mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].name + ": user changed child node name from " + mooltipass.memmgmt.clonedCurLoginNodes[child_index].name + " to " + mooltipass.memmgmt.memmgmtUpdateData[i].username);
					mooltipass.memmgmt.changeChildNodeLoginAndUpdateNodes(mooltipass.memmgmt.memmgmtUpdateData[i].address, mooltipass.memmgmt.memmgmtUpdateData[i].parent_address, mooltipass.memmgmt.memmgmtUpdateData[i].username);
				}
				else
				{
					// Here login & service are not changed....
				}
			}
			
			// Check if password is set
			if('password' in mooltipass.memmgmt.memmgmtUpdateData[i])
			{
				//mooltipass.memmgmt.consoleLog("New password is set: " + mooltipass.memmgmt.memmgmtUpdateData[i].password);
				mooltipass.memmgmt.consoleLog("New password set for " + mooltipass.memmgmt.memmgmtUpdateData[i].username + " on " + mooltipass.memmgmt.memmgmtUpdateData[i].context);
				mooltipass.memmgmt.changePasswordReqs.push({'service': mooltipass.memmgmt.memmgmtUpdateData[i].context, 'login': mooltipass.memmgmt.memmgmtUpdateData[i].username, 'password': mooltipass.memmgmt.memmgmtUpdateData[i].password});
			}
		}
		else
		{
			// We were fed incorrect data
			applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': 680, 'msg': "Data provided is invalid!"});
			console.log("Error with following update data:");
			console.log(mooltipass.memmgmt.memmgmtUpdateData[i]);
			return;
		}
	}
	
	// Delete parent nodes that don't have children
	for(var i = 0; i < mooltipass.memmgmt.clonedCurServiceNodes.length; i++)
	{
		if(mooltipass.memmgmt.getNumberOfChildrenForClonedServiceNode(mooltipass.memmgmt.clonedCurServiceNodes[i]) == 0)
		{
			mooltipass.memmgmt.consoleLog("Deleting service with no logins: " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
			mooltipass.memmgmt.clonedCurServiceNodes[i]['deleted'] = true;
			mooltipass.memmgmt.deleteServiceNodeFromCloneArrayAndGenerateDeletePacket(mooltipass.memmgmt.clonedCurServiceNodes[i]);
		}
	}

	// Tackling the deleted items
	mooltipass.memmgmt.consoleLog("Treating deleted items...");
	for(var i = 0; i < mooltipass.memmgmt.memmgmtDeleteData.length; i++)
	{
		// Check child & parent addresses
		var child_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, mooltipass.memmgmt.memmgmtDeleteData[i].address);
		var parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.memmgmtDeleteData[i].parent_address);
		
		if(child_index != null && parent_index != null)
		{
			// See how many children the parent has so we know if we should delete it
			var number_of_children = mooltipass.memmgmt.getNumberOfChildrenForClonedServiceNode(mooltipass.memmgmt.clonedCurServiceNodes[parent_index]);
			
			if(number_of_children == 1)
			{
				// Delete parent node
				mooltipass.memmgmt.clonedCurServiceNodes[parent_index]['deleted'] = true;
				mooltipass.memmgmt.deleteServiceNodeFromCloneArrayAndGenerateDeletePacket(mooltipass.memmgmt.clonedCurServiceNodes[parent_index]);
			}
			
			// Delete child node
			mooltipass.memmgmt.clonedCurLoginNodes[child_index]['deleted'] = true;
			mooltipass.memmgmt.deleteChildNodeFromCloneArrayAndGenerateDeletePacket(mooltipass.memmgmt.clonedCurLoginNodes[child_index]);
		}
		else
		{
			// We were fed incorrect data
			applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'code': 681, 'msg': "Data provided is invalid!"});
			console.log("Error with following delete data:");
			console.log(mooltipass.memmgmt.memmgmtDeleteData[i]);
			return;
		}
	}
	
	// Now we can do the difference between our local buffer and what is on the mooltipass
	mooltipass.memmgmt.consoleLog("Generating packets...");
	for(var i = 0; i < mooltipass.memmgmt.clonedCurServiceNodes.length; i++)
	{
		// Check that it wasn't deleted before
		if(mooltipass.memmgmt.clonedCurServiceNodes[i]['deleted'] == null)
		{
			// Try to find the same node in the current mooltipass memory contents
			var parent_node_address = mooltipass.memmgmt.clonedCurServiceNodes[i].address;
			var parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.curServiceNodes, parent_node_address);
			
			if(parent_index == null)
			{
				// We don't know the node, add it
				mooltipass.memmgmt.consoleLog("Adding new parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
				//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i]);
				mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurServiceNodes[i].address, mooltipass.memmgmt.clonedCurServiceNodes[i].data);					
			}
			else
			{
				// Check if data is unchanged
				if(!mooltipass.memmgmt.compareNodeData(mooltipass.memmgmt.clonedCurServiceNodes[i], mooltipass.memmgmt.curServiceNodes[parent_index]))
				{
					mooltipass.memmgmt.consoleLog("Node data different for parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
					//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.clonedCurServiceNodes[i]);
					//mooltipass.memmgmt.consoleLog(mooltipass.memmgmt.curServiceNodes[parent_index]);
					mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurServiceNodes[i].address, mooltipass.memmgmt.clonedCurServiceNodes[i].data);
				}
				else
				{
					//mooltipass.memmgmt.consoleLog("Node data identical for parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
				}					
			}
		}
	}
	for(var i = 0; i < mooltipass.memmgmt.clonedCurLoginNodes.length; i++)
	{
		// Check that it wasn't deleted before
		if(mooltipass.memmgmt.clonedCurLoginNodes[i]['deleted'] == null)
		{
			// Try to find the same node in the current mooltipass memory contents
			var child_node_address = mooltipass.memmgmt.clonedCurLoginNodes[i].address;
			var child_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.curLoginNodes, child_node_address);
			
			if(child_index == null)
			{
				// We don't know the node, add it
				mooltipass.memmgmt.consoleLog("Adding new child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
				mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurLoginNodes[i].address, mooltipass.memmgmt.clonedCurLoginNodes[i].data);					
			}
			else
			{
				// Check if data is unchanged
				if(!mooltipass.memmgmt.compareNodeData(mooltipass.memmgmt.clonedCurLoginNodes[i], mooltipass.memmgmt.curLoginNodes[child_index]))
				{
					mooltipass.memmgmt.consoleLog("Node data different for child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
					mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurLoginNodes[i].address, mooltipass.memmgmt.clonedCurLoginNodes[i].data);
				}
				else
				{
					//mooltipass.memmgmt.consoleLog("Node data identical for child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
				}					
			}
		}
	}	
	
	// Check if favorites have changed
	for(var i = 0; i < mooltipass.memmgmt.favoriteAddresses.length; i++)
	{
		// Did the favorite change?
		if(!mooltipass.memmgmt.isSameFavoriteAddress(mooltipass.memmgmt.favoriteAddresses[i], mooltipass.memmgmt.clonedFavoriteAddresses[i]))
		{
			// Send a packet to update the address
			mooltipass.memmgmt.consoleLog("Updating favorite " + i);
			var favorite_packet = new Uint8Array(5);
			favorite_packet.set([i], 0);
			favorite_packet.set(mooltipass.memmgmt.clonedFavoriteAddresses[i], 1);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], favorite_packet));
		}
	}	
	
	// Finally, changed if the starting address changed
	if(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.clonedStartingParent, mooltipass.memmgmt.startingParent) == false)
	{
		mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingParentAddress'], mooltipass.memmgmt.clonedStartingParent));	
	}
	
	return mooltipass.memmgmt.checkClonedParentList();
}

// Memory management mode save
mooltipass.memmgmt.memmgmtSave = function(callback, deleteData, updateData, addData)
{
	if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE)
	{
		// Save passed data
		mooltipass.memmgmt.statusCallback = callback;
		mooltipass.memmgmt.memmgmtDeleteData = deleteData;
		mooltipass.memmgmt.memmgmtUpdateData = updateData;
		mooltipass.memmgmt.memmgmtAddData = addData;	
		
		// Lower case all data
		for(var i = 0; i < mooltipass.memmgmt.memmgmtUpdateData.length; i++)
		{
			mooltipass.memmgmt.memmgmtUpdateData[i].context = mooltipass.memmgmt.memmgmtUpdateData[i].context.toLowerCase();
		}
		for(var i = 0; i < mooltipass.memmgmt.memmgmtAddData.length; i++)
		{
			mooltipass.memmgmt.memmgmtAddData[i].context = mooltipass.memmgmt.memmgmtAddData[i].context.toLowerCase();
		}
		
		//mooltipass.memmgmt.consoleLog(deleteData);
		//mooltipass.memmgmt.consoleLog(updateData);
		//mooltipass.memmgmt.consoleLog(addData);
		//return;
		
		// Count how many addresses we need to add
		if(addData != null)
		{
			mooltipass.memmgmt.totalAddressesRequired = addData.length*2;
			//mooltipass.memmgmt.consoleLog(addData);
		}
		else
		{
			mooltipass.memmgmt.totalAddressesRequired = 0;
		}		
		
		// We need a new address if the user changed the service address to something we don't know
		for(var i = 0; i < updateData.length; i++)
		{
			var new_parent_node_index = mooltipass.memmgmt.findIdByName(mooltipass.memmgmt.clonedCurServiceNodes, updateData[i].context);			
			if(new_parent_node_index == null)
			{				
				mooltipass.memmgmt.totalAddressesRequired++;
			}
		}	

		// Depending on how many free addresses we need...
		mooltipass.memmgmt.consoleLog("Requesting " + mooltipass.memmgmt.totalAddressesRequired + " addresses");
		mooltipass.memmgmt.totalAddressesReceived = 0;
		mooltipass.memmgmt.freeAddressesBuffer = [];
		if(mooltipass.memmgmt.totalAddressesRequired > 0)
		{
			// We need to request free addresses
			mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE_GET_FREE_ADDR;	
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getFreeSlotAddresses'], [0,0]);
			mooltipass.memmgmt_hid._sendMsg();		
		}
		else
		{
			// Generate save packets
			if(mooltipass.memmgmt.generateSavePackets() == false)
			{
				// Check that we didn't break the linked list
				applyCallback(mooltipass.memmgmt.statusCallback, null, {'success': false, 'msg': "Errors detected when merging data, please contact support at support@themooltipass.com"});
				return;
			}
	
			// Now check if actually have something to change
			if(mooltipass.memmgmt.packetToSendBuffer.length == 0)
			{
				mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE_NO_CHANGES;
				mooltipass.memmgmt.consoleLog("Save button pressed but no changes");
				// Leave mem management mode				
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
				mooltipass.memmgmt_hid._sendMsg();
			}
			else
			{
				mooltipass.memmgmt.consoleLog("Sending updating packets");
				mooltipass.memmgmt.currentMode = MGMT_USER_CHANGES_PACKET_SENDING;
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
				mooltipass.memmgmt_hid._sendMsg();
			}
		}
	}
	else
	{
		applyCallback(callback, null, {'success': false, 'code': 682, 'msg': "Memory management in another mode"});
		mooltipass.device.processQueue();
	}
}
 
// Memory integrity check
mooltipass.memmgmt.integrityCheckStart = function(progressCallback, statusCallback)
{	
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		mooltipass.memmgmt.statusCallback = statusCallback;
		mooltipass.memmgmt.progressCallback = progressCallback;
		mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_INT_CHECK_REQ;
		// First step is to query to user interaction timeout to set the correct packet timeout retry!
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
		mooltipass.memmgmt_hid.timeoutCallback = mooltipass.memmgmt.dataSendTimeOutCallback;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();
	}
	else
	{
		applyCallback(statusCallback, null, {'success': false, 'code': 683, 'msg': "Memory management in another mode"});
		mooltipass.device.processQueue();
	}
}

// Merge CSV credential file to current database
mooltipass.memmgmt.mergeCsvCredentialFileToMooltipassStart = function(statusCallback, progressCallback)
{
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		// Open the file first
		mooltipass.memmgmt.mergeFileTypeCsv = true;
		mooltipass.memmgmt.backupFromFileReq = true;
		mooltipass.memmgmt.statusCallback = statusCallback;
		mooltipass.memmgmt.progressCallback = progressCallback;
		mooltipass.memmgmt.currentMode = MGMT_DBFILE_MERGE_REQ;
		mooltipass.memmgmt.importMemoryState();
	}
	else
	{
		applyCallback(statusCallback, null, {'success': false, 'code': 697, 'msg': "Memory management in another mode"});
		mooltipass.device.processQueue();
	}
}

// Merge credential file to current database
mooltipass.memmgmt.mergeCredentialFileToMooltipassStart = function(statusCallback, progressCallback)
{
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		// Open the file first
		mooltipass.memmgmt.backupFromFileReq = true;
		mooltipass.memmgmt.mergeFileTypeCsv = false;
		mooltipass.memmgmt.statusCallback = statusCallback;
		mooltipass.memmgmt.progressCallback = progressCallback;
		mooltipass.memmgmt.currentMode = MGMT_DBFILE_MERGE_REQ;
		mooltipass.memmgmt.importMemoryState();
	}
	else
	{
		applyCallback(statusCallback, null, {'success': false, 'code': 684, 'msg': "Memory management in another mode"});
		mooltipass.device.processQueue();
	}
}

// Merge SyncFS credential file to current database
mooltipass.memmgmt.mergeSyncFSCredentialFileToMooltipassStart = function(statusCallback, progressCallback)
{
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{		
		// Check if we have the syncfs
		if(mooltipass.memmgmt.syncFSOK == true)
		{
			// Store callback etc
			mooltipass.memmgmt.backupFromFileReq = false;
			mooltipass.memmgmt.statusCallback = statusCallback;
			mooltipass.memmgmt.progressCallback = progressCallback;
			
			// First step is to query to user interaction timeout to set the correct packet timeout retry
			mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_DBFILE_MERGE_REQ;
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
			mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
			mooltipass.memmgmt_hid.timeoutCallback = mooltipass.memmgmt.dataSendTimeOutCallback;
			mooltipass.memmgmt_hid.nbSendRetries = 0;
			mooltipass.memmgmt_hid._sendMsg();				
		}
		else
		{
			applyCallback(statusCallback, null, {'success': false, 'code': 685, 'msg': "SyncFS offline, please make sure you logged into Chrome"});
			mooltipass.device.processQueue();
		}		
	}
	else
	{
		applyCallback(statusCallback, null, {'success': false, 'code': 686, 'msg': "Memory management in another mode"});
		mooltipass.device.processQueue();
	}
}

// Memory backup start
mooltipass.memmgmt.memoryBackupStart = function(to_file_bool, statusCallback, progressCallback)
{
//mooltipass.datamemmgmt.addFileToMooltipass();return;
//mooltipass.datamemmgmt.listDataNodeNames();return;
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		if(to_file_bool == false && mooltipass.memmgmt.syncFSOK == false)
		{
			applyCallback(statusCallback, null, {'success': false, 'code': 687, 'msg': "SyncFS offline, please make sure you logged into Chrome"});
			mooltipass.device.processQueue();
		}
		else
		{
			mooltipass.memmgmt.lastLetter = '0';
			mooltipass.memmgmt.backupToFileReq = to_file_bool;
			mooltipass.memmgmt.statusCallback = statusCallback;
			mooltipass.memmgmt.progressCallback = progressCallback;
			mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_MEM_BACKUP_REQ;
			// First step is to query to user interaction timeout to set the correct packet timeout retry!
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
			mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
			mooltipass.memmgmt_hid.timeoutCallback = mooltipass.memmgmt.dataSendTimeOutCallback;
			mooltipass.memmgmt_hid.nbSendRetries = 0;
			mooltipass.memmgmt_hid._sendMsg();			
		}
	}
	else
	{
		applyCallback(statusCallback, null, {'success': false, 'code': 688, 'msg': "Memory management in another mode"});
		mooltipass.device.processQueue();
	}
}





