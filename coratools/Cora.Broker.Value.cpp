/* Cora.Broker.Value.cpp

   Copyright (C) 1998, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 25 August 1999
   Last Change: Thursday 06 February 2020
   Last Commit: $Date: 2020-02-06 19:41:32 -0600 (Thu, 06 Feb 2020) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.Value.h"
#include "Cora.Broker.ValueDesc.h"


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class Value definitions
      ////////////////////////////////////////////////////////////
      void Value::format_name(std::ostream &out,
                              bool with_subscripts,
                              char const *subscript_prefix,
                              char const *subscript_postfix)
      {
         // This method will be implemented in terms of format_name_ex(). In order to preserve the
         // previous behaviour, the subscript prefix will be specified for the subscript list
         // prefix. Likewise, the subscript postfix will be specified as the subscript list postfix.
         format_name_ex(out,with_subscripts,
                        subscript_prefix,
                        subscript_prefix,
                        subscript_postfix,
                        subscript_postfix);
      } // format_name


      void Value::format_name_ex(
         std::ostream &out,
         bool with_subscripts,
         char const *subscript_list_prefix,
         char const *subscript_prefix,
         char const *subscript_postfix,
         char const *subscript_list_postfix) const
      {
         description->format_name(
            out,
            with_subscripts,
            subscript_list_prefix,
            subscript_prefix,
            subscript_postfix,
            subscript_list_postfix,
            !combined_with_adjacent_values);
      } // format_name_ex


      void Value::format_name_ex(
         std::wostream &out,
         bool with_subscripts,
         wchar_t const *subscript_list_prefix,
         wchar_t const *subscript_prefix,
         wchar_t const *subscript_postfix,
         wchar_t const *subscript_list_postfix) const
      {
         description->format_name(
            out,
            with_subscripts,
            subscript_list_prefix,
            subscript_prefix,
            subscript_postfix,
            subscript_list_postfix,
            !combined_with_adjacent_values);
      } // format_name_ex


      Csi::Json::ValueHandle Value::write_json()
      {
         Csi::OStrAscStream temp;
         format_json(temp);
         return new Csi::Json::String(temp.str());
      } // write_json
      

      CsiDbTypeCode Value::get_type() const
      { return description->data_type; }

      
      StrUni const &Value::get_process_string() const
      { return description->process; }

      
      StrUni const &Value::get_units_string() const
      { return description->units; }
   };
};
