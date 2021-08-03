/* Csi.LicenseUpdateLoader.cpp

   Copyright (C) 2018, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Saturday 26 May 2018
   Last Change: Tuesday 26 May 2020
   Last Commit: $Date: 2020-12-04 11:27:25 -0600 (Fri, 04 Dec 2020) $
   Last Changed by: $Author: amortenson $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.LicenseUpdateLoader.h"
#include "Csi.Utils.h"
#include "Csi.FileSystemObject.h"
#include "Csi.RegistryManager.h"
#include <deque>


namespace Csi
{
   LicenseUpdateLoader::LicenseUpdateLoader()
   {
      // we need to attempt to get the common path from the register manager.  If we don't find the
      // value, we will look up the path of the executable.
      StrAsc common_path;
      std::deque<StrAsc> path;
      StrAsc dll_path;

#ifndef _DEBUG
      RegistryManager::read_shared_value(common_path, "CommonDir");
      if(common_path.length() == 0)
      {
         StrAsc app_dir;
         get_app_dir(app_dir);
         path.push_back(app_dir);
         path.push_back("..");
         path.push_back("common");
      }
      else
         path.push_back(common_path);
#else
      get_app_dir(common_path);
      path.push_back(common_path);
#endif
      path.push_back("csi-license-update.dll");
      dll_path = join_path(path.begin(), path.end());
      
      // we now need to load DLL and look up the function pointers.
      dll_handle = ::LoadLibraryA(dll_path.c_str());
      if(dll_handle == 0)
         throw OsException("load library failed");
      try
      {
         initialise = reinterpret_cast<initialise_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_initialise"));
         if(initialise == 0)
            throw Csi::OsException("initialise lookup failed");
         close = reinterpret_cast<close_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_close"));
         if(close == 0)
            throw Csi::OsException("close lookup failed");
         update = reinterpret_cast<update_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_update"));
         if(update == 0)
            throw Csi::OsException("update lookup failed");
         check_update = reinterpret_cast<check_update_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_check_update"));
         if(check_update == 0)
            throw Csi::OsException("check_update lookup failed");
         close_update = reinterpret_cast<close_update_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_close_update"));
         if(close_update == 0)
            throw Csi::OsException("close_update lookup failed");
         get_changelog = reinterpret_cast<get_changelog_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_get_changelog"));
         if(get_changelog == 0)
            throw Csi::OsException("get_changelog lookup failed");
         check_get_changelog = reinterpret_cast<check_get_changelog_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_check_get_changelog"));
         if(check_get_changelog == 0)
            throw Csi::OsException("check_get_changelog lookup failed");
         close_get_changelog = reinterpret_cast<close_get_changelog_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_close_get_changelog"));
         if(close_get_changelog == 0)
            throw Csi::OsException("close_get_changelog lookup failed");
         get_versions = reinterpret_cast<get_versions_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_get_versions"));
         if(get_versions == 0)
            throw Csi::OsException("get_versions lookup failed");
         get_highest_version = reinterpret_cast<get_highest_version_type *>(
            ::GetProcAddress(dll_handle, "cslicense_get_highest_version"));
         if(get_highest_version == 0)
            throw Csi::OsException("get_highest_version lookup failed");
         check_get_versions = reinterpret_cast<check_get_versions_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_check_get_versions"));
         if(check_get_versions == 0)
            throw Csi::OsException("check_get_versions lookup failed");
         close_get_versions = reinterpret_cast<close_get_versions_function_type *>(
            ::GetProcAddress(dll_handle, "cslicense_close_get_versions"));
         if(close_get_versions == 0)
            throw Csi::OsException("close_get_versions lookup failed");
      }
      catch(std::exception &)
      {
         ::FreeLibrary(dll_handle);
         throw;
      }
   }


   LicenseUpdateLoader::~LicenseUpdateLoader()
   {
      ::FreeLibrary(dll_handle);
   }
};

