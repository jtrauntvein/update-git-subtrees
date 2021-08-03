/* Cora.Broker.ValueDesc.cpp

   Copyright (C) 1998, 2018 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 24 August 1999
   Last Change: Wednesday 16 May 2018
   Last Commit: $Date: 2018-05-16 16:20:52 -0600 (Wed, 16 May 2018) $: 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.ValueDesc.h"


namespace Cora
{
   namespace Broker
   {
      ValueDesc::ValueDesc():
         modifying_cmd(0),
         data_type(CsiUnknown)
      { }


      ValueDesc::ValueDesc(ValueDesc const &other)
      { operator =(other); }


      ValueDesc::ValueDesc(StrUni const &name_, CsiDbTypeCode data_type_):
         name(name_),
         data_type(data_type_)
      { }


      ValueDesc::~ValueDesc()
      { }


      ValueDesc &ValueDesc::operator =(ValueDesc const &other)
      {
         name = other.name;
         data_type = other.data_type;
         modifying_cmd = other.modifying_cmd;
         units = other.units;
         process = other.process;
         array_address = other.array_address;
         return *this;
      } // copy operator


      bool ValueDesc::read(
         Csi::Messaging::Message &in,
         bool read_value_description)
      {
         bool rtn;
         uint4 type_code;
         uint4 num_subscripts;
         
         rtn = in.readWStr(name) &&
            in.readUInt4(type_code) &&
            in.readUInt4(modifying_cmd) &&
            in.readWStr(units) &&
            in.readWStr(process);
         if(rtn && read_value_description)
            rtn = in.readWStr(description);
         if(rtn)
            rtn = in.readUInt4(num_subscripts);
         if(rtn)
         {
            if(isCsiDbTypeCode(type_code))
            {
               data_type = static_cast<CsiDbTypeCode>(type_code);
               array_address.clear();
               rtn = true;
               for(uint4 i = 0; i < num_subscripts && rtn; i++)
               {
                  uint4 subscript;
                  if(in.readUInt4(subscript))
                     array_address.push_back(subscript);
                  else
                     rtn = false;
               }
            }
            else
               rtn = false;
         }
         else
            rtn  = false;
         return rtn;
      } // read


      void ValueDesc::format_name(
         std::ostream &out,
         bool with_subscripts,
         char const *subscript_list_prefix,
         char const *subscript_prefix,
         char const *subscript_postfix,
         char const *subscript_list_postfix,
         bool with_last_subscript) const
      {
         out << name;
         if(with_subscripts && !empty())
         {
            if(with_last_subscript || size() > 1)
               out << subscript_list_prefix;
            for(const_iterator si = begin(); si != end(); ++si)
            {
               if(!with_last_subscript && si + 1 == end())
                  break;
               if(si != begin())
                  out << subscript_postfix << subscript_prefix;
               out << *si;
            }
            if(with_last_subscript || size() > 1)
               out << subscript_list_postfix;
         }
      } // format_name

      
      void ValueDesc::format_name(
         std::wostream &out,
         bool with_subscripts,
         wchar_t const *subscript_list_prefix,
         wchar_t const *subscript_prefix,
         wchar_t const *subscript_postfix,
         wchar_t const *subscript_list_postfix,
         bool with_last_subscript) const
      {
         out << name;
         if(with_subscripts && !empty())
         {
            if(with_last_subscript || size() > 1)
               out << subscript_list_prefix;
            for(const_iterator si = begin(); si != end(); ++si)
            {
               if(!with_last_subscript && si + 1 == end())
                  break;
               if(si != begin())
                  out << subscript_postfix << subscript_prefix;
               out << *si;
            }
            if(with_last_subscript || size() > 1)
               out << subscript_list_postfix;
         }
      } // format_name
   };
};
