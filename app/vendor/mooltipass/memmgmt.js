var mooltipass = mooltipass || {};
mooltipass.memmgmt = mooltipass.memmgmt || {};

// Defines
var NODE_SIZE						= 132;			// Node size
var HID_PAYLOAD_SIZE				= 62;			// HID payload

// State machine modes
var MGMT_IDLE 						= 0;			// Idle mode
var MGMT_PARAM_LOAD					= 1;			// Parameter loading
var MGMT_PARAM_LOAD_REQ				= 2;			// Parameter loading
var MGMT_PARAM_LOAD_INT_CHECK		= 3;			// Parameter loading for integrity check
var MGMT_PARAM_LOAD_INT_CHECK_REQ	= 4;			// Parameter loading for integrity check, request
var MGMT_INT_CHECK_SCAN				= 5;			// Scanning through the memory for integrity check
var MGMT_INT_CHECK_PACKET_SENDING	= 6;			// Sending the correction packets
var MGMT_NORMAL_SCAN				= 7;			// Normal credential scan, following the nodes

// Mooltipass memory params
mooltipass.memmgmt.nbMb = null;						// Mooltipass memory size
mooltipass.memmgmt.ctrValue = [];					// Mooltipass CTR value
mooltipass.memmgmt.CPZCTRValues = [];				// Mooltipass CPZ CTR values
mooltipass.memmgmt.startingParent = null;			// Mooltipass current starting parent
mooltipass.memmgmt.dataStartingParent = null;		// Mooltipass current data starting parrent
mooltipass.memmgmt.favoriteAddresses = [];			// Mooltipass current favorite addresses
mooltipass.memmgmt.curServiceNodes = [];			// Mooltipass current service nodes
mooltipass.memmgmt.curLoginNodes = [];				// Mooltipass current login nodes
mooltipass.memmgmt.curDataServiceNodes = [];		// Mooltipass current data service nodes
mooltipass.memmgmt.curDataNodes = [];				// Mooltipass current data nodes
mooltipass.memmgmt.clonedFavoriteAddresses = [];	// Mooltipass current favorite addresses
mooltipass.memmgmt.clonedCurServiceNodes = [];		// Mooltipass current service nodes
mooltipass.memmgmt.clonedCurLoginNodes = [];        // Mooltipass current login nodes
mooltipass.memmgmt.clonedCurDataServiceNodes = [];  // Mooltipass current data service nodes
mooltipass.memmgmt.clonedCurDataNodes = [];         // Mooltipass current data nodes

