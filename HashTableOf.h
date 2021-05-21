/* HashTableOf.h

   Copyright (C) 1996, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 7 February 1996
   Last Change: Friday 23 September 2005
   Last Committed: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Committed by: $Author: tmecham $
   
*/

#ifndef HashTableOf_h
#define HashTableOf_h

#include "Csi.Utils.h"
#include "StrBin.h"

////////// template HashTablePos
// This class represents a cursor in a hash table object. This can be used to
// iterate the elements of the hash table.
template <class T> class HashTablePos
{
public:
   HashTablePos()
   {
      index = 0;
      nodePtr = 0;
   }

   HashTablePos(const HashTablePos &other)
   {
      index = other.index;
      nodePtr = other.nodePtr;
   }

public:
   unsigned int index;
   void *nodePtr;
};

template <class T> class HashTableNode
{
public:
   HashTableNode(void):
      next(0)
   {  }

   T data;
   HashTableNode *next;
};


////////// template HashTableOf
// An intrusive container template that provides a fast look up/fast insertion structure based upon
// a key value provided by a method call to the object being inserted or searched for.
//
// An object stored in this class of container must provide the following methods:
//  - default constructor
//  - copy operator
//  - a function with the following signature: void getKey(StrBin &key) const
template <class T> 
class HashTableOf
{
public:
   ////////// constructor
   // The size specified in this constructor is will be the size of the hash
   // table array. It is possible to store more items in the container than
   // this number but this will also tend to slow down the collision resolution
   // algorithm.
   HashTableOf(unsigned int numElements_);

   ////////// destructor
   // The destructor will automatically destroy any items stored in the hash
   // table when it is invoked.
   ~HashTableOf();

   ////////// flush
   // Destroys any items that are in the hash table
   void flush();

   ////////// insert
   // Inserts a copy of the specified object into the hash table 
   void insert(const T &element);

   ////////// remove
   // Removes the element from the hash table identified by the key. Returns
   // true if the key was found or false if the key could not be found.
   bool remove(StrBin &key);

   ////////// get
   // Returns a reference to the first element that matches the key. Will throw
   // a MsgExcept object if no match is found
   T &get(StrBin &key);

   ////////// find
   // Locates the position of the first record that matches the key and returns
   // a pointer to that record. Will return a NULL pointer if the record cannot
   // be located. Sets the pos parameter so that a subsequent call to getAt
   // will return the same record.
   T *find(StrBin &key, HashTablePos<T> &pos);

   ////////// hasKey
   // Returns true if a record with the specified key exists in the table
   bool hasKey(StrBin &key);

   //////////  get_count
   // Returns the total number of elements stored in the container
   int get_count() const { return count; }

   ////////// get_memoryUsage
   // Calculates the amount of memory that is being consumed by the hash
   // table. This includes the overhead in the storage structures, etc.
   int get_memoryUsage();

   ////////// getFirst
   // Returns a pointer to the first item stored in the hash table. If the
   // hash table has no elements, this method will return a NULL pointer. This
   // method will also initialise the pos cursor so that the same value can be
   // retrieved using the getAt method.
   T *getFirst(HashTablePos<T> &pos);

   ////////// getAt
   // Returns a pointer to the element stored at the position indicated by the
   // cursor. The cursor should have been initialised by a previous call to
   // getFirst or getNext. Returns a NULL pointer if the position is invalid.
   T *getAt(HashTablePos<T> &pos);

   ////////// getNext
   // Returns a pointer to the element followingthe position in the hash table
   // indicated by the cursor. The cursor should have been initialised by a
   // previous call to getFirst. If there is no next element, the return value
   // will be a NULL pointer. This method will also position the cursor so that
   // a call to getAt will return the same pointer.
   T *getNext(HashTablePos<T> &pos);

protected:
   HashTableNode<T> **data;  // pointer to the table data
   unsigned int count,       // count of total number of elements
   numElements; // count of the table size

   ////////// hash
   // Implements the hash algorithm on the contents of the key. Defined to be
   // virtual so that derived classes can easily replace the hashing algorithm
   virtual unsigned int hash(StrBin &key);

   ////////// addConflict
   // Places the record at its appropriate hash index and resolves any
   // conflicts. This method is declared to be virtual and separate from the
   // insert method so that derived classes can modify the way that conflict
   // resolution is handled. This version is optimised for speed. Future
   // versions might need to gather records with duplicate keys so that they
   // are adjacent and/or in order.
   virtual void addConflict(HashTableNode<T> *node, uint4 hashIdx);
};

template <class T>
HashTableOf<T>::HashTableOf(unsigned int numElements_):
   numElements(numElements_),
   count(0),
   data(0)
{
   // allocate the array of data
   data = new HashTableNode<T> *[numElements];
   for(unsigned int i = 0; i < numElements; i++)
      data[i] = 0;
} // end constructor

template <class T>
HashTableOf<T>::~HashTableOf()
{
   if(data == 0)
      return;
   flush();
   delete[] data;
} // end destructor

template <class T>
void HashTableOf<T>::flush()
{
   // delete the hash table elements
   for(unsigned int i = 0; i < numElements; i++)
      {
         HashTableNode<T> *t1 = data[i],
            *t2;
         while(t1 != 0)
            {
               t2 = t1;
               t1 = t1->next;
               delete t2;
            } // end delete this line of buckets
         data[i] = 0;
      } // end delete hash table elements
   count = 0;
} // end flush

