/* Csi.Win32.RegistryManager.cpp

   Copyright (C) 2002, 2018 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: 30 October 2002
   Last Change: Monday 11 June 2018
   Last Commit: $Date: 2020-09-21 13:19:35 -0600 (Mon, 21 Sep 2020) $
   Last Changed by: $Author: amortenson $

*/

#pragma hdrstop

#include "Csi.Win32.RegistryManager.h"
#include "CsiTypeDefs.h"
#include "Csi.Utils.h"
#include "Csi.VersionNumber.h"
#include "Csi.StrAscStream.h"
#include <list>


namespace Csi
{
   namespace Win32
   {
      const StrAsc RegistryManager::reg_key_prefix = "SOFTWARE\\Campbell Scientific";

      
      RegistryManager::RegistryManager(
         StrAsc const &application_name,
         StrAsc const &version_):
         app_name(application_name),
         server_name("localhost")
      {
         Csi::VersionNumber v(version_.c_str());
         Csi::OStrAscStream ver_str;
         if( v.size() >= 2)
            ver_str << v[0] << "." << v[1];
         else
            throw std::exception("Invalid version number specified");
         version = ver_str.str().c_str();
      }


      RegistryManager::~RegistryManager()
      { }


      void RegistryManager::set_server_name(StrAsc const &server_name_)
      {
         server_name = server_name_;
      }


      bool RegistryManager::read_bool(
         bool &val,
         char const *value_name,
         HKEY hkey)
      {
         int bval = 0;
         if(val)
            bval = 1;
         bval = read_int(bval,value_name,hkey);
         if(bval)
            val = true;
         else
            val = false;
         return val;
      }


      void RegistryManager::write_bool(
         bool val,
         char const *value_name,
         HKEY hkey)
      {
         int bval = 0;
         if(val)
            bval = 1;
         write_int(bval,value_name,hkey);
      }


      namespace
      {
         int4 try_open_read_key(
            HKEY hive,
            HKEY *reg_key,
            StrAsc const &prefix,
            StrAsc const &app_name,
            StrAsc const &version,
            StrAsc const &server_name,
            bool needs_64bit_value = false)
         {
            REGSAM sam(KEY_READ);
            if(needs_64bit_value) sam |= KEY_WOW64_64KEY;
            else sam |= KEY_WOW64_32KEY;
            // we will form a list of the keys to try opening 
            std::list<StrAsc> keys_to_try;
            Csi::OStrAscStream temp;
            
            if(version.length() != 0)
            {
               temp << prefix << "\\" << app_name << "\\" << version;
               if(server_name.length() != 0 && server_name != "localhost")
                  temp << "\\" << server_name;
               keys_to_try.push_back(temp.str());
               temp.str("");
               temp << prefix << " Demo\\" << app_name << "\\" << version;
               if(server_name.length() != 0 && server_name != "localhost")
                  temp << "\\" << server_name;
               keys_to_try.push_back(temp.str());
               temp.str("");
            }
            temp << prefix << "\\" << app_name;
            if(server_name.length() > 0 && server_name != "localhost")
               temp << "\\" << server_name;
            keys_to_try.push_back(temp.str());
            temp.str("");
            temp << prefix << " Demo\\" << app_name;
            if(server_name.length() > 0 && server_name != "localhost")
               temp << "\\" << server_name;
            keys_to_try.push_back(temp.str());

            // we will try each key until the door opens
            int4 rtn = -1;
            while(rtn != ERROR_SUCCESS && !keys_to_try.empty())
            {
               rtn = RegOpenKeyExA(
                  hive,
                  keys_to_try.front().c_str(),
                  0,            // reserved value
                  sam,
                  reg_key);
               keys_to_try.pop_front();
            }
            return rtn;
         } // try_open_read_key
      };

      
      int RegistryManager::read_int(
         int &val, 
         char const *value_name, 
         HKEY hkey)
      {
         // Open the registry key
         HKEY rk;
         int4 rcd = try_open_read_key(
            hkey,
            &rk,
            reg_key_prefix,
            app_name,
            version,
            server_name);

         // read the value if the open succeeded
         long plValue = 0; 
         if(rcd == ERROR_SUCCESS)
         {
            DWORD dwSize = sizeof(DWORD);
            unsigned char * pBuf = reinterpret_cast<unsigned char *>(&plValue);
            rcd = RegQueryValueExA(rk,value_name,0,0,pBuf,&dwSize);
            if(rcd == ERROR_SUCCESS)
            {
               val = static_cast<int>(plValue);
            }   
            RegCloseKey(rk);
         }
         return val;
      } // read_int


