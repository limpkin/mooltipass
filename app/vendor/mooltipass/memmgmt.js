var mooltipass = mooltipass || {};
mooltipass.memmgmt = mooltipass.memmgmt || {};
 
// Defines
var NODE_SIZE							= 132;			// Node size
var HID_PAYLOAD_SIZE					= 62;			// HID payload
var MEDIA_BUNDLE_CHUNK_SIZE				= 33;			// How many bytes we send per chunk
	
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
mooltipass.memmgmt.preferences = {"memmgmtPrefsStored": false, "syncFSAllowed": false};

// State machines & temp variables
mooltipass.memmgmt.syncFS = null;							// SyncFS
mooltipass.memmgmt.syncFSOK = false;						// SyncFS state
mooltipass.memmgmt.syncFSMooltipassFileOK = false;			// SyncFS Mooltipass backup file state
mooltipass.memmgmt.currentMode = MGMT_IDLE;					// Current mode
mooltipass.memmgmt.currentFavorite = 0;						// Current favorite read/write
mooltipass.memmgmt.pageIt = 0;								// Page iterator
mooltipass.memmgmt.nodeIt = 0;								// Node iterator
mooltipass.memmgmt.scanPercentage = 0;						// Scanning percentage
mooltipass.memmgmt.nodePacketId = 0;						// Packet number for node sending/receiving
mooltipass.memmgmt.currentNode = [];						// Current node we're sending/receiving
mooltipass.memmgmt.packetToSendBuffer = [];					// Packets we need to send at the end of the checks etc...
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
mooltipass.memmgmt.memmgmtStartCallback = null;				// Callback function for memmgmt start
mooltipass.memmgmt.tempCallbackErrorString = null;			// Temp string used for callback
mooltipass.memmgmt.memmgmtStopCallback = null;				// Callback function for memmgmt stop
mooltipass.memmgmt.memmgmtSaveCallback = null;				// Callback function for memmgmt save

// State machines & temp variables related to media bundle upload
mooltipass.memmgmt.tempPassword = [];				// Temp password to unlock upload functionality
mooltipass.memmgmt.byteCounter = 0;					// Current byte counter
mooltipass.memmgmt.mediaBundle = [];				// Media bundle contents
 
 
// Node types
mooltipass.memmgmt.node_types = 
{
	0 : 'parent',
	1 : 'child',
	2 : 'dataparent',
	3 : 'data'
}
 
// Get node type
mooltipass.memmgmt.getNodeType = function(node)
{
	//console.log("Node Type is " + mooltipass.memmgmt.node_types[(node[1] >> 6) & 0x03]);
	return mooltipass.memmgmt.node_types[(node[1] >> 6) & 0x03];
}
 
// Get user ID
mooltipass.memmgmt.getUserId = function(node)
{
	console.log("User Id is " + (node[1] & 0x1F));
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
	//console.log("Parent Node: Service name is " + mooltipass.util.arrayToStr(node.subarray(8, node.length)));
	return mooltipass.util.arrayToStr(node.subarray(8, node.length)); 
}
 
// Get description
mooltipass.memmgmt.getDescription = function(node)
{
	//console.log("Child Node: Description is " + mooltipass.util.arrayToStr(node.subarray(6, node.length)));
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
	//console.log("Child Node: Login is " + mooltipass.util.arrayToStr(node.subarray(37, node.length)));
	return mooltipass.util.arrayToStr(node.subarray(37, node.length)); 
}
 
// Get date created
mooltipass.memmgmt.getDateCreated = function(node)
{
	var year = ((node[30] >> 1) & 0x7F) + 2010;
	var month = ((node[30] & 0x01) << 3) | ((node[31] >> 5) & 0x07);
	var day = (node[31] & 0x1F);
	//console.log("Date created is " + day + "/" + month + "/" + year);
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
	//console.log("Date last used is " + day + "/" + month + "/" + year);
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
		//console.log("Node valid");
		return true;
	}
	else
	{
		//console.log("Node not valid");
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
	for(var i = 0; i < mooltipass.memmgmt.favoriteAddresses; i++)
	{
		if(mooltipass.memmgmt.isSameFavoriteAddress(mooltipass.memmgmt.favoriteAddresses[i], [parentAddress[1], parentAddress[0], childAddress[1], childAddress[0]]))
		{
			return true;
		}
	}
	return false;
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
		//console.log(payload_to_send);
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
		console.log("Starting parent address ok");
	}	
	 
	// Check data service starting parent
	if((mooltipass.memmgmt.curDataServiceNodes.length > 0) && !mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.curDataServiceNodes[0].address, mooltipass.memmgmt.dataStartingParent))
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
		console.log("Starting data parent address ok");
	}
	
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
	console.log("Parent nodes check")
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
			console.log("Previous address for parent " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + " correct");
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
				console.log("Correct next address for parent " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + " at address " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].address + " : " + correct_next_node_addr + " (" + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index+1].name + ")");
			}
		}
		 
		cur_parent_node_index++;
	}
	 
	// Check the child nodes for the parent nodes...
	var cur_parent_node_index = 0;
	var cur_child_node_address = 0;
	console.log("Child nodes check");
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
			console.log("Parent node " + mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].name + " first child is " + mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].name + " at address " + cur_child_node_address);
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
					console.log("Previous child address is OK: " + prev_child_node_address);
				}
				else
				{
					console.log("Problem with previous child address: " + prev_child_node_address + " instead of " + correct_prev_child_node_address);
					mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[current_child_node_index].data, correct_prev_child_node_address);
				}
				 
				// Next child node address
				if(next_child_node_index != null)
				{
					// Known next address, continue looping
					console.log("Next child node is " + mooltipass.memmgmt.clonedCurLoginNodes[next_child_node_index].name + " at address " + next_child_node_address);
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
	console.log("Checking for orphan child nodes...");
	mooltipass.memmgmt.clonedCurLoginNodes.forEach( function (item, index, arr) 
													{
														// Check if it was pointed
														if(item.pointed == false)
														{
															console.log("Found orphan node: " + item.name + " , deleting it...");
															mooltipass.memmgmt.clonedCurLoginNodes.splice(index,1);
														}
													});
													 
	// Now that we possibility have removed nodes, check that the favorite addresses are correct
	for(var i = 0; i < mooltipass.memmgmt.favoriteAddresses.length; i++)
	{
		// Extract addresses
		var cur_favorite_address_parent = mooltipass.memmgmt.favoriteAddresses[i].subarray(0, 0 + 2);
		var cur_favorite_address_child = mooltipass.memmgmt.favoriteAddresses[i].subarray(2, 2 + 2);
		 
		// Only compare if both addresses are different than 0
		if(mooltipass.memmgmt.isSameAddress(cur_favorite_address_child, [0,0]) && mooltipass.memmgmt.isSameAddress(cur_favorite_address_parent,[0,0]))
		{
			console.log("Favorite " + i + " is empty");
		}
		else
		{
			var parent_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, cur_favorite_address_parent);		
			var child_node_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, cur_favorite_address_child);		
			 
			// Check if both addresses are correct
			if(parent_node_index != null && child_node_index != null)
			{
				console.log("Favorite " + i + " is valid: " + mooltipass.memmgmt.clonedCurLoginNodes[child_node_index].name + " on " + mooltipass.memmgmt.clonedCurServiceNodes[parent_node_index].name);
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
				console.log("Parent node " + mooltipass.memmgmt.curServiceNodes[i].name + " OK");
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
				console.log("Child node " + mooltipass.memmgmt.curLoginNodes[i].name + " OK");
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
			console.log("Updating favorite " + i);
			var favorite_packet = new Uint8Array(5);
			favorite_packet.set([i], 0);
			favorite_packet.set(mooltipass.memmgmt.clonedFavoriteAddresses[i], 1);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], favorite_packet));
		}
	}	
	 
	//console.log(mooltipass.memmgmt.packetToSendBuffer);
	return;
	 
	console.log("Services:");
	for(var i = 0; i < mooltipass.memmgmt.curServiceNodes.length; i++)
	{
		console.log(mooltipass.memmgmt.curServiceNodes[i].name + " at address " + mooltipass.memmgmt.curServiceNodes[i].address);
	}
	console.log("Cloned Services:");
	for(var i = 0; i < mooltipass.memmgmt.clonedCurServiceNodes.length; i++)
	{
		console.log(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " at address " + mooltipass.memmgmt.clonedCurServiceNodes[i].address);
	}
	console.log("Logins:");
	for(var i = 0; i < mooltipass.memmgmt.clonedCurLoginNodes.length; i++)
	{
		console.log(mooltipass.memmgmt.clonedCurLoginNodes[i].name + " pointed: " + mooltipass.memmgmt.clonedCurLoginNodes[i].pointed + " address: " + mooltipass.memmgmt.clonedCurLoginNodes[i].address);
	}
	console.log("Data services:");
	for(var i = 0; i < mooltipass.memmgmt.curDataServiceNodes.length; i++)
	{
		console.log(mooltipass.memmgmt.curDataServiceNodes[i].name);
	}
}
 
