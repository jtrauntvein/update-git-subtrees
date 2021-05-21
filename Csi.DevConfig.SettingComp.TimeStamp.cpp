/* Csi.DevConfig.SettingComp.TimeStamp.cpp

   Copyright (C) 2014, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 22 May 2014
   Last Change: Thursday 22 May 2014
   Last Commit: $Date: 2014-05-22 13:36:18 -0600 (Thu, 22 May 2014) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.TimeStamp.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         CompBase *TimeStampDesc::make_component(
            SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous)
         {
            return new TimeStamp(desc);
         }


         void TimeStamp::read(SharedPtr<Message> &message)
         {
            TimeStampDesc *temp(static_cast<TimeStampDesc *>(desc.get_rep()));
            int8 seconds = 0, nsec = 0;
            
            if(temp->representation == TimeStampDesc::representation_sec)
            {
               seconds = message->readInt4();
            }
            else if(temp->representation == TimeStampDesc::representation_nsec)
            {
               seconds = message->readInt4();
               nsec = message->readInt4();
            }
            value = LgrDate(seconds * LgrDate::nsecPerSec + nsec);
         } // read


         void TimeStamp::write(SharedPtr<Message> &message)
         {
            TimeStampDesc *temp(static_cast<TimeStampDesc *>(desc.get_rep()));
            if(temp->representation == TimeStampDesc::representation_sec)
            {
               message->addInt4(value.get_sec());
            }
            else if(temp->representation == TimeStampDesc::representation_nsec)
            {
               message->addInt4(value.get_sec());
               message->addUInt4(value.nsec());
            }
         } // write


         void TimeStamp::output(std::ostream &out, bool translate)
         {
            if(translate)
               value.format(out, "%Y-%m-%dT%H:%M:%S%x");
            else
            {
               TimeStampDesc *temp(static_cast<TimeStampDesc *>(desc.get_rep()));
               if(temp->representation == TimeStampDesc::representation_sec)
                  out << value.get_sec();
               else if(temp->representation == TimeStampDesc::representation_nsec)
                  out << value.get_sec() << " " << value.nsec();
            }
         } // output


         void TimeStamp::output(std::wostream &out, bool translate)
         {
            if(translate)
               value.format(out, L"%Y-%m-%dT%H:%M:%S%x");
            else
            {
               TimeStampDesc *temp(static_cast<TimeStampDesc *>(desc.get_rep()));
               if(temp->representation == TimeStampDesc::representation_sec)
                  out << value.get_sec();
               else if(temp->representation == TimeStampDesc::representation_nsec)
                  out << value.get_sec() << L" " << value.nsec();
            }
         } // output


         void TimeStamp::input(std::istream &in, bool translate)
         {
            if(translate)
            {
               StrAsc temp;
               temp.readLine(in);
               value = LgrDate::fromStr(temp.c_str());
               has_changed = true;
            }
            else
            {
               TimeStampDesc *temp(static_cast<TimeStampDesc *>(desc.get_rep()));
               int4 seconds;
               uint4 nsec = 0;
               if(temp->representation == TimeStampDesc::representation_sec)
                  in >> seconds;
               else if(temp->representation == TimeStampDesc::representation_nsec)
                  in >> seconds >> nsec;
               if(in)
               {
                  value = LgrDate(seconds * LgrDate::nsecPerSec + nsec);
                  has_changed = true;
               }
               else
                  throw std::invalid_argument(desc->get_name().c_str());
            }
         } // input
      };
   };
};