// State machines & temp variables
mooltipass.memmgmt.currentMode = MGMT_IDLE;			// Current mode
mooltipass.memmgmt.currentFavorite = 0;				// Current favorite read/write
mooltipass.memmgmt.pageIt = 0;						// Page iterator
mooltipass.memmgmt.nodeIt = 0;						// Node iterator
mooltipass.memmgmt.scanPercentage = 0;				// Scanning percentage
mooltipass.memmgmt.nodePacketId = 0;				// Packet number for node sending/receiving
mooltipass.memmgmt.currentNode = [];				// Current node we're sending/receiving
mooltipass.memmgmt.packetToSendBuffer = [];			// Packets we need to send at the end of the checks etc...
mooltipass.memmgmt.nextParentNodeTSAddress = [];	// Next parent node to scan address


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
			console.log("Wrong starting parent: " + current_starting_node[0].name + " at address " + current_starting_node[0].address + "(" + mooltipass.memmgmt.curServiceNodes[0].name + ") should be " + mooltipass.memmgmt.curServiceNodes[0].name + " at address " + mooltipass.memmgmt.curServiceNodes[0].address);
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setStartingParentAddress'], mooltipass.memmgmt.curServiceNodes[0].address));		
		}
		else
		{
			console.log("Current starting node set to invalid value");
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
	mooltipass.memmgmt.clonedCurLoginNodes.forEach(	function (item, index, arr) 
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
			mooltipass.memmgmt.packetToSendBuffer.push(mooltipass.device.createPacket(mooltipass.device.commands['setFavorite'], mooltipass.memmgmt.clonedFavoriteAddresses[i]));
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

// Export current Mooltipass memory state
mooltipass.memmgmt.exportMemoryState = function()
{
	var export_data = [mooltipass.memmgmt.ctrValue, mooltipass.memmgmt.CPZCTRValues, mooltipass.memmgmt.startingParent, mooltipass.memmgmt.dataStartingParent, mooltipass.memmgmt.favoriteAddresses, mooltipass.memmgmt.curServiceNodes, mooltipass.memmgmt.curLoginNodes, mooltipass.memmgmt.curDataServiceNodes, mooltipass.memmgmt.curDataNodes];
	mooltipass.filehandler.selectAndSaveFileContents("memory_export", new Blob([JSON.stringify(export_data)], {type: 'text/plain'}), mooltipass.memmgmt.fileWrittenCallback);
	// {type: 'application/octet-binary'}
}

// Export file written callback
mooltipass.memmgmt.fileWrittenCallback = function()
{
	console.log("File written!");
}

// Data received callback
mooltipass.memmgmt.dataReceivedCallback = function(packet)
{
	// If it is a leave memory management mode packet, process the queue and exit
	if(packet[1] == mooltipass.device.commands['endMemoryManagementMode'])
	{
		mooltipass.memmgmt.currentMode = MGMT_IDLE;
		
		// Did we succeed?
		if(packet[2] == 1)
		{
			console.log("Memory management mode exit");
			mooltipass.device.processQueue();
			return;
		}
		else
		{
		}
	}
	else if(packet[1] == mooltipass.device.commands['debug'])
	{
		console.log("Debug: " + mooltipass.util.arrayToStr(packet.subarray(2, packet.length)));
	}
	
	if(mooltipass.memmgmt.currentMode == MGMT_INT_CHECK_PACKET_SENDING)
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
	else if(mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_INT_CHECK_REQ || mooltipass.memmgmt.currentMode == MGMT_PARAM_LOAD_REQ)
	{
		if(packet[1] == mooltipass.device.commands['getMooltipassParameter'])
		{
			// We received the user interaction timeout, use it for our packets timeout
			console.log("Mooltipass interaction timeout is " + packet[2] + " seconds");
			// Create packet to go into memmgmt mode
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['startMemoryManagementMode'], null);
			mooltipass.memmgmt_hid.request.milliseconds = (packet[2]+2) * 1000;
			mooltipass.memmgmt_hid.nbSendRetries = 3;
			mooltipass.memmgmt_hid._sendMsg();
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
				console.log("Memory management mode entered");
			}
			else
			{
				console.log("Couldn't enter memory management mode");
				mooltipass.device.processQueue();
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
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getCTR'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getCTR'])
		{
			if(packet[0] == 1)
			{
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
			// Inform that all CPZ CTR packets were sent
			console.log("All CPZ CTR packets are received");			
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getStartingParentAddress'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getStartingParentAddress'])
		{
			if(packet[0] == 1)
			{
				// Request failed, leave memory management mode
				mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
				mooltipass.memmgmt_hid._sendMsg();
				return;
			}
			mooltipass.memmgmt.startingParent = [packet[2], packet[3]];
			console.log("Starting parent is " + mooltipass.memmgmt.startingParent);	
			mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getStartingDataParentAddress'], null);
			mooltipass.memmgmt_hid._sendMsg();
		}
		else if(packet[1] == mooltipass.device.commands['getStartingDataParentAddress'])
		{
			if(packet[0] == 1)
			{
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
	else if(mooltipass.memmgmt.currentMode == MGMT_NORMAL_SCAN)
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
						mooltipass.memmgmt.curServiceNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0)});
						mooltipass.memmgmt.clonedCurServiceNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getServiceName(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0)});
						console.log("Received service " + mooltipass.memmgmt.curServiceNodes[mooltipass.memmgmt.curServiceNodes.length - 1].name);
						
						// Store next parent address
						mooltipass.memmgmt.nextParentNodeTSAddress = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.currentNode);
						var first_child_address = mooltipass.memmgmt.getFirstChildAddress(mooltipass.memmgmt.currentNode);
						
						// If it has a child, request it...
						if(!mooltipass.memmgmt.isSameAddress(first_child_address, [0,0]))
						{
							mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], first_child_address);
							mooltipass.memmgmt_hid._sendMsg();							
						}		
						else
						{
							// No child, check that we haven't finished scanning
							if(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.nextParentNodeTSAddress, [0,0]))
							{
								// Finished scanning
								// Leave mem management mode				
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
								mooltipass.memmgmt_hid._sendMsg();
							}
							else
							{
								// Request next parent
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.memmgmt.nextParentNodeTSAddress);
								mooltipass.memmgmt_hid._sendMsg();										
							}
						}
					}
					else if(nodeType == 'child')
					{
						// Store names, addresses, nodes
						mooltipass.memmgmt.curLoginNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0), 'pointed': false});
						mooltipass.memmgmt.clonedCurLoginNodes.push({'address': [(mooltipass.memmgmt.nodeIt + (mooltipass.memmgmt.pageIt << 3)) & 0x00FF, (mooltipass.memmgmt.pageIt >> 5) & 0x00FF], 'name': mooltipass.memmgmt.getLogin(mooltipass.memmgmt.currentNode), 'data': mooltipass.memmgmt.currentNode.slice(0), 'pointed': false});
						console.log("Received login " + mooltipass.memmgmt.curLoginNodes[mooltipass.memmgmt.curLoginNodes.length - 1].name);
						
						var next_child_address = mooltipass.memmgmt.getNextAddress(mooltipass.memmgmt.currentNode);
						
						// If it has a next child, request it...
						if(!mooltipass.memmgmt.isSameAddress(next_child_address, [0,0]))
						{
							mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], next_child_address);
							mooltipass.memmgmt_hid._sendMsg();							
						}		
						else
						{
							// No next child, check that we haven't finished scanning
							if(mooltipass.memmgmt.isSameAddress(mooltipass.memmgmt.nextParentNodeTSAddress, [0,0]))
							{
								// Finished scanning
								// Leave mem management mode				
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['endMemoryManagementMode'], null);
								mooltipass.memmgmt_hid._sendMsg();
								mooltipass.memmgmt.exportMemoryState();
							}
							else
							{
								// Request next parent
								mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['readNodeInFlash'], mooltipass.memmgmt.nextParentNodeTSAddress);
								mooltipass.memmgmt_hid._sendMsg();										
							}
						}
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
			// TODO: inform the user the memory is corrupted			
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
				if(((mooltipass.memmgmt.nodeIt + 1) == mooltipass.memmgmt.getNodesPerPage(mooltipass.memmgmt.nbMb) && (mooltipass.memmgmt.pageIt + 1) == mooltipass.memmgmt.getNumberOfPages(mooltipass.memmgmt.nbMb)) || false)//(mooltipass.memmgmt.scanPercentage == 5))
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

// Memory integrity check
mooltipass.memmgmt.integrityCheckStart = function()
{
	if(mooltipass.memmgmt.currentMode == MGMT_IDLE)
	{
		mooltipass.memmgmt.currentMode = MGMT_PARAM_LOAD_REQ;
		// First step is to query to user interaction timeout to set the correct packet timeout retry!
		mooltipass.memmgmt_hid.request['packet'] = mooltipass.device.createPacket(mooltipass.device.commands['getMooltipassParameter'], [mooltipass.device.parameters['userInteractionTimeout']]);
		mooltipass.memmgmt_hid.responseCallback = mooltipass.memmgmt.dataReceivedCallback;
		mooltipass.memmgmt_hid.nbSendRetries = 0;
		mooltipass.memmgmt_hid._sendMsg();
	}return;
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

