// Function called with a read progress event, parse read file
mooltipass.memmgmt.processReadProgressEvent = function(e)
{
	console.log("Processing progress read event...");
	 
	if(e.type == "loadend")
	{
		if(e.target.result == "")
		{
			console.log("Read: Empty file");
			if(mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_REQ)
			{
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
			}
			return;
		}
		 
		var imported_data = JSON.parse(e.target.result);
		 
		// Check data format
		if(imported_data.length != 10)
		{
			console.log("Wrong data format!");
			if(mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_REQ)
			{
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
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
		if(checkString != "mooltipass")
		{
			console.log("Wrong data format!");
			if(mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_REQ)
			{
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
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
		 
		//console.log([mooltipass.memmgmt.importedCtrValue, mooltipass.memmgmt.importedCPZCTRValues, mooltipass.memmgmt.importedStartingParent, mooltipass.memmgmt.importedDataStartingParent, mooltipass.memmgmt.importedFavoriteAddresses, mooltipass.memmgmt.importedCurServiceNodes, mooltipass.memmgmt.importedCurLoginNodes, mooltipass.memmgmt.importedCurDataServiceNodes, mooltipass.memmgmt.importedCurDataNodes, checkString]);
		mooltipass.memmgmt.syncFSMooltipassFileOK = true;
		console.log("Data imported!");	
		
		// Depending on current mode, launch next actions....
		if(mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_REQ)
		{			
			// First step is to query to user interaction timeout to set the correct packet timeout retry!
			mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_DBFILE_MERGE_REQ;
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
			mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
			mooltipass.memmgmt_hid.nbSendRetries = 0;
			mooltipass.memmgmt_hid._sendMsg();
		}
	}
	else
	{
		console.log("Unsupported progress event type!");
	}
}
 
// Export current Mooltipass memory state
mooltipass.memmgmt.exportMemoryState = function()
{
	var export_data = [mooltipass.memmgmt.ctrValue, mooltipass.memmgmt.CPZCTRValues, mooltipass.memmgmt.startingParent, mooltipass.memmgmt.dataStartingParent, mooltipass.memmgmt.favoriteAddresses, mooltipass.memmgmt.curServiceNodes, mooltipass.memmgmt.curLoginNodes, mooltipass.memmgmt.curDataServiceNodes, mooltipass.memmgmt.curDataNodes, "mooltipass"];
	mooltipass.filehandler.selectAndSaveFileContents("memory_export", new Blob([JSON.stringify(export_data)], {type: 'text/plain'}), mooltipass.memmgmt.fileWrittenCallback);
	//console.log(export_data);
	
	if(mooltipass.memmgmt.syncFSOK)
	{
		mooltipass.filehandler.writeFileToSyncFS(mooltipass.memmgmt.syncFS, "mooltipassAutomaticBackup.bin", new Blob([JSON.stringify(export_data)], {type: 'text/plain'}), mooltipass.memmgmt.syncFSFileWrittenCallback);
	}
	// {type: 'application/octet-binary'}
}
 
// Callback after memory import file read
mooltipass.memmgmt.importMemoryStateCallBack = function(e)
{
	if(e == null)
	{
		console.log("Import memory state callback: error");
		if(mooltipass.memmgmt.currentMode == MGMT_DBFILE_MERGE_REQ)
		{
			mooltipass.memmgmt.currentMode = MGMT_IDLE;
		}
	}
	else
	{
		console.log("Received data from file");
		mooltipass.memmgmt.processReadProgressEvent(e);
	}	 
}
 
// Import a Mooltipass memory state file
mooltipass.memmgmt.importMemoryState = function()
{
	mooltipass.filehandler.selectAndReadContents("memory_export.bin", mooltipass.memmgmt.importMemoryStateCallBack);
}
 
// Export file written callback
mooltipass.memmgmt.fileWrittenCallback = function()
{
	console.log("File written!");
}

// SyncFS export file written callback
mooltipass.memmgmt.syncFSFileWrittenCallback = function()
{
	console.log("File written to syncFS!");
}
 
// Get syncable file system status callback
mooltipass.memmgmt.syncableFSStateCallback = function(status)
{
	if(status == "initializing")
	{
		console.log("SyncFS initializing");
		mooltipass.memmgmt.syncFSOK = false;
	}
	else if(status == "running")
	{
		console.log("SyncFS running");
		mooltipass.filehandler.requestSyncFS(mooltipass.memmgmt.syncFSGetCallback);
	}
	else if(status == "authentication_required")
	{
		console.log("SyncFS: authentication required");
		mooltipass.memmgmt.syncFSOK = false;
	}
	else if(status == "temporary_unavailable")
	{
		console.log("SyncFS temporary unavailable");
		mooltipass.memmgmt.syncFSOK = false;
	}
	else if(status == "disabled")
	{
		console.log("SyncFS disabled");
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
		console.log("Received SyncFS");
		mooltipass.memmgmt.syncFS = fs;
		mooltipass.memmgmt.syncFSOK = true;
		mooltipass.filehandler.requestOrCreateFileFromSyncFS(mooltipass.memmgmt.syncFS, "mooltipassAutomaticBackup.bin", mooltipass.memmgmt.syncFSRequestOrCreateFileCallback);
	}
}
 
// Callback when file from SyncFS is read
mooltipass.memmgmt.syncFSRequestOrCreateFileCallback = function(e)
{
	console.log("Received data for syncFS file");
	mooltipass.memmgmt.processReadProgressEvent(e);
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
		mooltipass.memmgmt.getPasswordCallback({'success': false, 'msg': "Unknown service or login"}, "not valid");
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
	console.log("Parent node " + node.name + " has been removed");
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
			console.log("It seems we removed the only parent node!");
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingParentAddress'], mooltipass.memmgmt.clonedStartingParent)); 
		}
		else
		{
			mooltipass.memmgmt.clonedStartingParent = mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].address;
			console.log("Changing starting parent to: ", mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].name);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingParentAddress'], mooltipass.memmgmt.clonedStartingParent)); 
			console.log("Changing previous address of " + mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].name + " to empty address (first node)");
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].data, [0,0]);
		}				
	}
	else
	{
		if(next_node_index == null)
		{
			console.log("Updating next node field of " + mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].name + " to empty address (last node)");
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].data, [0,0]);
		}
		else
		{					
			console.log("Updating next node field of " + mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].name + " to " + mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].name);
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].data, mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].address)
			console.log("Changing previous address of " + mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].name + " to " + mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].name);
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[next_node_index].data, mooltipass.memmgmt.clonedCurServiceNodes[prev_node_index].address);
		}
	}
}

// Do all the operations required by deleting a child node and generate delete node packet
mooltipass.memmgmt.deleteChildNodeFromCloneArrayAndGenerateDeletePacket = function(node)
{
	console.log("Child node " + node.name + " has been removed");
	mooltipass.memmgmt.addEmptyNodePacketToSendBuffer(node.address);
	
	// Next we need to update the prev/next nodes
	var prev_node_index = mooltipass.memmgmt.findIdByNextAddress(mooltipass.memmgmt.clonedCurLoginNodes, node.address);
	var next_node_index = mooltipass.memmgmt.findIdByPrevAddress(mooltipass.memmgmt.clonedCurLoginNodes, node.address);
	
	// Check if we are not dealing with the first node
	if(prev_node_index == null)
	{				
		// Find the parent for which this node is the first node (we are sure it exists!)
		var matching_parent_index = mooltipass.memmgmt.findIdByFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes, node.address);
		
		if(next_node_index == null)
		{
			console.log("It seems we removed the only child node!");
			mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[matching_parent_index].data, [0,0]);
		}
		else
		{
			console.log("Changing " + mooltipass.memmgmt.clonedCurServiceNodes[matching_parent_index].name + " first child to: ", mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].name);
			mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[matching_parent_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].address);
			console.log("Changing previous address of " + mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].name + " to empty address (first node)");
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].data, [0,0]);
		}				
	}
	else
	{
		if(next_node_index == null)
		{
			console.log("Updating next node field of " + mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].name + " to empty address (last node)");
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].data, [0,0]);
		}
		else
		{					
			console.log("Updating next node field of " + mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].name + " to " + mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].name);
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].data, mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].address)
			console.log("Changing previous address of " + mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].name + " to " + mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].name);
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[next_node_index].data, mooltipass.memmgmt.clonedCurLoginNodes[prev_node_index].address);
		}
	}
}
 
