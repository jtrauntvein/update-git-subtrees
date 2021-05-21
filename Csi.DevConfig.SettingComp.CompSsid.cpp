/* Csi.DevConfig.SettingComp.CompSsid.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Thursday 01 March 2012
   Last Change: Thursday 01 March 2012
   Last Commit: $Date: 2012-03-06 15:41:59 -0600 (Tue, 06 Mar 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.CompSsid.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         ////////////////////////////////////////////////////////////
         // class CompSsidDesc definitions
         ////////////////////////////////////////////////////////////
         CompSsidDesc::CompSsidDesc():
            DescBase(Components::comp_ssid)
         {
            pick_id = 0;
            ssid_pos = 0;
            dbm_pos = 0;
            encrypted_pos = 0;
         } // constructor


         void CompSsidDesc::init_from_xml(
            Xml::Element &xml_data, StrAsc const &library_dir)
         {
            static StrUni const pick_name(L"pick");
            static StrUni const ssid_pos_name(L"ssid-pos");
            static StrUni const dbm_pos_name(L"dbm-pos");
            static StrUni const encrypted_pos_name(L"encrypted-pos");
            
            DescBase::init_from_xml(xml_data, library_dir);
            pick_id = xml_data.get_attr_uint2(L"pick");
            ssid_pos = xml_data.get_attr_uint4(L"ssid-pos");
            dbm_pos = xml_data.get_attr_uint4(L"dbm-pos");
            encrypted_pos = xml_data.get_attr_uint4(L"encrypted-pos");
         } // init_from_xml


         CompBase *CompSsidDesc::make_component(
            SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous)
         {
            return new CompSsid(desc);
         }


         ////////////////////////////////////////////////////////////
         // class CompSsid definitions
         ////////////////////////////////////////////////////////////
         void CompSsid::input(std::istream &in, bool translate)
         {
            StrAsc temp;
            char ch;
            while(in.get(ch))
               temp.append(ch);
            while(isspace(temp.last()))
               temp.cut(temp.length() - 1);
            value = temp;
            has_changed = true;
         } // input
      };
   };
};


      
