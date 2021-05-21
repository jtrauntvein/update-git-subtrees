/* Csi.Win32.RegistryManager.h

   Copyright (C) 2002, 2017 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: Friday 30 October 2002
   Last Change: Saturday 21 October 2017
   Last Commit: $Date: 2017-10-21 08:16:09 -0600 (Sat, 21 Oct 2017) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Win32_RegistryManager_h
#define Csi_Win32_RegistryManager_h

#include "StrAsc.h"
#include "StrBin.h"
#include "CsiTypeDefs.h"
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>

namespace Csi
{
   namespace Win32
   {
      /**
       * Defines a component that encapsulates the details of reading and writing values to the
       * registry using the registry scheme adopted for Loggernet 2.2 for shared component
       * conventions.
       */
      class RegistryManager
      {
      public:
         /**
          * Constructor
          *
          * @param application_name Specifies the name of the application as it will be identified
          * in registry keys.
          *
          * @param version Specifies the expected application version.
          */
         RegistryManager(
            StrAsc const &application_name, StrAsc const &version = "1.0");

         /**
          * Destructor
          */
         virtual ~RegistryManager();

         /**
          * @return Reads an integer value from the application's registry key.
          *
          * @param val Specifies the value that will be returned.
          *
          * @param value_name Name of the value to be read.
          *
          * @param hkey Identifies the hive from which the key will be read.
          */
         int read_int(
            int &val, char const *value_name, HKEY hkey = HKEY_CURRENT_USER);

         /**
          * @return Reads an unsigned integer value from the application's registry key.
          *
          * @param val Specifies a reference to the value that will be returned.
          *
          * @param value_name Specifies the name of the value in the key.
          *
          * @param hkey Specifies the hive from which the key will be read.
          */
         uint4 read_uint4(
            uint4 &value, char const *value_name, HKEY hkey = HKEY_CURRENT_USER);

         /**
          * Writes an integer value to the application's registry key.
          *
          * @param val Specifies the value to write.
          *
          * @param value_name Specifies the name of the value under the aplication's key.
          *
          * @param hkey Specifies the hive.
          */
         void write_int(
            int val, char const *value_name, HKEY hkey = HKEY_CURRENT_USER);

         /**
          * Writes an unsigned integer value to the application's registry key.
          *
          * @param value Specifies the value to write.
          *
          * @param value_name Specifies the name of the value in the application's key.
          *
          * @param hkey Specifies the hive.
          */
         void write_uint4(
            uint4 value, char const *value_name, HKEY hkey = HKEY_CURRENT_USER);

         /**
          * @return Returns the identified boolean value in the application's registry key.
          *
          * @param val Reference to the return value.
          *
          * @param value_name Specifies the name of the value to read from the application's key.
          *
          * @param hkey Specifies the hive.
          */
         bool read_bool(
            bool &val, char const *value_name, HKEY hkey = HKEY_CURRENT_USER);

         /**
          * Writes a boolean value to the application's registry key.
          *
          * @param val Specifies the value to write.
          *
          * @param value_name Specifies the name of the value in the key.
          *
          * @param hkey Specifies the hive.
          */
         void write_bool(
            bool val, char const *value_name, HKEY hkey = HKEY_CURRENT_USER);

         /**
          * @return Reads a string value from the application's registry key.
          *
          * @param string Reference to the string to be read.
          *
          * @param value_name Specifies the name of the value in the application's registry key.
          *
          * @param hkey Specifies the hive.
          */
         char const *read_string(
            StrAsc &string, char const *value_name, HKEY hkey = HKEY_CURRENT_USER);

         /**
          * Writes a string value to the application's registry key.
          *
          * @param string Specifies the value to write.
          *
          * @param value_name Specifies the name of the value in the registry key.
          *
          * @param hkey Specifies the hive.
          */
         void write_string(
            StrAsc const &string, char const *value_name, HKEY hkey = HKEY_CURRENT_USER);

         /**
          * @return Reads the working directory value from the application's registry key in the
          * HKEY_LOCAL_MACHINE hive.
          *
          * @param string Reference to the value that will receive the working directory.
          */
         char const *read_work_dir(StrAsc &string);

         /**
          * @return Reads a strng value from the campbell scientific registry key.
          *
          * @param string Reference to the value that will be returned.
          *
          * @param value_name Specifies the name of the value in the campbell scientific registry
          * key.
          *
          * @param key Specifies the hive.
          */
         static char const *read_shared_value(
            StrAsc &string, char const *value_name, HKEY key = HKEY_LOCAL_MACHINE);

         /**
          * @return Reads an unsigned integer from the campbell scientific registry key.
          *
          * @param value Reference to the value that will be returned.
          *
          * @param value_name Specifies the name of the value to be read.
          *
          * @param key Specifies the hive.
          */
         static bool read_shared_uint4(
            uint4 &value, char const *value_name, HKEY key = HKEY_LOCAL_MACHINE);

         /**
          * Writes an unsigned integer to the campbell scientific registry key.
          *
          * @param value Specifies the value to write.
          *
          * @param value_name Specifies the name of the value in the CSI key.
          *
          * @param key Specifies the hive.
          */
         static bool write_shared_uint4(
            uint4 value, char const *value_name, HKEY key = HKEY_LOCAL_MACHINE);

         /**
          * Writes a string value from any registry key.
          *
          * @return Returns true if the key and value were located.
          *
          * @param value Reference to the value that will be returned.
          *
          * @param value_name Specifies the name of the value in the registry key.
          *
          * @param path Specifies the path to the registry key.
          *
          * @param key Specifies the hive.
          */
         static bool write_anywhere_string(
            StrAsc const &value, char const *value_name, char const *path, HKEY key = HKEY_LOCAL_MACHINE);
            
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
          * hive.
          */
         static char const *read_anywhere_string(
            StrAsc &value, char const *value_name, char const *path, HKEY key = HKEY_LOCAL_MACHINE, bool needs_64bit_value = false);

         /**
          * Attempts to write a binary value to a specified registry key.
          *
          * @return Returns true if the value was written.
          *
          * @param value Reference to the value that will be written.
          *
          * @param value_name Specifies the name of the value.
          *
          * @param path Specifies the path to the registry key.
          *
          * @param key Specifies the hive.
          */
         static bool write_anywhere_binary(StrBin const &value, char const *value_name, char const *path, HKEY key = HKEY_LOCAL_MACHINE);

         /**
          * Attempts to read binary data from a specified registry key.
          *
          * @param value Reference to the value that will be returned.
          *
          * @param value_name Specifies the name of the value to be read.
          *
          * @param path Specifies the path to the registry key.
          *
          * @param key Specifies the hive.
          */
         static void read_anywhere_binary(
            StrBin &value, char const *value_name, char const *path, HKEY key = HKEY_LOCAL_MACHINE);
            
         /**
          * @param server_name Specifies the server address to be used.
          */
         void set_server_name(StrAsc const &server_name);
         
         /**
          * @return Returns the server address to be used.
          */
         StrAsc const &get_server_name()
         { return server_name; }

         /**
          * @return Returns the value of the specified shell folder.
          *
          * @param value_name Specifies the name of the shell folder to be read.
          */
         static StrAsc get_shell_folder(char const *value_name);

      protected:
         /**
          * Specifies the address of the campbell scientific registry key.
          */
         static const StrAsc reg_key_prefix;

      private:
         /**
          * Specifies the name of the application,
          */
         StrAsc app_name;

         /**
          * Specifies the current version of the application.
          */
         StrAsc version;

         /**
          * Specifies the server address.
          */
         StrAsc server_name;
      };
   };
};

#endif //Csi_Win32_RegistryManager_h
