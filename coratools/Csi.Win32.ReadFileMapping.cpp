/* Csi.Win32.ReadFileMapping.cpp

   Copyright (C) 2002, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 29 August 2002
   Last Change: Thursday 21 November 2013
   Last Commit: $Date: 2013-11-21 08:07:29 -0600 (Thu, 21 Nov 2013) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Win32.ReadFileMapping.h"
#include "Csi.OsException.h"
#include "StrUni.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class ReadFileMapping definitions
      ////////////////////////////////////////////////////////////
      ReadFileMapping::ReadFileMapping(char const *file_name):
         file_handle(INVALID_HANDLE_VALUE),
         map_handle(INVALID_HANDLE_VALUE)
      {
         // we need to get a handle to the file before the mapping object can be created.
         StrUni wname(file_name);
         file_handle = ::CreateFileW(
            wname.c_str(),
            GENERIC_READ,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            0,                  // default security
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            0);                 // no template
         if(file_handle == INVALID_HANDLE_VALUE)
            throw OsException("file open failed");

         // we can now create the mapping object
         map_handle = ::CreateFileMapping(
            file_handle,
            0,                  // default security
            PAGE_READONLY,
            0,0,                // use file size
            0);                 // no name assigned to the mapping
         if(map_handle == INVALID_HANDLE_VALUE)
         {
            ::CloseHandle(file_handle);
            file_handle = INVALID_HANDLE_VALUE;
            throw OsException("faile mapping failed");
         }
      } // constructor

      
      ReadFileMapping::~ReadFileMapping()
      {
         // we will destroy the views, mapping, and finally the file handle
         while(!views.empty())
         {
            ::UnmapViewOfFile(views.front());
            views.pop_front();
         }
         if(map_handle != INVALID_HANDLE_VALUE)
            CloseHandle(map_handle);
         if(file_handle != INVALID_HANDLE_VALUE)
            CloseHandle(file_handle);
      } // destructor

      
      void const *ReadFileMapping::open_view(
         int8 offset,
         uint4 size)
      {
         void const *rtn = ::MapViewOfFile(
            map_handle,
            FILE_MAP_READ,
            static_cast<DWORD>((offset & 0xffffffff00000000) >> 32),
            static_cast<DWORD>(offset & 0xffffffff),
            size);
         if(rtn == 0)
            throw OsException("open_view failure");
         views.push_back(rtn);
         return rtn;
      } // open_view

      
      void ReadFileMapping::close_view(void const *view)
      {
         // search for the view in the list
         for(views_type::iterator vi = views.begin();
             vi != views.end();
             ++vi)
         {
            if(view == *vi)
            {
               views.erase(vi);
               ::UnmapViewOfFile(view);
               break;
            }
         }
      } // close_view

      
      int8 ReadFileMapping::file_size() const
      {
         uint4 high_word;
         int8 rtn = ::GetFileSize(file_handle,&high_word);
         rtn |= static_cast<int8>(high_word) << 32;
         return rtn;
      } // file_size 
   };
};
