/* Csi.DevConfig.SettingComp.Factory.cpp

   Copyright (C) 2003, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 17 December 2003
   Last Change: Friday 08 March 2019
   Last Commit: $Date: 2019-03-11 11:39:36 -0600 (Mon, 11 Mar 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.Factory.h"
#include "Csi.DevConfig.SettingComp.CompScalar.h"
#include "Csi.DevConfig.SettingComp.StringComp.h"
#include "Csi.DevConfig.SettingComp.Bitfield.h"
#include "Csi.DevConfig.SettingComp.CompIpAddr.h"
#include "Csi.DevConfig.SettingComp.CompIpAddr6.h"
#include "Csi.DevConfig.SettingComp.TlsKey.h"
#include "Csi.DevConfig.SettingComp.TlsCertificate.h"
#include "Csi.DevConfig.SettingComp.MdFloatArray.h"
#include "Csi.DevConfig.SettingComp.CompSsid.h"
#include "Csi.DevConfig.SettingComp.CompSerialNo.h"
#include "Csi.DevConfig.SettingComp.UrlAddress.h"
#include "Csi.DevConfig.SettingComp.TimeStamp.h"
#include "Csi.DevConfig.SettingComp.CompIpAddrAny.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         ////////////////////////////////////////////////////////////
         // class Factory definitions
         ////////////////////////////////////////////////////////////
         DescBase *Factory::make_component_desc(
            StrUni const &type_name)
         {
            DescBase *rtn = 0;
            if(type_name == L"int1")
               rtn = new CompScalar<int1>::desc_type;
            else if(type_name == L"uint1")
               rtn = new CompScalar<uint1>::desc_type;
            else if(type_name == L"int2")
               rtn = new CompScalar<int2>::desc_type;
            else if(type_name == L"uint2")
               rtn = new CompScalar<uint2>::desc_type;
            else if(type_name == L"int4")
               rtn = new CompScalar<int4>::desc_type;
            else if(type_name == L"uint4")
               rtn = new CompScalar<uint4>::desc_type;
            else if(type_name == L"float")
               rtn = new CompScalar<float>::desc_type;
            else if(type_name == L"double")
               rtn = new CompScalar<double>::desc_type;
            else if(type_name == L"enum") 
               rtn = new CompEnum::desc_type;
            else if(type_name == L"enumi4")
               rtn = new CompEnumI4::desc_type;
            else if(type_name == L"enumi2")
               rtn = new CompEnumI2::desc_type;
            else if(type_name == L"bool")
               rtn = new CompScalar<bool>::desc_type;
            else if(type_name == L"string")
               rtn = new StringComp::desc_type;
            else if(type_name == L"choice")
               rtn = new ChoiceComp::desc_type;
            else if(type_name == L"bitfield")
               rtn = new BitfieldDesc;
            else if(type_name == L"ipaddr")
               rtn = new CompIpAddrDesc;
            else if(type_name == L"tls-private-key")
               rtn = new TlsKeyDesc;
            else if(type_name == L"tls-certificate")
               rtn = new TlsCertificateDesc;
            else if(type_name == L"md-float-array")
               rtn = new MdFloatArrayDesc;
            else if(type_name == L"ssid")
               rtn = new CompSsidDesc;
            else if(type_name == L"ipaddr6")
               rtn = new CompIpAddr6Desc;
            else if(type_name == L"serial-no")
               rtn = new CompSerialNoDesc;
            else if(type_name == L"url-address")
               rtn = new UrlAddressDesc;
            else if(type_name == L"time-stamp")
               rtn = new TimeStampDesc;
            else if(type_name == L"ipaddrany")
               rtn = new CompIpAddrAnyDesc;
            return rtn;
         } // make_component_desc
      };
   };
};
