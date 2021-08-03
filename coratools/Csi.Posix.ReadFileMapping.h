/* Csi.Posix.ReadFileMapping.h

   Copyright (C) 2005, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 14 April 2005
   Last Change: Friday 29 August 2014
   Last Commit: $Date: 2014-09-02 08:44:49 -0600 (Tue, 02 Sep 2014) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Posix_ReadFileMapping_h
#define Csi_Posix_ReadFileMapping_h

#include "CsiTypeDefs.h"
#include <list>
#include <stddef.h>


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class ReadFileMapping
      //
      // Defines an object that provides read-only access to Posix memory
      // mapped file and provides views of the file as demanded by the
      // application.  The constructors will claim the resources needed and the
      // destructor will free up all resources that have been claimed.
      //
      // Files opened though this object are opened with requested priviliges
      // that will allow other applications to both read and write to the
      // files.
      ////////////////////////////////////////////////////////////
      class ReadFileMapping
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         ReadFileMapping(char const *file_name);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~ReadFileMapping();

         ////////////////////////////////////////////////////////////
         // open_view
         ////////////////////////////////////////////////////////////
         void const *open_view(
            int8 offset = 0,
            uint4 size = 0);

         ////////////////////////////////////////////////////////////
         // close_view
         ////////////////////////////////////////////////////////////
         typedef std::pair<void *, size_t> view_type;
         void close_view(void const *view_base);

         ////////////////////////////////////////////////////////////
         // file_size
         ////////////////////////////////////////////////////////////
         int8 file_size();

      private:
         ////////////////////////////////////////////////////////////
         // file_desc
         //
         // The file descriptor handle.  This value is opened in the
         // constructor using the provided file name.  This descriptor will be
         // used to map portions of the file opened view open_view().
         ////////////////////////////////////////////////////////////
         int file_desc;

         ////////////////////////////////////////////////////////////
         // views
         //
         // The list of views that have been created using open_view() but that
         // have not yet been closed using close_view().  Anything remaining in
         // this list will be closed by the destructor. 
         ////////////////////////////////////////////////////////////
         typedef std::list<view_type> views_type;
         views_type views;
      };
   };
};


#endif
