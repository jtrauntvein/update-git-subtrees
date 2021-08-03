/* Cora.LgrNet.LgrNetSettingTypes.cpp

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 07 September 2000
   Last Change: Monday 23 March 2009
   Last Commit: $Date: 2009-03-23 10:01:46 -0600 (Mon, 23 Mar 2009) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.LgrNetSettingTypes.h"
#include "Csi.Messaging.Message.h"
#include "Csi.CommandLine.h"
#include <sstream>
#include <stdlib.h>


namespace Cora
{
   namespace LgrNet
   {
      ////////////////////////////////////////////////////////////
      // class LogControl definitions
      ////////////////////////////////////////////////////////////
      bool LogControl::read(Csi::Messaging::Message *msg)
      {
         bool rtn = false;
         if(msg->readBool(to_disc) && msg->readUInt4(bale_count) && msg->readUInt4(bale_size))
            rtn = true;
         return rtn;
      } // read

      
      bool LogControl::read(char const *s)
      {
         bool rtn = true;

         try
         {
            Csi::CommandLine cmd;
            StrAsc temp;

            cmd.parse_command_line(s);
            if(cmd.get_argument(temp,0))
            {
               if(temp == "true" || temp == "1")
                  to_disc = true;
               else if(temp == "false" || temp == "0")
                  to_disc = false;
               else
                  throw Csi::MsgExcept("syntax error interpreting to_disc");
            }
            else
               throw Csi::MsgExcept("expected to_disc");

            if(cmd.get_argument(temp,1))
               bale_count = strtoul(temp.c_str(),0,10);
            else
               throw Csi::MsgExcept("expected bale_count");

            if(cmd.get_argument(temp,2))
               bale_size = strtoul(temp.c_str(),0,10);
            else
               throw Csi::MsgExcept("expected bale_size");
         }
         catch(std::exception &)
         { rtn = false; }
         return rtn;
      } // read

      
      void LogControl::write(Csi::Messaging::Message *out) const
      {
         out->addBool(to_disc);
         out->addUInt4(bale_count);
         out->addUInt4(bale_size);
      } // write

      
      void LogControl::format(std::ostream &out) const
      { out << to_disc << ' ' << bale_count << ' ' << bale_size; }


      ////////////////////////////////////////////////////////////
      // class SettingIpManagerKey definitions
      ////////////////////////////////////////////////////////////
      bool SettingIpManagerKey::read(Csi::Messaging::Message *msg)
      {
         bool rtn = msg->readStr(value);
         if(rtn && value.length() != 32)
            rtn = false;
         return rtn;
      } // read


      bool SettingIpManagerKey::read(char const *s)
      {
         // we will skip over the preceding whitespace.  We will then read up to 32 hex digits 
         StrAsc temp;
         bool rtn = false;
         int i = 0;

         while(s[i] != 0 && isspace(s[i]))
            ++i;
         while(s[i] != 0 && temp.length() < 32 && isxdigit(s[i]))
            temp.append(s[i++]);

         // check the terminating conditions
         if(temp.length() == 32 && (s[i] == 0 || isspace(s[i])))
         {
            rtn = true;
            value = temp;
         }
         return rtn;
      } // read (string)


      void SettingIpManagerKey::write(Csi::Messaging::Message *out) const
      { out->addStr(value); }


      void SettingIpManagerKey::format(std::ostream &out) const
      { out << value; }
   };
};
