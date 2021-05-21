/* Csi.Alarms.AlarmLogger.h

   Copyright (C) 2012, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 03 October 2012
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Alarms_AlarmLogger_h
#define Csi_Alarms_AlarmLogger_h

#include "Csi.LogBaler.h"
#include "Csi.StrAscStream.h"
#include "Csi.Xml.Element.h"

namespace Csi
{
   namespace Alarms
   {
      ////////////////////////////////////////////////////////////
      // class AlarmLogEvent
      ////////////////////////////////////////////////////////////
      class AlarmLogEvent: public LogRecord
      {
      private:
         ////////////////////////////////////////////////////////////
         // buffer
         ////////////////////////////////////////////////////////////
         OStrAscStream buffer;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         AlarmLogEvent(Xml::Element &elem)
         {
            elem.output(buffer, true, 1);
         }

         ////////////////////////////////////////////////////////////
         // format
         ////////////////////////////////////////////////////////////
         virtual void format(std::ostream &out) const
         { out << buffer.str(); }

         ////////////////////////////////////////////////////////////
         // formatReq
         ////////////////////////////////////////////////////////////
         virtual uint4 formatReq() const
         { return (uint4)buffer.length(); }
      };


      ////////////////////////////////////////////////////////////
      // class AlarmLogger
      //
      // Defines an object that logs events associated with alarms.  This class
      // subclasses LogBaler and ensures that the alarm-log envelope is
      // formatted each time that the log file is baled or an alarm written.
      ////////////////////////////////////////////////////////////
      class Manager;
      class AlarmLogger: public LogBaler
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         AlarmLogger(
            Manager *manager_,
            char const *path,
            char const *file_name):
            LogBaler(path, file_name),
            manager(manager_)
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~AlarmLogger();

         ////////////////////////////////////////////////////////////
         // wr
         ////////////////////////////////////////////////////////////
         virtual void wr(LogRecord const &rec);

      protected:
         ////////////////////////////////////////////////////////////
         // open_output
         ////////////////////////////////////////////////////////////
         virtual void open_output();

      private:
         ////////////////////////////////////////////////////////////
         // manager
         ////////////////////////////////////////////////////////////
         Manager *manager;
      };
   };
}


#endif
