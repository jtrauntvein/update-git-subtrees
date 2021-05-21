/* Csi.Posix.RegistryManager.h

   Copyright (C) 2005, 2017 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Tuesday 10 May 2005
   Last Change: Friday 28 July 2017
   Last Commit: $Date: 2017-07-28 16:16:14 -0600 (Fri, 28 Jul 2017) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Posix_RegistryManager_h
#define Csi_Posix_RegistryManager_h

#include "StrAsc.h"
#include "CsiTypeDefs.h"


#ifndef _WIN32
////////////////////////////////////////////////////////////
// enum HKEY
////////////////////////////////////////////////////////////
enum HKEY
{
   HKEY_CURRENT_USER,
   HKEY_LOCAL_MACHINE
}; 
#endif


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class RegistryManager
      //
      // Defines a component that handles the reading and writing of values to
      // files that mimic the structure of the WIN32 registry.
      ////////////////////////////////////////////////////////////
      class RegistryManager
      {
      private:
         ////////////////////////////////////////////////////////////
         // app_name
         ////////////////////////////////////////////////////////////
         StrAsc app_name;

         ////////////////////////////////////////////////////////////
         // version
         ////////////////////////////////////////////////////////////
         StrAsc version;

         ////////////////////////////////////////////////////////////
         // server_name
         ////////////////////////////////////////////////////////////
         StrAsc server_name;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         RegistryManager(
            StrAsc const &app_name_,
            StrAsc const &version_ = "1.0");

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~RegistryManager();

         ////////////////////////////////////////////////////////////
         // read_int
         //
         // Reads an integer from the appropriate key.  Returns the value.  If
         // the key or values does not exist, the original value will not be
         // disturbed. 
         ////////////////////////////////////////////////////////////
         int read_int(
            int &val,
            char const *value_name,
            HKEY key = HKEY_CURRENT_USER);

         ////////////////////////////////////////////////////////////
         // read_uint4
         ////////////////////////////////////////////////////////////
         uint4 read_uint4(
            uint4 &val,
            char const *value_name,
            HKEY key = HKEY_CURRENT_USER);

         ////////////////////////////////////////////////////////////
         // write_int
         //
         // Will write the supplied value and create the key if needed. 
         ////////////////////////////////////////////////////////////
         void write_int(
            int val,
            char const *value_name,
            HKEY key = HKEY_CURRENT_USER);

         ////////////////////////////////////////////////////////////
         // write_uint4
         //
         // Will write the supplied value and create the key if needed. 
         ////////////////////////////////////////////////////////////
         void write_uint4(
            uint4 val,
            char const *value_name,
            HKEY key = HKEY_CURRENT_USER);

         ////////////////////////////////////////////////////////////
         // read_bool
         ////////////////////////////////////////////////////////////
         bool read_bool(
            bool &val,
            char const *value_name,
            HKEY key = HKEY_CURRENT_USER);

         ////////////////////////////////////////////////////////////
         // write_bool
         ////////////////////////////////////////////////////////////
         void write_bool(
            bool &val,
            char const *value_name,
            HKEY key = HKEY_CURRENT_USER);

         ////////////////////////////////////////////////////////////
         // read_string
         ////////////////////////////////////////////////////////////
         char const *read_string(
            StrAsc &val,
            char const *value_name,
            HKEY key = HKEY_CURRENT_USER);

         ////////////////////////////////////////////////////////////
         // write_string
         ////////////////////////////////////////////////////////////
         void write_string(
            StrAsc const &val,
            char const *value_name,
            HKEY key = HKEY_CURRENT_USER);

         ////////////////////////////////////////////////////////////
         // read_work_dir
         ////////////////////////////////////////////////////////////
         char const *read_work_dir(StrAsc &buff);

         ////////////////////////////////////////////////////////////
         // read_shared_value
         //
         // Reads a string value from a non-application specific part of the
         // specified registry file.  The return value will be the pointer to
         // the supplied buffer.  The supplied buffer will not be changed if
         // the speciifed value name does not exist. 
         ////////////////////////////////////////////////////////////
         static char const *read_shared_value(
            StrAsc &val,
            char const *value_name,
            HKEY key = HKEY_LOCAL_MACHINE);

         ////////////////////////////////////////////////////////////
         // write_shared_value
         ////////////////////////////////////////////////////////////
         static bool write_shared_value(
            char const *value,
            char const *value_name,
            HKEY key = HKEY_LOCAL_MACHINE);

         ////////////////////////////////////////////////////////////
         // read_shared_uint4
         ////////////////////////////////////////////////////////////
         static bool read_shared_uint4(
            uint4 &value,
            char const *value_name,
            HKEY key = HKEY_LOCAL_MACHINE);

         ////////////////////////////////////////////////////////////
         // write_shared_uint4
         ////////////////////////////////////////////////////////////
         static bool write_shared_uint4(
            uint4 value,
            char const *value_name,
            HKEY key = HKEY_LOCAL_MACHINE);

         /**
          * Attempts to read a string value stored in the registry.
          *
          * @return Returns the string value that was read.
          *
          * @param value_name Specifies the name of the registry key value to read.
          *
          * @param path Specifies the path of the registry key.
          *
          * @param key Specifies the hive of the registry key.
          *
          * @param needs_64bit_value Set to true if the key needed is outside of the 32bit sandbox
          * hive.  This parameter is ignored on the Linux build.
          */
         static char const *read_anywhere_string(
            StrAsc &value,
            char const *value_name,
            char const *path,
            HKEY key = HKEY_LOCAL_MACHINE,
            bool needs_64bit_value = false);

         ////////////////////////////////////////////////////////////
         // set_server_name
         ////////////////////////////////////////////////////////////
         void set_server_name(StrAsc const &server_name_)
         { server_name = server_name_; }

         ////////////////////////////////////////////////////////////
         // get_server_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_server_name() const
         { return server_name; }
      };
   };
};


#endif
