/* Csi.Utils.cpp

   This module implements the free-standing functions whose prototypes are
   declared in CsiUtils.h.

   Copyright (C) 1998, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Friday 24 March 1996
   Last Change: Wednesday 07 October 2020
   Last Commit: $Date: 2020-10-07 16:29:13 -0600 (Wed, 07 Oct 2020) $ 
   Committed by: $Author: jon $
   
*/

#pragma warning(disable: 4996)
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.OsException.h"
#include "Csi.LgrDate.h"
#include "Csi.Utils.h"
#include "Csi.FileSystemObject.h"
#include "Csi.RegistryManager.h"
#include "Csi.StrAscStream.h"
#include "CsiTypes.h"
#include "Csi.VersionNumber.h"
#include "Csi.BlowFish.h"
#include "Csi.Xml.Element.h"
#include "Csi.MaxMin.h"
#include "Csi.CsvRec.h"
#include "coratools.strings.h"
#include "boost/format.hpp"
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCKAPI_ // prevent windows.h from including winsock.h
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <direct.h>
#include <sys/utime.h>
#else
#include <sys/statvfs.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <utime.h>
#include <fstream>
#endif
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <algorithm>
#include <limits>
#include "boost/version.hpp"
#if BOOST_VERSION >= 104200
#include "boost/uuid/uuid_generators.hpp"
#include "boost/uuid/uuid_io.hpp"
#else
#include <uuid/uuid.h>
#endif


namespace Csi
{
   using namespace UtilsStrings;

   
   uint2 calcCheckSumFor(void const *buff, uint4 len)
   {
      uint2 rtn = 0;
      byte const *str = (byte const *)buff;

      // Calculate the sum of all bytes in the buffer. Ignore any overflow. When
      // the iteration is finished, mask off the 13 least significant bits.
      for(uint4 i = 0; i < len; i++)
         rtn += (str[i] & 0x7f);
      return rtn&0x1FFF;
   } // end CalcCheckSumFor


   uint4 crcForFile(FILE *in)
   {
      uint4 rtn = 0;
      byte buff[128];
      uint4 numRead;

      while((numRead = (uint4)fread(buff, 1, sizeof(buff), in)) > 0)
         rtn = crc32(buff, numRead, rtn);
      return rtn;
   } // crcForFile 


   void efread(void *buffer, size_t size, size_t cnt, FILE *in)
   {
      if(cnt != 0 && size != 0)
      {
         size_t rcd = cnt;
         rcd = fread(buffer,size,cnt,in);
         if(rcd != cnt)
            throw OsException(my_strings[strid_file_read_error].c_str());
      }
   } // efread


   void efwrite(const void *buffer, size_t size, size_t cnt, FILE *out)
   {
      if(cnt != 0 && size != 0)
      {
         size_t rcd = 0;
         rcd = fwrite(buffer,size,cnt,out);
         if(rcd != cnt)
            throw OsException(
               my_strings[strid_file_write_error].c_str());
      }
   } // efwrite


   void createNestedDir(const char *path_)
   {
      // we can break the set of paths that need to be checked or created by calling
      // get_path_from_file_name() repeatedly and storing each path into a stack.  Once we hit the
      // beginning or the root path, we should be able to check and/or create each path as we go
      // along
      std::list<StrAsc> paths;
      StrAsc parent;

      if(path_[0] != 0)
      {
         paths.push_front(path_);
         do
         {
            split_path(&parent,0,paths.front());
            if(parent.length() > 0)
               paths.push_front(parent);
         }
         while(parent.length() > 0);
      }

      // the stack of paths should now have the root level path at its head.  We can now check out
      // each element in the list and create elements as needed
      while(!paths.empty())
      {
         FileSystemObject dir(paths.front().c_str());
         if(!dir.get_is_valid())
         {
#ifdef _WIN32
            StrUni path(paths.front(), true);
            BOOL rcd = ::CreateDirectoryW(path.c_str(), 0);
            if(!rcd && GetLastError() != ERROR_ALREADY_EXISTS)
            {
               Csi::OStrAscStream error_msg;
               error_msg << boost::format(my_strings[strid_unable_to_create_dir].c_str()) %
                  paths.front();
               throw OsException(error_msg.str().c_str());
            }
#else
            int rcd = mkdir(
               paths.front().c_str(),
               S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
            if(rcd != 0)
            {
               Csi::OStrAscStream error_msg;
               error_msg << boost::format(my_strings[strid_unable_to_create_dir].c_str()) %
                  paths.front();
               throw OsException(error_msg.str().c_str());
            }
#endif
         }
         else if(!dir.is_directory())
         {
            Csi::OStrAscStream error_msg;
            error_msg << boost::format(my_strings[strid_not_a_dir].c_str()) % paths.front();
            throw MsgExcept(error_msg.str().c_str());
         }
         paths.pop_front();
      }
   } // createNestedDir


   bool purgeDirectory(char const *dir_name, bool purge_dir)
   {
      // we can purge all of the files using the FileSystemObject and the standard remove() function
      // call. 
      FileSystemObject dir(dir_name);
      FileSystemObject::children_type children;
      bool rtn = true;
      bool anything_left = false;
      
      dir.get_children(children);
      while(!children.empty())
      {
         FileSystemObject child(children.front());
         children.pop_front();
         if(child.is_directory() && child.get_name() != "." && child.get_name() != "..")
         {
            if(!purgeDirectory(child.get_complete_name().c_str()))
               anything_left = true;
         }
         else if(!child.is_directory())
         {
            if(remove_file(child.get_complete_name().c_str()) != 0)
               anything_left = true;
         }
      }

      // we have purged all the children to this directory including child directories.  We now need
      // to remove this directory.  This call is OS specific
      if(!anything_left && purge_dir)
      {
#ifdef _WIN32
         StrUni path(dir_name);
         rtn = (::RemoveDirectoryW(path.c_str()) != 0);
#else
         rtn = (rmdir(dir_name) == 0);
#endif
      }
      else if(anything_left)
         rtn = false;
      return rtn;
   } // purgeDirectory


   uint2 calcSigFor(void const *buff, size_t len, uint2 seed)
   {
      uint2 j;
      uint2 rtn = seed;
      byte const *str = (byte const *)buff;

      // calculate a representative number for the byte block using the CSI
      // signature algorithm.
      for(size_t n = 0; n < len; n++)
      {
         j = rtn;
         rtn = (rtn << 1) & (uint2)0x01FF;
         if (rtn >= 0x100)
            rtn++;
         rtn = (((rtn + (j >> 8) + str[n]) & (uint2)0xFF) | (j << 8));
      } 
      return rtn;
   } // calcSigFor


   uint2 calc_file_sig(FILE *in, int8 len, uint2 seed)
   {
      uint2 rtn = seed;
      int8 count = 0;
      char buff[512];

      while(len < 0 || count < len)
      {
         size_t bytes_to_read = sizeof(buff);
         size_t bytes_read;
         
         if(count + (int8)bytes_to_read > len)
            bytes_to_read = static_cast<size_t>(len - count);
         bytes_read = fread(buff, 1, bytes_to_read, in);
         rtn = calcSigFor(buff, bytes_read, rtn);
         count += bytes_read;
      }
      return rtn;
   } // calc_file_sig


   uint2 calcSigNullifier(uint2 sig)
   {
      // calculate the value for the most significant byte. Then run this value through the signature
      // algorithm using the specified signature as seed. The calculation is designed to cause the
      // least significant byte in the signature to become zero.
      uint2 new_seed = (sig << 1)&0x1FF;
      byte null1;
      uint2 new_sig = sig;

      if(new_seed >= 0x0100)
         new_seed++;
      null1 = (byte)(0x0100 - (new_seed + (sig >> 8)));
      new_sig = calcSigFor(&null1,1,sig);

      // now perform the same calculation for the most significant byte in the signature. This time
      // we will use the signature that was calculated using the first null byte
      byte null2;

      new_seed = (new_sig << 1)&0x01FF;
      if(new_seed >= 0x0100)
         new_seed++;
      null2 = (byte)(0x0100 - (new_seed + (new_sig >> 8)));

      // now form the return value placing null1 one in the most signicant byte location
      uint2 rtn = null1;
      rtn <<= 8;
      rtn += null2;
      return rtn;
   } // calcSigNullifier


   void adjustForMidnight(LgrDate &stamp, int hour1, int hour2)
   {
      // adjust for differences in the clock where a one clock has crossed
      // midnight and the other clock is about to. We will only adjust if both
      // hour hands are on the top semi-circle of a 24-hour clock
      if((hour1 >= 18 || hour1 <= 6) && (hour2 >= 18 || hour2 <= 6))
      {
         if(hour1 < hour2)
            stamp = stamp + LgrDate(LgrDate::nsecPerDay);
         else
            stamp = stamp - LgrDate(LgrDate::nsecPerDay);
      }
   } // adjustForMidnight


   bool validateSig(void const *buff, uint4 len)
   {
      bool rtn = false;
      byte const *str = (byte const *)buff;

      if(len >= 2)
      {
         uint2 rxSig = str[len - 2]; // received signature
         uint2 cSig;               // calculated signature
      
         rxSig <<= 8;
         rxSig += str[len - 1];
         cSig = calcSigFor(buff,len - 2);
         if(rxSig == cSig)
            rtn = true;
      }
      return rtn;
   } // validateSig


   uint4 get_tick_count()
   {
      uint4 rtn;
#ifdef _WIN32
      rtn = GetTickCount();
#else
      struct tms process_times;
      int8 ticks = times(&process_times);
      uint4 times_unit = sysconf(_SC_CLK_TCK);
      
      ticks = (ticks*1000) / times_unit;
      rtn = static_cast<uint4>(ticks);
#endif
      return rtn;
   } // get_tick_count

   
   uint4 counter(uint4 cnt)
   {
      uint4 rtn;
      uint4 ticks = get_tick_count();

      if(ticks >= cnt)
         rtn = ticks - cnt;
      else
         rtn = (UInt4_Max - cnt) + ticks;
      return rtn;
   } // counter


   uint4 timeOfDay()
   {
      uint4 const msecPerSec = 1000;
      uint4 const msecPerMin = msecPerSec*60;
      uint4 const msecPerHour = msecPerMin*60;
#ifdef _WIN32
      SYSTEMTIME sysTime;
      uint4 rtn;
   
      GetLocalTime(&sysTime);
      rtn = sysTime.wMilliseconds;
      rtn += msecPerSec*sysTime.wSecond;
      rtn += msecPerMin*sysTime.wMinute;
      rtn += msecPerHour*sysTime.wHour;
#else
      struct timeval sys_time;
      struct tm local_broken;
      uint4 rtn;
      
      gettimeofday(&sys_time,0);
      localtime_r(&sys_time.tv_sec,&local_broken);
      rtn = sys_time.tv_usec/1000;
      rtn += msecPerSec * local_broken.tm_sec;
      rtn += msecPerMin * local_broken.tm_min;
      rtn += msecPerHour * local_broken.tm_hour;
#endif
      return rtn;
   } // timeOfDay


   int8 getDiscFreeSpace(char const *path_)
   {
#ifdef _WIN32
      // isolate the root directory from the path. This path might contain a UNC
      StrAsc root_dir(path_);
      size_t str_pos;
      char const *root_path_name;

      if((str_pos = root_dir.find("\\\\")) < root_dir.length())
      {
         size_t str_pos_2 = root_dir.find("\\",str_pos);
         if(str_pos_2 < root_dir.length())
            root_dir.cut(str_pos_2 + 1);
         else
            root_dir += '\\';
         root_path_name = root_dir.c_str();
      }
      else if((str_pos = root_dir.find(":\\")) < root_dir.length())
      {
         root_dir.cut(str_pos + 2);
         root_path_name = root_dir.c_str();
      }
      else
         root_path_name = 0;

      // if the GetFreeDiskSpaceEx() call is available, we need to use it because it will report
      // available space greater than 2GB.
      int8 rtn = 0;
      FARPROC extended_function = 
         ::GetProcAddress(
            ::GetModuleHandleW(L"kernel32.dll"),
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
            root_path_name,
            &free_available,
            &total_bytes,
            &total_free_bytes);
         if(rcd)
            rtn = static_cast<int8>(free_available.QuadPart);
         else
            rtn = 0;
      }
      else
      {
         // call the windows GetDiskFreeSpace method
         uint4 sectors_per_cluster;
         uint4 bytes_per_sector;
         uint4 free_cluster_count;
         uint4 total_clusters;
         BOOL rcd = ::GetDiskFreeSpaceA(
            root_path_name,
            &sectors_per_cluster,
            &bytes_per_sector,
            &free_cluster_count,
            &total_clusters);
         
         if(rcd)
         {
            rtn = free_cluster_count;
            rtn *= sectors_per_cluster;
            rtn *= bytes_per_sector;
         }
         else
            rtn = 0;
      }
#else
      // the specified path may or may not exist.  In order to assure success, we will start with
      // the given path and move up it until a valid object is found
      StrAsc path(path_);
      struct statvfs system_stat;
      int8 rtn = 0;

      while(rtn == 0)
      {
         if(statvfs(path.c_str(),&system_stat) == 0)
            rtn = system_stat.f_bavail * system_stat.f_frsize;
         else
         {
            StrAsc parent;
            split_path(&parent,0,path);
            if(parent == path)
               break;
            path = parent;
         }
      }
#endif
      return rtn; 
   } // csiGetDiscFreeSpace


