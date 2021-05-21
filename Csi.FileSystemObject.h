/* Csi.FileSystemObject.h

   Copyright (C) 2005, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 04 April 2005
   Last Change: Tuesday 05 March 2019
   Last Commit: $Date: 2019-03-05 17:29:13 -0600 (Tue, 05 Mar 2019) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_FileSystemObject_h
#define Csi_FileSystemObject_h

#ifdef _WIN32
#include "Csi.Win32.FileSystemObject.h"
#else
#include "Csi.Posix.FileSystemObject.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::FileSystemObject;
#else
   using Posix::FileSystemObject;
#endif


   /**
    * Defines a predicate that compares the creation date of two file system objects.
    */
   struct file_create_date_less
   {
      bool operator ()(FileSystemObject const &f1, FileSystemObject const &f2) const
      { return f1.get_creation_time() < f2.get_creation_time(); }
   };


   /**
    * Defines a predicate that compares the last modification date of two file system objects.
    */
   struct file_last_write_date_less
   {
      bool operator ()(FileSystemObject const &f1, FileSystemObject const &f2) const
      { return f1.get_last_write_date() < f2.get_last_write_date(); }
   };


   /**
    * Defines a predicate that compares the size of two file system objects.
    */
   struct file_size_less
   {
      bool operator ()(FileSystemObject const &f1, FileSystemObject const &f2) const
      { return f1.get_size() < f2.get_size(); }
   };


   /**
    * Defines a predicate the compares the names of two file system objects.
    */
   struct file_name_less
   {
      bool operator ()(FileSystemObject const &f1, FileSystemObject const &f2) const
      { return f1.get_name() < f2.get_name(); }
   };


   /**
    * Defines a template function that constructs a path string for the elements in the provided set
    * of iterators.
    *
    * @param begin Specifies the iterator that starts the sequence of path components.
    *
    * @param end Specifies the iterator beyond the end of path components.
    */
   template <typename iterator>
   StrAsc join_path(iterator begin, iterator end)
   {
      StrAsc rtn;
      iterator ni = begin;
      while(ni != end)
      {
         if(ni != begin && rtn.last() != FileSystemObject::dir_separator())
            rtn.append(FileSystemObject::dir_separator());
         rtn.append(*ni);
         ++ni;
      }
      return rtn;
   }
};


#endif