      uint4 RegistryManager::read_uint4(
         uint4 &value, 
         char const *value_name, 
         HKEY hkey)
      {
         // Open the registry key
         HKEY rk;
         int4 rcd = try_open_read_key(
            hkey,
            &rk,
            reg_key_prefix,
            app_name,
            version,
            server_name);

         // read the value if the open succeeded
         if(rcd == ERROR_SUCCESS)
         {
            uint4 temp;
            uint4 value_size = sizeof(temp);
            rcd = RegQueryValueExA(
               rk,
               value_name,
               0,               // reserved
               0,               // type code, we assume int
               reinterpret_cast<byte *>(&temp),
               &value_size);
            if(rcd == ERROR_SUCCESS)
            {
               value = temp;
            }
            RegCloseKey(rk);
         }
         return value;
      } // read_uint4


      char const *RegistryManager::read_string(
         StrAsc &string, 
         char const *value_name, 
         HKEY hkey)
      {
         // Open the registry key
         HKEY rk;
         int4 rcd = try_open_read_key(
            hkey,
            &rk,
            reg_key_prefix,
            app_name,
            version,
            server_name);

         // read the value
         if(rcd == ERROR_SUCCESS)
         {
            // try to read the values from the key
            char temp[MAX_PATH];
            uint4 val_len = sizeof(temp);
            memset(temp,'\0',val_len);
            val_len = sizeof(temp);
            rcd = RegQueryValueExA(rk,value_name,0,0,reinterpret_cast<byte*>(temp),&val_len);
            if(rcd == ERROR_SUCCESS)
            {
               string = temp;
            }   
            RegCloseKey(rk);
         }
         return string.c_str();
      }


      void RegistryManager::write_int(
         int val, 
         char const *value_name, 
         HKEY hkey)
      {
         // Open the registry key
         HKEY rk;
         int4 rcd = -1;
         uint4 disposition;
         StrAsc reg_key = reg_key_prefix + "\\" + app_name + "\\" + version;
         if(server_name != "localhost")
            reg_key += "\\" + server_name;
         rcd = RegCreateKeyExA(
            hkey,
            reg_key.c_str(),
            0,      // reserved value
            "",     // empty class string
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE, // open with all rights so we can create values
            0,      // default security
            &rk,    // registry key handle
            &disposition); // result code
         if(rcd == ERROR_SUCCESS)
         {
            rcd = RegSetValueExA(rk,value_name,0,REG_DWORD,reinterpret_cast<byte*>(&val),sizeof(val));
            RegCloseKey(rk);
         }
      }


      void RegistryManager::write_uint4(
         uint4 value, 
         char const *value_name, 
         HKEY hkey)
      {
         // Open the registry key
         HKEY rk;
         int4 rcd = -1;
         uint4 disposition;
         StrAsc reg_key = reg_key_prefix + "\\" + app_name + "\\" + version;
         if(server_name != "localhost")
            reg_key += "\\" + server_name;
         rcd = RegCreateKeyExA(
            hkey,
            reg_key.c_str(),
            0,      // reserved value
            "",     // empty class string
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE, // open with all rights so we can create values
            0,      // default security
            &rk,    // registry key handle
            &disposition); // result code
         if(rcd == ERROR_SUCCESS)
         {
            rcd = RegSetValueExA(
               rk,
               value_name,
               0,               // reserved
               REG_DWORD,       // type
               reinterpret_cast<byte const *>(&value),
               sizeof(value));
            RegCloseKey(rk);
         }
      }