// Generate merge operations once we have listed all the credentials inside the Mooltipass
mooltipass.memmgmt.generateMergeOperations = function()
{
	var virtual_address_counter = 0;				// When we add nodes, we give them an address based on this counter
	var new_children_for_existing_parents = [];		// New children for existing parent nodes
	
	console.log("");
	console.log("Merging procedure started");
	
	// Know if we need to add CPZ CTR packets (additive process)
	for(var i = 0; i < mooltipass.memmgmt.importedCPZCTRValues.length; i++)
	{
		if(!mooltipass.memmgmt.isCPZValueKnownInCPZCTRVector(mooltipass.memmgmt.importedCPZCTRValues[i].subarray(0, 8), mooltipass.memmgmt.CPZCTRValues))
		{
			// Send the unknown CPZ CTR value to the Mooltipass
			console.log("Unknown CPZ CTR values: " + mooltipass.memmgmt.importedCPZCTRValues[i]);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['addCPZandCTR'], mooltipass.memmgmt.importedCPZCTRValues[i]));
		}
		else
		{
			//console.log("Already known CPZ CTR values: " + mooltipass.memmgmt.importedCPZCTRValues[i]);
		}
	}
	
	// Check CTR value
	if(mooltipass.memmgmt.ctrValue[0] != mooltipass.memmgmt.importedCtrValue[0] || mooltipass.memmgmt.ctrValue[1] != mooltipass.memmgmt.importedCtrValue[1] || mooltipass.memmgmt.ctrValue[2] != mooltipass.memmgmt.importedCtrValue[2])
	{
		console.log("CTR Value mismatch: " + mooltipass.memmgmt.ctrValue + " instead of: " + mooltipass.memmgmt.importedCtrValue);
		mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setCTR'], mooltipass.memmgmt.importedCtrValue));
	}
	else
	{
		console.log("CTR value match");
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
				//console.log("Parent node core data match: " + mooltipass.memmgmt.importedCurServiceNodes[i].name);
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
							//console.log("Looking for child " + mooltipass.memmgmt.importedCurLoginNodes[k].name + " in the Mooltipass");
							
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
											//console.log("Child found in the mooltipass, comparing rest of the data");
											mooltipass.memmgmt.clonedCurLoginNodes[l]["mergeTagged"] = true;
											matched_login_found = true;
											
											if(mooltipass.memmgmt.compareChildNodeCoreData(mooltipass.memmgmt.clonedCurLoginNodes[l], mooltipass.memmgmt.importedCurLoginNodes[k]))
											{
												//console.log(mooltipass.memmgmt.importedCurServiceNodes[i].name + " : child core data match for child " + mooltipass.memmgmt.importedCurLoginNodes[k].name + " , nothing to do");
											}
											else
											{
												// Data mismatch, overwrite the important part
												console.log(mooltipass.memmgmt.importedCurServiceNodes[i].name + " : child core data mismatch for child " + mooltipass.memmgmt.importedCurLoginNodes[k].name + " , updating...");
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
								console.log(mooltipass.memmgmt.importedCurServiceNodes[i].name + " : adding new child " + mooltipass.memmgmt.importedCurLoginNodes[k].name + " in the mooltipass...");
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
			console.log("Adding new parent node: " + mooltipass.memmgmt.importedCurServiceNodes[i].name);
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
						//console.log("Browsing: " + mooltipass.memmgmt.clonedCurServiceNodes[k].name);
						if(mooltipass.memmgmt.importedCurServiceNodes[i].name < mooltipass.memmgmt.clonedCurServiceNodes[k].name)
						{
							//console.log("Next child set: " + mooltipass.memmgmt.clonedCurServiceNodes[k].name);		
							temp_parent_node_address = null;
							next_parent_node_index = k;
							break;
						}
						if(mooltipass.memmgmt.importedCurServiceNodes[i].name > mooltipass.memmgmt.clonedCurServiceNodes[k].name)
						{
							//console.log("Prev child set: " + mooltipass.memmgmt.clonedCurServiceNodes[k].name);	
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
				//console.log(mooltipass.memmgmt.importedCurServiceNodes[i].name + " will fit in front of the other parents");
				mooltipass.memmgmt.clonedStartingParent = temp_new_service_address;
				mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.importedCurServiceNodes[i].data, [0,0]);
			}
			else
			{
				//console.log(mooltipass.memmgmt.importedCurServiceNodes[i].name + " will fit after " + mooltipass.memmgmt.clonedCurServiceNodes[prev_parent_node_index].name);
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
				//console.log(mooltipass.memmgmt.importedCurServiceNodes[i].name + " will fit after all the other children");
			}
			else
			{
				//console.log(mooltipass.memmgmt.importedCurServiceNodes[i].name + " will fit before " + mooltipass.memmgmt.clonedCurServiceNodes[next_parent_node_index].name);
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
				mooltipass.memmgmt.clonedCurServiceNodes.push({'address': [0, 0], 'name': mooltipass.memmgmt.importedCurServiceNodes[i].name, 'data': mooltipass.memmgmt.importedCurServiceNodes[i].data, 'tempAddress': temp_new_service_address, 'tempChildAddress': 0, 'tempPrevAddress': temp_prev_address, 'tempNextAddress': temp_next_address, 'mergeTagged': true});
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
						console.log(mooltipass.memmgmt.importedCurServiceNodes[i].name + " : adding child node " + mooltipass.memmgmt.importedCurLoginNodes[k].name)
						
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
				var temp_child_node_address = mooltipass.memmgmt.clonedCurServiceNodes[i].tempFirstChildAddress;			
				
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
							//console.log("Browsing: " + mooltipass.memmgmt.clonedCurLoginNodes[k].name);
							if(new_children_for_existing_parents[j].name < mooltipass.memmgmt.clonedCurLoginNodes[k].name)
							{
								//console.log("Next child set: " + mooltipass.memmgmt.clonedCurLoginNodes[k].name);		
								temp_child_node_address = null;
								next_child_node_index = k;
								break;
							}
							if(new_children_for_existing_parents[j].name > mooltipass.memmgmt.clonedCurLoginNodes[k].name)
							{
								//console.log("Prev child set: " + mooltipass.memmgmt.clonedCurLoginNodes[k].name);	
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
					//console.log(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : " + new_children_for_existing_parents[j].name + " will fit in front of the other children");
					mooltipass.memmgmt.clonedCurServiceNodes[i]["tempFirstChildAddress"] = temp_new_login_address;
					mooltipass.memmgmt.changePrevAddress(new_children_for_existing_parents[j].data, [0,0]);
				}
				else
				{
					//console.log(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : " + new_children_for_existing_parents[j].name + " will fit after " + mooltipass.memmgmt.clonedCurLoginNodes[prev_child_node_index].name);
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
					//console.log(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : " + new_children_for_existing_parents[j].name + " will fit after all the other children");
				}
				else
				{
					//console.log(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : " + new_children_for_existing_parents[j].name + " will fit before " + mooltipass.memmgmt.clonedCurLoginNodes[next_child_node_index].name);
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
	console.log("Total number of addresses required: " + virtual_address_counter);
	return virtual_address_counter;
}
 
// Generate merge packets
mooltipass.memmgmt.generateMergePackets = function()
{
	console.log("");
	console.log("Generating merge packets");
	
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
		console.log("Changing starting parent to: ", free_addresses_vector[mooltipass.memmgmt.clonedStartingParent]);
		mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingParentAddress'], free_addresses_vector[mooltipass.memmgmt.clonedStartingParent])); 
	}
	
	// Here we are going to make a direct temp address to free_addresses_vector[temp_address] translation
	for(var i = 0; i < mooltipass.memmgmt.clonedCurServiceNodes.length; i++)
	{
		if(mooltipass.memmgmt.clonedCurServiceNodes[i].tempAddress != null)
		{
			//console.log(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempAddress]);
			mooltipass.memmgmt.clonedCurServiceNodes[i].address = free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempAddress];
		}
		if(mooltipass.memmgmt.clonedCurServiceNodes[i].tempFirstChildAddress != null)
		{
			//console.log(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : child address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempFirstChildAddress]);		
			mooltipass.memmgmt.changeFirstChildAddress(mooltipass.memmgmt.clonedCurServiceNodes[i].data, free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempFirstChildAddress]);
		}
		if(mooltipass.memmgmt.clonedCurServiceNodes[i].tempPrevAddress != null)
		{
			//console.log(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : prev address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempPrevAddress]);
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurServiceNodes[i].data, free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempPrevAddress]);
		}
		if(mooltipass.memmgmt.clonedCurServiceNodes[i].tempNextAddress != null)
		{
			//console.log(mooltipass.memmgmt.clonedCurServiceNodes[i].name + " : next address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempNextAddress]);	
			mooltipass.memmgmt.changeNextAddress(mooltipass.memmgmt.clonedCurServiceNodes[i].data, free_addresses_vector[mooltipass.memmgmt.clonedCurServiceNodes[i].tempNextAddress]);		
		}
	}
	for(var i = 0; i < mooltipass.memmgmt.clonedCurLoginNodes.length; i++)
	{
		if(mooltipass.memmgmt.clonedCurLoginNodes[i].tempAddress != null)
		{
			//console.log(mooltipass.memmgmt.clonedCurLoginNodes[i].name + " : address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempAddress]);
			mooltipass.memmgmt.clonedCurLoginNodes[i].address = free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempAddress];
		}
		if(mooltipass.memmgmt.clonedCurLoginNodes[i].tempPrevAddress != null)
		{
			//console.log(mooltipass.memmgmt.clonedCurLoginNodes[i].name + " : prev address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempPrevAddress]);
			mooltipass.memmgmt.changePrevAddress(mooltipass.memmgmt.clonedCurLoginNodes[i].data, free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempPrevAddress]);
		}
		if(mooltipass.memmgmt.clonedCurLoginNodes[i].tempNextAddress != null)
		{
			//console.log(mooltipass.memmgmt.clonedCurLoginNodes[i].name + " : next address set to " + free_addresses_vector[mooltipass.memmgmt.clonedCurLoginNodes[i].tempNextAddress]);	
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
						console.log("Node data different for parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
						mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurServiceNodes[i].address, mooltipass.memmgmt.clonedCurServiceNodes[i].data);
					}
					else
					{
						//console.log("Node data identical for parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
					}
					break;
				}
			}
			
			// Check if we found the node
			if(parent_node_found == false)
			{
				console.log("Adding new parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
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
						console.log("Node data different for child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
						mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurLoginNodes[i].address, mooltipass.memmgmt.clonedCurLoginNodes[i].data);
					}
					else
					{
						//console.log("Node data identical for child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
					}
					break;
				}
			}
			
			// Check if we found the node
			if(child_node_found == false)
			{
				console.log("Adding new child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
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
			//console.log("Favorite " + i + " is empty");
			// Check that the same favorite is also empty on our DB
			if(!mooltipass.memmgmt.isSameFavoriteAddress(mooltipass.memmgmt.favoriteAddresses[i], [0,0,0,0]))
			{
				console.log("However favorite " + i + " isn't empty on our MP... deleting it");
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
				//console.log("Favorite " + i + " : " + mooltipass.memmgmt.importedCurLoginNodes[imported_child_node_index].name + " on " + mooltipass.memmgmt.importedCurServiceNodes[imported_parent_node_index].name);
				
				// Try to find the same parent in our DB
				var cur_parent_node_index = mooltipass.memmgmt.findIdByName(mooltipass.memmgmt.clonedCurServiceNodes, mooltipass.memmgmt.importedCurServiceNodes[imported_parent_node_index].name);
				
				if(cur_parent_node_index == null)
				{
					console.log("Favorite " + i + " : couldn't find parent node " + mooltipass.memmgmt.importedCurServiceNodes[imported_parent_node_index].name);
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
								var actual_favorite_address = [mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].address[0], mooltipass.memmgmt.clonedCurServiceNodes[cur_parent_node_index].address[1], mooltipass.memmgmt.clonedCurLoginNodes[imported_child_node_index].address[0], mooltipass.memmgmt.clonedCurLoginNodes[imported_child_node_index].address[1]];
								
								// Compare favorite addresses
								if(!mooltipass.memmgmt.isSameFavoriteAddress(actual_favorite_address, mooltipass.memmgmt.favoriteAddresses[i]))
								{
									console.log("Different address for favorite " + i + " : " + mooltipass.memmgmt.importedCurLoginNodes[imported_child_node_index].name + " on " + mooltipass.memmgmt.importedCurServiceNodes[imported_parent_node_index].name + " : " + mooltipass.memmgmt.favoriteAddresses[i] + " instead of " + actual_favorite_address);
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
							console.log("Favorite " + i + " : couldn't find child node at address " + child_node_address);
							mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], [i, 0, 0, 0, 0]));
							break;
						}
					}
					
					if(child_node_found == false)
					{
						// Couldn't find child node
						console.log("Favorite " + i + " : couldn't find child node " + mooltipass.memmgmt.importedCurLoginNodes[imported_child_node_index].name + " on " + mooltipass.memmgmt.importedCurServiceNodes[imported_parent_node_index].name);
						mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], [i, 0, 0, 0, 0]));
					}
				}
			}
			else
			{
				// Reset favorite
				console.log("Favorite " + i + " is incorrect");
				mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], [i, 0, 0, 0, 0]));
			}
		}
	}
} 
 
