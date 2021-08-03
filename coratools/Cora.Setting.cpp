/* Cora.Setting.cpp

   Copyright (C) 2019, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Saturday 19 October 2019
   Last Change: Saturday 19 October 2019
   Last Commit: $Date: 2019-10-21 11:46:53 -0600 (Mon, 21 Oct 2019) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Setting.h"
#include "coratools.strings.h"
#include <ostream>


namespace Cora
{
   void Setting::format_set_outcome(std::ostream &out, set_outcome_type outcome)
   {
      using namespace SettingStrings;
      switch(outcome)
      {
      case outcome_no_attempt_made:
         out << my_strings[strid_outcome_no_attempt_made];
         break;
         
      case outcome_set:
         out << my_strings[strid_outcome_set];
         break;
         
      case outcome_read_only:
         out << my_strings[strid_outcome_read_only];
         break;
         
      case outcome_setting_locked:
         out << my_strings[strid_outcome_locked];
         break;
         
      case outcome_invalid_value:
         out << my_strings[strid_outcome_invalid_value];
         break;
         
      case outcome_unsupported:
         out << my_strings[strid_outcome_unsupported];
         break;
         
      case outcome_network_locked:
         out << my_strings[strid_outcome_network_locked];
         break;

      default:
         out << "unknown outcome";
         break;
      }
   } // format_set_outcome
};
