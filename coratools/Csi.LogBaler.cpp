/* LogBaler.cpp

   Copyright (C) 1998, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 28 January 1998
   Last Change: Wednesday 15 April 2020
   Last Commit: $Date: 2020-04-16 09:32:31 -0600 (Thu, 16 Apr 2020) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "CsiTypeDefs.h"
#include "Csi.LogBaler.h"
#include "Csi.MaxMin.h"
#include "Csi.OsException.h"
#include "trace.h"
#include "Csi.Utils.h"
#include "Csi.FileSystemObject.h"
#include "Csi.StrAscStream.h"
#include <deque>
#include <algorithm>


namespace Csi
{
   namespace
   {
      class file_written_less
      {
      public:
         bool operator() (FileSystemObject const &f1, FileSystemObject const &f2) const
         { return f1.get_last_write_date() < f2.get_last_write_date(); }
      };
   };


   const uint4 LogBaler::minBaleCnt = 1;
   const uint4 LogBaler::minBaleSize = 1024;

   
   LogBaler::LogBaler(char const *path_, char const *fileName_):
      path(path_),
      fileName(fileName_),
      baleSize(1200000),
      baleCnt(10),
      is_enabled(false),
      time_based_baling(false),
      baling_interval(0),
      bale_id(0)
   { createNestedDir(path_); }


   LogBaler::~LogBaler()
   {
      if(scheduler != 0)
      {
         if(bale_id)
            scheduler->cancel(bale_id);
         scheduler.clear();
      }
   } // destructor


   void LogBaler::setBaleParams(uint4 baleSize_, uint4 baleCnt_)
   {
      if(baleSize_ < minBaleSize)
         baleSize = minBaleSize;
      else
         baleSize = baleSize_;
      if(baleCnt_ < minBaleCnt)
         baleCnt = minBaleCnt;
      else
         baleCnt = baleCnt_;
   } // setBaleParams


   void LogBaler::setEnable(bool isEnabled_)
   {
      // if the state is changing from enabled to disabled, release the stream
      is_enabled = isEnabled_;
      if(output != 0 && !is_enabled)
         output.clear();

      // open the stream if the state is changing from disabled to enabled
      if(is_enabled && time_based_baling)
      {
         if(bale_id != 0)
            scheduler->cancel(bale_id);
         bale_id = scheduler->start(
            this,
            0,
            static_cast<uint4>(baling_interval / Csi::LgrDate::nsecPerMSec),
            true);
      }
      if(output == 0 && is_enabled)
         open_output();
   } // setEnable


   void LogBaler::set_time_based_baling(
      bool time_based_baling_,
      int8 baling_interval_msec,
      SharedPtr<OneShot> timer)
   {
      time_based_baling = time_based_baling_;
      if(baling_interval_msec < 1000)
         baling_interval_msec = 1000;
      baling_interval = baling_interval_msec * LgrDate::nsecPerMSec;
      if(scheduler != 0 && bale_id)
         scheduler->cancel(bale_id);
      if(time_based_baling)
      {
         if(scheduler == 0)
         {
            if(timer == 0)
               timer.bind(new OneShot);
            scheduler.bind(new Scheduler(timer));
         }
         if(is_enabled)
            bale_id = scheduler->start(this, 0, static_cast<uint4>(baling_interval_msec));
      }
   } // set_time_based_baling


   void LogBaler::wr(LogRecord const &rec)
   {
      // check on baling and write the record
      if(is_enabled)
      {
         if(output == 0)
            open_output();
         if(output != 0)
         {
            checkBaling(rec.formatReq());
            if(output != 0)
            {
               *output << rec << "\r\n";
               output->flush();
               last_write = LgrDate::system();
            }
         }
      }
   } // wr


   void LogBaler::onScheduledEvent(uint4 id)
   {
      if(id == bale_id)
      {
         last_write = LgrDate::system() - baling_interval - 1;
         checkBaling(0);
      }
   } // onScheduledEvent

   
   void LogBaler::checkBaling(uint4 additional)
   {
      // we assume here that locking issues are already handled. We also assume that the output is
      // enabled.
      //
      // check the size of the log file. If it is within the threshold, we need do nothing
      if(!time_based_baling)
      {
         if(currentLogSize() + additional < baleSize)
            return;
      }
      else
      {
         LgrDate now(LgrDate::system());
         LgrDate interval_begin = now - (now.get_nanoSec() % baling_interval);
         if(last_write >= interval_begin)
            return;
      }

      // if we made it here, it is because a baling operation needs to take place.
      // start by closing the log file
      output.clear();

      // Put together a search pattern. We will replace the replacement marker '$' closest to the end
      // of the string with a wild card character '*'
      StrAsc pattern(fileName);
      size_t pos;

      pos = pattern.findRev("$");
      if(pos < pattern.length())
         pattern[pos] = '*';
      else
      {
         pos = pattern.length();
         pattern += "*";
      }
      
      // we will create a vector of all of the files and then sort it by date.
      FileSystemObject log_dir(path.c_str());
      FileSystemObject::children_type children;

      log_dir.get_children(children,pattern.c_str());
      children.sort(file_written_less());
      
      // if the size of the file list is greater than the size will be, we must delete all of the files
      // that are left
      StrAsc oldest_file_path;
      while(children.size() >= baleCnt)
      {
         oldest_file_path = children.front().get_complete_name();
         children.pop_front();
         if(remove_file(oldest_file_path.c_str()))
         {
            OsException e("Failed to remove oldest file");
            trace("LogBaler::checkBaling: \"%s\"",e.what());
         }
      }
      
      // if one or more files were deleted, the oldest_file_path string will be non-empty. In this
      // case, we will re-use the name of the last file that was deleted. Otherwise, the name will
      // be created from the size of the list.
      bool log_baled = false;
      OStrAscStream  new_path;
      if(oldest_file_path.length() > 0)
      {
         log_baled = true;
         if(!Csi::move_file(workFileName.c_str(), oldest_file_path.c_str()))
         {
            OsException e("Failed to rename the working file");
            trace("LogBaler::checkBaling: \"%s\"",e.what());
            log_baled = false;
         }
      }
      else
      {
         OStrAscStream temp;
         StrAsc new_name(pattern);
         bool file_exists = true;
         uint4 i = 1;
         while(file_exists)
         {
            temp.str("");
            new_path.str("");
            new_path << path;
            if(path.last() != FileSystemObject::dir_separator())
               new_path << FileSystemObject::dir_separator();
            new_name = pattern;
            new_name.cut(pos, 1);
            temp << i;
            new_name.insert(temp.str().c_str(), pos);
            new_path << new_name;
            if(can_use_baled_name(new_path.str()))
               file_exists = false;
            else
               ++i;
         }
         log_baled = true;
         if(!Csi::move_file(workFileName.c_str(), new_path.str().c_str()))
         {
            OsException e("Failed to rename working file");
            trace("LogBaler::checkBaling: \"%s\"",e.what());
            log_baled = false;
         }
      }
      
      // reopen the working log file
      open_output();
      if(log_baled)
         on_log_baled(new_path.c_str());
   } // checkBaling


   void LogBaler::flush()
   {
      if(output != 0)
         output->flush();
   } // flush


   bool LogBaler::can_use_baled_name(StrAsc const &file_name)
   { return !file_exists(file_name.c_str()); }


   uint4 LogBaler::currentLogSize()
   {
      uint4 rtn = static_cast<uint4>(output->tellp());
      return rtn;
   } // currentLogSize


   void LogBaler::open_output()
   {
      try
      {
         // we may need to ensure that the path exists
         if(!file_exists(path.c_str()))
            createNestedDir(path.c_str());
         
         // attempt to open the file name
         workFileName = path;
         if(workFileName.last() != FileSystemObject::dir_separator())
            workFileName.append(FileSystemObject::dir_separator());
         workFileName.append(fileName);
         output.bind(new OFileStream(workFileName.c_str(),std::ios::app));
         if(!(*output))
            output.clear();
         
         // we also need to get the date the file was last written
         FileSystemObject file_info(workFileName.c_str());
         if(file_info.get_is_valid())
            last_write = file_info.get_last_write_date();
      }
      catch(std::exception &)
      {
         output.clear();
      }
   } // open_output 
};
