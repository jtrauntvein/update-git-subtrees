/* Csi.Win32.FileSystemObject.cpp

   Copyright (C) 2003, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 16 October 2003
   Last Change: Friday 16 November 2012
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>
#include <direct.h>
#include "Csi.Win32.FileSystemObject.h"


namespace Csi
{
   namespace Win32
   {
      ////////////////////////////////////////////////////////////
      // class FileSystemObject definitions
      ////////////////////////////////////////////////////////////
      FileSystemObject::FileSystemObject():
         is_valid(false),
         flags(0),
         size(0)
      { }

      
      FileSystemObject::FileSystemObject(char const *object_name):
         is_valid(false),
         flags(0),
         size(0)
      { set_object_name(object_name); }

      
      FileSystemObject::FileSystemObject(FileSystemObject const &other):
         is_valid(other.is_valid),
         name(other.name),
         path(other.path),
         flags(other.flags),
         creation_time(other.creation_time),
         last_read_date(other.last_read_date),
         last_write_date(other.last_write_date),
         size(other.size)
      { }

      
      FileSystemObject::~FileSystemObject()
      { }

      
      FileSystemObject &FileSystemObject::operator =(FileSystemObject const &other)
      {
         is_valid = other.is_valid;
         name = other.name;
         path = other.path;
         flags = other.flags;
         creation_time = other.creation_time;
         last_read_date = other.last_read_date;
         last_write_date = other.last_write_date;
         size = other.size;
         return *this;
      } // copy operator

      
      FileSystemObject &FileSystemObject::operator =(char const *object_name_)
      {
         set_object_name(object_name_);
         return *this;
      } // string assignment operator

      
      void FileSystemObject::get_volume_info(
         int8 &total_size,
         int8 &free_space)
      {
         FileVolume volume(get_root().c_str());
         if(volume.get_type() != FileVolume::type_removable)
            volume.get_volume_limits(total_size,free_space);
         else
            total_size = free_space = 0;
      } // get_volume_info


      namespace
      {
         void set_file_information(
            FileSystemObject &obj,
            WIN32_FIND_DATAW &object_info)
         {
            obj.is_valid = true;
            if(object_info.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)
               obj.flags |= obj.flag_compressed;
            if(object_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
               obj.flags |= obj.flag_directory;
            if(object_info.dwFileAttributes & FILE_ATTRIBUTE_ENCRYPTED)
               obj.flags |= obj.flag_encrypted;
            if(object_info.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)
               obj.flags |= obj.flag_hidden;
            if(object_info.dwFileAttributes & FILE_ATTRIBUTE_OFFLINE)
               obj.flags |= obj.flag_offline;
            if(object_info.dwFileAttributes & FILE_ATTRIBUTE_READONLY)
               obj.flags |= obj.flag_read_only;
            if(object_info.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM)
               obj.flags |= obj.flag_system;
            if(object_info.dwFileAttributes & FILE_ATTRIBUTE_TEMPORARY)
               obj.flags |= obj.flag_temporary;

            // set the creation date
            SYSTEMTIME temp_time;
            FILETIME local_time;
            FileTimeToLocalFileTime(&object_info.ftCreationTime,&local_time);
            FileTimeToSystemTime(&local_time,&temp_time);
            obj.creation_time.setDate(
               temp_time.wYear,
               temp_time.wMonth,
               temp_time.wDay);
            obj.creation_time.setTime(
               temp_time.wHour,
               temp_time.wMinute,
               temp_time.wSecond,
               temp_time.wMilliseconds * static_cast<uint4>(LgrDate::nsecPerMSec));

            // set the last access time
            FileTimeToLocalFileTime(&object_info.ftLastAccessTime,&local_time);
            FileTimeToSystemTime(&local_time,&temp_time);
            obj.last_read_date.setDate(
               temp_time.wYear,
               temp_time.wMonth,
               temp_time.wDay);
            obj.last_read_date.setTime(
               temp_time.wHour,
               temp_time.wMinute,
               temp_time.wSecond,
               temp_time.wMilliseconds * static_cast<uint4>(LgrDate::nsecPerMSec));

            // set the last write time
            FileTimeToLocalFileTime(&object_info.ftLastWriteTime,&local_time);
            FileTimeToSystemTime(&local_time,&temp_time);
            obj.last_write_date.setDate(
               temp_time.wYear,
               temp_time.wMonth,
               temp_time.wDay);
            obj.last_write_date.setTime(
               temp_time.wHour,
               temp_time.wMinute,
               temp_time.wSecond,
               temp_time.wMilliseconds * static_cast<uint4>(LgrDate::nsecPerMSec));

            // set the size
            obj.size = (static_cast<int8>(object_info.nFileSizeHigh) << 32) +
               object_info.nFileSizeLow;
         } // set_file_information
      };

      
      void FileSystemObject::get_children(
         children_type &children,
         char const *mask)
      {
         children.clear();
         if(is_valid && (is_directory() || is_root()))
         {
            // form the search path
            StrUni search_path(path);
            if(search_path.last() != L'\\')
               search_path += L'\\';
            if(!is_root())
               search_path.append_utf8(name.c_str());
            if(search_path.last() != L'\\')
               search_path += L"\\";
            search_path.append_utf8(mask);

            // search for the name
            WIN32_FIND_DATAW file_info;
            HANDLE search_handle = FindFirstFileW(search_path.c_str(),&file_info);
            if(search_handle != INVALID_HANDLE_VALUE)
            {
               do
               {
                  FileSystemObject child;
                  child.path = path;
                  if(child.path.last() != '\\')
                     child.path += '\\';
                  if(path != name)
                     child.path += name;
                  set_file_information(child,file_info);
                  child.name = StrUni(file_info.cFileName).to_utf8();
                  children.push_back(child);
               }
               while(FindNextFileW(search_handle,&file_info));
               FindClose(search_handle);
            }                                    
         }
      } // get_children


      StrAsc FileSystemObject::get_root() const
      {
         StrAsc buff;
         if(is_valid)
         {
            size_t str_pos;

            buff = path;
            if((str_pos = buff.find("\\\\")) == 0 && buff.length() > 2)
            {
               size_t str_pos2 = buff.find("\\",2);
               if(str_pos2 < buff.length())
                  buff.cut(str_pos2 + 1);
               else
                  buff += '\\';
            }
            else if((str_pos = buff.find(":\\")) == 1 && str_pos < buff.length())
               buff.cut(str_pos + 2);
            else
               buff.cut(0);
         }
         return buff;
      } // get_root


      void FileSystemObject::set_object_name(char const *object_name_)
      {
         // The object name might be specified using a relative path.  This should be converted to
         // an absolute path.  We can accomplish this using the GetFullPathName() function.  We put
         // this code inside of braces so that the temporaries allocated to accoomplish this will
         // not be inadvertently accessed later on.
         StrUni object_name(object_name_);

         flags = 0;
         if(object_name.length() > 3 && object_name[object_name.length() - 1] == L'\\')
            object_name.cut(object_name.length() - 1);
         if(object_name.length() > 0)
         {
            uint4 path_buffer_size = 256;
            wchar_t *path_buffer = new wchar_t[path_buffer_size];
            wchar_t *file_name_ptr = 0;
            
            uint4 rcd = ::GetFullPathNameW(
               object_name.c_str(),
               path_buffer_size,
               path_buffer,
               &file_name_ptr);
            if(rcd >= path_buffer_size)
            {
               delete[] path_buffer;
               path_buffer_size = rcd + 1;
               path_buffer = new wchar_t[path_buffer_size];
               ::GetFullPathNameW(
                  object_name.c_str(),
                  path_buffer_size,
                  path_buffer,
                  &file_name_ptr);
            }
            
            // now that we have the full path name for the file, we will set the file path
            if(file_name_ptr != 0)
            {
               StrUni temp_name(file_name_ptr);
               StrUni temp_path(path_buffer, file_name_ptr - path_buffer);
               name = temp_name.to_utf8();
               path = temp_path.to_utf8();
            }
            else
            {
               StrUni temp_name(path_buffer);
               name = path = temp_name.to_utf8();
               flags |= flag_root;
               flags |= flag_directory;
            }
            delete[] path_buffer;
            is_valid = true;
         }
         else
            is_valid = false;
               
         // ask the operating system for information about the object
         if(is_valid && !is_root())
         {
            WIN32_FIND_DATAW object_info;
            HANDLE search_handle = ::FindFirstFileW(object_name.c_str(),&object_info);
            
            if(search_handle != INVALID_HANDLE_VALUE)
            {
               // initialise the file information
               is_valid = true;
               set_file_information(*this,object_info);
               FindClose(search_handle);
            }
            else
               is_valid = false;
         }
      } // set_object_name


      ////////////////////////////////////////////////////////////
      // class FileVolume definitions
      ////////////////////////////////////////////////////////////
      FileVolume::FileVolume(char const *name_):
         name(name_),
         type(type_unknown)
      {
         UINT drive_type = ::GetDriveTypeA(
            name.length() > 0 ? name.c_str() : 0);
         switch(drive_type)
         {
         case DRIVE_NO_ROOT_DIR:
            type = type_no_root;
            break;

         case DRIVE_REMOVABLE:
            type = type_removable;
            break;

         case DRIVE_FIXED:
            type = type_fixed;
            break;

         case DRIVE_REMOTE:
            type = type_remote;
            break;

         case DRIVE_CDROM:
            type = type_cdrom;
            break;

         case DRIVE_RAMDISK:
            type = type_ramdisk;
            break;
         } 
      } // constructor


      void FileVolume::get_volume_limits(
         int8 &volume_size,
         int8 &volume_free)
      {
         // we need to temporarily set the error mode so thatthe operating system will not block on
         // removable media.
         int old_error_mode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
         volume_size = volume_free = 0;
         if(type != type_unknown)
         {
            // if the GetFreeDiskSpaceEx() call is available, we need to use it because it will
            // report available space greater than 2GB.
            volume_size = volume_free = 0;
            FARPROC extended_function = 
               ::GetProcAddress(
                  ::GetModuleHandle(TEXT("kernel32.dll")),
                  "GetDiskFreeSpaceExA");
            if(extended_function)
            {
               typedef BOOL (WINAPI * function_type)(
                  LPCSTR lpDirectoryName,
                  PULARGE_INTEGER lpFreeBytesAvailableToCaller,
                  PULARGE_INTEGER lpTotalNumberOfBytes,
                  PULARGE_INTEGER lpTotalNumberOfFreeBytes);
               function_type function = reinterpret_cast<function_type>(extended_function);
               ULARGE_INTEGER free_available;
               ULARGE_INTEGER total_bytes;
               ULARGE_INTEGER total_free_bytes;
               BOOL rcd = function(
                  name.length () > 0 ? name.c_str() : 0,
                  &free_available,
                  &total_bytes,
                  &total_free_bytes);
               if(rcd)
               {
                  volume_size = static_cast<int8>(total_bytes.QuadPart);
                  volume_free = static_cast<int8>(free_available.QuadPart);
               }
            }
            else
            {
               // call the windows GetDiskFreeSpace method
               uint4 sectors_per_cluster;
               uint4 bytes_per_sector;
               uint4 free_cluster_count;
               uint4 total_clusters;
               BOOL rcd = ::GetDiskFreeSpaceA(
                  name.length() > 0 ? name.c_str() : 0,
                  &sectors_per_cluster,
                  &bytes_per_sector,
                  &free_cluster_count,
                  &total_clusters);
               
               if(rcd)
               {
                  volume_free = free_cluster_count;
                  volume_free *= sectors_per_cluster;
                  volume_free *= bytes_per_sector;
                  volume_size = total_clusters;
                  volume_size *= sectors_per_cluster;
                  volume_size *= bytes_per_sector; 
               }
            }
         }
         SetErrorMode(old_error_mode);
      } // get_volume_limits


      char const *FileVolume::get_type_string() const
      {
         static char *type_names[] = {
            "unknown",
            "fixed",
            "removable",
            "remote",
            "cdrom",
            "ramdisk",
            "no root"
         };
         uint4 tp = type;
         if(tp > 7)
            tp = 0;
         return type_names[tp];
      } // get_type_string

      
      ////////////////////////////////////////////////////////////
      // function list_file_volumes
      ////////////////////////////////////////////////////////////
      void list_file_volumes(std::list<FileVolume> &volumes)
      {
         // the most efficient method of getting the list of drives is to get the mask from the
         // operating system.  The alternative is to use the GetLogicalDriveStrings() which involves
         // alloicating a buffer large enough to hold all possible strings.  There are 26 drive letter
         // possibilities so all we have to do is iterate over these.
         uint4 logical_drives_mask = ::GetLogicalDrives();
         char drive_string[4];
         
         volumes.clear();
#pragma warning(disable: 4996)
         strcpy(drive_string,"x:\\");
#pragma warning(default: 4996)
         for(int1 i = 0; i < 26; ++i)
         {
            uint4 selector = 1 << i;
            if((logical_drives_mask & selector) != 0)
            {
               drive_string[0] = 'A' + i;
               volumes.push_back(FileVolume(drive_string));
            }
         }
      } // list_file_volumes
   };
};

