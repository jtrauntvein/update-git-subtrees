/* Csi.fstream.h

   Copyright (C) 2012, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Friday 16 November 2012
   Last Change: Thursday 29 November 2012
   Last Commit: $Date: 2012-11-29 08:16:15 -0600 (Thu, 29 Nov 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_fstream_h
#define Csi_fstream_h

#include "StrUni.h"
#include <fstream>


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class ifstream
   //
   // Defines an extension of class std::ifstream that is able to open files with unicode file
   // names. 
   ////////////////////////////////////////////////////////////
   class ifstream: public std::ifstream
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      ifstream(char const *file_name, std::ios_base::openmode mode = std::ios_base::in)
      {
         open(StrUni(file_name, true), mode);
      }

      ////////////////////////////////////////////////////////////
      // unicode constructor
      ////////////////////////////////////////////////////////////
      ifstream(StrUni const &file_name, std::ios_base::openmode mode = std::ios_base::in)
      {
         open(file_name, mode);
      }

      ////////////////////////////////////////////////////////////
      // default constructor
      ////////////////////////////////////////////////////////////
      ifstream()
      { }

      ////////////////////////////////////////////////////////////
      // open (unicode)
      ////////////////////////////////////////////////////////////
      void open(StrUni const &file_name, std::ios_base::openmode mode = std::ios_base::in)
      {
#ifdef WIN32
         std::ifstream::open(file_name.c_str(), mode);
#else
         std::ifstream::open(file_name.to_utf8().c_str(), mode);
#endif
      }
      
      ////////////////////////////////////////////////////////////
      // open (utf-8)
      ////////////////////////////////////////////////////////////
      void open(char const *file_name, std::ios_base::openmode mode = std::ios_base::in)
      {
#ifdef WIN32
         open(StrUni(file_name), mode);
#else
         std::ifstream::open(file_name, mode);
#endif
      }
   };


   ////////////////////////////////////////////////////////////
   // class ofstream
   //
   // Provides an extension of std::ofstream that is able to open files with
   // unicode specified file names.
   ////////////////////////////////////////////////////////////
   class ofstream: public std::ofstream
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      ofstream(char const *file_name, std::ios_base::openmode mode = std::ios_base::out)
      {
         open(StrUni(file_name, true), mode);
      }

      ////////////////////////////////////////////////////////////
      // unicode constructor
      ////////////////////////////////////////////////////////////
      ofstream(StrUni const &file_name, std::ios_base::openmode mode = std::ios_base::out)
      {
         open(file_name, mode);
      }

      ////////////////////////////////////////////////////////////
      // default constructor
      ////////////////////////////////////////////////////////////
      ofstream()
      { }

      ////////////////////////////////////////////////////////////
      // open (unicode)
      ////////////////////////////////////////////////////////////
      void open(StrUni const &file_name, std::ios_base::openmode mode = std::ios_base::out)
      {
#ifdef WIN32
         std::ofstream::open(file_name.c_str(), mode);
#else
         std::ofstream::open(file_name.to_utf8().c_str(), mode);
#endif
      }
      
      ////////////////////////////////////////////////////////////
      // open (utf-8)
      ////////////////////////////////////////////////////////////
      void open(char const *file_name, std::ios_base::openmode mode = std::ios_base::out)
      {
#ifdef WIN32
         open(StrUni(file_name), mode);
#else
         std::ofstream::open(file_name, mode);
#endif
      }
   };
};


#endif
