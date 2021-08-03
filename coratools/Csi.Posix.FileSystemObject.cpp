/* Csi.Posix.FileSystemObject.cpp

   Copyright (C) 2005, 2009 Campbell Scientific, Inc.

   Written by: 
   Date Begun: Thursday 02 June 2005
   Last Change: Friday 26 June 2009
   Last Commit: $Date: 2009-06-26 13:57:23 -0600 (Fri, 26 Jun 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.FileSystemObject.h"
#include "Csi.Utils.h"
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <dirent.h>
#include <fnmatch.h>



namespace Csi
{
   namespace Posix
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
      }


      void FileSystemObject::get_volume_info(
         int8 &total_size,
         int8 &free_space)
      {
         struct statvfs system_stat;
         total_size = free_space = 0;
         if(statvfs(path.c_str(),&system_stat) == 0)
         {
            free_space = system_stat.f_bavail * system_stat.f_frsize;
            total_size = system_stat.f_blocks * system_stat.f_frsize;
         }
      } // get_volume_info


      void FileSystemObject::get_children(
         children_type &children,
         char const *mask)
      {
         children.clear();
         if(is_valid && (is_directory() || is_root()))
         {
            StrAsc complete_name(get_complete_name());
            DIR *dir = opendir(complete_name.c_str());
            if(dir)
            {
               struct dirent *child_info;
               while((child_info = readdir(dir)) != 0)
               {
                  if(fnmatch(mask,child_info->d_name,0) == 0)
                  {
                     StrAsc child_name(complete_name.c_str());
                     if(child_name.last() != '/')
                        child_name.append('/');
                     if(strcmp(child_info->d_name,".") != 0 &&
                        strcmp(child_info->d_name,"..") != 0)
                     {
                        child_name.append(child_info->d_name);
                        children.push_back(FileSystemObject(child_name.c_str()));
                     }
                  }
               }
               closedir(dir);
            }
         }
      } // get_children


      namespace
      {
         ////////////////////////////////////////////////////////////
         // expand_local_path
         //
         // Evaluates whether the provided string respresents a local path name (looks for the
         // presence of the '/' at the beginning of the string) and places the current working
         // directory if the path is relative.  Will also eliminate any references to .. and
         // . within the string.
         ////////////////////////////////////////////////////////////
         void expand_local_path(
            StrAsc &dest,
            char const *path)
         {
            // we will expand the tilde to equal the home directory reference
            if(path[0] == '~')
            {
               char *home_dir = getenv("HOME");
               if(home_dir != 0)
                  dest = home_dir;
               if(dest.last() != '/' && path[1] != '/')
                  dest.append('/');
               dest.append(path + 1);
            }
            // if the first character of the provided string is a slash, the path should already be
            // considered to be absolute
            else if(path[0] != '/')
            {
               // we need to obtain the current working directory.  This is done within a loop
               // because this is the only way to get a guaranteed result. 
               char *rcd = 0; 
               dest.fill(0,1024);
               while(rcd == 0)
               {
                  rcd = getcwd(dest.getContents_writable(),dest.length() - 1);
                  if(rcd == 0)
                     dest.fill(0,dest.length() * 2);
               }
               dest.cut(strlen(dest.c_str()));

               // we can now append the provided path to the cwd
               if(dest.last() != '/')
                  dest.append('/');
               dest.append(path);
            }
            else
               dest = path;

            // we now have the path specified in terms of the root.  This might have to be
            // simplified, however, to eliminate references to ..
            size_t parent_ref_pos = dest.find("/../");
            while(parent_ref_pos < dest.length())
            {
               size_t prev_sep_pos = dest.rfind("/",parent_ref_pos - 1);
               if(prev_sep_pos < dest.length())
                  dest.cut(prev_sep_pos, parent_ref_pos - prev_sep_pos + 5);
               else
                  dest.cut(parent_ref_pos,4);
               parent_ref_pos = dest.find("/../");
            }
            parent_ref_pos = dest.find("/..");
            if(parent_ref_pos < dest.length())
               dest.cut(parent_ref_pos);

            // we can further simplify the path by eliminating references to self ('/./')
            size_t self_ref_pos = dest.find("/./");
            while(self_ref_pos < dest.length())
            {
               dest.cut(self_ref_pos,3);
               self_ref_pos = dest.find("/./",self_ref_pos);
            }
            self_ref_pos = dest.find("/.");
            if(self_ref_pos == dest.length() - 3)
               dest.cut(self_ref_pos);
         } // expand_local_path
      };
      

      void FileSystemObject::set_object_name(char const *object_name_)
      {
         // we will take the name provided and make certain that it is an absolute and not relative
         // path.  we can then separate it into name and path components.
         StrAsc object_name;
         expand_local_path(object_name,object_name_);
         split_path(&path,&name,object_name);
         flags = 0;
         if(path.length() == 0)
         {
            path = name;
            flags |= (flag_root | flag_directory);
         }
         else if(path.last() != '/')
            path.append('/');

         // we now need to get operating system information for the name
         struct stat file_stat;
         int rcd = stat(object_name.c_str(),&file_stat);
         if(rcd == 0)
         {
            struct tm broken_down;
            flags = 0;
            if(S_ISDIR(file_stat.st_mode))
               flags |= flag_directory;
            if(S_ISLNK(file_stat.st_mode))
               flags |= flag_symlink;
            localtime_r(
               &file_stat.st_ctime,
               &broken_down);
            creation_time.setDate(
               broken_down.tm_year + 1900,
               broken_down.tm_mon + 1,
               broken_down.tm_mday + 1);
            creation_time.setTime(
               broken_down.tm_hour,
               broken_down.tm_min,
               broken_down.tm_sec,
               0);
            localtime_r(
               &file_stat.st_atime,
               &broken_down);
            last_read_date.setDate(
               broken_down.tm_year + 1900,
               broken_down.tm_mon + 1,
               broken_down.tm_mday + 1);
            last_read_date.setTime(
               broken_down.tm_hour,
               broken_down.tm_min,
               broken_down.tm_sec,
               0);
            localtime_r(
               &file_stat.st_mtime,
               &broken_down);
            last_write_date.setDate(
               broken_down.tm_year + 1900,
               broken_down.tm_mon + 1,
               broken_down.tm_mday + 1);
            last_write_date.setTime(
               broken_down.tm_hour,
               broken_down.tm_min,
               broken_down.tm_sec,
               0);
            size = file_stat.st_size;
            is_valid = true;
         }
         else
            is_valid = false;
      } // set_object_name 
   };
};

