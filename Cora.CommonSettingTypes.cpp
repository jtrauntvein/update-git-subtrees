/* Cora.CommonSettingTypes.cpp

   Copyright (C) 2000, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 20 July 2000
   Last Change: Wednesday 19 February 2020
   Last Commit: $Date: 2020-02-19 13:39:29 -0600 (Wed, 19 Feb 2020) $ 
   Committed By: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.CommonSettingTypes.h"
#include "Csi.Messaging.Message.h"
#include "Csi.CommandLine.h"
#include "Csi.BuffStream.h"
#include "Csi.StrAscStream.h"
#include <algorithm>
#include <iostream>


namespace Cora
{
   ////////////////////////////////////////////////////////////
   // class SettingBool definitions
   ////////////////////////////////////////////////////////////
   bool SettingBool::read(Csi::Messaging::Message *msg) { return msg->readBool(value); }


   bool SettingBool::read(char const *s)
   {
      bool rtn = true;
      StrAsc temp(s);
      if(temp == "true" || temp == "1")
         value = true;
      else if(temp == "false" || temp == "0")
         value = false;
      else
         rtn = false;
      return rtn;
   } // read


   void SettingBool::write(Csi::Messaging::Message *out) const
   { out->addBool(value); }


   void SettingBool::format(std::ostream &out) const
   { out << value; }


   ////////////////////////////////////////////////////////////
   // class SettingUInt4 definitions
   ////////////////////////////////////////////////////////////

   bool SettingUInt4::read(Csi::Messaging::Message *msg)
   { return msg->readUInt4(value); }


   bool SettingUInt4::read(char const *s)
   {
      value = strtoul(s,0,10);
      return true;
   } // read


   void SettingUInt4::write(Csi::Messaging::Message *out) const
   { out->addUInt4(value); }


   void SettingUInt4::format(std::ostream &out) const
   { out << value; }


   ////////////////////////////////////////////////////////////
   // class SettingByte definitions
   ////////////////////////////////////////////////////////////

   bool SettingByte::read(Csi::Messaging::Message *msg)
   { return msg->readByte(value); }


   bool SettingByte::read(char const *s)
   {
      value = static_cast<byte>(strtoul(s,0,10));
      return true;
   } // read


   void SettingByte::write(Csi::Messaging::Message *out) const
   { out->addByte(value); }


   void SettingByte::format(std::ostream &out) const
   { out << static_cast<uint2>(value); }


   ////////////////////////////////////////////////////////////
   // class SettingUInt2 definitions
   ////////////////////////////////////////////////////////////

   bool SettingUInt2::read(Csi::Messaging::Message *msg)
   { return msg->readUInt2(value); }


   bool SettingUInt2::read(char const *s)
   {
      value = static_cast<uint2>(strtoul(s,0,10));
      return true;
   }


   void SettingUInt2::write(Csi::Messaging::Message *out)  const
   { out->addUInt2(value); }


   void SettingUInt2::format(std::ostream &out)  const
   { out << value; }


   ////////////////////////////////////////////////////////////
   // class SettingInt4 definitions
   ////////////////////////////////////////////////////////////

   bool SettingInt4::read(Csi::Messaging::Message *msg)
   { return msg->readInt4(value); }


   bool SettingInt4::read(char const *s)
   {
      value = strtol(s,0,10);
      return true;
   } // read


   void SettingInt4::write(Csi::Messaging::Message *out)  const
   { out->addInt4(value); }


   void SettingInt4::format(std::ostream &out)  const
   { out << value; }


   ////////////////////////////////////////////////////////////
   // class SettingStrAsc definitions
   ////////////////////////////////////////////////////////////

   bool SettingStrAsc::read(Csi::Messaging::Message *msg)
   {
      StrAsc val;
      bool rtn(msg->readStr(val));
      if(rtn && validate(val))
         value = val;
      else
         rtn = false;
      return rtn;
   } // read (message)


   bool SettingStrAsc::read(char const *s)
   {
      StrAsc val(s);
      bool rtn(validate(val));
      if(rtn)
         value = s;
      return rtn;
   } // read


   void SettingStrAsc::write(Csi::Messaging::Message *out)  const
   { out->addStr(value.c_str()); }


   void SettingStrAsc::format(std::ostream &out)  const
   { out << value; }


   ////////////////////////////////////////////////////////////
   // class SettingAscPassword definitions
   ////////////////////////////////////////////////////////////
   void SettingAscPassword::format(std::ostream &out) const
   {
      if(obscure)
      {
         if(value.length() == 0)
            out << '*';
         for(size_t i = 0; i < value.length(); ++i)
            out << '*';
      }
      else
         SettingStrAsc::format(out);
   } // format


   ////////////////////////////////////////////////////////////
   // class SettingStrAscList definitions
   ////////////////////////////////////////////////////////////
   bool SettingStrAscList::read(Csi::Messaging::Message *in)
   {
      uint4 count;
      bool rtn = in->readUInt4(count);
      strings_type value;
      StrAsc temp;
      
      for(uint4 i = 0; i < count && rtn; ++i)
      {
         rtn = in->readStr(temp);
         if(rtn)
            value.push_back(temp);
      }
      if(rtn)
         strings = value;
      return rtn;
   } // read


   bool SettingStrAscList::read(char const *s)
   {
      Csi::CommandLine cmd;
      bool rtn = true;
      try
      {
         cmd.parse_command_line(s);
         strings.clear();
         strings.insert(strings.begin(), cmd.begin(), cmd.end());
      }
      catch(std::exception &)
      { rtn = false; }
      return rtn;
   } // read
   

   void SettingStrAscList::write(Csi::Messaging::Message *out) const
   {
      out->addUInt4((uint4)strings.size());
      for(const_iterator si = begin(); si != end(); ++si)
         out->addStr(*si);
   } // write


   void SettingStrAscList::format(std::ostream &out) const
   {
      for(const_iterator si = begin(); si != end(); ++si)
         out << "{" << *si << "}" << std::endl;
   } // format

   
   ////////////////////////////////////////////////////////////
   // class SettingStrUni definitions
   ////////////////////////////////////////////////////////////

   bool SettingStrUni::read(Csi::Messaging::Message *msg)
   { return msg->readWStr(value); }


   bool SettingStrUni::read(char const *s)
   {
      value = s;
      return true;
   } // read


   void SettingStrUni::write(Csi::Messaging::Message *out)  const
   { out->addWStr(value.c_str()); }


   void SettingStrUni::format(std::ostream &out)  const
   { out << value; }


   ////////////////////////////////////////////////////////////
   // class SettingNameSet definitions
   ////////////////////////////////////////////////////////////
   SettingNameSet::SettingNameSet(uint4 identifier):
      Setting(identifier)
   { }


   SettingNameSet::~SettingNameSet()
   { }


   bool SettingNameSet::read(Csi::Messaging::Message *in)
   {
      bool rtn;
      uint4 count;

      clear();
      if(in->readUInt4(count))
      {
         StrUni name;

         rtn = true;
         for(uint4 i = 0; i < count && rtn; ++i)
         {
            if(in->readWStr(name))
               push_back(name);
            else
               rtn = false;
         }
      }
      else
         rtn = false;
      return rtn;
   } // read


   bool SettingNameSet::read(char const *s)
   {
      Csi::CommandLine cmd;
      StrAsc temp;
      bool rtn = true;

      try
      {
         clear();
         cmd.parse_command_line(s);
         if(cmd.get_argument(temp,0))
         {
            uint4 count = strtoul(temp.c_str(),0,10);
            for(uint4 i = 0; i < count; ++i)
            {
               if(cmd.get_argument(temp,i + 1))
               {
                  Csi::CommandLine name;
                  name.parse_command_line(temp.c_str());
                  if(name.get_argument(temp,0))
                     push_back(StrUni(temp.c_str()));
                  else
                     throw Csi::MsgExcept("");
               }
               else
                  throw Csi::MsgExcept("Expected a name");
            }
         }
         else
            throw Csi::MsgExcept("Expected the count");
      }
      catch(std::exception &)
      { rtn = false; }
      return rtn;
   } // read


   void SettingNameSet::write(Csi::Messaging::Message *out) const
   {
      out->addUInt4((uint4)size());
      for(const_iterator i = begin(); i != end(); ++i)
         out->addWStr(i->c_str());
   } // write


   void SettingNameSet::format(std::ostream &out) const
   {
      out << size() << std::endl;
      for(const_iterator i = begin(); i != end(); ++i)
         out << '{' << *i << '}' << std::endl;
   } // write


   bool SettingNameSet::has_name(StrUni const &name) const
   { return std::find(begin(),end(),name) != end(); }


   ////////////////////////////////////////////////////////////
   // class SettingAsciiNameSet definitions
   ////////////////////////////////////////////////////////////
   SettingAsciiNameSet::SettingAsciiNameSet(uint4 identifier):
      Setting(identifier)
   { }


   SettingAsciiNameSet::~SettingAsciiNameSet()
   { }


   bool SettingAsciiNameSet::read(Csi::Messaging::Message *in)
   {
      bool rtn = true;
      uint4 values_count = 0;
      StrAsc temp;
      
      clear();
      rtn = in->readUInt4(values_count);
      for(uint4 i = 0; i < values_count && rtn; ++i)
      {
         rtn = in->readStr(temp);
         push_back(temp);
         temp.cut(0);
      }
      return rtn;
   } // read


   bool SettingAsciiNameSet::read(char const *s)
   {
      bool rtn = true;
      Csi::CommandLine cmd;
      StrAsc temp;

      try
      {
         clear();
         cmd.parse_command_line(s);
         if(cmd.get_argument(temp,0))
         {
            uint4 count = strtoul(temp.c_str(), 0, 10);
            for(uint4 i = 0; i < count && rtn; ++i)
            {
               if(cmd.get_argument(temp, i + 1))
               {
                  Csi::CommandLine name;
                  name.parse_command_line(temp.c_str());
                  if(name.get_argument(temp, 0))
                     push_back(temp);
                  else
                     rtn = false;
               }
               else
                  rtn = false;
            }
         }
         else
            rtn = false;
      }
      catch(std::exception &)
      { rtn = false; }
      return rtn;
   } // read (string)


   void SettingAsciiNameSet::write(Csi::Messaging::Message *out) const
   {
      out->addUInt4((uint4)size());
      for(const_iterator si = begin(); si != end(); ++si)
         out->addStr(*si);
   } // write


   void SettingAsciiNameSet::format(std::ostream &out) const
   {
      out << size() << std::endl;
      for(const_iterator si = begin(); si != end(); ++si)
         out << "{" << *si << "}" << std::endl;
   } // format


   ////////////////////////////////////////////////////////////
   // class SettingEnumeration definitions
   ////////////////////////////////////////////////////////////
   SettingEnumeration::SettingEnumeration(uint4 identifier):
      Setting(identifier)
   { }

   
   SettingEnumeration::~SettingEnumeration()
   { }

   
   bool SettingEnumeration::read(Csi::Messaging::Message *in)
   {
      uint4 temp;
      bool rtn = in->readUInt4(temp);
      if(rtn && supported_values.find(temp) != supported_values.end())
         value = temp;
      else
         rtn = false;
      return rtn;
   } // read

   
   bool SettingEnumeration::read(char const *s)
   {
      // we will support either a numerical specification for the value or we will match against one
      // of the supported names. Either way, the number or the name must match one of the entries in
      // the supported values list.
      //
      // We begin by attempting to convert the value to an integer and looking it up in the list of
      // supported values
      uint4 temp = strtoul(s,0,10);
      bool rtn = false;
      
      if(supported_values.find(temp) != supported_values.end())
      {
         value = temp;
         rtn = true;
      }
      for(supported_values_type::iterator svi = supported_values.begin();
          !rtn && svi != supported_values.end();
          ++svi)
      {
         if(svi->second == s)
         {
            rtn = true;
            value = svi->first;
         }
      }
      return rtn;
   } // read
   
   
   void SettingEnumeration::write(Csi::Messaging::Message *out) const
   { out->addUInt4(value); }
   
   
   void SettingEnumeration::format(std::ostream &out) const
   {
      supported_values_type::const_iterator cvi = supported_values.find(value);
      if(cvi != supported_values.end())
         out << cvi->second;
      else
         out << value;
   } // format


   ////////////////////////////////////////////////////////////
   // class TimeSetting definitions
   ////////////////////////////////////////////////////////////
   bool TimeSetting::read(Csi::Messaging::Message *in)
   {
      int8 temp;
      bool rtn = in->readInt8(temp);
      if(rtn)
         time = temp;
      return rtn;
      } // read
   
   
   void TimeSetting::write(Csi::Messaging::Message *out) const
   { out->addInt8(time.get_nanoSec()); }


   ////////////////////////////////////////////////////////////
   // class SettingUInt4Set definitions
   ////////////////////////////////////////////////////////////
   bool SettingUInt4Set::read(char const *s)
   {
      bool rtn(true);
      Csi::CommandLine line;
      clear();
      try
      {
         line.parse_command_line(s);
         for(Csi::CommandLine::args_iterator ai = line.begin(); rtn && ai != line.end(); ++ai)
            insert(strtoul(ai->c_str(), 0, 10));
      }
      catch(std::exception &)
      { rtn = false; }
      return rtn;
   }


   bool SettingUInt4Set::read(Csi::Messaging::Message *message)
   {
      bool rtn(true);
      uint4 values_count(0);
      uint4 value;
      clear();
      rtn = message->readUInt4(values_count);
      for(uint4 i = 0; rtn && i < values_count; ++i)
      {
         rtn = message->readUInt4(value);
         if(rtn)
            insert(value);
      }
      return rtn;
   } // read


   void SettingUInt4Set::write(Csi::Messaging::Message *out) const
   {
      out->addUInt4((uint4)size());
      for(const_iterator si = begin(); si != end(); ++si)
         out->addUInt4(*si);
   } // write


   void SettingUInt4Set::format(std::ostream &out) const
   {
      for(const_iterator si = begin(); si != end(); ++si)
      {
         if(si != begin())
            out << " ";
         out << *si;
      }
   } // format


   ////////////////////////////////////////////////////////////
   // class SettingInt8 definitions
   ////////////////////////////////////////////////////////////
   bool SettingInt8::read(Csi::Messaging::Message *in)
   { return in->readInt8(value); }


   bool SettingInt8::read(char const *s)
   {
      bool rtn(true);
      Csi::IBuffStream temp(s, strlen(s));
      temp.imbue(std::locale::classic());
      temp >> value;
      return rtn;
   } // read


   void SettingInt8::write(Csi::Messaging::Message *out) const
   { out->addInt8(value); }


   void SettingInt8::format(std::ostream &out) const
   { out << value; }


   bool JsonString::read(char const *source)
   {
      bool rtn(true);
      try
      {
         Csi::IBuffStream temp(source, strlen(source));
         content->parse(temp);
      }
      catch(std::exception &)
      { rtn = false; }
      return rtn;
   }


   bool JsonString::read(Csi::Messaging::Message *message)
   {
      StrAsc temp;
      bool rtn(message->readStr(temp));
      if(rtn)
         rtn = read(temp.c_str());
      return rtn;
   } // read


   void JsonString::write(Csi::Messaging::Message *message) const
   {
      Csi::OStrAscStream temp;
      content->format(temp);
      message->addStr(temp.c_str());
   } // write
};