   char const *longFileNameToShort(StrAsc &dest, char const *src)
   {
      // the old algorithm could get fooled regarding the real file extension if the name had
      // multiple periods.  We will work around this by copying the whole name into the buffer and
      // then locating the position of the LAST period in the string to manipulate it. 
      size_t last_period_pos;
      dest = src;
      last_period_pos = dest.rfind(".");
      if(last_period_pos < dest.length())
      {
         // the extension can have at most four characters counting the period.  We will make sure
         // that anything trailing is cut off. 
         dest.cut(last_period_pos + 4);

         // we can have at most eight characters preceding the period
         if(last_period_pos > 8)
         {
            dest.cut(8,last_period_pos - 8);
            last_period_pos = 8;
         }

         // finally, we have to eliminate any other periods preceding the last one.
         size_t other_period_pos = dest.find(".");
         while(other_period_pos < last_period_pos)
         {
            dest[other_period_pos] = '_';
            other_period_pos = dest.find(".",other_period_pos);
         }
      }
      else if(dest.length() > 8)
         dest.cut(8);
      return dest.c_str();
   } // longFileNameToShort


   bool backup_file(
      char const *base_name,
      char const *backup_extension,
      bool make_unique,
      bool use_rename)
   {
      bool rtn = false;
      Csi::OStrAscStream backup_file_name;
      uint4 attempts = 0;

      backup_file_name << base_name << "." << backup_extension;
      while(make_unique && file_exists(backup_file_name.str().c_str()))
      {
         backup_file_name.str("");
         backup_file_name << base_name << "." << (++attempts) << "." << backup_extension;
      }
      if(use_rename)
      {
         if(!make_unique)
            remove_file(backup_file_name.str().c_str());
         rtn = move_file(base_name, backup_file_name.str().c_str());
      }
      else
         rtn = copy_file(backup_file_name.str().c_str(),base_name);
      return rtn;
   } // backup_file


   bool copy_file(char const *destination, char const *source)
   {
      bool rtn = false;
#ifdef _WIN32
      // under WIN32, we can use the supplied CopyFile function, under another OS, another method
      // would have to be used.
      StrUni src(source, true);
      StrUni dest(destination, true);
      if(::CopyFileW(src.c_str(), dest.c_str(), FALSE))
         rtn = true;
#else
      FILE *sf = Csi::open_file(source, "rb");
      FILE *df = Csi::open_file(destination, "wb");
      char buff[2048];
      size_t bytes_read;
      
      if(sf && df)
         rtn = true;
      while(rtn && (bytes_read = fread(buff,1,sizeof(buff),sf)) > 0)
         fwrite(buff,bytes_read,1,df);
      if(sf)
         fclose(sf);
      if(df)
         fclose(df);
      if(rtn)
      {
         // we need to apply the attributes (ownership, access, and dates to the destination file as
         // the source file had.
         struct stat source_stat;
         struct utimbuf dest_time;
         
         stat(source,&source_stat);
         chmod(destination,source_stat.st_mode);
         chown(destination,source_stat.st_uid,source_stat.st_gid);
         dest_time.actime = source_stat.st_atime;
         dest_time.modtime = source_stat.st_mtime;
         utime(destination,&dest_time);
      }
#endif
      return rtn;
   } // copy_file


