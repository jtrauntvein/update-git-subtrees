/* Csi.Win32.ReadFileMapping.h

   Copyright (C) 2002, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 29 August 2002
   Last Change: Wednesday 16 March 2011
   Last Commit: $Date: 2012-07-23 12:19:48 -0600 (Mon, 23 Jul 2012) $ 
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_Win32_ReadFileMapping_h
#define Csi_Win32_ReadFileMapping_h

#define  WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>
#include <list>
#include "CsiTypeDefs.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class ReadFileMapping
      //
      // Defines an object that provides read only access to a WIN32 file mapping object.  WIN32
      // calls are made to create a file mapping and to provide mapped views of the file as demanded
      // by the application.  The constructors and destructor will clean up all resources associated
      // with the file mapping when the object is destroyed.
      //
      // Files opened through this object are opened such that other applications can open them such
      // that the files can be opened by other applications for reading but not for writing.
      ////////////////////////////////////////////////////////////
      class ReadFileMapping
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         //
         // Opens the file specified by the file_name parameter as a memory mapped file in read only
         // mode.  If the file cannot be opened or the mapping not created, an exception object
         // derived from std::exception will be thrown.
         ////////////////////////////////////////////////////////////
         ReadFileMapping(char const *file_name);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         ~ReadFileMapping();

         ////////////////////////////////////////////////////////////
         // open_view
         //
         // Opens a view to the entire file.  The return value will be the base pointer for the
         // view.  The object will keep track of all views opened through this method and will
         // automatically unmap them in the destructor if they have not already been unmapped by the
         // application calling close_view().
         //
         // The offset and size parameters can be used by the application to map a region of the
         // file.  The default values of these parameters will map the entire file for reading.  If
         // opening the view fails, an object derived from std::exception will be thrown.
         ////////////////////////////////////////////////////////////
         void const *open_view(
            int8 offset = 0,
            uint4 size = 0);

         ////////////////////////////////////////////////////////////
         // close_view
         //
         // Closes a view that was created using open_view().
         ////////////////////////////////////////////////////////////
         void close_view(void const *view);

         ////////////////////////////////////////////////////////////
         // file_size
         //
         // Returns the total size of the file.
         ////////////////////////////////////////////////////////////
         int8 file_size() const;

      private:
         ////////////////////////////////////////////////////////////
         // file_handle
         //
         // The handle to open file object.
         ////////////////////////////////////////////////////////////
         HANDLE file_handle;

         ////////////////////////////////////////////////////////////
         // map_handle
         //
         // Handle to the memory mapped object
         ////////////////////////////////////////////////////////////
         HANDLE map_handle;

         ////////////////////////////////////////////////////////////
         // views
         ////////////////////////////////////////////////////////////
         typedef std::list<void const *> views_type;
         views_type views;
      };
   };
};


#endif
