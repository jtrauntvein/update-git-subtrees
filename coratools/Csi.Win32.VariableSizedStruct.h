/* Csi.Win32.VariableSizedStruct.h

   Copyright (C) 2000, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Wednesday 27 September 2000
   Last Change: Friday 06 July 2001
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)

*/

#ifndef Csi_Win32_VariableSizedStruct_h
#define Csi_Win32_VariableSizedStruct_h

#include "CsiTypeDefs.h"

namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // template VariableSizedStruct
      //
      // Defines a class template that is intended to simplify the use of variable sized WIN32
      // structures such as those used within the telephony API (TAPI) version 2. The template
      // parameter is a specific structure tag defined by the WIN32 api. This template assumes that
      // the supplied class defines the following members:
      //
      //   dwTotalSize - Records the total number of bytes allocated for the structure
      //
      //   dwNeededSize - Records the number of bytes that was needed to store all of the
      //                  information following the API call.
      //
      //   dwUsedSize - Records the number of bytes that were used of the total allocation following
      //                the API call.
      //
      // This template defines a predicate that the application can call that evaluates whether an
      // API call was successful. It also provides operators that provide access to the underlying
      // structure pointer and its members.
      //
      // Finally, this template manages the memory for the structure.
      ////////////////////////////////////////////////////////////
      template <class win32_struct_type>
      class VariableSizedStruct
      {
      private:
         ////////////////////////////////////////////////////////////
         // storage
         //
         // The buffer that is used to provide storage for the underlying structure
         ////////////////////////////////////////////////////////////
         byte *storage;

         ////////////////////////////////////////////////////////////
         // win32_struct
         //
         // Reinterprets the storage buffer in terms of the underlying structure type.
         ////////////////////////////////////////////////////////////
         win32_struct_type *win32_struct;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //////////////////////////////////////////////////////////// 
         VariableSizedStruct():
            storage(0),
            win32_struct(0)
         {
            // allocate and initialise the storage
            storage = new byte[sizeof(win32_struct_type)];
            win32_struct = reinterpret_cast<win32_struct_type *>(storage);
            memset(win32_struct,0,sizeof(win32_struct_type));
            win32_struct->dwTotalSize = sizeof(win32_struct_type);
         } // constructor

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         ~VariableSizedStruct()
         {
            if(storage)
               delete[] storage;
         } // destructor

         ////////////////////////////////////////////////////////////
         // structure dereference operators
         //
         // Returns a reference to the structure. This makes it possible to reference structure
         // members as if this object is a pointer to the structure.
         ////////////////////////////////////////////////////////////
         win32_struct_type *operator ->() { return win32_struct; }
         win32_struct_type const *operator ->() const { return win32_struct; }
         win32_struct_type &operator *() { return *win32_struct; }
         win32_struct_type const &operator *() const { return *win32_struct; }
         
         ////////////////////////////////////////////////////////////
         // get_struct
         //
         // Returns a pointer to the structure
         ////////////////////////////////////////////////////////////
         win32_struct_type *get_struct() { return win32_struct; }
         win32_struct_type const *get_struct() const { return win32_struct; }

         ////////////////////////////////////////////////////////////
         // get_storage
         //
         // Returns a pointer to the storage buffer
         ////////////////////////////////////////////////////////////
         void *get_storage() { return storage; }
         void const *get_storage() const { return storage; }

         ////////////////////////////////////////////////////////////
         // get_storage_bytes
         ////////////////////////////////////////////////////////////
         byte *get_storage_bytes() { return storage; }
         byte const *get_storage_bytes() const { return storage; }

         ////////////////////////////////////////////////////////////
         // has_sufficient_storage
         //
         // This method compares the dwNeededSize of the structure with dwTotalSize to see if the
         // last call to the API function had sufficient storage. The return value will be true if
         // there was sufficient storage allocated. Otherwise, the return value will be false.
         ////////////////////////////////////////////////////////////
         bool has_sufficient_storage() const
         {
            bool rtn = (win32_struct->dwNeededSize <= win32_struct->dwTotalSize);
            return rtn;
         }

         ////////////////////////////////////////////////////////////
         // reallocate
         //
         // Reallocates the structure storage buffer if needed. Uses the dwTotalSize and
         // dwNeededSize members of the win32 structure to determine the amount of space that should
         // be allocated in the new structure.
         ////////////////////////////////////////////////////////////
         void reallocate()
         {
            if(!has_sufficient_storage())
            {
               // allocate a new storage buffer and initialise it with the contents of the old one
               // and fill the new space with zeroes
               byte *new_storage = new byte[win32_struct->dwNeededSize];
               memcpy(new_storage,storage,win32_struct->dwTotalSize);
               memset(
                  new_storage + win32_struct->dwTotalSize,
                  0,
                  win32_struct->dwNeededSize - win32_struct->dwTotalSize);

               // we can now get rid of the old buffer and re-assign pointers. The new allocated
               // length can also be recorded in the structure so that the next call will succeed.
               delete[] storage;
               storage = new_storage;
               win32_struct = reinterpret_cast<win32_struct_type *>(storage);
               win32_struct->dwTotalSize = win32_struct->dwNeededSize;
            }
         } // has_sufficient_storage
      };
   };
};

#endif