   bool move_file(char const *old_name, char const *new_name)
   {
      bool rtn = false;
#ifdef _WIN32
      StrUni source(old_name, true);
      StrUni dest(new_name, true);
      BOOL rcd = ::MoveFileExW(
         source.c_str(),
         dest.c_str(),
         MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
      if(rcd)
         rtn = true;
#else
      int rcd = ::rename(old_name, new_name);
      if(rcd != 0)
      {
         rtn = copy_file(new_name, old_name);
         if(rtn)
            remove_file(old_name);
      }
      else
         rtn = (rcd == 0);
#endif
      return rtn;
   } // move_file


   int remove_file(char const *file_name)
   {
#ifdef WIN32
      StrUni name(file_name, true);
      return  ::_wremove(name.c_str());
#else
      return ::remove(file_name);
#endif
   }


   void split_path(
      StrAsc *parent_path,
      StrAsc *name,
      StrAsc const &path)
   {
      // we need to search for the end of the root specifier
      size_t root_end_pos = path.length();
      if(path.length() >= 3 && path[1] == ':' && path[2] == '\\')
         root_end_pos = 2;      // windows root
      else if(path.length() > 3 && path[0] == '\\' && path[1] == '\\')
         root_end_pos = path.find("\\",2); // unc-root
      else if(path.length() >= 1 && path[0] == '/')
         root_end_pos = 0;      // unix root
      else if(path.length() >= 2 && path[0] == '~' && path[1] == '/')
         root_end_pos = 1;      // unix home root

      // we now need to search for the last separator in the path,  if the path ends with a
      // separator, we will search for the one before that
      size_t last_sep_pos = path.rfind("\\");
      if(last_sep_pos >= path.length())
         last_sep_pos = path.rfind("/");
      if(last_sep_pos + 1 == path.length())
      {
         last_sep_pos = path.rfind("\\",last_sep_pos - 1);
         if(last_sep_pos >= path.length())
            last_sep_pos = path.rfind("/",last_sep_pos - 1);
      }

      // we will use the two positions to determine the split between the name and the path
      if(last_sep_pos > root_end_pos && last_sep_pos < path.length())
      {
         if(parent_path != 0)
            path.sub(*parent_path,0,last_sep_pos);
         if(name != 0)
            path.sub(*name,last_sep_pos + 1,path.length());
      }
      else if(root_end_pos < path.length() - 1)
      {
         if(parent_path != 0)
            path.sub(*parent_path,0,root_end_pos + 1);
         if(name != 0)
            path.sub(*name,root_end_pos + 1,path.length());
      }
      else if(last_sep_pos < path.length() && root_end_pos >= path.length())
      {
         if(parent_path != 0)
            path.sub(*parent_path,0,last_sep_pos + 1);
         if(name != 0)
            path.sub(*name,last_sep_pos + 1,path.length());
      }
      else
      {
         if(parent_path != 0)
            parent_path->cut(0);
         if(name != 0)
            *name = path;
      }
      if(parent_path != 0 && *parent_path == path)
         parent_path->cut(0);
   } // split_path

   
   char const *get_path_from_file_name(StrAsc &dest, char const *file_name)
   {
      split_path(&dest,0,file_name);
      return dest.c_str();
   }


   char const *get_name_from_file_path(StrAsc &dest, char const *file_path)
   {
      split_path(0,&dest,file_path); 
      return dest.c_str();
   } // get_name_from_file_path


   void check_file_name_path(char const *file_name)
   {
      // extract the path from the file name and run it through createNestedDir
      StrAsc path;
      get_path_from_file_name(path,file_name);
      createNestedDir(path.c_str());
   } // check_file_name_path


   char const *make_temp_file_name(
      char const *directory_,
      StrAsc &file_name)
   {
#ifdef _WIN32
      // the _tempnam() function depends on the value of the TMP environment variable.  We will save
      // off its current value and set it to the directory.
      wchar_t const *tmp_value(_wgetenv(L"TMP"));
      StrUni current_temp_dir;
      StrUni directory(directory_);
      StrUni new_temp_dir(StrUni(L"TMP=") + directory);

      if(tmp_value != 0)
         current_temp_dir = StrUni(L"TMP=") + tmp_value;
      _wputenv(new_temp_dir.c_str());
      createNestedDir(directory_);
      
      // we will assume that the directory already exists.  We will use the _tempnam() function in
      // the microsoft "C" run time to generate the name.
      wchar_t *result = _wtempnam(directory.c_str(), L"CSI");
      StrUni rtn;
      if(result == 0)
      {
         OsException error(my_strings[strid_temp_file_failed].c_str());
         _wputenv(current_temp_dir.c_str());
         throw error;
      }
      if(result[0] == L'\\' && result[1] != L'\\')
         rtn = directory + result;
      else
         rtn = result;
      free(result);
      _wputenv(current_temp_dir.c_str());
      file_name = rtn.to_utf8();
#else
      try
      {
         // make sure that the specified directory exists
         int trial_no = 0;
         Csi::OStrAscStream trial_name;
         
         createNestedDir(directory_);
         do
         {
            trial_name.str("");
            trial_name << directory_ << "/tmp" << trial_no++;
         }
         while(file_exists(trial_name.str().c_str()));
         file_name = trial_name.str().c_str();
      }
      catch(std::exception &)
      { file_name.cut(0); } 
#endif
      return file_name.c_str();
   } // make_temp_file_name


   namespace
   {
      char const *reserved_names[] =
      {
         "CON",
         "PRN",
         "AUX",
         "NUL",
         "COM1",
         "COM2",
         "COM3",
         "COM4",
         "COM5",
         "COM6",
         "COM7",
         "COM8",
         "COM9",
         "LPT1",
         "LPT2",
         "LPT3",
         "LPT4",
         "LPT5",
         "LPT6",
         "LPT7",
         "LPT8",
         "LPT9",
         0
      };
   };

   
   bool is_reserved_file_name(char const *file_name)
   {
      bool rtn = false;
#ifdef _WIN32
      StrAsc name;
      
      get_name_from_file_path(name, file_name);
      name.cut(name.rfind("."));
      for(int i = 0; !rtn && reserved_names[i] != 0; ++i)
      {
         if(name == reserved_names[i])
            rtn = true;
      }
#endif
      return rtn;
   } // is_reserved_file_name
   
   
   FILE *make_temp_file(
      char const *directory,
      StrAsc &file_name,
      char const *open_mode)
   {
      // open the file handle
      FILE *rtn = open_file(
         make_temp_file_name(directory,file_name),
         open_mode);
      if(rtn == 0)
         throw OsException(my_strings[strid_temp_file_failed].c_str());
      return rtn;
   } // make_temp_file


   bool file_exists(char const *file_name)
   {
      bool rtn(false);
      if(file_name[0] != 0)
      {
         Csi::FileSystemObject file(file_name);
         rtn = file.get_is_valid();
      }
      return rtn;
   } // file_exists


   bool delete_file(char const *file_name)
   {
      return remove(file_name) == 0;
   } //delete_file


   void reverse_byte_order(void *buffer, size_t buff_len)
   {
      byte *buff = static_cast<byte *>(buffer);
      for(size_t i = 0; i < buff_len/2; ++i)
         std::swap(buff[i], buff[buff_len - i - 1]);
   } // reverse_byte_order


   char const *get_app_dir(StrAsc &dir)
   {
      StrAsc temp;
      return get_path_from_file_name(
         dir,
         get_program_path(temp));
   }


   char const *get_work_dir(
      StrAsc &dir, 
      StrAsc const &app_name,
      StrAsc const &version)
   {
      RegistryManager reg_man(app_name,version);
      return reg_man.read_work_dir(dir);
   } // get_work_dir


   StrAsc get_app_home_dir(StrAsc const &app_name)
   {
      StrAsc rtn;
#ifdef _WIN32
      char *env(::getenv("LOCALAPPDATA"));
      if(env)
      {
         rtn = env;
         if(rtn.last() != '\\')
            rtn.append('\\');
         rtn.append("Campbell_Scientific_Inc\\");
         rtn.append(app_name);
      }
#else
      char *home(::getenv("HOME"));
      if(home)
      {
         rtn = home;
         if(rtn.last() != '/')
            rtn.append('/');
         rtn.append(".Campbell_Scientific_Inc/");
         rtn.append(app_name);
      }
      else
         rtn = "~/.Campbell_Scientific_Inc/" + app_name;
#endif
      return rtn;
   } // get_app_home_dir


   char const *get_program_path(StrAsc &dest)
   {
#ifdef _WIN32
      wchar_t path[MAX_PATH];
      GetModuleFileNameW(
         GetModuleHandle(0),
         path,
         sizeof(path));
      dest = StrUni(path).to_utf8();
#else
      // the following code is linux specific.  There is no portable way to do this on all intended
      // operating systems.  Todo:  Use a macro to select the linux build.
      char path[4096];
      memset(path,0,sizeof(path));
      int rcd = readlink(
         "/proc/self/exe",
         path,
         sizeof(path) - 1);
      if(rcd > 0)
         dest = path;
      else
         dest.cut(0);
#endif
      return dest.c_str();
   } // get_program_path


   StrAsc get_system_working_dir()
   {
      StrAsc rtn;
#ifdef _WIN32
      wchar_t buff[1024];
      ::GetCurrentDirectoryW(sizeof(buff), buff);
      rtn = StrUni(buff).to_utf8();
#else
      char buff[1024];
      ::getcwd(buff, sizeof(buff));
      rtn = buff;
#endif
      return StrAsc(rtn);
   }
   

   int8 get_available_virtual_memory()
   {
#ifdef _WIN32
      MEMORYSTATUS mem_stat;
      GlobalMemoryStatus(&mem_stat);
      return mem_stat.dwAvailVirtual;
#else
      // we need to discover the limit placed upon this process by the system. Under posix, this is
      // done via getrlimit() which is posix standard.
      uint4 rtn = 0;
      uint4 max_limit = 0x7fffffff; // we assume 2 GB max as the upper limit
      struct rlimit os_limit;
      if(getrlimit(RLIMIT_AS,&os_limit) == 0 && os_limit.rlim_cur != RLIM_INFINITY)
         max_limit = static_cast<uint4>(os_limit.rlim_cur);

      // we now need to discover how much memory our process has used.  The following code is linux
      // specific.  Todo: put this code inside of linux specific conditional compilation
      std::ifstream stat_file("/proc/self/stat");
      if(stat_file)
      {
         int pid;
         StrAsc comm;
         char state;
         int ppid;
         int pgrp;
         int session;
         int tty_nr;
         int tpgid;
         unsigned long flags;
         unsigned long minflt;
         unsigned long cminflt;
         unsigned long majflt;
         unsigned long cmajflt;
         unsigned long utime;
         unsigned long stime;
         unsigned long cutime;
         unsigned long cstime;
         unsigned long priority;
         unsigned long nice;
         unsigned long placeholder;
         unsigned long itrealvalue;
         unsigned long starttime;
         unsigned long vsize;
         stat_file >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
                   >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
                   >> stime >> cutime >> cstime >> priority >> nice >> placeholder
                   >> itrealvalue >> starttime >> vsize;
         rtn = max_limit - static_cast<uint4>(vsize);
      }
   
      return rtn;
#endif
   } // get_available_virtual_memory


   uint4 file_length(FILE *f)
   {
#ifdef _WIN32
      return _filelength(_fileno(f)); 
#else
      struct stat os_stat;
      int rcd = fstat(fileno(f),&os_stat);
      return static_cast<uint4>(os_stat.st_size);
#endif
   } // file_length

   
   int8 long_file_length(FILE *f)
   {
#ifdef _WIN32
      return _filelengthi64(_fileno(f));
#else
      struct stat os_stat;
      int rcd = fstat(fileno(f),&os_stat);
      return static_cast<int8>(os_stat.st_size);
#endif
   } // long_file_length


   int file_seek(FILE *file, int8 offset, int where)
   {
#ifdef _WIN32
      return _fseeki64(file, offset, where);
#else
      return fseeko(file, static_cast<off_t>(offset), where);
#endif
   } // file_seek


   int8 file_tell(FILE *file)
   {
#ifdef _WIN32
      return _ftelli64(file);
#else
      return ftello(file);
#endif
   } // file_tell


   size_t locate_sub_string(
      void const *buff_,
      size_t buff_len,
      void const *pattern_,
      size_t pattern_len)
   {
      // if the pattern is empty or longer than the provided buffer lenght, there is no possible
      // match.
      size_t rtn = buff_len;
      if(buff_len > 0 && pattern_len > 0 && pattern_len < buff_len)
      {
         // we will use a naive algorithm to check for the pattern at each position.
         byte const *buff = static_cast <byte const *>(buff_);
         byte const *pattern = static_cast<byte const *>(pattern_);
         for(size_t i = 0; i + pattern_len <= buff_len; ++i)
         {
            size_t j;
            for(j = 0;
                j < pattern_len && buff[i + j] == pattern[j];
                ++j)
               0;
            if(j >= pattern_len)
            {
               rtn = i;
               break;
            }
         }
      }
      return rtn;
   } // locate_sub_string


   namespace
   {
      ////////////////////////////////////////////////////////////
      // application_command_line
      ////////////////////////////////////////////////////////////
      StrAsc application_command_line;
   };
   

   char const *get_command_line()
   {
#ifdef WIN32
      if(application_command_line.length() == 0)
      {
         StrUni temp(::GetCommandLineW());
         application_command_line = temp.to_utf8();
      }
#endif
      return application_command_line.c_str();
   } // get_command_line


   void set_command_line(
      int argc,
      char const *argv[])
   {
#ifndef _WIN32
      application_command_line.cut(0);
      for(int i = 0; i < argc; ++i)
      {
         application_command_line.append(argv[i]);
         if(i + 1 < argc)
            application_command_line.append(' ');
      }
#endif
   } // set_command_line


   bool matches_wildcard_expr(
      StrAsc const &str,
      StrAsc const &expr)
   {
      bool rtn = true;
      size_t str_pos = 0;
      size_t expr_pos = 0;
      size_t last_partial = expr.length();
      
      while(rtn && str_pos < str.length() && expr_pos < expr.length())
      {
         char str_ch = toupper(str[str_pos]);
         char expr_ch = toupper(expr[expr_pos]);

         if(expr_ch == '*')
         {
            if(expr_pos + 1 < expr.length())
            {
               expr_ch = toupper(expr[expr_pos + 1]);
               if(str_ch == expr_ch)
               {
                  last_partial = expr_pos;
                  expr_pos += 2;
               }
               ++str_pos;
            }
            else
            {
               expr_pos = expr.length();
               str_pos = str.length();
            }
         }
         else if(expr_ch == '?')
         {
            ++str_pos;
            ++expr_pos;
         }
         else
         {
            if(str_ch != expr_ch)
            {
               if(last_partial < expr.length())
               {
                  expr_pos = last_partial;
                  last_partial = expr.length();
               }
               else
                  rtn = false;
            }
            else
            {
               ++str_pos;
               ++expr_pos;
            }
         }
      }
      while(expr_pos < expr.length() && expr[expr_pos] == '*')
         ++expr_pos;
      if(str_pos < str.length() || expr_pos < expr.length())
         rtn = false;
      return rtn;
   } // matches_wildcard_expr


   data_file_types get_data_file_type(
      FILE *in,
      int8 *append_offset)
   {
      // read the first 1K of the file.
      data_file_types rtn = data_file_type_unknown;
      char buff[1024];
      fseek(in, 0, SEEK_SET);
      uint4 len = (uint4)fread(buff,1,sizeof(buff),in);
      
      // the default append offset will be the end of the file
      if(append_offset)
         *append_offset = long_file_length(in);
      
      // for all formats but csixml, the format name will appear in quotes as the item on the
      // first line.  we will skip over any initial whitespace
      uint4 i = 0;
      while(i < len && isspace(buff[i]))
         ++i;
      // if the first non-white space character is a quote, look for the format name to follow.
      if(i < len && buff[i] == '\"')
      {
         // we will extract the format name between this char and the next quote
         StrAsc format_name;
         ++i;
         while(i < len && buff[i] != '\"')
            format_name.append(buff[i++]);
         if(format_name == "TOACI1" ||
            format_name == "TOACSI" ||
            format_name == "TOARI1")
            rtn = data_file_type_toaci1;
         else if(format_name == "TOA5")
            rtn = data_file_type_toa5;
         else if(format_name == "TOA6")
            rtn = data_file_type_toa6;
         else if(format_name == "TOB1")
            rtn = data_file_type_tob1;
         else if(format_name == "TOB2")
            rtn = data_file_type_tob2;
         else if(format_name == "TOB3")
            rtn = data_file_type_tob3;
         else if(format_name.length() > 0 && isdigit(format_name[0]))
            rtn = data_file_type_mixed_csv;
      }
      // if the first non-white space character appears to start a tag, we will assume csixml
      else if(i < len && buff[i] == '<')
      {
         // we will verify the format by searching forward for "<csixml" in the buffer.
         char const token[] = "<csixml";
         size_t token_pos = locate_sub_string(
            buff,
            len,
            token,
            sizeof(token) - 1);
         if(token_pos < len)
         {
            if(append_offset)
            {
               // the file may by shorter than the buffer length
               int8 start_end_pos = 0;
               char const end_token[] = "</data>";
               
               if(*append_offset > sizeof(buff))
                  start_end_pos = *append_offset - sizeof(buff);
               file_seek(in, start_end_pos, SEEK_SET);
               len = (uint4)fread(buff, 1, sizeof(buff), in);
               token_pos = locate_sub_string(
                  buff,
                  len,
                  end_token,
                  sizeof(end_token) - 1);
               if(token_pos < len)
                  *append_offset = start_end_pos + token_pos;
            }
            rtn = data_file_type_csixml;
         }
      }
      // there appears to be no header or XML structure.  The next step is to examine for one of
      // the three mixed array options
      else if(i < len && ((buff[i] >= '0' && buff[i] <= '9') || buff[i] == ','))
      {
         // this appears to be a text file.
         while(i < len && rtn == data_file_type_unknown)
         {
            if(buff[i] == ' ')
               rtn = data_file_type_mixed_printable;
            else if(buff[i] == ',')
               rtn = data_file_type_mixed_csv;
            ++i;
         }
      }
      else if(i + 2 < len && csiFsfType(buff + i) != FsfCantTell)
         rtn = data_file_type_mixed_binary;
      return rtn;
   } // get_data_file_type

   
   data_file_types get_data_file_type(
      char const *file_name,
      int8 *append_offset)
   {
      data_file_types rtn = data_file_type_unknown; 
      FILE *in = open_file(file_name,"rb");
      if(in != 0)
      {
         rtn = get_data_file_type(in, append_offset);
         fclose(in);
      }
      else
         rtn = data_file_type_error;
      return rtn;
   } // get_data_file_type


   char const *data_file_type_name(int file_type_code)
   {
      char const *rtn = "unknown";
      switch(file_type_code)
      {
      case data_file_type_toaci1:
         rtn = "TOACI1";
         break;
         
      case data_file_type_toa5:
         rtn = "TOA5";
         break;

      case data_file_type_toa6:
         rtn = "TOA6";
         break;
         
      case data_file_type_tob1:
         rtn = "TOB1";
         break;
         
      case data_file_type_tob2:
         rtn = "TOB2";
         break;
         
      case data_file_type_tob3:
         rtn = "TOB3";
         break;
         
      case data_file_type_csixml:
         rtn = "CSIXML";
         break;
         
      case data_file_type_mixed_csv:
         rtn = "CSV";
         break;
         
      case data_file_type_mixed_printable:
         rtn = "PRINTABLE";
         break;
         
      case data_file_type_mixed_binary:
         rtn = "BINARY";
         break;
      }
      return rtn;
   } // data_file_type_name


   bool is_toa_time(char const *s)
   {
      enum state_type {
         state_error,
         state_parse_year,
         state_after_year,
         state_parse_month,
         state_after_month,
         state_parse_day,
         state_min_success,
         state_parse_hour,
         state_after_hour,
         state_parse_min,
         state_parse_sec,
         state_after_day,
         state_after_min,
         state_after_sec
      } state = state_parse_year;
      StrAsc token;
      
      for(int i = 0; s[i] != 0 && state != state_error && state != state_after_sec; ++i)
      {
         switch(state)
         {
         case state_parse_year:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               if(token.length() > 4)
                  state = state_error;
            }
            else if(s[i] == '-')
            {
               state = state_after_year;
               token.cut(0);
            }
            else
               state = state_error;
            break;
                  
         case state_after_year:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               state = state_parse_month;
            }
            else
               state = state_error;
            break;
            
         case state_parse_month:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               if(token.length() > 2)
                  state = state_error;
            }
            else if(s[i] == '-')
            {
               int month = atoi(token.c_str());
               if(month > 12)
                  state = state_error;
               else
                  state = state_after_month; 
               token.cut(0);
            }
            else
               state = state_error;
            break;
            
         case state_after_month:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               state = state_parse_day;
            }
            else
               state = state_error;
            break;
            