template <class T>
void HashTableOf<T>::insert(const T &element)
{
   // get the hash key from the element
   StrBin key;
   element.getKey(key);

   // get the hash value for the element
   unsigned int hashValue = hash(key);
   hashValue %= numElements;

   // allocate a node to contain the element
   HashTableNode<T> *node;
   node = new HashTableNode<T>;
   node->data = element;

   // now insert the new node into the hash table
   if(data[hashValue] == 0)
      data[hashValue] = node;
   else
   {
#ifdef TRACE
      TRACE("HashTable: %p collision at element %d\n",this,hashValue);
#endif
      addConflict(node,hashValue);
   }
   count++;
} // end insert

template <class T>
bool HashTableOf<T>::remove(StrBin &key)
{
   // calculate the hash value for the key
   bool rtn = false;
   unsigned int hashValue = hash(key);
   
   hashValue %= numElements;

   // search for an exact match to the value
   HashTableNode<T> *node = data[hashValue];
   HashTableNode<T> *parent = NULL;
   StrBin nodeKey;

   while(node != 0)
   {
      node->data.getKey(nodeKey);
      if(nodeKey == key)
      {
         // remove the node link from the list
         if(parent == NULL)
            data[hashValue] = node->next;
         else
            parent->next = node->next;
         
         // delete the node
         delete node;
         count--;
         rtn = true;
         break;
      } // end found exact match
      parent = node;
      node = node->next;
   } // end search for an exact match
   return rtn;
} // end remove

template <class T>
T &HashTableOf<T>::get(StrBin &key)
{
   // calculate the hash value
   unsigned int hashValue = hash(key);

   // search for an exact match to the value
   HashTableNode<T> *node = data[hashValue];
   StrBin nodeKey;

   while(node != 0)
   {
      node->data.getKey(nodeKey);
      if(nodeKey == key)
         break;
      node = node->next;
   }
   if(node == 0)
      throw Csi::MsgExcept("Key not found in hash table");
   return node->data;
} // get


template <class T>
T *HashTableOf<T>::find(StrBin &key, HashTablePos<T> &pos)
{
   // calculate the hash value for the key
   uint4 hashValue = hash(key);

   // search for the first match to the key
   HashTableNode<T> *node = data[hashValue];
   StrBin nodeKey;
   
   while(node != NULL)
   {
      node->data.getKey(nodeKey);
      if(nodeKey == key)
         break;
      node = node->next;
   }

   // set the return value and the position parameter
   if(node == NULL)
   {
      pos.index = 0;
      pos.nodePtr = NULL;
   }
   else
   {
      pos.index = hashValue;
      pos.nodePtr = node;
   }
   return &node->data;
} // find 


template <class T>
bool HashTableOf<T>::hasKey(StrBin &key)
{
   // calculate the hash value for the key
   unsigned int hashValue = hash(key);

   // search for an exact match to the key
   HashTableNode<T> *node = data[hashValue];
   StrBin nodeKey;
   bool rtn = false;
   
   while(!rtn && node != NULL)
   {
      node->data.getKey(nodeKey);
      if(nodeKey == key)
         rtn = true;
      else
         node = node->next;
   }
   return rtn;
} // hasKey


template <class T>
unsigned int HashTableOf<T>::hash(StrBin &key)
{
   // calculate the 32 bit check sum of the key
   unsigned int rtn = Csi::crc32((unsigned char *)key.getContents(),key.getLen());
   rtn %= numElements;
   return rtn;
} // end hash

template <class T>
int HashTableOf<T>::get_memoryUsage()
{
   unsigned int rtn = numElements*sizeof(data[0]);

   rtn += count*sizeof(HashTableNode<T>);
   return rtn;
} // end get_memoryUsage

template <class T>
T *HashTableOf<T>::getFirst(HashTablePos<T> &pos)
{
   // search for the first valid element
   unsigned int i;
   for(i = 0; i < numElements && data[i] == NULL; i++)
      0;

   // initialise the position and return the element
   T *rtn;

   if(i >= numElements)
   {
      rtn = NULL;
      pos.index = 0;
      pos.nodePtr = NULL;
   }
   else
   {
      rtn = &(data[i]->data);
      pos.index = i;
      pos.nodePtr = data[i];
   }
   return rtn;
} // end getFirst

template <class T>
T *HashTableOf<T>::getAt(HashTablePos<T> &pos)
{
   // make sure that the position still exists
   T *rtn = NULL;

   if(pos.index < numElements)
   {
      // search for the node
      HashTableNode<T> *node = data[pos.index];
      while(node != NULL)
      {
         if((HashTableNode<T> *)pos.nodePtr == node)
         {
            rtn = &(node->data);
            break;
         }
         node = node->next;
      } // end search for the node in the linked list
   } // end verify the index
   return rtn;
} // end getAt

template <class T>
T *HashTableOf<T>::getNext(HashTablePos<T> &pos)
{
   T *rtn = NULL;

   if(pos.index < numElements)
   {
      // search for the node
      HashTableNode<T> *node = data[pos.index];
      while(node != NULL)
      {
         if((HashTableNode<T> *)pos.nodePtr == node)
            break;
         node = node->next;
      } // end search for the specified position
      
      if(node == NULL)
         return NULL;
      
      if(node->next != NULL)
      {
         pos.nodePtr = node->next;
         rtn = &(node->next->data);
      }
      else
      {
         pos.index++;
         while(pos.index < numElements)
         {
            if(data[pos.index] != NULL)
            {
               pos.nodePtr = data[pos.index];
               rtn = &(data[pos.index]->data);
               break;
            } // end found the next node
            pos.index++;
         } // end search for the next node
      } // end need to search for the next node
   } // end verify the index
   return rtn;
} // end getNext


template <class T>
void HashTableOf<T>::addConflict(HashTableNode<T> *node, uint4 hashIdx)
{
   node->next = data[hashIdx];
   data[hashIdx] = node;
} // addConflict


#endif
