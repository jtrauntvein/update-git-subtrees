/* Csi.DevConfig.SettingComp.CompSerialNo.cpp

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 08 August 2012
   Last Change: Wednesday 08 August 2012
   Last Commit: $Date: 2012-08-08 11:45:33 -0600 (Wed, 08 Aug 2012) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.CompSerialNo.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         ////////////////////////////////////////////////////////////
         // class CompSerialNo definitions
         ////////////////////////////////////////////////////////////
         void CompSerialNo::output(std::ostream &out, bool translate)
         {
            if(!translate)
               CompScalar<uint4, CompSerialNoDesc>::output(out, translate);
            else
            {
               uint4 value(get_val_uint4());
               if((value & 0xFF000000) != 0)
               {
                  char country(static_cast<char>((value & 0xFF000000) >> 24));
                  value &= 0x00FFFFFF;
                  out << country << value;
               }
               else
                  out << value;
            }
         } // output

         
         void CompSerialNo::output(std::wostream &out, bool translate)
         {
            if(!translate)
               CompScalar<uint4, CompSerialNoDesc>::output(out, translate);
            else
            {
               uint4 value(get_val_uint4());
               if((value & 0xFF000000) != 0)
               {
                  wchar_t country(static_cast<wchar_t>((value & 0xFF000000) >> 24));
                  value &= 0x00FFFFFF;
                  out << country << value;
               }
               else
                  out << value;
            }
         } // output


         void CompSerialNo::input(std::istream &in, bool translate)
         {
            if(!translate)
               CompScalar<uint4, CompSerialNoDesc>::input(in, translate);
            else
            {
               // skip preceding white space
               char ch(' ');
               while(in.get(ch) && isspace(ch))
                  0;
               if(isdigit(ch))
               {
                  in.unget();
                  CompScalar<uint4, CompSerialNoDesc>::input(in, translate);
               }
               else
               {
                  uint4 country(ch);
                  uint4 value;
                  country <<= 24;
                  in >> value;
                  set_val_uint4(country | value);
               }
            }
         } // input
      };
   };
};


