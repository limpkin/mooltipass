var mooltipass = mooltipass || {};
mooltipass.memmgmt = mooltipass.memmgmt || {};

// State machine modes
var MGMT_IDLE 						= 0;			// Idle mode
var MGMT_PARAM_LOAD					= 1;			// Parameter loading
var MGMT_PARAM_LOAD_INT_CHECK		= 2;			// Parameter loading for integrity check
var MGMT_PARAM_LOAD_INT_CHECK_REQ	= 3;			// Parameter loading for integrity check, request
var MGMT_INT_CHECK_SCAN				= 4;			// Scanning through the memory for integrity check

// Mooltipass memory params
mooltipass.memmgmt.nbMb = null;						// Mooltipass memory size
mooltipass.memmgmt.startingParent = null;			// Mooltipass current starting parent
mooltipass.memmgmt.dataStartingParent = null;		// Mooltipass current data starting parrent
mooltipass.memmgmt.favoriteAddresses = [];			// Mooltipass current favorite addresses
mooltipass.memmgmt.curServiceNames = [];			// Mooltipass current service names
mooltipass.memmgmt.curServiceAddresses = [];		// Mooltipass current service addresses
mooltipass.memmgmt.curServiceNodes = [];			// Mooltipass current service nodes
mooltipass.memmgmt.curLoginNames = [];				// Mooltipass current login names
mooltipass.memmgmt.curLoginAddresses = [];			// Mooltipass current login addresses
mooltipass.memmgmt.curLoginNodes = [];				// Mooltipass current login nodes
mooltipass.memmgmt.curDataServiceNames = [];		// Mooltipass current data service names
mooltipass.memmgmt.curDataServiceAddresses = [];	// Mooltipass current data service addresses
mooltipass.memmgmt.curDataServiceNodes = [];		// Mooltipass current data service nodes
mooltipass.memmgmt.curDataNodeAddresses = [];		// Mooltipass current data node addresses
mooltipass.memmgmt.curDataNodes = [];				// Mooltipass current data nodes

// State machines & temp variables
mooltipass.memmgmt.currentMode = MGMT_IDLE;			// Current mode
mooltipass.memmgmt.currentFavorite = 0;				// Current favorite read/write
mooltipass.memmgmt.pageIt = 0;						// Page iterator
mooltipass.memmgmt.nodeIt = 0;						// Node iterator
mooltipass.memmgmt.scanPercentage = 0;				// Scanning percentage
mooltipass.memmgmt.nodePacketId = 0;				// Packet number for node sending/receiving
mooltipass.memmgmt.currentNode = [];				// Current node we're sending/receiving


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
	console.log("Node Type is " + mooltipass.memmgmt.node_types[(node[0] >> 6) & 0x03]);
	return mooltipass.memmgmt.node_types[(node[0] >> 6) & 0x03];
}

// Get user ID
mooltipass.memmgmt.getUserId = function(node)
{
	console.log("User Id is " + (node[0] & 0x1F));
	return (node[0] & 0x1F);
}

// Get previous address
mooltipass.memmgmt.getPrevAddress = function(node)
{
	return [node[2], node[3]];
}

// Get next address
mooltipass.memmgmt.getNextAddress = function(node)
{
	return [node[4], node[5]];
}

// Get first child address
mooltipass.memmgmt.getFirstChildAddress = function(node)
{
	return [node[6], node[7]];
}

// Get next data node address
mooltipass.memmgmt.getNextDataNodeAddress = function(node)
{
	return [node[2], node[3]];
}

// Get service name
mooltipass.memmgmt.getServiceName = function(node)
{
	console.log("Service name is " + mooltipass.util.arrayToStr(node.splice(8, node.length)));
	return mooltipass.util.arrayToStr(node.splice(8, node.length)); 
}

// Get description
mooltipass.memmgmt.getDescription = function(node)
{
	console.log("Description is " + mooltipass.util.arrayToStr(node.splice(6, node.length)));
	return mooltipass.util.arrayToStr(node.splice(6, node.length)); 
}

