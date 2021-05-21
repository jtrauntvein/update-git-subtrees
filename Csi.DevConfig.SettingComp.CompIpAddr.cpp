/* Csi.DevConfig.SettingComp.CompIpAddr.cpp

   Copyright (C) 2005, 2013 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 02 June 2005
   Last Change: Wednesday 12 June 2013
   Last Commit: $Date: 2013-06-12 14:57:06 -0600 (Wed, 12 Jun 2013) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.CompIpAddr.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         ////////////////////////////////////////////////////////////
         // class CompIpAddrDesc definitions
         ////////////////////////////////////////////////////////////
         CompBase *CompIpAddrDesc::make_component(
            SharedPtr<DescBase> &desc,
            SharedPtr<CompBase> &previous_component)
         { return new CompIpAddr(desc); }

         
         ////////////////////////////////////////////////////////////
         // class CompIpAddr definitions
         ////////////////////////////////////////////////////////////
         void CompIpAddr::output(std::ostream &out, bool translate)
         {
            CompIpAddrDesc *temp(static_cast<CompIpAddrDesc *>(get_desc().get_rep()));
            if(translate || temp->get_as_string())
            {
               out << ((value & 0xFF000000) >> 24) << "."
                   << ((value & 0x00FF0000) >> 16) << "."
                   << ((value & 0x0000FF00) >> 8) << "."
                   << ( value & 0x000000FF);
            }
            else
               out << value;
         } // output


         void CompIpAddr::output(std::wostream &out, bool translate)
         {
            CompIpAddrDesc *temp(static_cast<CompIpAddrDesc *>(get_desc().get_rep()));
            if(translate || temp->get_as_string())
            {
               out << ((value & 0xFF000000) >> 24) << L"."
                   << ((value & 0x00FF0000) >> 16) << L"."
                   << ((value & 0x0000FF00) >> 8) << L"."
                   << ( value & 0x000000FF);
            }
            else
               out << value;
         } // output


         void CompIpAddr::input(std::istream &in, bool translate)
         {
            CompIpAddrDesc *temp_desc(static_cast<CompIpAddrDesc *>(get_desc().get_rep()));
            if(!translate && !temp_desc->get_as_string())
            {
               uint4 temp;
               in >> temp;
               if(in)
               {
                  value = temp;
                  has_changed = true;
               }
               else
                  throw std::invalid_argument(desc->get_name().c_str());
            }
            else
            {
               uint4 t1, t2, t3, t4;
               char sep1, sep2, sep3;
               in >> t1 >> sep1 >> t2 >> sep2 >> t3 >> sep3 >> t4;
               if(in)
               {
                  if(t1 >= 0 && t1 <= 255 &&
                     t2 >= 0 && t2 <= 255 &&
                     t3 >= 0 && t3 <= 255 &&
                     t4 >= 0 && t4 <= 255 &&
                     sep1 == '.' && sep2 == '.' && sep3 == '.')
                  {
                     has_changed = true;
                     value = (t1 << 24) + (t2 << 16) + (t3 << 8) + t4;
                  }
                  else
                     throw std::invalid_argument(desc->get_name().c_str());
               }
               else if(in.eof())
                  value = 0;
               else
                  throw std::invalid_argument(desc->get_name().c_str());
            }
         } // input 
      };
   };
};

