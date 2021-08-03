/* ListOf.h

   Copyright (C) 1998, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Friday 13 October 1995
   Last Change: Monday 09 August 1999

*/

#ifndef ListOf_h
#define ListOf_h

#include <stdlib.h>
#include <stdio.h>
#include "MsgExcept.h"
#include "CsiTypeDefs.h"

template <class T> class DblListHolder
{
public:
   ////////// default constructor
   DblListHolder(void):
      next(0),
      prev(0)
   { }

   ////////// next
   // Pointer to the next holder in the list
   DblListHolder *next;

   ////////// prev
   // pointer to the previous holder in the list
   DblListHolder *prev;

   ////////// data
   // The data object for this node
   T data;
};


////////// class ListOf
//  This template implements a container that uses a doubly-linked list internally and provides an
//  arry type index externally. It provides some optimisations that should speed up subscript
//  iteration by keeping track of the last element accessed.
template <class T> class ListOf
{
public:
   ////////// default constructor
   // default constructor creates a list with no elements
   ListOf(void):
      head(0),
      tail(0),
      count(0),
      lastAccess(0),
      lastPos(0)
   { }

   ////////// copy constructor
   // copy constructor creates a new list with duplicated elements
   ListOf(const ListOf &other):
      head(0),
      tail(0),
      count(0),
      lastAccess(0),
      lastPos(0)
   { copy(other); }

   ////////// destructor
   // Destructor will automatically remove all elements in the list
   ~ListOf(void)
   { flush(); }

   ////////// copy operator
   // Copy operator performs the same function as he copy constructor. If there
   // are elements in the list previous to this operator being called, those
   // elements will be lost
   ListOf &operator =(const ListOf &other)
   { copy(other); return *this; }

   ////////// get_count
   // Returns the number of items that have been stored in the list
   uint4 get_count(void) const { return count; }
    
   ////////// subscript operator
   // Subscript operator returns the element at the indicated position. Will
   // throw a MsgExcept object if the specified index is out of bounds
   T &operator [](uint4 index) { return getData(index); }

   ////////// getData
   // Returns the element at the indicated position. Will throw a MsgExcept
   // object if the index is out of bounds
   T &getData(uint4 index);

   ////////// setData
   // Overwrites the object stored at the indicated position. Will throw a
   // MsgExcept object if the specified index is out of bounds
   void setData(uint4 index, T const &data_);

   ////////// remove
   // Removes the object stored at the indicated position from the list. Will
   // throw a MsgExcept object if the index is out of bounds.
   void remove(uint4 index);
    
   ////////// copy
   // copies the elements of the other list into this list. If there were any
   // elements in this list before this method was invoked, those elements will
   // be lost.
   void copy(ListOf const &other);

   ////////// flush
   // Empties the contents of this list
   void flush(void);
    
   ////////// add
   // Adds a copy of the specified entry at the end of the list
   void add(T const &entry);

   ////////// insert
   // Inserts a copy of the specified entry in the indicated position in the
   // list. Will throw a MsgExcept object if the specified location is out of
   // bounds.
   void insert(T const &entry, uint4 n);

   ////////// getFirst
   // Returns the first item in the list. Will throw a MsgExcept object if the
   // list is empty
   T &getFirst()
   {
      if(head == NULL)
         throw MsgExcept("The list is empty");
      return head->data;
   }

   ////////// front
   // synonym for getFirst
   T &front()
   {
      return getFirst();
   }

   ////////// getLast
   // Returns the last item on the list. Will throw a MsgExcept object if the
   // list is empty
   T &getLast()
   {
      if(tail == NULL)
         throw MsgExcept("The list is empty");
      return tail->data;
   }

   ////////// back
   // Synonym for getLast
   T &back()
   {
      return getLast();
   }

   ////////// isValidIdx
   // Returns true if the indicated index is within the bounds of the list
   bool isValidIdx(uint4 i) const
   {
      bool rtn = false;
      if(i >= 0 && i < count)
         rtn = true;
      return rtn;
   }

   ////////// empty
   // Returns true if the list has no objects
   bool empty() const
   {
      return count == 0;
   } 
    
private:
   ////////// head
   // Pointer to the first element in the list
   DblListHolder<T> *head;

   ////////// tail
   // Pointer to the last element in the list 
   DblListHolder<T> *tail;

   ////////// lastAccess
   // Pointer to the last element that was accessed
   DblListHolder<T> *lastAccess;

   ////////// count
   // Keeps track of the number of elements stored in the list
   uint4 count;

   ////////// lastPos
   // Keeps track of the last position that was accessed
   uint4 lastPos;

   // set the lastAccess and lastPos variables to be set to the point specified
   void seekToIndex(uint4 index);
};