// Get login
mooltipass.memmgmt.getLogin = function(node)
{
	console.log("Login is " + mooltipass.util.arrayToStr(node.splice(37, node.length)));
	return mooltipass.util.arrayToStr(node.splice(37, node.length)); 
}

// Get date created
mooltipass.memmgmt.getDateCreated = function(node)
{
	var year = ((node[30] >> 1) & 0x7F) + 2010;
	var month = ((node[30] & 0x01) << 3) | ((node[31] >> 5) & 0x07);
	var day = (node[31] & 0x1F);
	console.log("Date created is " + day + "/" + month + "/" + year);
	return new Date(year, month, day, 0, 0, 0, 0);
}

// Get date last used
mooltipass.memmgmt.getDateLastUsed = function(node)
{
	var year = ((node[32] >> 1) & 0x7F) + 2010;
	var month = ((node[32] & 0x01) << 3) | ((node[33] >> 5) & 0x07);
	var day = (node[33] & 0x1F);
	console.log("Date last used is " + day + "/" + month + "/" + year);
	return new Date(year, month, day, 0, 0, 0, 0);
}

// Find if node is valid
mooltipass.memmgmt.isNodeValid = function(node)
{
	if(node[0] & 0x20 == 0x00)
	{
		return true;
	}
	else
	{
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

// Data received callback
mooltipass.memmgmt.dataReceivedCallback = function(packet)
{
	if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK_REQ)
	{
		// Here should be the answer of the go to management mode request
		if(packet[1] == mooltipass.device.commands['startMemoryManagementMode'])
		{
			mooltipass.memmgmt.currentMode = MGMT_IDLE;
			
			// Did we succeed?
			if(packet[2] == 1)
			{
				// Load memory params
				mooltipass.memmgmt.loadMemoryParamsStart(MGMT_PARAM_LOAD_INT_CHECK);	
				console.log("Memory management mode entered");
			}
		}	
	}
	else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK)
	{
		// Parameter loading
		if(packet[1] == mooltipass.device.commands['getVersion'])
		{
			mooltipass.memmgmt.nbMb = packet[2];
			console.log("Mooltipass is " + mooltipass.memmgmt.nbMb + "Mb");
			XXXXXXX
		}
		else if(packet[1] == mooltipass.device.commands['getStartingParentAddress'])
		{
			mooltipass.memmgmt.startingParent = [packet[2], packet[3]];
			console.log("Starting parent is " + mooltipass.memmgmt.startingParent);
			XXXXXXX
		}
		else if(packet[1] == mooltipass.device.commands['getStartingDataParentAddress'])
		{
			mooltipass.memmgmt.dataStartingParent = [packet[2], packet[3]];
			console.log("Data starting parent is " + mooltipass.memmgmt.dataStartingParent);
			XXXXXXX
		}
		else if(packet[1] == mooltipass.device.commands['getFavorite'])
		{
			// Extract addresses and append them to our current ones
			mooltipass.memmgmt.favoriteAddresses.push(packet.splice(2, 2 + 4));			
			// Check if we have done all favorites
			if(++mooltipass.memmgmt.currentFavorite == 14)
			{
				console.log("Favorites loaded: " + mooltipass.memmgmt.favoriteAddresses);
				
				// Depending on the current mode, check what to do next
				if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK)
				{
					// Start looping through all the nodes
					mooltipass.memmgmt.curServiceNames = [];			
					mooltipass.memmgmt.curServiceAddresses = [];		
					mooltipass.memmgmt.curServiceNodes = [];			
					mooltipass.memmgmt.curLoginNames = [];				
					mooltipass.memmgmt.curLoginAddresses = [];			
					mooltipass.memmgmt.curLoginNodes = [];				
					mooltipass.memmgmt.curDataServiceNames = [];		
					mooltipass.memmgmt.curDataServiceAddresses = [];	
					mooltipass.memmgmt.curDataServiceNodes = [];		
					mooltipass.memmgmt.curDataNodeAddresses = [];		
					mooltipass.memmgmt.curDataNodes = [];				
					mooltipass.memmgmt.currentMode = MGMT_INT_CHECK_SCAN;
					mooltipass.memmgmt.scanPercentage = 0;
					mooltipass.memmgmt.nodePacketId = 0;
					mooltipass.memmgmt.currentNode = [];
					mooltipass.memmgmt.pageIt = 128;
					mooltipass.memmgmt.nodeIt = 0;
					XXXXXXX
				}
				else
				{
					// Todo: follow the normal procedure
					XXXXXXX
				}
			}
			else
			{
				// Otherwise, ask for the next one
				XXXXXXX
			}
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
			mooltipass.memmgmt.currentNode.push.apply(mooltipass.memmgmt.currentNode, packet.splice(2, 2 + packet[0]));
			
			// check if is the last packet for a given node
			if(++mooltipass.memmgmt.nodePacketId == 3)
			{
				console.log("Node received: " + mooltipass.memmgmt.currentNode);
				mooltipass.memmgmt.nodePacketId = 0;
				
				// Parse node
				if(mooltipass.memmgmt.isNodeValid(mooltipass.memmgmt.currentNode))
				{
					// Get node type
					var nodeType = mooltipass.memmgmt.getNodeType(mooltipass.memmgmt.currentNode);
					
					if(nodeType == 'parent')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curServiceNames.push(mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode));
						mooltipass.memmgmt.curServiceAddresses.push([(nodei + (pagei << 3)) & 0x00FF, (pagei >> 5) & 0x00FF]);
						mooltipass.memmgmt.curServiceNodes.push(mooltipass.memmgmt.currentNode);
					}
					else if(nodeType == 'child')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curLoginNames.push(mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode));
						mooltipass.memmgmt.curLoginAddresses.push([(nodei + (pagei << 3)) & 0x00FF, (pagei >> 5) & 0x00FF]);
						mooltipass.memmgmt.curLoginNodes.push(mooltipass.memmgmt.currentNode);
					}
					else if(nodeType == 'dataparent')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curDataServiceNames.push(mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode));
						mooltipass.memmgmt.curDataServiceAddresses.push([(nodei + (pagei << 3)) & 0x00FF, (pagei >> 5) & 0x00FF]);
						mooltipass.memmgmt.curDataServiceNodes.push(mooltipass.memmgmt.currentNode);
					}
					else if(nodeType == 'data')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curDataNodeAddresses.push([(nodei + (pagei << 3)) & 0x00FF, (pagei >> 5) & 0x00FF]);
						mooltipass.memmgmt.curDataNodes.push(mooltipass.memmgmt.currentNode);
					}
				}
				
				// Check that we didn't finish the scanning, ask next node
				if((mooltipass.memmgmt.nodeIt + 1) == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb) && (mooltipass.memmgmt.pageIt + 1) == mooltipass.memmgmt.getNumberOfPages(mooltipass.memmgmt.nbMb))
				{
					//
				}
				else
				{
					if(++mooltipass.memmgmt.nodeIt == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb))
					{
						// Changing pages
						mooltipass.memmgmt.nodeIt = 0;
						mooltipass.memmgmt.pageIt++;
					}
					XXXXXXX
					//currentNode = mooltipass.memmgmt.readNode([(nodei + (pagei << 3)) & 0x00FF, (pagei >> 5) & 0x00FF]);
				}
				
				// Reset current node
				mooltipass.memmgmt.currentNode = [];
			}			
		}
		else
		{
			// If we couldn't read the node, ask the next one
			mooltipass.memmgmt.nodePacketId = 0;
			// Check that we didn't finish the scanning, ask next node
			if((mooltipass.memmgmt.nodeIt + 1) == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb) && (mooltipass.memmgmt.pageIt + 1) == mooltipass.memmgmt.getNumberOfPages(mooltipass.memmgmt.nbMb))
			{
				//
			}
			else
			{
				if(++mooltipass.memmgmt.nodeIt == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb))
				{
					// Changing pages
					mooltipass.memmgmt.nodeIt = 0;
					mooltipass.memmgmt.pageIt++;
				}
				XXXXXXX
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
		XXXXXXX
		return true;
	}
	else
	{
		return false;
	}
}

// Memory integrity check
mooltipass.memmgmt.integrityCheckStart = function()
{
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_INT_CHECK_REQ;
		XXXXXXX
	}
}

