      void RegistryManager::write_string(
         StrAsc const &string,
         char const *value_name, 
         HKEY hkey)
      {
         // Open the registry key
         HKEY rk;
         int4 rcd = -1;
         uint4 disposition;
         StrAsc reg_key = reg_key_prefix + "\\" + app_name + "\\" + version;
         if(server_name != "localhost")
            reg_key += "\\" + server_name;
         rcd = RegCreateKeyExA(
            hkey,
            reg_key.c_str(),
            0,      // reserved value
            "",     // empty class string
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE, // open with all rights so we can create values
            0,      // default security
            &rk,    // registry key handle
            &disposition); // result code
         if(rcd == ERROR_SUCCESS)
         {
            rcd = RegSetValueExA(
               rk,
               value_name,
               0,
               REG_SZ,
               reinterpret_cast<const byte*>(string.c_str()),
               (DWORD)string.length());
            RegCloseKey(rk);
         }
      }


      char const *RegistryManager::read_work_dir(StrAsc &string)
      {
         read_string(string,"WorkDir",HKEY_LOCAL_MACHINE);
         if(string == "")
         {
            string = "c:\\CampbellSci\\" + app_name + "\\";
            if(version != "")
               string += version + "\\";
         }
         else
         {
            //Make sure the directory has an ending slash
            char last_char = string[string.length()-1];
            if(last_char != '\\')
               string += "\\";
         }
         return string.c_str();
      }


      char const *RegistryManager::read_shared_value(
         StrAsc &string, 
         char const *value_name,
         HKEY key)
      {
         // Open the registry key
         HKEY rk;
         int4 rcd = -1;
         rcd = RegOpenKeyExA(
            key,
            reg_key_prefix.c_str(),
            0,      // reserved value
            KEY_READ | KEY_WOW64_32KEY,
            &rk);    // registry key handle
         if(rcd == ERROR_SUCCESS)
         {
            // try to read the values from the key
            char temp[MAX_PATH];
            uint4 val_len = sizeof(temp);
            memset(temp,'\0',val_len);
            val_len = sizeof(temp);
            rcd = RegQueryValueExA(rk,value_name,0,0,reinterpret_cast<byte*>(temp),&val_len);
            if(rcd == ERROR_SUCCESS)
            {
               string = temp;
            }   
            RegCloseKey(rk);
         }
         return string.c_str();
      } // read_shared_value


      bool RegistryManager::read_shared_uint4(
         uint4 &value,
         char const *value_name,
         HKEY key)
      {
         // Open the registry key
         HKEY rk;
         int4 rcd = -1;
         rcd = RegOpenKeyExA(
            key,
            reg_key_prefix.c_str(),
            0,      // reserved value
            KEY_READ,
            &rk);    // registry key handle
         bool rtn = false;
         
         if(rcd == ERROR_SUCCESS)
         {
            uint4 temp;
            uint4 value_size = sizeof(temp);
            rcd = RegQueryValueExA(
               rk,
               value_name,
               0,               // reserved
               0,               // type code, we assume int
               reinterpret_cast<byte *>(&temp),
               &value_size);
            if(rcd == ERROR_SUCCESS)
            {
               value = temp;
               rtn = true;
            }
            RegCloseKey(rk);
         }
         return rtn;
      } // read_shared_uint4


      bool RegistryManager::write_shared_uint4(
         uint4 value,
         char const *value_name,
         HKEY key)
      {
         HKEY rk;
         int4 rcd;
         uint4 disposition;
         bool rtn = false;
         
         rcd = RegCreateKeyExA(
            key,
            reg_key_prefix.c_str(),
            0,                  // reserved value
            "",                 // empty class string
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,          // requested writes
            0,                  // default security
            &rk,                // registry key handle
            &disposition);      // stored result code
         if(rcd == ERROR_SUCCESS)
         {
            rcd = RegSetValueExA(
               rk,
               value_name,
               0,               // reserved
               REG_DWORD,       // type
               reinterpret_cast<byte const *>(&value),
               sizeof(value));
            if(rcd == ERROR_SUCCESS)
               rtn = true;
            RegCloseKey(rk);
         }
         return rtn;
      } // write_shared_uint4


