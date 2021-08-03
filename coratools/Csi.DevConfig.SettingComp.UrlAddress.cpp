/* Csi.DevConfig.SettingComp.UrlAddress.cpp

   Copyright (C) 2013, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 21 November 2013
   Last Change: Thursday 21 November 2013
   Last Commit: $Date: 2013-11-25 13:08:56 -0600 (Mon, 25 Nov 2013) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.UrlAddress.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         ////////////////////////////////////////////////////////////
         // class UrlAddressDesc definitions
         ////////////////////////////////////////////////////////////
         CompBase *UrlAddressDesc::make_component(
            SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous)
         { return new UrlAddress(desc); }


         ////////////////////////////////////////////////////////////
         // class UrlAddress definitions
         ////////////////////////////////////////////////////////////
         void UrlAddress::input(std::istream &in, bool translate)
         {
            // skip over any whitespace
            StrAsc token;
            enum state_type
            {
               state_before_token,
               state_in_token,
               state_after_token
            } state = state_before_token;
            char ch;
            while(state != state_after_token && in.get(ch))
            {
               switch(state)
               {
               case state_before_token:
                  if(!isspace(ch))
                  {
                     token.append(ch);
                     state = state_in_token;
                  }
                  break;
                  
               case state_in_token:
                  if(!isspace(ch))
                     token.append(ch);
                  else
                     state = state_after_token;
                  break;
               }
            }
            set_val_str(token, true);
         } // input


         void UrlAddress::set_val_str(StrAsc const &val, bool do_check)
         {
            UrlAddressDesc *temp_desc(static_cast<UrlAddressDesc *>(get_desc().get_rep()));
            if(val.length() < temp_desc->get_min_length())
               throw std::invalid_argument("address is too short");
            if(val.length() > temp_desc->get_max_length())
               throw std::invalid_argument("address is too long");
            value = val;
            set_has_changed(true);
         } // set_val_str
      };
   };
};