// Data received from USB callback
mooltipass.memmgmt.dataReceivedCallback = function(packet)
{
	// If it is a leave memory management mode packet, process the queue and exit
	if(packet[1] == mooltipass.device.commands['endMemoryManagementMode'])
	{		 
		// Did we succeed?
		if(packet[2] == 1)
		{
			if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_FAIL || mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_FAIL)
			{
				// Something wrong happened during parameter loading, temp string contains more details
				mooltipass.memmgmt.memmgmtStartCallback({'success': false, 'msg': mooltipass.memmgmt.tempCallbackErrorString}, null);
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE || mooltipass.memmgmt.currentMode == MGMT_IDLE)
			{
				mooltipass.memmgmt.memmgmtStopCallback({'success': true, 'msg': "Memory management mode exit"});
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE_NO_CHANGES)
			{
				mooltipass.memmgmt.memmgmtSaveCallback({'success': true, 'msg': "No changes were applied"});
			}
			else if(mooltipass.memmgmt.currentMode == MGMT_USER_CHANGES_PACKET_SENDING)
			{
				mooltipass.memmgmt.memmgmtSaveCallback({'success': true, 'msg': "Changes were applied"});				
			}
			mooltipass.memmgmt.currentMode = MGMT_IDLE;
			console.log("Memory management mode exit");
			mooltipass.device.processQueue();
			return;
		}
		else
		{
			if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE || mooltipass.memmgmt.currentMode == MGMT_IDLE)
			{
				mooltipass.memmgmt.memmgmtStopCallback({'success': false, 'msg': "Memory management mode exit fail"});
			}
			console.log("This shouldn't happen!");
		}
	}
	else if(packet[1] == mooltipass.device.commands['debug'])
	{
		console.log("Debug: " + mooltipass.util.arrayToStr(packet.subarray(2, packet.length)));
	}
	 
	if(mooltipass.memmgmt.currentMode == MGMT_PASSWORD_REQ)
	{
		if(packet[1] == mooltipass.device.commands['setContext'])
		{
			if(packet[2] == 0)
			{
				// Fail
				console.log("Set context fail");
				mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;
				mooltipass.memmgmt.getPasswordCallback({'success': false, 'msg': "Context invalid"}, "not valid");
			}
			else
			{
				// Set login
				// TO CHANGE ON MOOLTIPASS V2, to SET LOGIN DIRECTLY
				//mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['setLogin'], mooltipass.util.strToArray(mooltipass.memmgmt.getPasswordLogin));
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getLogin'], null);				
				mooltipass.memmgmt_hid._sendMsg();
			}
		}
		else if(packet[1] == mooltipass.device.commands['getLogin'])
		{
			if(packet[2] == 0)
			{
				// Fail
				console.log("Get login fail");
				mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;
				mooltipass.memmgmt.getPasswordCallback({'success': false, 'msg': "User didn't select a login"}, "not valid");
			}
			else
			{
				// Get password
				if(mooltipass.memmgmt.getPasswordLogin != mooltipass.util.arrayToStr(packet.subarray(2, 2 + packet[0])))
				{
					console.log("Wrong login selected");
					mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE;
					mooltipass.memmgmt.getPasswordCallback({'success': false, 'msg': "User didn't select the right login"}, "not valid");
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
				mooltipass.memmgmt.getPasswordCallback({'success': false, 'msg': "Login invalid"}, "not valid");
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
			if(packet[2] == 0)
			{
				// Fail
				console.log("Get password fail");
				mooltipass.memmgmt.getPasswordCallback({'success': false, 'msg': "Request denied"}, "not valid");
			}
			else
			{
				mooltipass.memmgmt.getPasswordCallback({'success': true, 'msg': "Request approved"}, mooltipass.util.arrayToStr(packet.subarray(2, 2 + packet[0])));
			}
		}
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_DB_FILE_MERGE_GET_FREE_ADDR)
	{
		if(packet[1] == mooltipass.device.commands['getFreeSlotAddresses'])
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
			
			console.log("Received " + packet[0]/2 + " free addresses, current total: " + mooltipass.memmgmt.totalAddressesReceived);			
			// Check if we need to receive more address
			if(mooltipass.memmgmt.totalAddressesReceived >= mooltipass.memmgmt.totalAddressesRequired)
			{
				mooltipass.memmgmt.generateMergePackets();
				
				// Now check if actually have something to change
				if(mooltipass.memmgmt.packetToSendBuffer.length == 0)
				{
					console.log("Mooltipass already synced with our credential file!");
					// Leave mem management mode				
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
					mooltipass.memmgmt_hid._sendMsg();
				}
				else
				{
					console.log("Sending merging packets");
					mooltipass.memmgmt.currentMode = MGMT_DB_FILE_MERGE_PACKET_SENDING;
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
					mooltipass.memmgmt_hid._sendMsg();
				}
			}
			else
			{
				// Ask more addresses
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getFreeSlotAddresses'], mooltipass.memmgmt.lastFreeAddressReceived);
				mooltipass.memmgmt_hid._sendMsg();	
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
				//console.log(mooltipass.memmgmt.packetToSendBuffer[0]);
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
			console.log("Packet fail!!! ");
			console.log(mooltipass.memmgmt.packetToSendBuffer[0]);
		}
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK_REQ || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_REQ || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE_REQ || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_MEM_BACKUP_REQ)
	{
		if(packet[1] == mooltipass.device.commands['getMooltipassParameter'])
		{
			// We received the user interaction timeout, use it for our packets timeout
			console.log("Mooltipass interaction timeout is " + packet[2] + " seconds");
			
			if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE_REQ)
			{
				// If we want to do a merge operation, we need to check that the CTR is known			
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getCurrentCardCPZ'], null);
				mooltipass.memmgmt_hid.request.milliseconds = (packet[2]) * 2000;
				mooltipass.memmgmt_hid.nbSendRetries = 3;
				mooltipass.memmgmt_hid._sendMsg();
			}
			else
			{
				// Create packet to go into memmgmt mode
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['startMemoryManagementMode'], null);
				mooltipass.memmgmt_hid.request.milliseconds = (packet[2]) * 2000;
				mooltipass.memmgmt_hid.nbSendRetries = 3;
				mooltipass.memmgmt_hid._sendMsg();
			}			 
		}
		else if(packet[1] == mooltipass.device.commands['getCurrentCardCPZ'])
		{
			if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE_REQ)
			{
				// Was it a success
				if(packet[0] == 1)
				{
					console.log("No card inserted in the Mooltipass!");
					mooltipass.memmgmt.currentMode = MGMT_IDLE;
				}
				else
				{
					// We just received the current card CPZ, check that we know it before proceeding to the merge
					if(mooltipass.memmgmt.isCPZValueKnownInCPZCTRVector(packet.subarray(2, 2 + packet[0]), mooltipass.memmgmt.importedCPZCTRValues))
					{
						mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['startMemoryManagementMode'], null);
						mooltipass.memmgmt_hid.request.milliseconds = (packet[2]) * 2000;
						mooltipass.memmgmt_hid.nbSendRetries = 3;
						mooltipass.memmgmt_hid._sendMsg();					
					}
					else
					{
						console.log("Inserted card isn't known by the save file!!!");
						mooltipass.memmgmt.currentMode = MGMT_IDLE;
					}
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
				if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK_REQ)
				{
					mooltipass.memmgmt.currentMode = MGMT_IDLE;
					mooltipass.memmgmt.loadMemoryParamsStart(MGMT_PARAM_LOAD_INT_CHECK);	
				}
				else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_REQ)
				{
					mooltipass.memmgmt.currentMode = MGMT_IDLE;
					mooltipass.memmgmt.loadMemoryParamsStart(MGMT_PARAM_LOAD);	
				}
				else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE_REQ)
				{
					mooltipass.memmgmt.currentMode = MGMT_IDLE;
					mooltipass.memmgmt.loadMemoryParamsStart(MGMT_PARAM_LOAD_DBFILE_MERGE);	 
				}
				else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_MEM_BACKUP_REQ)
				{
					mooltipass.memmgmt.currentMode = MGMT_IDLE;
					mooltipass.memmgmt.loadMemoryParamsStart(MGMT_PARAM_LOAD_MEM_BACKUP);					
				}
				console.log("Memory management mode entered");
			}
			else
			{
				if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_REQ)
				{
					mooltipass.memmgmt.memmgmtStartCallback({'success': false, 'msg': "User denied memory management"}, null);
				}
				console.log("Couldn't enter memory management mode");
				mooltipass.memmgmt.currentMode = MGMT_IDLE;
				mooltipass.device.processQueue();
			}
		}	
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_MEM_BACKUP)
	{
		// Parameter loading
		if(packet[1] == mooltipass.device.commands['getVersion'])
		{
			mooltipass.memmgmt.nbMb = packet[2];
			console.log("Mooltipass is " + mooltipass.memmgmt.nbMb + "Mb");			
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getCTR'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getCTR'])
		{
			if(packet[0] == 1)
			{
				if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD)
				{
					mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_FAIL;
					mooltipass.memmgmt.tempCallbackErrorString = "Error during CTR request (card removed?)";
				}
				// Request failed, leave memory management mode
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
				mooltipass.memmgmt_hid._sendMsg();
				return;
			}
			mooltipass.memmgmt.ctrValue = packet.subarray(2, 2 + packet[0]);
			console.log("Mooltipass CTR value is " + mooltipass.memmgmt.ctrValue);			
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getCPZandCTR'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['exportCPZandCTR'])
		{
			// CPZ CTR packet export
			mooltipass.memmgmt.CPZCTRValues.push(packet.subarray(2, 2 + packet[0]));
			console.log("CPZ CTR packet export packet received: " + packet.subarray(2, 2 + packet[0])); 
			// Arm receive
			mooltipass.memmgmt_hid.receiveMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getCPZandCTR'])
		{
			if(packet[2] == 0)
			{
				if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD)
				{
					mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_FAIL;
					mooltipass.memmgmt.tempCallbackErrorString = "Error during CPZ/CTR request (card removed?)";
				}
				// Request failed, leave memory management mode
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
				mooltipass.memmgmt_hid._sendMsg();
				return;
			}
			// Inform that all CPZ CTR packets were sent
			console.log("All CPZ CTR packets are received");			
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getStartingParentAddress'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getStartingParentAddress'])
		{
			if(packet[0] == 1)
			{
				if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD)
				{
					mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_FAIL;
					mooltipass.memmgmt.tempCallbackErrorString = "Error during get starting parent request (card removed?)";
				}
				// Request failed, leave memory management mode
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
				mooltipass.memmgmt_hid._sendMsg();
				return;
			}
			mooltipass.memmgmt.startingParent = [packet[2], packet[3]];
			mooltipass.memmgmt.clonedStartingParent = [packet[2], packet[3]];
			console.log("Starting parent is " + mooltipass.memmgmt.startingParent); 
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getStartingDataParentAddress'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getStartingDataParentAddress'])
		{
			if(packet[0] == 1)
			{
				if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD)
				{
					mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_FAIL;
					mooltipass.memmgmt.tempCallbackErrorString = "Error during get data starting parent request (card removed?)";
				}
				// Request failed, leave memory management mode
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
				mooltipass.memmgmt_hid._sendMsg();
				return;
			}
			mooltipass.memmgmt.favoriteAddresses = [];
			mooltipass.memmgmt.clonedFavoriteAddresses = [];
			mooltipass.memmgmt.dataStartingParent = [packet[2], packet[3]];
			console.log("Data starting parent is " + mooltipass.memmgmt.dataStartingParent);
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getFavorite'], [mooltipass.memmgmt.currentFavorite]);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getFavorite'])
		{
			if(packet[0] == 1)
			{
				if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD)
				{
					mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_FAIL;
					mooltipass.memmgmt.tempCallbackErrorString = "Error during get favorite request (card removed?)";
				}
				// Request failed, leave memory management mode
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
				mooltipass.memmgmt_hid._sendMsg();
				return;
			}
			// Extract addresses and append them to our current ones
			mooltipass.memmgmt.favoriteAddresses.push(packet.subarray(2, 2 + 4));	
			mooltipass.memmgmt.clonedFavoriteAddresses.push(packet.subarray(2, 2 + 4));			
			// Check if we have done all favorites
			if(++mooltipass.memmgmt.currentFavorite == 14)
			{
				console.log("Favorites loaded: " + mooltipass.memmgmt.favoriteAddresses);
				mooltipass.memmgmt.packetToSendBuffer = [];
				mooltipass.memmgmt.curServiceNodes = [];				
				mooltipass.memmgmt.curLoginNodes = [];				
				mooltipass.memmgmt.curDataServiceNodes = [];		
				mooltipass.memmgmt.curDataNodes = [];	
				mooltipass.memmgmt.clonedCurServiceNodes = [];				
				mooltipass.memmgmt.clonedCurLoginNodes = [];				
				mooltipass.memmgmt.clonedCurDataServiceNodes = [];		
				mooltipass.memmgmt.clonedCurDataNodes = []; 
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
					// Follow the nodes, starting with the parent one
					mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN;
					mooltipass.memmgmt.curNodeAddressRequested = mooltipass.memmgmt.startingParent;
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.memmgmt.startingParent);
					mooltipass.memmgmt_hid._sendMsg();
				}
				else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_DBFILE_MERGE)
				{
					// Follow the nodes, starting with the parent one
					mooltipass.memmgmt.currentMode = MGMT_DBFILE_MERGE_NORMAL_SCAN;
					mooltipass.memmgmt.curNodeAddressRequested = mooltipass.memmgmt.startingParent;
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.memmgmt.startingParent);
					mooltipass.memmgmt_hid._sendMsg();
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
				//console.log("Node received: " + mooltipass.memmgmt.currentNode);
				mooltipass.memmgmt.nodePacketId = 0;
				 
				// Parse node
				if(mooltipass.memmgmt.isNodeValid(mooltipass.memmgmt.currentNode))
				{
					// Get node type
					var nodeType = mooltipass.memmgmt.getNodeType(mooltipass.memmgmt.currentNode);
					 
					if(nodeType == 'parent')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curServiceNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0)});
						mooltipass.memmgmt.clonedCurServiceNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0)});
						console.log("Received service " + mooltipass.memmgmt.curServiceNodes[mooltipass.memmgmt.curServiceNodes.length - 1].name + " at address " + mooltipass.memmgmt.curServiceNodes[mooltipass.memmgmt.curServiceNodes.length - 1].address);
						 
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
									mooltipass.memmgmt.memmgmtStartCallback({'success': true, 'msg': "Credential listing done"}, mooltipass.memmgmt.credentialArrayForGui);
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
										mooltipass.memmgmt.generateMergePackets();
										if(mooltipass.memmgmt.packetToSendBuffer.length == 0)
										{
											console.log("Mooltipass already synced with our credential file!");											
											// Leave mem management mode				
											mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
											mooltipass.memmgmt_hid._sendMsg();
										}
										else
										{
											console.log("Sending merging packets");
											mooltipass.memmgmt.currentMode = MGMT_DB_FILE_MERGE_PACKET_SENDING;
											mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
											//console.log(mooltipass.memmgmt.packetToSendBuffer[0]);
											mooltipass.memmgmt_hid._sendMsg();
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
						mooltipass.memmgmt.curLoginNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0), 'pointed': false});
						mooltipass.memmgmt.clonedCurLoginNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0), 'pointed': false});
						mooltipass.memmgmt.credentialArrayForGui.push({	'context': mooltipass.memmgmt.clonedCurServiceNodes[mooltipass.memmgmt.clonedCurServiceNodes.length-1].name,
																		'username': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode),
																		'address': mooltipass.memmgmt.curNodeAddressRequested,
																		'description': mooltipass.memmgmt.getDescription(mooltipass.memmgmt.currentNode),
																		'date_modified': mooltipass.memmgmt.getDateCreated(mooltipass.memmgmt.currentNode),
																		'date_lastused': mooltipass.memmgmt.getDateLastUsed(mooltipass.memmgmt.currentNode),
																		'favorite': mooltipass.memmgmt.isParentChildAFavorite(mooltipass.memmgmt.clonedCurServiceNodes[mooltipass.memmgmt.clonedCurServiceNodes.length-1].address, mooltipass.memmgmt.curNodeAddressRequested),
																		'parent_address': mooltipass.memmgmt.clonedCurServiceNodes[mooltipass.memmgmt.clonedCurServiceNodes.length-1].address
																		});
						
						console.log("Received login " + mooltipass.memmgmt.curLoginNodes[mooltipass.memmgmt.curLoginNodes.length - 1].name + " at address " + mooltipass.memmgmt.curLoginNodes[mooltipass.memmgmt.curLoginNodes.length - 1].address);
						 
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
									mooltipass.memmgmt.memmgmtStartCallback({'success': true, 'msg': "Credential listing done"}, mooltipass.memmgmt.credentialArrayForGui);
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
										mooltipass.memmgmt.generateMergePackets();
										if(mooltipass.memmgmt.packetToSendBuffer.length == 0)
										{
											console.log("Mooltipass already synced with our credential file!");											
											// Leave mem management mode				
											mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
											mooltipass.memmgmt_hid._sendMsg();
										}
										else
										{
											console.log("Sending merging packets");
											mooltipass.memmgmt.currentMode = MGMT_DB_FILE_MERGE_PACKET_SENDING;
											mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
											//console.log(mooltipass.memmgmt.packetToSendBuffer[0]);
											mooltipass.memmgmt_hid._sendMsg();
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
								//console.log(mooltipass.memmgmt.credentialArrayForGui);
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
						mooltipass.memmgmt.curDataServiceNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0)});
						mooltipass.memmgmt.clonedCurDataServiceNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0)});
					}
					else if(nodeType == 'data')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curDataNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'data': mooltipass.memmgmt.currentNode.slice(0), 'pointed': false});
						mooltipass.memmgmt.clonedCurDataNodes.push({'address': mooltipass.memmgmt.curNodeAddressRequested, 'data': mooltipass.memmgmt.currentNode.slice(0), 'pointed': false});
					}
				}
				else
				{
					// Well this isn't a good situation... we read an empty node
					console.log("Empty node!!!");	
					// TODO: inform the user the memory is corrupted
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
			console.log("Not allowed to read node !!!");			 
			if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN)
			{
				mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_FAIL;
				mooltipass.memmgmt.tempCallbackErrorString = "Error during listing credentials (card removed/memory corrupted?)";
			}	   
			// Leave memory management mode
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_INT_CHECK_SCAN)
	{
		// compute completion percentage
		var tempCompletion = Math.round(((mooltipass.memmgmt.pageIt-128)/(mooltipass.memmgmt.getNumberOfPages(mooltipass.memmgmt.nbMb)-128))*100);
		if(tempCompletion != mooltipass.memmgmt.scanPercentage)
		{
			mooltipass.memmgmt.scanPercentage = tempCompletion;
			console.log(tempCompletion + "%");
		}
		 
		// check if we actually could read the node (permission problems....)
		if(packet[0] > 1)
		{
			// extend current node
			mooltipass.memmgmt.currentNode.set(packet.subarray(2, 2 + packet[0]), mooltipass.memmgmt.nodePacketId*HID_PAYLOAD_SIZE);
			 
			// check if is the last packet for a given node
			if(++mooltipass.memmgmt.nodePacketId == 3)
			{
				//console.log("Node received: " + mooltipass.memmgmt.currentNode);
				mooltipass.memmgmt.nodePacketId = 0;
				 
				// Parse node
				if(mooltipass.memmgmt.isNodeValid(mooltipass.memmgmt.currentNode))
				{
					// Get node type
					var nodeType = mooltipass.memmgmt.getNodeType(mooltipass.memmgmt.currentNode);
					 
					if(nodeType == 'parent')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curServiceNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0)});
						mooltipass.memmgmt.clonedCurServiceNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0)});
					}
					else if(nodeType == 'child')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curLoginNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0), 'pointed': false});
						mooltipass.memmgmt.clonedCurLoginNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0), 'pointed': false});
					}
					else if(nodeType == 'dataparent')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curDataServiceNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0)});
						mooltipass.memmgmt.clonedCurDataServiceNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0)});
					}
					else if(nodeType == 'data')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curDataNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'data': mooltipass.memmgmt.currentNode.slice(0), 'pointed': false});
						mooltipass.memmgmt.clonedCurDataNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'data': mooltipass.memmgmt.currentNode.slice(0), 'pointed': false});
					}
				}
				else
				{
					// Free slot
				}
				 
				// Check that we didn't finish the scanning, ask next node
				//if(((mooltipass.memmgmt.nodeIt + 1) == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb) && (mooltipass.memmgmt.pageIt + 1) == mooltipass.memmgmt.getNumberOfPages(mooltipass.memmgmt.nbMb)) || false)//(mooltipass.memmgmt.scanPercentage == 5))
				if(((mooltipass.memmgmt.nodeIt + 1) == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb) && (mooltipass.memmgmt.pageIt + 1) == mooltipass.memmgmt.getNumberOfPages(mooltipass.memmgmt.nbMb)) || (mooltipass.memmgmt.scanPercentage == 5))
				{
					mooltipass.memmgmt.integrityCheck();
					// Check if there are changes to do or not
					if(mooltipass.memmgmt.packetToSendBuffer.length > 0)
					{
						// Changes to make, change mode, start sending baby!
						mooltipass.memmgmt.currentMode = MGMT_INT_CHECK_PACKET_SENDING;
						//console.log(mooltipass.memmgmt.packetToSendBuffer[0]);
						mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
						mooltipass.memmgmt_hid._sendMsg();
					}
					else
					{
						// No changes, exit memory management mode
						console.log("Memory OK, no changes to make!");
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
			//console.log("Not allowed to read node");
			 
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
					//console.log(mooltipass.memmgmt.packetToSendBuffer[0]);
					mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
					mooltipass.memmgmt_hid._sendMsg();
				}
				else
				{
					// No changes, exit memory management mode
					console.log("Memory OK, no changes to make!");
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
 
// Init global vars for memory mgmt
mooltipass.memmgmt.loadMemoryParamsStart = function(wanted_mode)
{
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		// Start process by requesting the memory size
		mooltipass.memmgmt.currentMode = wanted_mode;
		mooltipass.memmgmt.currentFavorite = 0;
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getVersion'], null);
		mooltipass.memmgmt_hid._sendMsg();
		return true;
	}
	else
	{
		return false;
	}
}

// Data received from USB callback, for media bundle related comms
mooltipass.memmgmt.mediaBundleDataReceivedCallback = function(packet)
{
	if(packet[1] == mooltipass.device.commands['startMediaImport'])
	{
		// Answer to start media import packet
		mooltipass.memmgmt.tempPassword = null;
		if(packet[2] == 0)
		{
			// Fail
			mooltipass.memmgmt.currentMode = MGMT_IDLE;
			console.log("Media import start fail!");
		}
		else
		{
			// Epic win, send first data packet
			mooltipass.memmgmt.currentMode = MGMT_BUNDLE_UPLOAD;
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['mediaImport'], mooltipass.memmgmt.mediaBundle.subarray(0, MEDIA_BUNDLE_CHUNK_SIZE));
			mooltipass.memmgmt.byteCounter += MEDIA_BUNDLE_CHUNK_SIZE;
			mooltipass.memmgmt_hid.request['milliseconds'] = 2000;
			mooltipass.memmgmt_hid.nbSendRetries = 3;
			mooltipass.memmgmt_hid._sendMsg();
			console.log("Media import has started, please wait a few minutes...");
		}
	}
	else if(packet[1] == mooltipass.device.commands['mediaImport'])
	{
		// Acknowledge of previous packet send
		if(packet[2] == 0)
		{
			// Fail... we kind of are stuck here...
			mooltipass.memmgmt.currentMode = MGMT_IDLE;
			console.log("Media import fail!");
		}
		else
		{
			// Check how many bytes we need to send
			var nb_bytes_to_send = MEDIA_BUNDLE_CHUNK_SIZE;			
			if(mooltipass.memmgmt.mediaBundle.length - mooltipass.memmgmt.byteCounter < MEDIA_BUNDLE_CHUNK_SIZE)
			{
				nb_bytes_to_send = mooltipass.memmgmt.mediaBundle.length - mooltipass.memmgmt.byteCounter;
			}
			
			if(nb_bytes_to_send == 0)
			{
				// We finished sending data
				console.log("Last packet sent!");
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMediaImport'], null);
				mooltipass.memmgmt_hid._sendMsg();
			}
			else
			{
				// We still have data to send
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['mediaImport'], mooltipass.memmgmt.mediaBundle.subarray(mooltipass.memmgmt.byteCounter, mooltipass.memmgmt.byteCounter + nb_bytes_to_send));
				mooltipass.memmgmt.byteCounter += nb_bytes_to_send;	
				mooltipass.memmgmt_hid._sendMsg();		
				//console.log(mooltipass.memmgmt.mediaBundle.length - mooltipass.memmgmt.byteCounter);
			}			
		}
	}
	else if(packet[1] == mooltipass.device.commands['endMediaImport'])
	{
		mooltipass.memmgmt.currentMode = MGMT_IDLE;
		if(packet[2] == 0)
		{
			// Fail... we kind of are stuck here...
			console.log("Media import end fail!");
		}
		else
		{
			console.log("Media import end win!");			
		}
	}
}