      bool RegistryManager::write_anywhere_string(
         StrAsc const &value,
         char const *name,
         char const *path,
         HKEY key)
      {
         bool rtn = false;
         int4 rcd;
         uint4 disposition;
         HKEY rk;

         rcd = RegCreateKeyExA(
            key,
            path,
            0,   // reserved value
            "",  // empty class string
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            0,   // default security
            &rk, // registry key handle
            &disposition); // stored result code
         if(rcd == ERROR_SUCCESS)
         {
            rcd = RegSetValueExA(
               rk,
               name,
               0,  // reserved
               REG_SZ,
               reinterpret_cast<byte const *>(value.c_str()),
               (DWORD)value.length());
            if(rcd == ERROR_SUCCESS)
               rtn = true;
            RegCloseKey(rk);
         }
         return rtn;
      }


      char const *RegistryManager::read_anywhere_string(
         StrAsc &value,
         char const *value_name,
         char const *path,
         HKEY key,
         bool needs_64bit_value)
      {
         // Open the registry key
         HKEY rk;
         int4 rcd = -1;
         REGSAM sam(KEY_READ);
         if(needs_64bit_value)
            sam |= KEY_WOW64_64KEY;
         value.cut(0);
         rcd = RegOpenKeyExA(
            key,
            path,
            0,      // reserved value
            sam,
            &rk);    // registry key handle
         if(rcd == ERROR_SUCCESS)
         {
            // try to read the values from the key
            char temp[MAX_PATH];
            uint4 val_len = sizeof(temp);
            memset(temp,'\0',val_len);
            val_len = sizeof(temp);
            rcd = RegQueryValueExA(rk,value_name,0,0,reinterpret_cast<byte*>(temp),&val_len);
            if(rcd == ERROR_SUCCESS)
               value = temp;
            RegCloseKey(rk);
         }
         return value.c_str();
      }


      bool RegistryManager::write_anywhere_binary(
         StrBin const &value,
         char const *name,
         char const *path,
         HKEY key)
      {
         bool rtn = false;
         int4 rcd;
         uint4 disposition;
         HKEY rk;

         rcd = RegCreateKeyExA(
            key,
            path,
            0,   // reserved value
            "",  // empty class string
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            0,   // default security
            &rk, // registry key handle
            &disposition); // stored result code
         if(rcd == ERROR_SUCCESS)
         {
            rcd = RegSetValueExA(
               rk,
               name,
               0,  // reserved
               REG_BINARY,
               reinterpret_cast<byte const *>(value.getContents()),
               (DWORD)value.length());
            if(rcd == ERROR_SUCCESS)
               rtn = true;
            RegCloseKey(rk);
         }
         return rtn;
      }


      void RegistryManager::read_anywhere_binary(
         StrBin &value,
         char const *value_name,
         char const *path,
         HKEY key)
      {
         // Open the registry key
         HKEY rk;
         int4 rcd = -1;
         rcd = RegOpenKeyExA(
            key,
            path,
            0,      // reserved value
            KEY_READ,
            &rk);    // registry key handle
         if(rcd == ERROR_SUCCESS)
         {
            // try to read the values from the key
            char temp[MAX_PATH];
            uint4 val_len = sizeof(temp);
            memset(temp,'\0',val_len);
            val_len = sizeof(temp);
            rcd = RegQueryValueExA(rk,value_name,0,0,reinterpret_cast<byte*>(temp),&val_len);
            if(rcd == ERROR_SUCCESS)
            {
               value.setContents(temp,val_len);
            }   
            RegCloseKey(rk);
         }
      }


      StrAsc RegistryManager::get_shell_folder(char const *value_name)
      {
         StrAsc rtn;
         size_t profile_pos;
         StrAsc user_profile_name("%USERPROFILE%");
         read_anywhere_string(
            rtn,
            value_name,
            "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders",
            HKEY_CURRENT_USER);
         profile_pos = rtn.find(user_profile_name.c_str());
         if(profile_pos < rtn.length())
         {
            char *env(::getenv("USERPROFILE"));
            if(env)
            {
               rtn.cut(profile_pos, user_profile_name.length());
               rtn.insert(env, profile_pos);
            }
         }
         return rtn;
      }
   };
};