         case state_parse_day:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               if(token.length() > 2)
                  state = state_error;
            }
            else if(s[i] == ' ' || s[i] == 'T')
            {
               int day = atoi(token.c_str());
               if(day > 31)
                  state = state_error;
               else
                  state = state_after_day;
               token.cut(0);
            }
            else
               state = state_error;
            break;
            
         case state_after_day:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               state = state_parse_hour;
            }
            else
               state = state_error;
            break;
            
         case state_parse_hour:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               if(token.length() > 2)
                  state = state_error;
            }
            else if(s[i] == ':')
            {
               int hour = atoi(token.c_str());
               if(hour > 24)
                  state = state_error;
               else
                  state = state_after_hour;
               token.cut(0);
            }
            else
               state = state_error;
            break;
            
         case state_after_hour:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               state = state_parse_min;
            }
            else
               state = state_error;
            break;
            
         case state_parse_min:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               if(token.length() > 2)
                  state = state_error; 
            }
            else if(s[i] == ':')
            {
               int minute = atoi(token.c_str());
               if(minute >= 60)
                  state = state_error;
               else
                  state = state_after_min;
               token.cut(0);
            }
            else
               state = state_error;
            break;
            
         case state_after_min:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               state = state_parse_sec;
            }
            else
               state = state_error;
            break;
            
         case state_parse_sec:
            if(isdigit(s[i]))
            {
               token.append(s[i]);
               if(token.length() > 2)
                  state = state_error;
            }
            else if(s[i] == '.')
            {
               int second = atoi(token.c_str());
               if(second >= 60)
                  state = state_error;
               else
                  state = state_after_sec;
               token.cut(0);
            }
            else
               state = state_error;
            break;
         }
      } 
      return state > state_min_success;
   } // is_toa_time


   void extract_csi_xml_header(
      StrAsc &header_buffer,
      FILE *in)
   {
      StrAsc tag_name;
      enum {
         state_before_begin_tag,
         state_within_possible_begin, 
         state_in_header,
         state_in_header_tag,
         state_in_possible_end,
         state_after_header
      } state = state_before_begin_tag;
      int ch;
      
      while((ch = fgetc(in)) != EOF && state != state_after_header)
      {
         switch(state)
         {
         case state_before_begin_tag:
            if(ch == '<')
            {
               state = state_within_possible_begin;
               tag_name.cut(0);
               tag_name.append(static_cast<char>(ch));
            }
            break;
            
         case state_within_possible_begin:
            if(ch == '>' || isspace(ch))
            {
               if(tag_name == "<head")
               {
                  state = state_in_header;
                  header_buffer = tag_name;
                  header_buffer.append(static_cast<char>(ch));
                  tag_name.cut(0);
               }
               else
               {
                  state = state_before_begin_tag;
                  tag_name.cut(0);
               }
            }
            else
               tag_name.append(static_cast<char>(ch));
            break;
            
         case state_in_header:
            header_buffer.append(static_cast<char>(ch));
            if(ch == '<')
            {
               tag_name.cut(0);
               tag_name.append('<');
               state = state_in_header_tag;
            }
            break;
            
         case state_in_header_tag:
            header_buffer.append(static_cast<char>(ch));
            if(ch == '/')
            {
               tag_name.append('/');
               state = state_in_possible_end;
            }
            else if(!isspace(ch))
               state = state_in_header;
            break;
            
         case state_in_possible_end:
            header_buffer.append(static_cast<char>(ch));
            if(ch == '>')
            {
               if(tag_name == "</head")
                  state = state_after_header;
               else
                  state = state_in_header;
               tag_name.cut(0);
            }
            else if(!isspace(ch))
               tag_name.append(static_cast<char>(ch));
            break;
         }
      }
      if(state != state_after_header)
         throw Csi::MsgExcept(my_strings[strid_extract_xml_header_failed].c_str());
   } // extract_csi_xml_header


   namespace
   {
      void append_header_check_warning(StrAsc *reason, StrAsc const &warning)
      {
         if(reason)
         {
            if(reason->length() > 0)
               reason->append(',');
            reason->append(warning);
         }
      }
      
         
      int toa1_header_check(
         FILE *in, char const *target_header, uint4 target_header_len, StrAsc *reason)
      {
         // validate the first line
         int rtn = can_append_success;
         try
         {
            IBuffStream header(target_header, target_header_len);
            CsvRec line, header_line;
            
            line.read(in);
            header_line.read(header);
            if(line.size() < 3 && header_line.size() < 3)
               throw std::invalid_argument(my_strings[strid_append_invalid_environment_count].c_str());
            if(line[0] != "TOACI1" || header_line[0] != "TOACI1")
               throw std::invalid_argument(my_strings[strid_append_invalid_file_type].c_str());
            if(StrUni(line[1]) != StrUni(header_line[1]))
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_station_names_dont_match]);
            }
            if(StrUni(line[2]) != StrUni(header_line[2]))
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_table_names_dont_match]);
            }

            // read the second line from the two sources
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_number_columns_dont_match].c_str());
            for(uint4 i = 0; i < line.size(); ++i)
               if(StrUni(line[i]) != StrUni(header_line[i]))
                  throw std::invalid_argument(my_strings[strid_append_column_names_dont_match].c_str());
         }
         catch(std::exception &e)
         {
            rtn = can_append_incompatible_headers;
            if(reason)
               *reason = e.what();
         }
         return rtn;
      } // toa1_header_check


      int toa5_header_check(
         FILE *in, char const *target_header, uint4 target_header_len, StrAsc *reason)
      {
         int rtn = can_append_success;
         try 
         {
            CsvRec line, header_line;
            IBuffStream header(target_header, target_header_len);
            CsvRec field_names;
            
            line.read(in);
            header_line.read(header);
            if(line.size() < 8 || header_line.size() < 8)
               throw std::invalid_argument(my_strings[strid_append_invalid_environment_count].c_str());
            if(line[0] != "TOA5" || header_line[0] != "TOA5")
               throw std::invalid_argument(my_strings[strid_append_invalid_file_type].c_str());
            if(line[2] != header_line[2])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_model_numbers_dont_match]);
            }
            if(line[3] != header_line[3])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_serial_numbers_dont_match]);
            }
            if(line[4] != header_line[4])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_os_versions_dont_match]);
            }
            if(line[5] != header_line[5])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_program_names_dont_match]);
            }
            if(line[6] != header_line[6])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_program_sigs_dont_match]);
            }
            if(StrUni(line[1]) != StrUni(header_line[1]))
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_station_names_dont_match]);
            }
            if(StrUni(line[7]) != StrUni(header_line[7]))
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_table_names_dont_match]);
            }

            // we now need to evaluate the field names
            uint4 columns_count = 0;
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_number_columns_dont_match].c_str());
            columns_count = (uint4)line.size();
            for(uint4 i = 0; i < line.size(); ++i)
               if(StrUni(line[i]) != StrUni(header_line[i]))
                  throw std::invalid_argument(my_strings[strid_append_column_names_dont_match].c_str());
            field_names = line;
            
            // we need to evaluate the units strings
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_units_count_dont_match].c_str());
            if(line.size() != columns_count)
               throw std::invalid_argument(my_strings[strid_append_invalid_units_count].c_str());
            for(uint4 i = 0; i < columns_count; ++i)
            {
               if(field_names[i] != "TIMESTAMP" &&
                  field_names[i] != "RECORD" &&
                  StrUni(line[i]) != StrUni(header_line[i]))
               {
                  Csi::OStrAscStream msg;
                  msg << boost::format(my_strings[strid_append_column_units_dont_match].c_str()) % field_names[i];
                  rtn = can_append_with_severe_warnings;
                  append_header_check_warning(reason, msg.str());
               }
            }

            // we need to evaluate the process strings
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_process_count_dont_match].c_str());
            if(line.size() != columns_count)
               throw std::invalid_argument(my_strings[strid_append_invalid_process_count].c_str());
            for(uint4 i = 0; i < columns_count; ++i)
            {
               if(field_names[i] != "TIMESTAMP" &&
                  field_names[i] != "RECORD" &&
                  StrUni(line[i]) != StrUni(header_line[i]))
               {
                  Csi::OStrAscStream msg;
                  msg << boost::format(my_strings[strid_append_column_process_dont_match].c_str()) % field_names[i];
                  rtn = can_append_with_severe_warnings;
                  append_header_check_warning(reason, msg.str());
               }
            }
         }
         catch(std::exception &e)
         {
            rtn = can_append_incompatible_headers;
            if(reason)
               *reason = e.what();
         }
         return rtn;
      } // toa5_header_check


      int toa6_header_check(
         FILE *in, char const *target_header, uint4 target_header_len, StrAsc *reason)
      {
         int rtn = can_append_success;
         try 
         {
            CsvRec line(true), header_line(true);
            IBuffStream header(target_header, target_header_len);
            CsvRec field_names(true);
            
            line.read(in);
            header_line.read(header);
            if(line.size() < 8 || header_line.size() < 8)
               throw std::invalid_argument(my_strings[strid_append_invalid_environment_count].c_str());
            if(line[0] != "TOA6" || header_line[0] != "TOA6")
               throw std::invalid_argument(my_strings[strid_append_invalid_file_type].c_str());
            if(line[2] != header_line[2])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_model_numbers_dont_match]);
            }
            if(line[3] != header_line[3])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_serial_numbers_dont_match]);
            }
            if(line[4] != header_line[4])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_os_versions_dont_match]);
            }
            if(line[5] != header_line[5])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_program_names_dont_match]);
            }
            if(line[6] != header_line[6])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_program_sigs_dont_match]);
            }
            if(StrUni(line[1]) != StrUni(header_line[1]))
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_station_names_dont_match]);
            }
            if(StrUni(line[7]) != StrUni(header_line[7]))
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_table_names_dont_match]);
            }

            // we now need to evaluate the field names
            uint4 columns_count = 0;
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_number_columns_dont_match].c_str());
            columns_count = (uint4)line.size();
            for(uint4 i = 0; i < line.size(); ++i)
               if(StrUni(line[i]) != StrUni(header_line[i]))
                  throw std::invalid_argument(my_strings[strid_append_column_names_dont_match].c_str());
            field_names = line;
            
            // we need to evaluate the units strings
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_units_count_dont_match].c_str());
            if(line.size() != columns_count)
               throw std::invalid_argument(my_strings[strid_append_invalid_units_count].c_str());
            for(uint4 i = 0; i < columns_count; ++i)
            {
               if(field_names[i] != "TIMESTAMP" &&
                  field_names[i] != "RECORD" &&
                  StrUni(line[i]) != StrUni(header_line[i]))
               {
                  Csi::OStrAscStream msg;
                  msg << boost::format(my_strings[strid_append_column_units_dont_match].c_str()) % field_names[i];
                  rtn = can_append_with_severe_warnings;
                  append_header_check_warning(reason, msg.str());
               }
            }

            // we need to evaluate the process strings
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_process_count_dont_match].c_str());
            if(line.size() != columns_count)
               throw std::invalid_argument(my_strings[strid_append_invalid_process_count].c_str());
            for(uint4 i = 0; i < columns_count; ++i)
            {
               if(field_names[i] != "TIMESTAMP" &&
                  field_names[i] != "RECORD" &&
                  StrUni(line[i]) != StrUni(header_line[i]))
               {
                  Csi::OStrAscStream msg;
                  msg << boost::format(my_strings[strid_append_column_process_dont_match].c_str()) % field_names[i];
                  rtn = can_append_with_severe_warnings;
                  append_header_check_warning(reason, msg.str());
               }
            }

            // we need to evaluate the data types line
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_data_types_count_dont_match].c_str());
            for(uint4 i = 0; i < line.size(); ++i)
               if(line[i] != header_line[i])
                  throw std::invalid_argument(my_strings[strid_append_data_types_dont_match].c_str());
         }
         catch(std::exception &e)
         {
            rtn = can_append_incompatible_headers;
            if(reason)
               *reason = e.what();
         }
         return rtn;
      } // toa6_header_check

      
      int tob1_header_check(
         FILE *in, char const *target_header, uint4 target_header_len, StrAsc *reason)
      {
         int rtn = can_append_success;
         try
         {
            
            CsvRec line, header_line;
            IBuffStream header(target_header, target_header_len);
            CsvRec field_names;
            
            line.read(in);
            header_line.read(header);
            if(line.size() < 8 || header_line.size() < 8)
               throw std::invalid_argument(my_strings[strid_append_invalid_environment_count].c_str());
            if(line[0] != "TOB1" || header_line[0] != "TOB1")
               throw std::invalid_argument(my_strings[strid_append_invalid_file_type].c_str());
            if(line[2] != header_line[2])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_model_numbers_dont_match]);
            }
            if(line[3] != header_line[3])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_serial_numbers_dont_match]);
            }
            if(line[4] != header_line[4])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_os_versions_dont_match]);
            }
            if(line[5] != header_line[5])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_program_names_dont_match]);
            }
            if(line[6] != header_line[6])
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_program_sigs_dont_match]);
            }
            if(StrUni(line[1]) != StrUni(header_line[1]))
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_station_names_dont_match]);
            }
            if(StrUni(line[7]) != StrUni(header_line[7]))
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_table_names_dont_match]);
            }

            // the number of fields and the names of fields should match
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_number_columns_dont_match].c_str());
            for(uint4 i = 0; i < line.size(); ++i)
               if(StrUni(line[i]) != StrUni(header_line[i]))
                  throw std::invalid_argument(my_strings[strid_append_column_names_dont_match].c_str());
            field_names = line;
            
            // check the units declarations
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_units_count_dont_match].c_str());
            if(line.size() != field_names.size())
               throw std::invalid_argument(my_strings[strid_append_invalid_units_count].c_str());
            for(uint4 i = 0; i < line.size(); ++i)
            {
               if(StrUni(line[i]) != StrUni(header_line[i]))
               {
                  OStrAscStream msg;
                  msg << boost::format(my_strings[strid_append_column_units_dont_match].c_str()) % field_names[i];
                  rtn = can_append_with_severe_warnings;
                  append_header_check_warning(reason, msg.str());
               }
            }
            
            // check the process declarations
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_process_count_dont_match].c_str());
            if(line.size() != field_names.size())
               throw std::invalid_argument(my_strings[strid_append_invalid_process_count].c_str());
            for(uint4 i = 0; i < line.size(); ++i)
            {
               if(StrUni(line[i]) != StrUni(header_line[i]))
               {
                  OStrAscStream msg;
                  msg << boost::format(my_strings[strid_append_column_process_dont_match].c_str()) % field_names[i];
                  rtn = can_append_with_severe_warnings;
                  append_header_check_warning(reason, msg.str());
               }
            }
            
            // check the data types
            line.read(in);
            header_line.read(header);
            if(line.size() != header_line.size())
               throw std::invalid_argument(my_strings[strid_append_data_types_count_dont_match].c_str());
            for(uint4 i = 0; i < line.size(); ++i)
               if(line[i] != header_line[i])
                  throw std::invalid_argument(my_strings[strid_append_data_types_dont_match].c_str());
         }
         catch(std::exception &e)
         {
            rtn = can_append_incompatible_headers;
            if(reason)
               *reason = e.what();
         }
         return rtn;
      } // tob1_header_check

      
      int csixml_header_check(
         FILE *in, char const *target_header, uint4 target_header_len, StrAsc *reason)
      {
         int rtn = can_append_success;
         try
         {
            // we need to extract the <header> element from the data file
            StrAsc header_buff;
            extract_csi_xml_header(header_buff, in);
            IBuffStream header_str(header_buff.c_str(), header_buff.length());
            Xml::Element file_header(L"head");
            file_header.input(header_str);

            // we also need to extract the header element from application supplied "header"
            char const head_name[] = "<head";
            size_t head_pos = locate_sub_string(
               target_header, target_header_len, head_name, sizeof(head_name) - 1);
            if(head_pos >= target_header_len)
               throw std::invalid_argument(my_strings[strid_append_invalid_target_header].c_str());
            IBuffStream target_header_str(target_header + head_pos, target_header_len - head_pos);
            Xml::Element target_header_xml(L"head");
            target_header_xml.input(target_header_str);

            // we can now evaluate the differences in environment
            static StrUni const environment_name(L"environment");
            static StrUni const station_name_name("station-name");
            static StrUni const table_name_name(L"table-name");
            static StrUni const model_name(L"model");
            static StrUni const serial_no_name(L"serial-no");
            static StrUni const os_version_name(L"os-version");
            static StrUni const dld_name_name(L"dld-name");
            static StrUni const dld_sig_name(L"dld-sig");
            Xml::Element::value_type file_header_env(file_header.find_elem(environment_name));
            Xml::Element::value_type target_header_env(target_header_xml.find_elem(environment_name));
            Xml::Element::value_type file_station_name(file_header_env->find_elem(station_name_name));
            Xml::Element::value_type target_station_name(target_header_env->find_elem(station_name_name));
            Xml::Element::value_type file_table_name(file_header_env->find_elem(table_name_name));
            Xml::Element::value_type target_table_name(target_header_env->find_elem(table_name_name));
            Xml::Element::iterator file_model_it(file_header_env->find(model_name));
            Xml::Element::iterator target_model_it(target_header_env->find(model_name));
            Xml::Element::iterator file_serial_no_it(file_header_env->find(serial_no_name));
            Xml::Element::iterator target_serial_no_it(target_header_env->find(serial_no_name));
            Xml::Element::iterator file_os_version_it(file_header_env->find(os_version_name));
            Xml::Element::iterator target_os_version_it(target_header_env->find(os_version_name));
            Xml::Element::iterator file_dld_name_it(file_header_env->find(dld_name_name));
            Xml::Element::iterator target_dld_name_it(target_header_env->find(dld_name_name));
            Xml::Element::iterator file_dld_sig_it(file_header_env->find(dld_sig_name));
            Xml::Element::iterator target_dld_sig_it(target_header_env->find(dld_sig_name));
            
            if(file_station_name->get_cdata_wstr() != target_station_name->get_cdata_wstr())
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_station_names_dont_match]);
            }
            if(file_table_name->get_cdata_wstr() != target_table_name->get_cdata_wstr())
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_table_names_dont_match]);
            }
            if(file_model_it != file_header_env->end() && target_model_it != target_header_env->end())
            {
               Xml::Element::value_type &file_model = *file_model_it;
               Xml::Element::value_type &target_model = *target_model_it;
               if(file_model->get_cdata_wstr() != target_model->get_cdata_wstr())
               {
                  rtn = can_append_with_severe_warnings;
                  append_header_check_warning(reason, my_strings[strid_append_model_numbers_dont_match]);
               }
            }
            else if(file_model_it != file_header_env->end() || target_model_it != target_header_env->end())
            {
               rtn = can_append_with_severe_warnings;
               append_header_check_warning(reason, my_strings[strid_append_model_numbers_dont_match]);
            }
            if(file_serial_no_it != file_header_env->end() && target_serial_no_it != target_header_env->end())
            {
               Xml::Element::value_type &file_serial_no = *file_serial_no_it;
               Xml::Element::value_type &target_serial_no = *target_serial_no_it;
               if(file_serial_no->get_cdata_wstr() != target_serial_no->get_cdata_wstr())
               {
                  rtn = can_append_with_warnings;
                  append_header_check_warning(reason, my_strings[strid_append_serial_numbers_dont_match]);
               }
            }
            else if(file_serial_no_it != file_header_env->end() || target_serial_no_it != target_header_env->end())
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_serial_numbers_dont_match]);
            }
            if(file_os_version_it != file_header_env->end() && target_os_version_it != target_header_env->end())
            {
               Xml::Element::value_type &file_os_version = *file_os_version_it;
               Xml::Element::value_type &target_os_version = *target_os_version_it;
               if(file_os_version->get_cdata_wstr() != target_os_version->get_cdata_wstr())
               {
                  rtn = can_append_with_warnings;
                  append_header_check_warning(reason, my_strings[strid_append_os_versions_dont_match]);
               }
            }
            else if(file_os_version_it != file_header_env->end() || target_os_version_it != target_header_env->end())
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_os_versions_dont_match]);
            }
            if(file_dld_name_it != file_header_env->end() && target_dld_name_it != target_header_env->end())
            {
               Xml::Element::value_type &file_dld_name = *file_dld_name_it;
               Xml::Element::value_type &target_dld_name = *target_dld_name_it;
               if(file_dld_name->get_cdata_wstr() != target_dld_name->get_cdata_wstr())
               {
                  rtn = can_append_with_warnings;
                  append_header_check_warning(reason, my_strings[strid_append_program_names_dont_match]);
               }
            }
            else if(file_dld_name_it != file_header_env->end() || target_dld_name_it != target_header_env->end())
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_program_names_dont_match]);
            }
            if(file_dld_sig_it != file_header_env->end() && target_dld_sig_it != target_header_env->end())
            {
               Xml::Element::value_type &file_dld_sig = *file_dld_sig_it;
               Xml::Element::value_type &target_dld_sig = *target_dld_sig_it;
               if(file_dld_sig->get_cdata_wstr() != target_dld_sig->get_cdata_wstr())
               {
                  rtn = can_append_with_warnings;
                  append_header_check_warning(reason, my_strings[strid_append_program_sigs_dont_match]);
               }
            }
            else if(file_dld_sig_it != file_header_env->end() || target_dld_sig_it != target_header_env->end())
            {
               rtn = can_append_with_warnings;
               append_header_check_warning(reason, my_strings[strid_append_program_sigs_dont_match]);
            }

            // we need to evaluate differences in the fields
            static StrUni const fields_name(L"fields");
            static StrUni const field_name_name(L"name");
            static StrUni const field_type_name(L"type");
            static StrUni const field_units_name(L"units");
            static StrUni const field_process_name(L"process");
            Xml::Element::value_type file_fields(file_header.find_elem(fields_name));
            Xml::Element::value_type target_fields(target_header_xml.find_elem(fields_name));
            Xml::Element::iterator file_field_it = file_fields->begin();
            Xml::Element::iterator target_field_it = target_fields->begin();
            
            if(file_fields->size() != target_fields->size())
               throw std::invalid_argument(my_strings[strid_append_number_columns_dont_match].c_str());
            while(file_field_it != file_fields->end() && target_field_it != target_fields->end())
            {
               Xml::Element::value_type &file_field = *file_field_it;
               Xml::Element::value_type &target_field = *target_field_it;
               Xml::Element::attr_iterator file_units_it(file_field->find_attr(field_units_name));
               Xml::Element::attr_iterator target_units_it(target_field->find_attr(field_units_name));
               Xml::Element::attr_iterator file_process_it(file_field->find_attr(field_process_name));
               Xml::Element::attr_iterator target_process_it(target_field->find_attr(field_process_name));
               
               if(file_field->get_attr_wstr(field_name_name) != target_field->get_attr_wstr(field_name_name))
                  throw std::invalid_argument(my_strings[strid_append_column_names_dont_match].c_str());
               if(file_field->get_attr_wstr(field_type_name) != target_field->get_attr_wstr(field_type_name))
                  throw std::invalid_argument(my_strings[strid_append_data_types_dont_match].c_str());
               if(file_units_it != file_field->attr_end() && target_units_it != target_field->attr_end())
               {
                  if(file_units_it->second != target_units_it->second)
                  {
                     OStrAscStream msg;
                     msg << boost::format(my_strings[strid_append_column_units_dont_match].c_str()) %
                        file_field->get_attr_wstr(field_name_name);
                     rtn = can_append_with_severe_warnings;
                     append_header_check_warning(reason, msg.str());
                  }
               }
               else if(file_units_it != file_field->attr_end() || target_units_it != target_field->attr_end())
               {
                  OStrAscStream msg;
                  rtn = can_append_with_severe_warnings;
                  msg << boost::format(my_strings[strid_append_column_units_dont_match].c_str()) %
                     file_field->get_attr_wstr(field_name_name);
                  append_header_check_warning(reason, msg.str());
               }
               if(file_process_it != file_field->attr_end() && target_process_it != target_field->attr_end())
               {
                  if(file_process_it->second != target_process_it->second)
                  {
                     OStrAscStream msg;
                     rtn = can_append_with_severe_warnings;
                     msg << boost::format(my_strings[strid_append_column_process_dont_match].c_str()) %
                        file_field->get_attr_wstr(field_name_name);
                     append_header_check_warning(reason, msg.str());
                  }
               }
               else if(file_process_it != file_field->attr_end() || target_process_it != target_field->attr_end())
               {
                  OStrAscStream msg;
                  rtn = can_append_with_severe_warnings;
                  msg << boost::format(my_strings[strid_append_column_process_dont_match].c_str()) %
                     file_field->get_attr_wstr(field_name_name);
                  append_header_check_warning(reason, msg.str());
               }
               ++file_field_it;
               ++target_field_it;
            }
         }
         catch(std::exception &e)
         {
            rtn = can_append_incompatible_headers;
            if(reason)
               *reason = e.what();
         }
         return rtn;
      } // csixml_header_check
   };

   
   int data_file_can_append(
      char const *file_name,
      int target_type,
      char const *target_header,
      uint4 target_header_len,
      int8 *append_offset,
      StrAsc *reason,
      int8 max_size)
   {
      int rtn = can_append_unknown_error;
      FILE *in = Csi::open_file(file_name, "rb");
      if(in != 0)
      {
         if(max_size <= 0 || long_file_length(in) < max_size)
         {
            int existing_format = get_data_file_type(in, append_offset);
            if(target_type == existing_format)
            {
               // this now comes down to determining if the header formats are compatible.  This
               // decision will depend upon the specific type of file.
               fseek(in, 0, SEEK_SET);
               switch(target_type)
               {
               case data_file_type_toaci1:
                  rtn = toa1_header_check(in, target_header, target_header_len, reason);
                  break;
                  
               case data_file_type_toa5:
                  rtn = toa5_header_check(in, target_header, target_header_len, reason);
                  break;

               case data_file_type_toa6:
                  rtn = toa6_header_check(in, target_header, target_header_len, reason);
                  break;
                  
               case data_file_type_tob1:
                  rtn = tob1_header_check(in, target_header, target_header_len, reason); 
                  break;
                  
               case data_file_type_tob2:
               case data_file_type_tob3:
                  rtn = can_append_not_supported;
                  break;
                  
               case data_file_type_csixml:
                  rtn = csixml_header_check(in, target_header, target_header_len, reason);
                  break;
                  
               case data_file_type_mixed_csv:
               case data_file_type_mixed_printable:
               case data_file_type_mixed_binary:
                  rtn = can_append_success;
                  break;
               }
            }
            else
            {
               rtn = can_append_incompatible_types;
               if(reason != 0)
               {
                  Csi::OStrAscStream temp;
                  temp << boost::format(my_strings[strid_append_formats_dont_match].c_str()) %
                     data_file_type_name(existing_format) % data_file_type_name(target_type);
                  *reason = temp.str();
               }
            }
         }
         else
         {
            rtn = can_append_too_large;
            if(reason != 0)
            {
               Csi::OStrAscStream temp;
               temp << boost::format(my_strings[strid_append_too_large].c_str()) % max_size;
               *reason = temp.str();
            }
         }
         fclose(in);
      }
      else
         rtn = can_append_open_failure;
      return rtn;
   } // data_file_can_append


   int8 search_file_backward(
      FILE *f,
      void const *pattern_,
      uint4 pattern_len)
   {
      byte const *pattern = static_cast<byte const *>(pattern_);
      int8 start_pos = file_tell(f); // original file offset
      char buff[1024];
      uint4 match_count = 0;    // number of characters that have been matched
      int8 seek_pos = start_pos; // position of the last seek
      int8 rtn = 0;
      bool last_buff = false;

      while(match_count < pattern_len && !last_buff)
      {
         // we need to determine where to seek next.  we can then read that buffer full
         uint4 read_len = sizeof(buff);
         if(seek_pos >= sizeof(buff))
            seek_pos = seek_pos - sizeof(buff);
         else
         {
            read_len = static_cast<uint4>(seek_pos);
            seek_pos = 0;
            last_buff = true;
         }
         file_seek(f, seek_pos, SEEK_SET);
         fread(buff, 1, read_len, f);

         // we will now search backward through the buffer we just read
         uint4 i = 0;
         while(match_count < pattern_len && i < read_len)
         {
            // we will use the iterator index and the match_count to determine the locations of the
            // characters that need to be compared
            uint4 current_idx = read_len - i - 1;
            uint4 match_idx = pattern_len - match_count - 1;
            if(buff[current_idx] == pattern[match_idx])
            {
               ++match_count;
               ++i;
               if(match_count >= pattern_len)
               {
                  rtn = seek_pos + current_idx;
                  file_seek(f, rtn, SEEK_SET);
               }
            }
            else if(match_count > 0)
            {
               // we had a partial match.  There are two cases: 1 - the partial match is within the
               // current buffer, or, 2 - the partial match has spanned buffers.  If the partial
               // match is within the current buffer, we have only to adjust i, otherwise, we need
               // to reposition the file pointer
               if(match_count < current_idx)
               {
                  i -= (match_count - 1);
                  match_count = 0;
               }
               else
               {
                  seek_pos = seek_pos + current_idx + match_count;
                  match_count = 0;
                  break;
               }
            }
            else
               ++i;
         }

         // if no match was found and this is the last buffer, we need to position the pointer to
         // the end of the file
         if(match_count < pattern_len && last_buff)
         {
            rtn = long_file_length(f);
            file_seek(f, rtn, SEEK_SET);
         }
      }
      return rtn;
   } // search_file_backward


   bool set_file_time(char const *file_name, Csi::LgrDate const &stamp)
   {
      bool rtn = true;
#ifdef _WIN32
      struct _utimbuf times;
      int rcd;

      times.actime = times.modtime = stamp.to_time_t();
      rcd = ::_utime(file_name, &times);
      if(rcd != 0)
         rtn = false;
#else
      struct utimbuf times;
      int rcd;
      
      times.actime = times.modtime = stamp.to_time_t();
      rcd = ::utime(file_name, &times);
      if(rcd != 0)
         rtn = false;
#endif
      return rtn;
   } // set_file_time


   bool is_demo_app(
      StrAsc const &app_name,
      StrAsc const &version)
   {
      // A Version string value will be placed at 
      // "HKEY_LOCAL_MACHINE\SOFTWARE\Campbell Scientific\<application>"
      // for the most recent registered common app.  When a common app first runs, it will check this version.
      // If myVersion > Version then I am Demo, else I am not demo.
      bool rtn = false;
      RegistryManager reg_man(app_name, version);
      StrAsc reg_version;
      reg_man.read_string(reg_version, "AppVersion", HKEY_LOCAL_MACHINE);

      // we can now build two version objects from the strings
      Csi::VersionNumber my_version(version.c_str());
      Csi::VersionNumber reported_version(reg_version.c_str());

      if(my_version.size() > 2)
         my_version.erase(my_version.begin() + 2, my_version.end());
      if(reported_version.size() > 2)
         reported_version.erase(reported_version.begin() + 2, reported_version.end());
      if(my_version > reported_version)
         rtn = true;
      return rtn;
   } // is_demo_app
   
   
   bool can_demo_run(
      StrAsc &demo_msg,
      StrAsc const &app_name,
      StrAsc const &app_version,
      bool is_product)
   {
#ifdef _DEBUG
      return true;
#endif
#ifdef _WIN32
#if LoggerNet_BuildSpAr
      return true;
#else
      // If determined to be demo, it must find the product it was installed with to determine if it can still run. 
      // A product string value will be placed at "HKEY_LOCAL_MACHINE\SOFTWARE\Campbell Scientific Demo\<Common App Name>\ProductName"
      // Get the  GUID found at HKey_Local_Machine\Software\Campbell Scientific\<ProductName>\id
      // Using this GUID, it must read the following Blowfish encrypted xml stored in the registry 
      // under HKey_Classes_Root\Cora\<Product GUID>.
      //
      // eg. <product name="RTDAQ” version=1.0 LastDate="32589" DaysLeft="29">
      //        name:     name of the Product.
      //	version:  version of Product.
      //	LastDate: last date any application associated with the product was run.
      //	DaysLeft: The number of days left in the demo.  -1 then valid (non demo) install.
      bool rtn = false;

      if(is_demo_app(app_name, app_version))
      {
         RegistryManager reg_man(app_name, app_version);
         OStrAscStream path;
         StrAsc product_name;
         StrAsc product_version;

         if(is_product) //we are a product, don't look in the demo section
         {
            path << "SOFTWARE\\Campbell Scientific\\" << app_name << "\\";
            product_name = app_name;
         }
         else //We are a demo app, so check in the demo section
         {
            path << "SOFTWARE\\Campbell Scientific Demo\\" << app_name << "\\";
            reg_man.read_anywhere_string(product_name,"ProductName",path.str().c_str(),HKEY_LOCAL_MACHINE);
         }

         reg_man.read_anywhere_string(product_version,"ProductVersion",path.str().c_str(),HKEY_LOCAL_MACHINE);
         if(product_name.length())
         {
            StrAsc guid;
            path.str("");
            path << "SOFTWARE\\Campbell Scientific\\" << product_name << "\\";
            reg_man.read_anywhere_string(guid,"id",path.str().c_str(),HKEY_LOCAL_MACHINE);
            if(guid.length())
            {
               path.str("");
               path << "Cora\\";
               StrBin encrypted_settings;
               reg_man.read_anywhere_binary(
                  encrypted_settings,
                  guid.c_str(),
                  path.str().c_str(),
                  HKEY_CLASSES_ROOT);
               if(encrypted_settings.length())
               {
                  // decrypt the settings stored in the key
                  char const blowfish_key[] = "0TOG799A"; 
                  BlowFish blowfish(blowfish_key, sizeof(blowfish_key) - 1);
                  StrBin decrypted_settings;
                  blowfish.decrypt(decrypted_settings, encrypted_settings.getContents(), encrypted_settings.length());

                  // now parse the decrypted data as XML
                  static StrUni const root_name(L"product");
                  static StrUni const product_name_name(L"name");
                  static StrUni const version_name(L"version");
                  static StrUni const last_date_name(L"lastdate");
                  static StrUni const days_left_name(L"daysleft");
                  IBuffStream encrypted_data(decrypted_settings.getContents(), decrypted_settings.length());
                  Xml::Element product_xml(root_name);
                  product_xml.input(encrypted_data);

                  // we can now extract the attributes of the data
                  StrAsc product_name = product_xml.get_attr_str(product_name_name);
                  StrAsc prod_version = product_xml.get_attr_str(version_name);
                  int4 last_date = product_xml.get_attr_int4(last_date_name); //system date/time truncated to days only
                  int4 days_left = product_xml.get_attr_int4(days_left_name);
                  Csi::VersionNumber reported_version(prod_version.c_str());
                  Csi::VersionNumber my_version(product_version.c_str());

                  if(reported_version.size() > 2)
                     reported_version.erase(reported_version.begin() + 2, reported_version.end());
                  if(my_version.size() > 2)
                     my_version.erase(my_version.begin() + 2, my_version.end());
                  if(my_version == reported_version)
                  {
                     if(days_left >= 0)
                     {
                        int4 today = static_cast<int4>(Csi::LgrDate::local().to_variant());
                        if(days_left > 0)
                        {
                           if(last_date != today)
                           {
                              if(today > last_date)
                                 days_left = Csi::csimax(days_left - (today - last_date), 0L);
                              else
                                 days_left -= 1;

                              // Update the XML structure, output it to a string and encrypt it
                              OStrAscStream decrypted_data;
                              StrBin encrypted_write_settings;                              
                              product_xml.set_attr_int4(today, last_date_name);
                              product_xml.set_attr_int4(days_left, days_left_name);
                              product_xml.output(decrypted_data);
                              blowfish.encrypt(encrypted_write_settings,decrypted_data.str().c_str(),decrypted_data.str().length());
                              reg_man.write_anywhere_binary(
                                 encrypted_write_settings,
                                 guid.c_str(),
                                 path.str().c_str(),
                                 HKEY_CLASSES_ROOT);

                              // format the message to be shown to the user
                              OStrAscStream msg;
                              msg << boost::format(my_strings[strid_demo_remaining_days].c_str()) %
                                 days_left % app_name % app_version;
                              demo_msg = msg.str();
                           }

                           rtn = (days_left > 0); //Issue #25691

                           if(!rtn)
                           {
                              OStrAscStream msg;
                              msg << boost::format(my_strings[strid_demo_expired].c_str()) %
                                 app_name % app_version;
                              demo_msg = msg.str();
                           }
                        }
                        else
                        {
                           OStrAscStream msg;
                           msg << boost::format(my_strings[strid_demo_expired].c_str()) %
                              app_name % app_version;
                           demo_msg = msg.str();
                           rtn = false;
                        }
                     }
                     else
                     {
                        //When fully registered, days left is set to -1
                        rtn = true;
                     }
                  }
               }
            }
         }
      }
      else
      {
         rtn = true;
      }

      //If are a demo and can't run and haven't explained why, assume the installation is invalid
      if(rtn == false && demo_msg.length() == 0)
      {
         Csi::VersionNumber my_version(app_version.c_str());
         OStrAscStream version;
         version << my_version[0] << "." << my_version[1];

         OStrAscStream msg;
         msg << boost::format(my_strings[strid_demo_not_installed].c_str()) %
            app_name % version.str();
         demo_msg = msg.str();
      }
      return rtn;
#endif
#else
      return true;
#endif
   } // can_demo_run


   uint2 encrypt_sig(
      StrBin &output, void const *data_, uint4 data_len, uint2 seed)
   {
      int j(seed & 0xFF);
      int i(seed >> 8);
      byte const *data(static_cast<byte const *>(data_));

      output.cut(0);
      for(uint4 k = 0; k < data_len; ++k)
      {
         byte const c = data[k];
         seed = i & 0xFFFF;
         if(i >= 0x80)
            i = ((i << 1) + 1 + j) & 0xFF;
         else
            i = ((i << 1) + j) & 0xFF;
         output.append(static_cast<byte>(i ^ c));
         j = seed;
      }
      return static_cast<uint2>((j << 8) + i);
   } // encrypt_sig


   bool valid_wep_password(StrAsc const &s)
   {
      bool rtn(true);
      int ascii_count = 0;
      int hex_count = 0;
      for(size_t i = 0; rtn && i < s.length(); ++i)
      {
         char ch(s[i]);
         if(ch >= ' ' && ch <= '~')
         {
            if(!isxdigit(ch))
               hex_count = -1;
            ++ascii_count;
         }
         else
            rtn = false;
         if(isxdigit(ch) && hex_count >= 0)
            ++hex_count;
      }
      if(ascii_count != 5 && ascii_count != 13 &&
         hex_count != 10 && hex_count != 26)
         rtn = false;
      return rtn;
   } // valid_wep_password


   void parse_uri_address(
      StrAsc &address_portion,
      uint2 &port_portion,
      StrAsc const &address)
   {
      enum state_type
      {
         state_start,
         state_quoted_address,
         state_quoted_before_port,
         state_quoted_port,
         state_unquoted,
         state_unquoted_port,
         state_unquoted_ipv6,
         state_ipv6_scope,
         state_done
      } state = state_start;
      StrAsc port_buff;
      size_t pos(0);

      address_portion.cut(0);
      port_portion = 0;
      while(pos < address.length() && state != state_done)
      {
         char ch(address[pos]);
         bool advance(true);
         switch(state)
         {
         case state_start:
            if(ch == '[')
               state = state_quoted_address;
            else
            {
               advance = false;
               state = state_unquoted;
            }
            break;

         case state_quoted_address:
            if(ch == ']')
               state = state_quoted_before_port;
            else
               address_portion.append(ch);
            break;

         case state_quoted_before_port:
            if(ch == ':')
               state = state_quoted_port;
            else
               state = state_done;
            break;

         case state_quoted_port:
            if(isdigit(ch))
               port_buff.append(ch);
            else
               state = state_done;
            break;

         case state_unquoted:
            if(ch == ':')
            {
               port_buff.append(ch);
               state = state_unquoted_port;
            }
            else if(!isspace(ch))
               address_portion.append(ch);
            else
               state = state_done;
            break;

         case state_unquoted_port:
            if(isdigit(ch))
               port_buff.append(ch);
            else if(ch == ':')
            {
               address_portion.append(port_buff);
               address_portion.append(ch);
               port_buff.cut(0);
               state = state_unquoted_ipv6;
            }
            else
               state = state_done;
            break;

         case state_unquoted_ipv6:
            if(isxdigit(ch) || ch == ':' || ch == '.')
               address_portion.append(ch);
            else if(ch == '%')
            {
               address_portion.append(ch);
               state = state_ipv6_scope;
            }
            else
               state = state_done;
            break;

         case state_ipv6_scope:
            if(!isspace(ch))
               address_portion.append(ch);
            else
               state = state_done;
            break;
         }
         if(advance)
            ++pos;
      }
      if(port_buff.length() > 0 && port_buff.first() == ':')
         port_buff.cut(0, 1);
      if(port_buff.length() > 0)
         port_portion = static_cast<uint2>(strtoul(port_buff.c_str(), 0, 10));
   } // parse_uri_address


   void unicode_to_utf8(StrAsc &s, uint4 entity)
   {
      // RFC2279 describes the scheme for UTF-8 encoding.  This function implements the code
      // required to convert a unicode character point into the appropriate UTF-8 sequence. 
      if(entity <= 0x0000007f)
         s.append(static_cast<char>(entity));
      else if(entity <= 0x000007FF)
      {
         unsigned long b2 = 0x80 |  (entity & 0x3F);
         unsigned long b1 = 0xC0 | ((entity & 0x7C0) >> 6);
         s.append(static_cast<char>(b1));
         s.append(static_cast<char>(b2));
      }
      else if(entity <= 0x0000FFFF)
      {
         unsigned long b3 = 0x80 |  (entity & 0x003F);
         unsigned long b2 = 0x80 | ((entity & 0x0FC0) >> 6);
         unsigned long b1 = 0xE0 | ((entity & 0xF000) >> 12);
         s.append(static_cast<char>(b1));
         s.append(static_cast<char>(b2));
         s.append(static_cast<char>(b3));
      }
      else if(entity <= 0x0010FFFF)
      {
         unsigned long b4 = 0x80 |  (entity & 0x00003F);
         unsigned long b3 = 0x80 | ((entity & 0x000FC0) >> 6);
         unsigned long b2 = 0x80 | ((entity & 0x03F000) >> 12);
         unsigned long b1 = 0xF0 | ((entity & 0xFC0000) >> 18);
         s.append(static_cast<char>(b1));
         s.append(static_cast<char>(b2));
         s.append(static_cast<char>(b3));
         s.append(static_cast<char>(b4));
      }
      else
      {
         // this is not a valid unicode character so we simply output a question mark as a placeholder
         s.append('?');
      }
   } // unicode_to_utf8


   FILE *open_file(char const *file_name_, char const *mode_)
   {
      StrUni const file_name(file_name_, true);
      StrUni const mode(mode_, true);
      return open_file(file_name, mode);
   } // open_file


   FILE *open_file(StrUni const &file_name, StrUni const &mode)
   {
      FILE *rtn(0);
#ifdef WIN32
      rtn = _wfopen(file_name.c_str(), mode.c_str());
#else
      rtn = fopen(file_name.to_utf8().c_str(), mode.to_utf8().c_str());
#endif
      return rtn;
   } // open_file


   bool is_wow64()
   {
      bool rtn(false);
#ifdef WIN32
      typedef BOOL (WINAPI *function_type)(HANDLE, PBOOL);
      function_type function(
         reinterpret_cast<function_type>(
            ::GetProcAddress(
               ::GetModuleHandleW(L"kernel32"), "IsWow64Process")));
      if(function)
      {
         BOOL rcd;
         function(GetCurrentProcess(), &rcd);
         rtn = (rcd != 0);
      }
#endif
      return rtn;
   } // is_wow64


   StrAsc make_guid()
   {
      StrAsc rtn;
#if BOOST_VERSION >= 104200
         boost::uuids::random_generator uuid_gen;
         boost::uuids::uuid uuid(uuid_gen());
         OStrAscStream temp;
         temp << uuid;
         rtn = temp.str();
#else
         uuid_t uuid;
         char temp[37];
         uuid_generate_random(uuid);
         uuid_unparse(uuid, temp);
         rtn = temp;
#endif      
      return rtn;
   } // make_guid


   namespace
   {
      uint2 crc16tab[256] = {
         0x0000,0x1021,0x2042,0x3063,0x4084,0x50a5,0x60c6,0x70e7,
         0x8108,0x9129,0xa14a,0xb16b,0xc18c,0xd1ad,0xe1ce,0xf1ef,
         0x1231,0x0210,0x3273,0x2252,0x52b5,0x4294,0x72f7,0x62d6,
         0x9339,0x8318,0xb37b,0xa35a,0xd3bd,0xc39c,0xf3ff,0xe3de,
         0x2462,0x3443,0x0420,0x1401,0x64e6,0x74c7,0x44a4,0x5485,
         0xa56a,0xb54b,0x8528,0x9509,0xe5ee,0xf5cf,0xc5ac,0xd58d,
         0x3653,0x2672,0x1611,0x0630,0x76d7,0x66f6,0x5695,0x46b4,
         0xb75b,0xa77a,0x9719,0x8738,0xf7df,0xe7fe,0xd79d,0xc7bc,
         0x48c4,0x58e5,0x6886,0x78a7,0x0840,0x1861,0x2802,0x3823,
         0xc9cc,0xd9ed,0xe98e,0xf9af,0x8948,0x9969,0xa90a,0xb92b,
         0x5af5,0x4ad4,0x7ab7,0x6a96,0x1a71,0x0a50,0x3a33,0x2a12,
         0xdbfd,0xcbdc,0xfbbf,0xeb9e,0x9b79,0x8b58,0xbb3b,0xab1a,
         0x6ca6,0x7c87,0x4ce4,0x5cc5,0x2c22,0x3c03,0x0c60,0x1c41,
         0xedae,0xfd8f,0xcdec,0xddcd,0xad2a,0xbd0b,0x8d68,0x9d49,
         0x7e97,0x6eb6,0x5ed5,0x4ef4,0x3e13,0x2e32,0x1e51,0x0e70,
         0xff9f,0xefbe,0xdfdd,0xcffc,0xbf1b,0xaf3a,0x9f59,0x8f78,
         0x9188,0x81a9,0xb1ca,0xa1eb,0xd10c,0xc12d,0xf14e,0xe16f,
         0x1080,0x00a1,0x30c2,0x20e3,0x5004,0x4025,0x7046,0x6067,
         0x83b9,0x9398,0xa3fb,0xb3da,0xc33d,0xd31c,0xe37f,0xf35e,
         0x02b1,0x1290,0x22f3,0x32d2,0x4235,0x5214,0x6277,0x7256,
         0xb5ea,0xa5cb,0x95a8,0x8589,0xf56e,0xe54f,0xd52c,0xc50d,
         0x34e2,0x24c3,0x14a0,0x0481,0x7466,0x6447,0x5424,0x4405,
         0xa7db,0xb7fa,0x8799,0x97b8,0xe75f,0xf77e,0xc71d,0xd73c,
         0x26d3,0x36f2,0x0691,0x16b0,0x6657,0x7676,0x4615,0x5634,
         0xd94c,0xc96d,0xf90e,0xe92f,0x99c8,0x89e9,0xb98a,0xa9ab,
         0x5844,0x4865,0x7806,0x6827,0x18c0,0x08e1,0x3882,0x28a3,
         0xcb7d,0xdb5c,0xeb3f,0xfb1e,0x8bf9,0x9bd8,0xabbb,0xbb9a,
         0x4a75,0x5a54,0x6a37,0x7a16,0x0af1,0x1ad0,0x2ab3,0x3a92,
         0xfd2e,0xed0f,0xdd6c,0xcd4d,0xbdaa,0xad8b,0x9de8,0x8dc9,
         0x7c26,0x6c07,0x5c64,0x4c45,0x3ca2,0x2c83,0x1ce0,0x0cc1,
         0xef1f,0xff3e,0xcf5d,0xdf7c,0xaf9b,0xbfba,0x8fd9,0x9ff8,
         0x6e17,0x7e36,0x4e55,0x5e74,0x2e93,0x3eb2,0x0ed1,0x1ef0
      };
   };

   
   uint2 crc16(void const *buff, uint4 buff_len, uint2 seed)
   {
      byte const *b(static_cast<byte const *>(buff));
      uint2 rtn(seed);
      for(uint4 i = 0; i < buff_len; ++i)
         rtn = (rtn << 8) ^ crc16tab[((rtn >> 8) ^ b[i]) & 0x00ff];
      return rtn;
   } // crc16


   uint4 checksum(void const *buff, uint4 buff_len, uint4 mask)
   {
      uint4 rtn(0);
      byte const *b(static_cast<byte const *>(buff));
      for(uint4 i = 0; i < buff_len; ++i)
         rtn += b[i];
      return rtn & mask;
   } // checksum


   namespace
   {
      content_encoding_type mime_extensions[] = {
         std::make_pair(StrAsc("htm"), StrAsc("text/html; charset=utf-8")),
         std::make_pair(StrAsc("html"), StrAsc("text/html; charset=utf-8")),
         std::make_pair(StrAsc("jpg"), StrAsc("image/jpeg")),
         std::make_pair(StrAsc("jpeg"), StrAsc("image/jpeg")),
         std::make_pair(StrAsc("jpe"),  StrAsc("image/jpeg")),
         std::make_pair(StrAsc("png"), StrAsc("image/png")),
         std::make_pair(StrAsc("gif"), StrAsc("image/gif")),
         std::make_pair(StrAsc("bmp"), StrAsc("image/bmp")),
         std::make_pair(StrAsc("tif"), StrAsc("image/tiff")),
         std::make_pair(StrAsc("tiff"), StrAsc("image/tiff")),
         std::make_pair(StrAsc("pdf"),  StrAsc("application/pdf")),
         std::make_pair(StrAsc("swf"), StrAsc("application/x-shockwave-flash")),
         std::make_pair(StrAsc("mpd"), StrAsc("application/dash+xml")),
         std::make_pair(StrAsc("mp3"), StrAsc("audio/mpeg")),
         std::make_pair(StrAsc("mpga"), StrAsc("audio/mpeg")),
         std::make_pair(StrAsc("mp2"), StrAsc("audio/mpeg")),
         std::make_pair(StrAsc("kar"), StrAsc("audio/mpeg")),
         std::make_pair(StrAsc("mid"), StrAsc("audio/midi")),
         std::make_pair(StrAsc("midi"), StrAsc("audio/midi")),
         std::make_pair(StrAsc("wav"), StrAsc("audio/x-wav")),
         std::make_pair(StrAsc("ogg"), StrAsc("audio/ogg")),
         std::make_pair(StrAsc("au"), StrAsc("audio/basic")),
         std::make_pair(StrAsc("snd"), StrAsc("audio/basic")),
         std::make_pair(StrAsc("aif"), StrAsc("audio/x-aiff")),
         std::make_pair(StrAsc("aifc"), StrAsc("audio/x-aiff")),
         std::make_pair(StrAsc("aiff"), StrAsc("audio/x-aiff")),
         std::make_pair(StrAsc("xml"), StrAsc("application/xml")),
         std::make_pair(StrAsc("css"), StrAsc("text/css")),
         std::make_pair(StrAsc("js"), StrAsc("application/javascript")),
         std::make_pair(StrAsc("json"), StrAsc("application/json")),
         std::make_pair(StrAsc("ico"), StrAsc("image/vnd.microsoft.icon")),
         std::make_pair(StrAsc("jar"), StrAsc("application/x-java-archive")),
         std::make_pair(StrAsc("jnlp"), StrAsc("application/java-jnlp-file")),
         std::make_pair(StrAsc("class"), StrAsc("application/octet-stream")),
         std::make_pair(StrAsc("mp4"), StrAsc("video/mp4v-es")),
         std::make_pair(StrAsc("mpeg"), StrAsc("video/mp4v-es")),
         std::make_pair(StrAsc("mpg"),  StrAsc("video/mp4v-es")),
         std::make_pair(StrAsc("mpe"),  StrAsc("video/mp4v-es")),
         std::make_pair(StrAsc("qt"),   StrAsc("video/quicktime")),
         std::make_pair(StrAsc("mov"),  StrAsc("video/quicktime")),
         std::make_pair(StrAsc("mxu"),  StrAsc("video/vnd.mpegurl")),
         std::make_pair(StrAsc("flv"),  StrAsc("video/x-flv")),
         std::make_pair(StrAsc("mp4"), StrAsc("video/mp4")),
         std::make_pair(StrAsc("m4v"), StrAsc("video/mp4")),
         std::make_pair(StrAsc("ogv"), StrAsc("video/ogg")),
         std::make_pair(StrAsc("webm"), StrAsc("video/webm")),
         std::make_pair(StrAsc("avi"), StrAsc("video/avi")),
         std::make_pair(StrAsc("txt"),  StrAsc("text/plain")),
         std::make_pair(StrAsc("svg"),  StrAsc("image/svg+xml")),
         std::make_pair(StrAsc("svge"), StrAsc("image/svg+xml")),
         std::make_pair(StrAsc(""), StrAsc(""))
      };
   };


   content_encoding_type resolve_content_type(StrAsc const &file_name)
   {
      content_encoding_type rtn(std::make_pair(StrAsc("application/octet-stream"), StrAsc("")));
      size_t extension_pos(file_name.rfind("."));
      if(extension_pos < file_name.length())
      {
         StrAsc extension;
         file_name.sub(extension, extension_pos + 1, file_name.length());
         if(extension == "gz" || extension == "jgz")
         {
            size_t extension_end_pos(extension_pos);
            rtn.second = "gzip";
            extension_pos = file_name.rfind(".", extension_end_pos - 1);
            if(extension_pos < file_name.length())
               file_name.sub(extension, extension_pos + 1, extension_end_pos - extension_pos - 1);
         }
         for(int i = 0; mime_extensions[i].first.length() > 0; ++i)
         {
            if(mime_extensions[i].first == extension)
            {
               rtn.first = mime_extensions[i].second;
               break;
            }
         }
      }
      return rtn;
   } // resolve_content_type


   StrAsc content_type_ext(StrAsc const &content_type_)
   {
      // there is a chance that the application passed in more than just the content type.  We will
      // trim to just the portion of interest.
      StrAsc content_type(content_type_);
      size_t semicolon_pos(content_type.find(";"));
      if(semicolon_pos < content_type.length())
         content_type.cut(semicolon_pos);

      // we can now search for the content type.
      StrAsc rtn;
      for(int i = 0; rtn.length() == 0 && mime_extensions[i].first.length() > 0; ++i)
      {
         if(mime_extensions[i].second == content_type)
            rtn = mime_extensions[i].first;
      }
      return rtn;
   } // content_type_ext
};
