/* Csi.ByteQueue.h

   Copyright (C) 1998, 2012 Campbell Scientific, Inc.
   
   Written by: Jon Trauntvein
   Date Begun: Friday 29 May 1998
   Last Change: Monday 01 October 2012
   Last Commit: $Date: 2013-03-25 13:19:05 -0600 (Mon, 25 Mar 2013) $ 
   Committed by: $Author: jon $
   
*/

#ifndef Csi_ByteQueue_h
#define Csi_ByteQueue_h

#include "StrBin.h"
#include "StrAsc.h"
#include "Csi.CriticalSection.h"


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class ByteQueue
   //
   // Defines an expandable ring buffer of bytes (unsigned characters) that works
   // as a queue (first in, first out).
   ////////////////////////////////////////////////////////////
   class ByteQueue
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      //
      // Specifies the initial capacity of the queue.  The use_protector flag
      // will control whether the various methods of this queue are protected
      // using a critical section.  It would be set to true if there is a
      // possibility of multiple threads attempting to access the same queue at
      // the same time.
      //////////////////////////////////////////////////////////// 
      ByteQueue(uint4 initial_cap = 100, bool use_protector_ = false);
      
      ////////////////////////////////////////////////////////////
      // copy constructor
      //////////////////////////////////////////////////////////// 
      ByteQueue(ByteQueue const &other);

      ////////////////////////////////////////////////////////////
      // copy operator
      ////////////////////////////////////////////////////////////
      ByteQueue &operator =(ByteQueue const &other);
      
      ////////////////////////////////////////////////////////////
      // destructor
      //////////////////////////////////////////////////////////// 
      ~ByteQueue();
      
      ////////////////////////////////////////////////////////////
      // size
      //
      // Returns the number of elements presently stored in the queue by
      // calculating the difference between the read loacation and the write
      // location
      //////////////////////////////////////////////////////////// 
      uint4 size() const;
      
      ////////////////////////////////////////////////////////////
      // empty
      //
      // Returns true if there are no bytes in the queue that have not been popped
      //////////////////////////////////////////////////////////// 
      bool empty() const;
      
      ////////////////////////////////////////////////////////////
      // push
      //
      // Pushes the supplied bytes onto the queue making sure that there is enough
      // capacity to accomodate them
      //////////////////////////////////////////////////////////// 
      void push(void const *src, uint4 len);
      
      ////////////////////////////////////////////////////////////
      // copy
      //
      // Copies up to the specified number of characters from the head of the
      // queue without affecting the queue's read location. Returns the actual
      // number of characters that were copied
      //////////////////////////////////////////////////////////// 
      uint4 copy(void *dest, uint4 len, uint4 start_pos = 0);
      
      ////////////////////////////////////////////////////////////
      // pop
      //
      // Pops up to len bytes from the end of the queue and copies them into the
      // supplied buffer. Returns the actual number of bytes that were popped
      //////////////////////////////////////////////////////////// 
      uint4 pop(void *dest, uint4 len); 
      
      ////////////////////////////////////////////////////////////
      // pop
      //
      // Removes the up to the specified number of characters from the queue
      // without copying them. Returns the actual number of characters that were
      // removed
      //////////////////////////////////////////////////////////// 
      uint4 pop(uint4 len);
      
      ////////////////////////////////////////////////////////////
      // pop
      //
      // Removes up to the specified number of characters from the queue and
      // copies them into the specified buffer object. Returns the actual number
      // of characters that were popped
      //////////////////////////////////////////////////////////// 
      uint4 pop(StrBin &dest, uint4 len);
      
      ////////////////////////////////////////////////////////////
      // pop (to StrAsc)
      //
      // Removes up to the specified number of characters from the queue and
      // copies them into the specified StrAsc object.  The return value will
      // reflect the actual number of characters that were read. 
      ////////////////////////////////////////////////////////////
      uint4 pop(StrAsc &dest, uint4 len, bool clear_first = true);
      
      ////////////////////////////////////////////////////////////
      // pop
      //
      // Removes the specified number of character from the queue and places them
      // in the destination queue.  Returns the actual number of characters that
      // were removed. 
      ////////////////////////////////////////////////////////////
      uint4 pop(ByteQueue &dest, uint4 len);
      
      ////////////////////////////////////////////////////////////
      // find
      //
      // Searches for the specified pattern in the bytes stored in the queue. If
      // the pattern could be found, the return value will indicate an offset
      // between the beginning of the queue and the beginning of the pattern. If
      // the pattern cannot be found, the return value will be greater than the
      // current size of the queue
      //////////////////////////////////////////////////////////// 
      uint4 find(void const *pat, uint4 pat_len) const;
      
      ////////////////////////////////////////////////////////////
      // subscript operator (const version)
      //
      // Returns the byte at the specified offset
      //////////////////////////////////////////////////////////// 
      byte const &operator [](uint4 offset) const; 
      
      ////////////////////////////////////////////////////////////
      // reserve
      //
      // Makes sure that the queue has enough capacity to hold the specified amount.
      //////////////////////////////////////////////////////////// 
      void reserve(uint4 extra)
      { check_cap(extra); }

      ////////////////////////////////////////////////////////////
      // lock
      //
      // Engages the critical section if the use_protector flag is set. 
      ////////////////////////////////////////////////////////////
      void lock() const
      {
         if(use_protector)
            protector.lock();
      }

      ////////////////////////////////////////////////////////////
      // unlock
      //
      // Releases the critical section if the use_protector flag is set. 
      ////////////////////////////////////////////////////////////
      void unlock() const
      {
         if(use_protector)
            protector.unlock();
      }
      
   private:
      ////////////////////////////////////////////////////////////
      // check_cap
      //
      // Checks the queue capacity to see if the specified number of bytes can be
      // added. If the capacity is insufficient, the capacity will be increased to
      // the maximum of twice the current capacity or the amount of needed extra
      // growth. A new buffer will then be allocated and the old contents copied
      // into it.
      //////////////////////////////////////////////////////////// 
      void check_cap(uint4 needed);
      
   private:
      ////////////////////////////////////////////////////////////
      // buff
      //
      // Storage for the byte queue
      //////////////////////////////////////////////////////////// 
      byte *buff;
      
      ////////////////////////////////////////////////////////////
      // write_loc
      //
      // Offset of the next byte to be written (pushed) from the queue
      //////////////////////////////////////////////////////////// 
      uint4 write_loc;
      
      ////////////////////////////////////////////////////////////
      // read_loc
      //
      // Offset on the next byte 
      //////////////////////////////////////////////////////////// 
      uint4 read_loc;
      
      ////////////////////////////////////////////////////////////
      // capacity
      //
      // The current capacity of the queue 
      //////////////////////////////////////////////////////////// 
      uint4 capacity;

      ////////////////////////////////////////////////////////////
      // protector
      ////////////////////////////////////////////////////////////
      mutable CriticalSection protector;

      ////////////////////////////////////////////////////////////
      // use_protector
      ////////////////////////////////////////////////////////////
      bool const use_protector;
   };
};


#endif
