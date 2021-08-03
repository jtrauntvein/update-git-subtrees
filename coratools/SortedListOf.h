/* SortedListOf.h

This header defines the SortedListOf template. This template extends the ListOf
template by adding sorting behaviour when a node is inserted.

This template requires that the following operations be defined for a node:

  - Default Constructor

  - Copy operator

  - Less than operator

Copyright (C) 1997, Campbell Scientific, Inc.

Version 1.00
  Written by: Jon Trauntvein
  Date Begun: Wednesday 19 November 1997

*/

#ifndef SortedListOf_h
#define SortedListOf_h

#include "ListOf.h"

template <class T> class SortedListOf
{
public:
   ////////// default constructor
   SortedListOf()
   {
   }

   ////////// copy constructor
   SortedListOf(SortedListOf const &other):
      list(other.list)
   {
   }

   ////////// Destructor
   virtual ~SortedListOf()
   {
      list.flush();
   }

   ////////// copy operator
   SortedListOf &operator =(SortedListOf const &other)
   {
      list = other.list;
      return *this;
   }
   
   ////////// get_count
   uint4 get_count()
   {
      return list.get_count();
   }

   ////////// subscript operator
   // Returns a reference to the element stored at the indicated index. Note
   // that this reference is not protected. A MsgExcept object will be thrown
   // if the index is out of bounds
   T &operator [](uint4 idx)
   {
      return list.getData(idx);
   }

   ////////// getData
   // Returns a reference to the object at the specified index. Will throw a
   // MsgExcept object if that index is out of bounds.
   T &getData(uint4 idx)
   {
      return list.getData(idx);
   }

   ////////// remove
   // Removes the object stored at the specifed index. Will throw a MsgExcept
   // object if the index is out of bounds
   void remove(uint4 idx)
   {
      list.remove(idx);
   }

   ////////// copy
   // Copies the elements of another list into this list. This list will be
   // flushed prioir to the copy taking place.
   void copy(SortedListOf const &other)
   {
      list.copy(other.list);
   }

   ////////// flush
   // Empties the list of all elements
   void flush()
   {
      list.flush();
   }

   ////////// add
   // Adds a new node to the list in sorted order
   void add(T const &entry);

   ////////// getFirst
   // Returns the first item in the list. Will throw a MsgExcept object if the
   // list is empty
   T &getFirst()
   {
      return list.getFirst();
   }

   ////////// getLast
   // returns the last item in the list. Will throw a MsgExcept object if the
   // list is empty.
   T &getLast()
   {
      return list.getLast();
   }

   ////////// isValidIdx
   bool isValidIdx(uint4 idx)
   {
      return list.isValidIdx(idx);
   } 
   
private:
   ListOf<T> list;
};


template <class T>
void SortedListOf<T>::add(T const &entry)
{
   uint4 insertIdx;
   
   for(insertIdx = 0; list.isValidIdx(insertIdx); insertIdx++)
      if(entry < list[insertIdx])
         break;
   list.insert(entry,insertIdx);
} // add

#endif
