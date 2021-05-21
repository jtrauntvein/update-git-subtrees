/* QueueOf.h

   Copyright (C) 1998, Campbell Scientific, Inc
   
   Written by: Jon Trauntvein
   Date Begun: Monday 06 July 1998
   Last Change: Friday 06 April 2001
   Last Commity: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef QueueOf_h
#define QueueOf_h

#include "RingBuffIdx.h"
#include "MsgExcept.h"
#include "Csi.MaxMin.h"


////////// template QueueOf
// Defines a generic queue container for classes that define default constructors, copy
// operators. The interface and implementation is based upon that for class ByteQueue.
template <class T> class QueueOf
{
public:
   ////////// constructor
   // Specifies the initial capacity of the queue
   QueueOf(uint4 initial_cap):
      buff(0),
      write_loc(0),
      read_loc(0),
      capacity(0)
   {
      check_cap(initial_cap);
   } 

   ////////// destructor
   ~QueueOf()
   {
      if(buff)
         delete[] buff;
   } 

   ////////// size
   // Returns the number of unread elements presently available in the queue
   uint4 size() const
   {
      RingBuffIdx<uint4> idx(capacity);
      return idx.diff(read_loc,write_loc);
   }

   ////////// empty
   // Returns true if there are no unread elements available in the queue
   bool empty() const
   {
      return read_loc == write_loc;
   } 

   ////////// push
   // Pushes a single object onto the queue. Calls check_cap with an argument of 1 to ensure that
   // there is sufficient space for the object.
   void push(T const &s);
   
   ////////// push
   // Pushes each of the specified array of objects into the queue. Calls check_cap to ensure that
   // sufficient capacity is present for the operation
   void push(T const *src, uint4 len);

   ////////// copy
   // Copies up to the specified number of objects from the head of the queue without affecting the
   // head of the queue. Returns the actual number of objects that were copied.
   uint4 copy(T *dest, uint4 len);

   ////////// pop
   // Copies up to the specified number of objects into the specified array and advances the read
   // pointer. Returns the actual number of objects that were copied.
   uint4 pop(T *dest, uint4 len);

   ////////// pop
   // Advances the read pointer past a maximum of len objects. Returns the actual number of objects
   // that were passed.
   uint4 pop(uint4 len);

   ////////// front
   // Returns a reference to the object at the front of the queue. Will throw MsgExcept object if
   // the container is empty
   T &front()
   {
      if(read_loc == write_loc)
         throw MsgExcept("QueueOf::front: Access attempt on an empty queue");
      return buff[read_loc];
   } 

   ////////// subscript operator
   // Returns a reference to the object stored in the queue relative to the front. Will throw a
   // MsgExcept object if the specified offset is out of bounds
   T &operator[] (uint4 idx);
   
private:
   ////////// buff
   // Array of T objects used for storing the queue. The size of this array can exceed the number of
   // objects stored within it.
   T *buff;

   ////////// write_loc
   // Offset of the next object that will be written (pushed) to the queue
   uint4 write_loc;

   ////////// read_loc
   // The offset of the next object that will be read (popped) from the queue. The number of
   // elements available for reading in the queue is calculated by subtracting this value from the
   // value stored in write_loc
   uint4 read_loc;

   ////////// capacity
   // The total number of elements that can be stored in the buffer without expanding it. This value
   // can change after the check_cap method is invoked.
   uint4 capacity;

   ////////// check_cap
   // Checks the capacity of the queue to determine if the specified number of objects can be
   // written to the queue without overwriting unread objects. If the capacity is insufficient, the
   // buffer will be reallocated to the maximum of twice the current capacity or the required
   // amount.
   void check_cap(uint4 needed);
};


template <class T>
void QueueOf<T>::push(T const &s)
{
   check_cap(1);
   buff[write_loc++] = s;
   if(write_loc >= capacity)
      write_loc = 0;
} // push (single)


template <class T>
void QueueOf<T>::push(T const *src, uint4 len)
{
   check_cap(len);
   for(uint4 i = 0; i < len; i++)
   {
      buff[write_loc++] = src[i];
      if(write_loc >= capacity)
         write_loc = 0;
   } 
} // push


template <class T>
uint4 QueueOf<T>::copy(T *dest, uint4 len)
{
   uint4 loc = read_loc;
   uint4 rtn = 0;

   while(rtn < len && loc != write_loc)
   {
      dest[rtn++] = buff[loc++];
      if(loc >= capacity)
         loc = 0;
   }
   return rtn;
} // copy


template <class T>
uint4 QueueOf<T>::pop(T *dest, uint4 len)
{
   uint4 rtn = 0;
   while(rtn < len && read_loc != write_loc)
   {
      dest[rtn++] = buff[read_loc++];
      if(read_loc >= capacity)
         read_loc = 0;
   }
   return rtn;
} // pop


template <class T>
uint4 QueueOf<T>::pop(uint4 len)
{
   uint4 rtn = Csi::csimin(len,size());
   read_loc += rtn;
   if(read_loc >= capacity)
      read_loc -= capacity;
   return rtn;
} // pop (no copy)


template <class T>
void QueueOf<T>::check_cap(uint4 needed)
{
   uint4 old_size = size();
   uint4 new_size = old_size + needed;

   if(new_size >= capacity)
   {
      // allocate a new buffer for the queue
      uint4 new_cap = Csi::csimax(capacity*2,new_size);
      T *new_buff = new T[new_cap];
      uint4 new_write_loc = 0;

      // copy the contents of the old buffer into the new one
      for(uint4 i = 0; i < old_size; i++)
      {
         new_buff[new_write_loc++] = buff[read_loc++];
         if(read_loc >= capacity)
            read_loc = 0;
      }

      // we can now clean up
      delete[] buff;
      buff = new_buff;
      read_loc = 0;
      write_loc = new_write_loc;
      capacity = new_cap;
   }
} // check_cap


template <class T>
T &QueueOf<T>::operator[] (uint4 idx)
{
   uint4 i = read_loc + idx;

   if(idx >= size())
      throw MsgExcept("QueueOf::subscript: Access out of bounds");
   if(i >= capacity)
      i -= capacity;
   return buff[i];
 } // subscript operator


#endif
