/* ByteQueue.cpp

   Copyright (C) 1998, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 29 May 1998
   Last Change: Monday 25 March 2013
   Last Commit: $Date: 2013-03-25 13:19:05 -0600 (Mon, 25 Mar 2013) $ 
   Commited By: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.ByteQueue.h"
#include "RingBuffIdx.h"
#include "Csi.MaxMin.h"
#include <algorithm>


namespace Csi
{
   namespace
   {
      ////////////////////////////////////////////////////////////
      // class ByteQueueLocker
      ////////////////////////////////////////////////////////////
      class ByteQueueLocker
      {
      private:
         ////////////////////////////////////////////////////////////
         // queue
         ////////////////////////////////////////////////////////////
         ByteQueue const *queue;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ByteQueueLocker(ByteQueue const *queue_):
            queue(queue_)
         { queue->lock(); }
         
         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         ~ByteQueueLocker()
         { queue->unlock(); }
      };
   };

   
   ////////////////////////////////////////////////////////////
   // class ByteQueue definitions
   ////////////////////////////////////////////////////////////
   ByteQueue::ByteQueue(uint4 initial_cap, bool use_protector_):
      buff(0),
      write_loc(0),
      read_loc(0),
      capacity(0),
      use_protector(use_protector_)
   {
      check_cap(initial_cap);
   } // constructor


   ByteQueue::ByteQueue(ByteQueue const &other):
      buff(0),
      write_loc(0),
      read_loc(0),
      capacity(0),
      use_protector(other.use_protector)
   {
      ByteQueueLocker lock(&other);
      uint4 other_read = other.read_loc;

      check_cap(other.size());
      while(other_read != other.write_loc)
      {
         buff[write_loc++] = other.buff[other_read++];
         if(other_read >= other.capacity)
            other_read = 0;
      }
   } // copy constructor


   ByteQueue &ByteQueue::operator =(ByteQueue const &other)
   {
      ByteQueueLocker this_lock(this);
      ByteQueueLocker other_lock(&other);
      uint4 other_read(other.read_loc);
      
      check_cap(other.size());
      while(other_read != other.write_loc)
      {
         buff[write_loc++] = other.buff[other_read++];
         if(other_read >= other.capacity)
            other_read = 0;
      }
      return *this;
   } // copy operator


   ByteQueue::~ByteQueue()
   {
      if(buff)
         delete[] buff;
   } // destructor


   uint4 ByteQueue::size() const
   {
      ByteQueueLocker lock(this);
      RingBuffIdx<uint4> idx(capacity);
      return idx.diff(read_loc,write_loc);
   } // size


   bool ByteQueue::empty() const
   {
      ByteQueueLocker lock(this);
      return read_loc == write_loc;
   } // empty


   void ByteQueue::push(void const *src, uint4 len)
   {
      ByteQueueLocker lock(this);
      byte *bsrc = (byte *)src;
      check_cap(len);
      for(uint4 i = 0; i < len; i++)
      {
         buff[write_loc++] = bsrc[i];
         if(write_loc >= capacity)
            write_loc = 0;
      }
   } // push


   uint4 ByteQueue::copy(void *dest, uint4 len, uint4 start_pos)
   {
      ByteQueueLocker lock(this);
      uint4 loc = read_loc;
      uint4 rtn = 0;
      byte *bdest = (byte *)dest;
      uint4 pos(0);
      
      while(rtn < len && loc != write_loc)
      {
         if(pos >= start_pos)
            bdest[rtn++] = buff[loc];
         ++pos;
         ++loc;
         if(loc >= capacity)
            loc = 0;
      }
      return rtn;
   } // copy


   uint4 ByteQueue::pop(void *dest, uint4 len)
   {
      ByteQueueLocker lock(this);
      byte *bdest = (byte *)dest;
      uint4 rtn = 0;

      while(rtn < len && read_loc != write_loc)
      {
         bdest[rtn++] = buff[read_loc++];
         if(read_loc >= capacity)
            read_loc = 0;
      }
      return rtn;
   } // pop (with copy)


   uint4 ByteQueue::pop(uint4 len)
   {
      ByteQueueLocker lock(this);
      uint4 rtn = Csi::csimin(len,size());
      read_loc += rtn;
      if(read_loc >= capacity)
         read_loc -= capacity;
      return rtn;
   } // pop (no copy)


   uint4 ByteQueue::pop(StrBin &dest, uint4 len)
   {
      ByteQueueLocker lock(this);
      uint4 rtn = 0;
      dest.cut(0);
      dest.reserve(len);
      while(rtn < len && read_loc != write_loc)
      {
         dest.append(this->buff[read_loc++]);
         if(read_loc >= capacity)
            read_loc = 0;
         rtn++;
      }
      return rtn;
   } // pop (to StrBin)


   uint4 ByteQueue::pop(StrAsc &dest, uint4 len, bool clear_first)
   {
      ByteQueueLocker lock(this);
      uint4 rtn = 0;
      if(clear_first)
         dest.cut(0);
      dest.reserve(dest.length() + len);
      while(rtn < len && read_loc != write_loc)
      {
         dest.append(this->buff[read_loc++]);
         if(read_loc >= capacity)
            read_loc = 0;
         ++rtn;
      }
      return rtn;
   } // pop


   uint4 ByteQueue::pop(ByteQueue &dest, uint4 len)
   {
      ByteQueueLocker this_lock(this);
      ByteQueueLocker dest_lock(&dest);
      uint4 rtn = 0;
      dest.reserve(len);
      while(rtn < len && read_loc != write_loc)
      {
         dest.buff[dest.write_loc++] = this->buff[read_loc++];
         if(dest.write_loc >= dest.capacity)
            dest.write_loc = 0;
         if(read_loc >= capacity)
            read_loc = 0;
         ++rtn;
      }
      return rtn;
   } // pop (to other queue)


   uint4 ByteQueue::find(void const *pat, uint4 pat_len) const
   {
      // first check to see if it even possible for the queue to hold the pattern
      ByteQueueLocker lock(this);
      uint4 current_size = size();
      uint4 rtn;
      byte const *test = (byte const *)pat;
      bool found = false;

      if(pat_len > 0)
      {
         for(rtn = 0; !found && rtn + pat_len <= current_size; rtn++)
         {
            uint4 i = (rtn + read_loc)%capacity;
            uint4 j = 0;
         
            while(j < pat_len && buff[(i + j)%capacity] == test[j])
               ++j;
            if(j >= pat_len)
            { found = true; break; }
         }
         if(!found)
            rtn = current_size;
      }
      else
         rtn = current_size;
      return rtn;
   } // find


   byte const &ByteQueue::operator [](uint4 offset) const
   {
      ByteQueueLocker lock(this);
      return buff[(read_loc + offset)%capacity];
   } // subscript operator


   void ByteQueue::check_cap(uint4 needed)
   {
      ByteQueueLocker lock(this);
      uint4 old_size = size();
      uint4 new_size = old_size + needed;
   
      if(new_size >= capacity)
      {
         // allocate a new buffer for the queue
         uint4 new_cap = Csi::csimax(capacity*2,new_size + 1);
         byte *new_buff = new byte[new_cap];
         uint4 new_write_loc = 0;
      
         // now copy the old buffer contents into the new buffer
         for(uint4 i = 0; i < old_size; i++)
         {
            new_buff[new_write_loc++] = buff[read_loc++];
            if(read_loc >= capacity)
               read_loc = 0;
         }
      
         // we can now delete the old buffer
         delete[] buff;
         buff = new_buff;
         read_loc = 0;
         write_loc = new_write_loc;
         capacity = new_cap;
      } 
   } // check_cap
};
