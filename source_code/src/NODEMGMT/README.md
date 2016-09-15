#Node Management Library for Mooltipass
Michael Neiderhauser
Jun-15-2014

Files:
- node_mgmt.c
- node_mgmt.h

The Node Management Library was written to be the primary flash data structure for the Mooltipass.
The Node Management Library makes use of the Flash Memory Library for Mooltipass.
The library is implemented as a linked list.

Details about the node layout as well as how they are represented in memory can be found at the link below:
https://docs.google.com/drawings/d/1og2-RgrFOL5XlW0ICNUDv2R0IQK8geOhonX9XfuuFjk/edit

Currently basic credential storage has been implemented and tested.  There are still a few features that  need to be implemented and they should arrive soon.  Some of the features in the Node Management Library require Mooltipass plugin support.

Nodes are stored in a stack and heap layout.  All nodes are doubly linked.
A parent node and child node (child node start of data, or data node) can co-exist on the same memory page.

The Node Management library requires the user to provide allocated space of a handle.  The user should not directly modify this handle.
The Node Management library also implements support for basic user profiles.  A user profile is used to store the users first parent node (credential) and is used to allow a user to store 'favourite' credentials.  There are a maximum of 14 favourites allowed and a total of 16 possible users.

## Node Details
### Parent Node
Used to store the base 'service' of a credential.
- flags (used internally to indicate validity and type of the node. associates user id with the node)
- nextChildAddress (Node Address of the parents first child)
- prevParentAddress (Used to implement the linked list)
- nextParentAddress (Used to implement the linked list)
- service 58B (Used to indicate the 'service' of the credential e.g. 'hackaday.io')

### Child Node
Used to store the user name and password of a credential
 - flags (used internally to indicate validity and type of the node)
 - prevChildAddress (Used to implement the linked list)
 - nextChildAddress (Used to implement the linked list)
 - description 24B (Plain-text description of the credential)
 - dateCreated (Date the credential was first added to the Mooltipass. plug-in required)
 - dateLastUsed (Date the credential was last used. plug-in required)
 - ctr 3B (Used for encryption)
 - login 63B (Plain-text user name)
 - password 32B (encrypted password)

### Child start of data Node
More later
### Data Node
More Later


## Library Usage

```cpp
// Declare a return variable
RET_TYPE ret = RETURN_NOK;

// Select a User ID
uint8_t uid = 0;

// (OPTIONAL) Format the user profile memory and check the return code
ret = formatUserProfileMemory(uid);
if(ret != RETURN_OK)
{
  // do something
}

// Obtain a Node Management Handle with the user id and check the return code
mgmtHandle h;
mgmtHandle *hp = &h;
ret = initNodeManagementHandle(hp, uid);
if(ret != RETURN_OK)
{
  // do something
}

// Creating a Parent Node
pNode parent;
parent.service = "Some Text";

ret = createParentNode(hp, &parent);
if(ret != RETURN_OK)
{
  // do something
}

// Updating a Parent Node
// only field the user should update is service
parent.service = "Some Text";
ret = updateParentNode(hp, &parent);
if(ret != RETURN_OK)
{
  // do something
}

// Constructing a Parent Node Address
uint16_t pageNumber = 128;
uint8_t nodeNumber = 0;
uint16_t parentNodeAddress = constructAddress(pageNumber, nodeNumber);

// Reading a Parent Node
ret = readParentNode(hp, &parent, parentNodeAddress);
if(ret != RETURN_OK)
{
  // do something
}

// Deleting a Parent Node
/*
typedef enum
{
    DELETE_POLICY_WRITE_NOTHING = 1, /*!< Flip valid bit */
    DELETE_POLICY_WRITE_ZEROS = 0x00,   /*!< Write node with all 0's */
    DELETE_POLICY_WRITE_ONES = 0xff,    /*!< Write node with all 1's */
} deletePolicy;
*/
ret = deleteParentNode(hp, parentNodeAddress, DELETE_POLICY_WRITE_ONES);
if(ret != RETURN_OK)
{
  // do something
}

// Deconstruction of a Node Address
pageNumber = pageNumberFromAddress(parentNodeAddress);
nodeNumber = nodeNumberFromAddress(parentNodeAddress);

// Creating a Child Node
uint16_t addressOfChildParent = constructAddress(128,1);
cNode child;

// modify child here
ret = createChildNode(hp, addressOfChildParent, &child);
if(ret != RETURN_OK)
{
  // do something
}

// Reading a Child Node
ret = readParentNode(hp, addressOfChildParent);
if(ret != RETURN_OK)
{
  // do something
}

ret = readChildNode(hp, &child, parent.nextChildAddress); // use specific child node address
if(ret != RETURN_OK)
{
  // do something
}

// Updating a Child Node
// modify child node
ret = updateChildNode(hp, &parent, &child, addressOfChildParent, parent.nextChildAddress); // use specific child node address
if(ret != RETURN_OK)
{
  // do something
}

// Deleting a Child Node
/*
typedef enum
{
    DELETE_POLICY_WRITE_NOTHING = 1, /*!< Flip valid bit */
    DELETE_POLICY_WRITE_ZEROS = 0x00,   /*!< Write node with all 0's */
    DELETE_POLICY_WRITE_ONES = 0xff,    /*!< Write node with all 1's */
} deletePolicy;
*/
ret = deleteChildNode(hp, addressOfChildParent, parent.nextChildAddress, DELETE_POLICY_WRITE_ONES); // use specific child node address
if(ret != RETURN_OK)
{
  // do something
}

// Setting a User Profile Favourite
// Assuming you know the address of a Parent / Child Credential Set
uint16_t parentAddress = KNOWN_PARENT;
uint16_t childAddress = KNOWN_CHILD;
uint8_t favId = 0;
ret = setFav(hp, favId, parentAddress, childAddress);
if(ret != RETURN_OK)
{
  // do something
}

// Clearing a User Profile Favourite
uint16_t parentAddress = NODE_ADDR_NULL;
uint16_t childAddress = NODE_ADDR_NULL;
uint8_t favId = 0;
ret = setFav(hp, favId, parentAddress, childAddress);
if(ret != RETURN_OK)
{
  // do something
}

// Setting a User Profile Favourite
// Assuming you know the favId
uint16_t parentAddress = 0;
uint16_t childAddress = 0;
uint8_t favId = KNOWN_FAV_ID;
ret = readFav(hp, favId, &parentAddress, &childAddress);
if(ret != RETURN_OK)
{
  // do something
}

// Other functions can be found in node_mgmt.h
Note: Always check the return code
```

## Flash Memory Testing
Files:
- node_test.c
- node_test.h

Node Testing attempts to exercise the library for linked list functionality.  It also unit tests the helper functions of the library.
To run Flash Testing:
Include the node_test.h file and call the nodeTest() Function.

To run Flash Testing on the Mooltipass:
In the file tests.c uncomment the (pound)define TEST_NODE in the afterFlashInitTests() function.

Note:  The flash_test.c file does use oled and usb libs of the mooltipass however this should be easy to remove if needed.
Note: Check the enums of node testing errors in the node_test.h file.


## Library TODO's
- Implement algorithm for child start of data and data nodes to allow for 'large' data storage (SSH Keys anyone?)
- Optimize Implementation (Share code when possible)
- Fix creatChildNode shared buffer issue.