// Media bundle read callback
mooltipass.memmgmt.mediaBundleReadCallback = function(e)
{
	console.log("Media bundle read event...");
	 
	if(e.type == "loadend" && mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		// Change state
		mooltipass.memmgmt.currentMode = MGMT_BUNDLE_UPLOAD_REQ;
		
		// Init vars
		mooltipass.memmgmt.byteCounter = 0;		
		mooltipass.memmgmt.mediaBundle = new Uint8Array(e.target.result);
		console.log("Media bundle read, " + mooltipass.memmgmt.mediaBundle.length + " bytes long");
		
		// Set the timeouts & callbacks then send a media import start packet
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['startMediaImport'], mooltipass.memmgmt.tempPassword);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.mediaBundleDataReceivedCallback;
		mooltipass.memmgmt_hid.request['milliseconds'] = 20000;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();
	}	
}

// Media bundle upload
mooltipass.memmgmt.mediaBundlerUpload = function(password)
{
	// Check password length
	if(password.length != 124)
	{
		console.log("Wrong password length!");
		return;
	}
	
	// Convert the password
	mooltipass.memmgmt.tempPassword = new Uint8Array(62);
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
	//console.log(items);
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
			console.log("Preferences storage: No preferences stored!");
			mooltipass.prefstorage.setStoredPreferences(mooltipass.memmgmt.preferences);
		}
		else
		{
			console.log("Preferences storage: Loaded preferences!");
			mooltipass.memmgmt.preferences = items;
			console.log(mooltipass.memmgmt.preferences);
		}
	}
}
 
