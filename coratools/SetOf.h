/* SetOf.h

This header defines the SetOf template. The purpose of this template is to use an arrya of the
specified object type to hold a collection of unique objects (unique within the scope of the set)
that supports standard set operations (membership test, union, difference).

Copyright (C) 1998, Campbell Scientific, Inc.

Version 1.00
  Written by: Jon Trauntvein
  Date Begun: Thursday 15 January 1998

*/

#ifndef SetOf_h
#define SetOf_h

template<class T, uint4 maxSize> class SetOf
{
public:
   ////////// Default constructor
   // Constructs the empty set
   SetOf():
      count(0)
   {
   }
   
   
   ////////// Array constructor
   // Inserts the collection of values in the array. Duplicated values in the array will be ignored
   SetOf(T const *ary, uint4 len);

   ////////// copy constructor
   // Makes an exact duplicate of another set
   SetOf(SetOf const &other):
      count(0)
   {
      copy(other);
   }
   
   ////////// destructor
   ~SetOf()
   {
   }

   ////////// copy operator (same operation as the copy constructor)
   SetOf &operator =(SetOf const &other)
   {
      copy(other);
      return *this;
   }

   ////////// get_count
   // Returns the number of items in the set. The empty set will have a count of zero
   uint4 get_count() const
   {
      return count;
   } 

   ////////// in
   // Returns a value of true if the the object specified has an object in the set that is equal
   bool in(T const &val) const;

   ////////// flush
   // Empties all values from the set. Makes this set object into an empty set
   void flush();

   ////////// insert
   // Inserts a new value into the set. If the same value already exists in the set, no operation
   // will occur and the return value will be false. 
   bool insert(T const &val);

   ////////// remove
   // Removes a value from the set that is equal to the specified value. If the value does not exist
   // in the set, nothing will be done. 
   void remove(T const &val);

   ////////// isValidIdx
   // Returns true if the specified index is valid
   bool isValidIdx(uint4 pos) const
   {
      bool rtn = true;

      if(pos >= count)
         rtn = false;
      return rtn;
   }

   ////////// subscript operator
   // Returns the value at the specified position in the set. Will throw a MsgExcept object if the
   // specified index is out of bounds
   T const &operator [](uint4 pos) const
   {
      if(!isValidIdx(pos))
         throw MsgExcept("SetOf::operator []: Invalid index");
      return members[pos];
   } 
   
private:
   ////////// members
   // Storage for all possible members
   T members[maxSize];

   ////////// count
   // The number of members that have been stored
   uint4 count;
   
   ////////// copy
   // Performs the copy operation for the copy operator and copy contructor
   void copy(SetOf const &other); 
};

template<class T, uint4 maxSize>
SetOf<T,maxSize>::SetOf(T const *ary, uint4 len):
   count(0)
{
   for(uint4 i = 0; i < len; i++)
      insert(ary[i]);
} // array constructor


template<class T, uint4 maxSize>
bool SetOf<T,maxSize>::in(T const &val) const
{
   bool rtn = false;

   for(uint4 i = 0; !rtn && i < count; i++)
      if(members[i] == val)
         rtn = true;
   return rtn;
} // in


template<class T, uint4 maxSize>
bool SetOf<T,maxSize>::insert(T const &val)
{
   // scan the members for a duplicate value
   bool rtn = true;
   uint4 i;
   
   for(i = 0; rtn && i < count; i++)
      if(members[i] == val)
         rtn = false;
   if(rtn && count < maxSize)
   {
      members[count] = val;
      count++;
   }
   return rtn;
} // insert


template<class T, uint4 maxSize>
void SetOf<T,maxSize>::remove(T const &val)
{
   for(uint4 i = 0; i < count; i++)
      if(members[i] == val)
      {
         for(uint4 j = 0; i + j + 1 < count; j++)
            members[i + j] = members[i + j + 1];
         count--;
      }
} // remove


template<class T, uint4 maxSize>
void SetOf<T,maxSize>::copy(SetOf<T,maxSize> const &other)
{
   for(uint4 i = 0; i < count; i++)
      members[i] = other.members[i];
   count = other.count;
} // copy


#endif
