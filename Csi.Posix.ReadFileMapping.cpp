/* Csi.Posix.ReadFileMapping.cpp

   Copyright (C) 2005, 2014 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 03 August 2005
   Last Change: Friday 29 August 2014
   Last Commit: $Date: 2014-09-02 08:44:49 -0600 (Tue, 02 Sep 2014) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.ReadFileMapping.h"
#include "Csi.Posix.OsException.h"
#include <algorithm>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class ReadFileMapping definitions
      ////////////////////////////////////////////////////////////
      ReadFileMapping::ReadFileMapping(char const *path)
      {
         file_desc = open(path,O_RDONLY);
         if(file_desc == -1)
            throw OsException("ReadFileMapping::constructor failed");
      } // constructor


      ReadFileMapping::~ReadFileMapping()
      {
         while(!views.empty())
            close_view(views.front().first);
         close(file_desc);
      } // destructor


      void const *ReadFileMapping::open_view(
         int8 offset,
         uint4 size)
      {
         void *rtn = mmap(
            0,                  // don't specify the address
            static_cast<size_t>(size),
            PROT_READ,
            MAP_PRIVATE,
            file_desc,
            static_cast<off_t>(offset));
         if(rtn == MAP_FAILED)
            throw OsException("ReadFileMapping::open_view failed");
         views.push_back(view_type(rtn,size));
         return rtn;
      } // open_view


      namespace
      {
         ////////////////////////////////////////////////////////////
         // class has_view
         ////////////////////////////////////////////////////////////
         class has_view
         {
         public:
            ////////////////////////////////////////////////////////////
            // view
            ////////////////////////////////////////////////////////////
            void const *view;

            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            has_view(void const *view_):
               view(view_)
            { }

            ////////////////////////////////////////////////////////////
            // functor
            ////////////////////////////////////////////////////////////
            bool operator () (ReadFileMapping::view_type &v)
            { return v.first == view; }
         };
      };

      
      void ReadFileMapping::close_view(void const *view_base)
      {
         views_type::iterator vi = std::find_if(
            views.begin(),
            views.end(),
            has_view(view_base));
         if(vi != views.end())
         {
            munmap(vi->first,vi->second);
            views.erase(vi);
         }
      } // close_view


      int8 ReadFileMapping::file_size()
      {
         struct stat file_info;
         int8 rtn = 0;
         
         if(fstat(file_desc,&file_info) == 0)
            rtn = file_info.st_size;
         return rtn;
      } // file_size
   };
};