template <class T>
T &ListOf<T>::getData(uint4 index)
{
   seekToIndex(index);
   return lastAccess->data;
} // end getData


template <class T>
void ListOf<T>::setData(uint4 index,const T &data_)
{
   seekToIndex(index);
   lastAccess->data = data_;
} // end setData


template <class T>
void ListOf<T>::remove(uint4 index)
{
   // seek to the index
   seekToIndex(index);
    
    // extract the prev and next pointers from the node
   DblListHolder<T> *prev = lastAccess->prev,
      *next = lastAccess->next;
    
    // set the links on the prev and next pointers
   if(prev != 0)
      prev->next = next;
   if(lastAccess == head)
      head = next;
   if(next != 0)
      next->prev = prev;
   if(lastAccess == tail)
      tail = prev; 
    
    // delete the indexed node
   delete lastAccess;
   lastAccess = 0;
   count--;
} // end method


template <class T>
void ListOf<T>::copy(const ListOf<T> &other)
{
   flush();
   DblListHolder<T> *temp = other.head,
      *newHolder = 0,
      *lastHolder = 0;
   while(temp != 0)
   {
      // create a new node
      newHolder = new DblListHolder<T>;
      newHolder->prev = lastHolder;
      newHolder->data = temp->data;
        
      // assign the head (if not already assigned)
      if(head == 0)
         head = newHolder;
      if(lastHolder != 0)
         lastHolder->next = newHolder;
        
      // finally assign the lastHolder to the newHolder created in this iteration
      lastHolder = newHolder;
      temp = temp->next;
   } // end copy loop
    
   // finally assign the tail pointer
   tail = newHolder;
   count = other.count;
} // end copy


template <class T>
void ListOf<T>::flush(void)
{
   DblListHolder<T> *temp;
   while(head != 0)
   {
      temp = head;
      head = head->next;
      delete temp;
   }
   head = tail = lastAccess = 0;
   lastPos = count  = 0;
} // end flush


template <class T>
void ListOf<T>::add(const T &entry)
{
   DblListHolder<T> *newHolder;
   newHolder = new DblListHolder<T>;
   newHolder->data = entry;
   newHolder->prev = tail;
   if(tail != 0)
      tail->next = newHolder;
   if(head == 0)
      head = newHolder;
   lastAccess = tail = newHolder;
   lastPos = count++;
} // end add


template <class T>
void ListOf<T>::insert(const T &entry, uint4 index)
{
   if(index == count)
      add(entry);
   else
   {
      // seek to the insertion point
      seekToIndex(index);
        
      // allocate the new record at the insertion point
      DblListHolder<T> *newHolder;
      DblListHolder<T> *prev = lastAccess->prev;
        
      newHolder = new DblListHolder<T>;
      newHolder->prev = lastAccess->prev;
      newHolder->next = lastAccess;
      newHolder->data = entry;
      if(prev != 0)
         prev->next = newHolder;
      lastAccess->prev = newHolder;
        
      // adjust the head pointer (if insertion at beginning of list)
      if(index == 0)
         head = newHolder;
      lastAccess = newHolder;
      count++;
   }
} // end insert


template <class T>
void ListOf<T>::seekToIndex(uint4 index)
{
   // check the index bounds
   if(index < 0 || index >= count)
   {
      throw MsgExcept("ListOf::seekToIndex: bounds error");
   }
    
   // set the last access (if not already set)
   if(lastAccess == 0)
   {
      lastAccess = head;
      lastPos = 0;
   }
    
   // determine the closest starting point and direction of travel
   int direction = 1;
   uint4 distLastPos;
   uint4 distTail;
   uint4 distance;
   DblListHolder<T> *temp = head;
    
   if(index > lastPos)
      distLastPos = index - lastPos;
   else
      distLastPos = lastPos - index;
   distTail = count - index - 1;

   if(index < distLastPos && index < distTail)
   {
      direction = 1;
      temp = head;
      distance = index;
   } // count from head
   else if(distTail < index && distTail < distLastPos)
   {
      direction = -1;
      temp = tail;
      distance = distTail;
   } // count from tail
   else if(index <= lastPos)
   {
      direction = -1;
      temp = lastAccess;
      distance = distLastPos;
   }
   else
   {
      direction = 1;
      temp = lastAccess;
      distance = distLastPos;
   }

   // now seek in a loop to the desired position
   for(unsigned int i = 0; i < distance; i++)
      if(direction > 0)
         temp = temp->next;
      else
         temp = temp->prev;
    
    // finally set the lastAccess and lastPos variables
   lastAccess = temp;
   lastPos = index;
} // seekToIndex

#endif