// Memory management mode start
mooltipass.memmgmt.memmgmtStart = function(callback) 
{
	// Store the callback function
	mooltipass.memmgmt.memmgmtStartCallback = callback;
	mooltipass.prefstorage.getStoredPreferences(mooltipass.memmgmt.preferencesCallback);
	mooltipass.filehandler.getSyncableFileSystemStatus(mooltipass.memmgmt.syncableFSStateCallback);
	mooltipass.filehandler.setSyncFSStateChangeCallback(mooltipass.memmgmt.syncableFSStateChangeCallback);
	
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_REQ;
		// First step is to query to user interaction timeout to set the correct packet timeout retry!
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();
	}
	else
	{
		mooltipass.memmgmt.memmgmtStartCallback({'success': false, 'msg': "Memory management in another mode"}, null);
	}
}

// Memory management mode start
mooltipass.memmgmt.memmgmtStop = function(callback)
{
	mooltipass.memmgmt.memmgmtStopCallback = callback;
	
	if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE || mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		// Leave memory management mode
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
		mooltipass.memmgmt_hid._sendMsg();
	}
	else
	{
		mooltipass.memmgmt.memmgmtStopCallback({'success': false, 'msg': "Memory management in another mode"});
	}
}

// Memory management mode save
mooltipass.memmgmt.memmgmtSave = function(callback, deleteData, updateData, addData)
{
	if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN_DONE)
	{
		// Save callback
		mooltipass.memmgmt.memmgmtSaveCallback = callback;
		
		// Tackling the updated items
		console.log("");
		console.log("Treating updated items...");
		for(var i = 0; i < updateData.length; i++)
		{
			// Check child & parent addresses
			var child_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, updateData[i].address);
			var parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, updateData[i].parent_address);
			
			if(child_index != null && parent_index != null)
			{
				// Check modified date
				if(mooltipass.memmgmt.getDateCreated(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data) != updateData[i].date_modified)
				{					
					console.log("User changed date modified from " + mooltipass.memmgmt.getDateCreated(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data) + " to " + updateData[i].date_modified);
					mooltipass.memmgmt.setDateCreated(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, updateData[i].date_modified);
				}
				// Check last used date
				if(mooltipass.memmgmt.getDateLastUsed(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data) != updateData[i].date_lastused)
				{					
					console.log("User changed date last used from " + mooltipass.memmgmt.getDateLastUsed(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data) + " to " + updateData[i].date_lastused);
					mooltipass.memmgmt.setDateLastUsed(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, updateData[i].date_lastused);
				}
				// Check if the description has been changed
				if(mooltipass.memmgmt.getDescription(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data) != updateData[i].description)
				{					
					console.log("User changed child node description from " + mooltipass.memmgmt.getDescription(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data) + " to " + updateData[i].description);
					mooltipass.memmgmt.setDescription(mooltipass.memmgmt.clonedCurLoginNodes[child_index].data, updateData[i].description);
				}
				// Check if the login name has been changed
				if(mooltipass.memmgmt.clonedCurLoginNodes[child_index].name != updateData[i].username)
				{
					console.log("User changed child node name from " + mooltipass.memmgmt.clonedCurLoginNodes[child_index].name + " to " + updateData[i].username);
				}
				// Check if the parent name has been changed
				if(mooltipass.memmgmt.clonedCurServiceNodes[parent_index].name != updateData[i].context)
				{
					console.log("User changed parent node name from " + mooltipass.memmgmt.clonedCurServiceNodes[parent_index].name + " to " + updateData[i].context);					
				}
				// Check if password is set
				if('password' in updateData[i])
				{
					console.log("New password is set: " + updateData[i].password);
				}
			}
			else
			{
				// We were fed incorrect data
				mooltipass.memmgmt.memmgmtSaveCallback({'success': false, 'msg': "Data provided is invalid!"});
				console.log("Error with following update data:");
				console.log(updateData[i]);
				return;
			}
		}
		
		// Tackling the deleted items
		console.log("Treating deleted items...");
		for(var i = 0; i < deleteData.length; i++)
		{
			// Check child & parent addresses
			var child_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurLoginNodes, deleteData[i].address);
			var parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.clonedCurServiceNodes, deleteData[i].parent_address);
			
			if(child_index != null && parent_index != null)
			{
				// See how many children the parent has so we know if we should delete it
				var number_of_children = mooltipass.memmgmt.getNumberOfChildrenForClonedServiceNode(mooltipass.memmgmt.clonedCurServiceNodes[parent_index]);
				
				if(number_of_children == 1)
				{
					// Delete parent node
					mooltipass.memmgmt.deleteServiceNodeFromCloneArrayAndGenerateDeletePacket(mooltipass.memmgmt.clonedCurServiceNodes[parent_index]);
				}
				
				// Delete child node
				mooltipass.memmgmt.deleteChildNodeFromCloneArrayAndGenerateDeletePacket(mooltipass.memmgmt.clonedCurLoginNodes[child_index]);
			}
			else
			{
				// We were fed incorrect data
				mooltipass.memmgmt.memmgmtSaveCallback({'success': false, 'msg': "Data provided is invalid!"});
				console.log("Error with following delete data:");
				console.log(deleteData[i]);
				return;
			}
		}
		
		// Now we can do the difference between our local buffer and what is on the mooltipass
		for(var i = 0; i < mooltipass.memmgmt.clonedCurServiceNodes.length; i++)
		{
			// To be changed
			if(true)
			{
				// Try to find the same node in the current mooltipass memory contents
				var parent_node_address = mooltipass.memmgmt.clonedCurServiceNodes[i].address;
				var parent_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.curServiceNodes, parent_node_address);
				
				if(parent_index == null)
				{
					// We don't know the node, add it
					console.log("Adding new parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
					mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurServiceNodes[i].address, mooltipass.memmgmt.clonedCurServiceNodes[i].data);					
				}
				else
				{
					// Check if data is unchanged
					if(!mooltipass.memmgmt.compareNodeData(mooltipass.memmgmt.clonedCurServiceNodes[i], mooltipass.memmgmt.curServiceNodes[parent_index]))
					{
						console.log("Node data different for parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
						mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurServiceNodes[i].address, mooltipass.memmgmt.clonedCurServiceNodes[i].data);
					}
					else
					{
						//console.log("Node data identical for parent " + mooltipass.memmgmt.clonedCurServiceNodes[i].name);
					}					
				}
			}
		}
		for(var i = 0; i < mooltipass.memmgmt.clonedCurLoginNodes.length; i++)
		{
			// To be changed
			if(true)
			{
				// Try to find the same node in the current mooltipass memory contents
				var child_node_address = mooltipass.memmgmt.clonedCurLoginNodes[i].address;
				var child_index = mooltipass.memmgmt.findIdByAddress(mooltipass.memmgmt.curLoginNodes, child_node_address);
				
				if(child_index == null)
				{
					// We don't know the node, add it
					console.log("Adding new child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
					mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurLoginNodes[i].address, mooltipass.memmgmt.clonedCurLoginNodes[i].data);					
				}
				else
				{
					// Check if data is unchanged
					if(!mooltipass.memmgmt.compareNodeData(mooltipass.memmgmt.clonedCurLoginNodes[i], mooltipass.memmgmt.curLoginNodes[child_index]))
					{
						console.log("Node data different for child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
						mooltipass.memmgmt.addWriteNodePacketToSendBuffer(mooltipass.memmgmt.clonedCurLoginNodes[i].address, mooltipass.memmgmt.clonedCurLoginNodes[i].data);
					}
					else
					{
						//console.log("Node data identical for child " + mooltipass.memmgmt.clonedCurLoginNodes[i].name);
					}					
				}
			}
		}
		
		// Now check if actually have something to change
		if(mooltipass.memmgmt.packetToSendBuffer.length == 0)
		{
			mooltipass.memmgmt.currentMode = MGMT_NORMAL_SCAN_DONE_NO_CHANGES;
			console.log("Saved button pressed but no changes");
			// Leave mem management mode				
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else
		{
			console.log("Sending updating packets");
			mooltipass.memmgmt.currentMode = MGMT_USER_CHANGES_PACKET_SENDING;
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.memmgmt.packetToSendBuffer[0];
			mooltipass.memmgmt_hid._sendMsg();
		}
	}
	else
	{
		mooltipass.memmgmt.memmgmtSaveCallback({'success': false, 'msg': "Memory management in another mode"});
	}
}
 
// Memory integrity check
mooltipass.memmgmt.integrityCheckStart = function()
{	
	mooltipass.prefstorage.getStoredPreferences(mooltipass.memmgmt.preferencesCallback);
	mooltipass.filehandler.getSyncableFileSystemStatus(mooltipass.memmgmt.syncableFSStateCallback);
	mooltipass.filehandler.setSyncFSStateChangeCallback(mooltipass.memmgmt.syncableFSStateChangeCallback);
	
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_INT_CHECK_REQ;
		// First step is to query to user interaction timeout to set the correct packet timeout retry!
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();
	}
}

// Merge credential file to current database
mooltipass.memmgmt.mergeCredentialFileToMooltipassStart = function()
{
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		// Open the file first
		mooltipass.memmgmt.currentMode = MGMT_DBFILE_MERGE_REQ;
		mooltipass.memmgmt.importMemoryState();
	}
}

// Memory backup start
mooltipass.memmgmt.memoryBackupStart = function()
{
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_MEM_BACKUP_REQ;
		// First step is to query to user interaction timeout to set the correct packet timeout retry!
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();
	}return;
}





