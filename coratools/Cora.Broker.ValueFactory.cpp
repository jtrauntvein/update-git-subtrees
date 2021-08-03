/* Cora.Broker.ValueFactory.cpp

   Copyright (C) 1998, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 25 August 1999
   Last Change: Friday 29 June 2012
   Last Commit: $Date: 2012-07-03 13:15:08 -0600 (Tue, 03 Jul 2012) $ 
   Committed By: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Broker.ValueFactory.h"
#include "Cora.Broker.Value.h"
#include "Cora.Broker.ValueDesc.h"
#include "Cora.Broker.ValueTypes.h"
#include "Csi.Messaging.Message.h"
#include <stdlib.h>
#include <iomanip>


namespace Cora
{
   namespace Broker
   {
      ////////////////////////////////////////////////////////////
      // class ValueFactory definitions
      ////////////////////////////////////////////////////////////
      Csi::SharedPtr<Value> ValueFactory::make_value(Csi::SharedPtr<ValueDesc> &description)
      {
         Csi::SharedPtr<Value> rtn;
         switch(description->data_type)
         {
         case CsiAscii:
            rtn.bind(new ValueTypes::VAscii(description));
            break;

         case CsiBool:
            rtn.bind(new ValueTypes::VBool(description));
            break;

         case CsiBool2:
            rtn.bind(new ValueTypes::VBool2(description));
            break;

         case CsiBool4:
            rtn.bind(new ValueTypes::VBool4(description));
            break;

         case CsiInt1:
            rtn.bind(new ValueTypes::VSignedByte(description));
            break;
            
         case CsiUInt1:
            rtn.bind(new ValueTypes::VByte(description));
            break;

         case CsiInt2:
         case CsiInt2Lsf:
            rtn.bind(new ValueTypes::VInt2(description));
            break;
            
         case CsiUInt2:
         case CsiUInt2Lsf:
            rtn.bind(new ValueTypes::VUInt2(description));
            break;

         case CsiUInt4:
         case CsiUInt4Lsf:
            rtn.bind(new ValueTypes::VUInt4(description));
            break;

         case CsiInt4:
         case CsiInt4Lsf:
            rtn.bind(new ValueTypes::VInt4(description));
            break;

         case CsiFs2:
            rtn.bind(new ValueTypes::VFs2(description));
            break;
            
         case CsiIeee4:
         case CsiIeee4Lsf:
            rtn.bind(new ValueTypes::VFloat(description));
            break;

         case CsiIeee8:
         case CsiIeee8Lsf:
            rtn.bind(new ValueTypes::VDouble(description));
            break;

         case CsiLgrDate:
         case CsiLgrDateLsf:
         case CsiNSec:
         case CsiNSecLsf:
         case CsiUSec:
            rtn.bind(new ValueTypes::VStamp(description));
            break;

         case CsiInt8Lsf:
         case CsiInt8:
            rtn.bind(new ValueTypes::VInt8(description));
            break;

         case CsiBool8:
            rtn.bind(new ValueTypes::VBool8(description));
            break;

         default:
            throw ExcUnsupportedValueType();
            break;
         }
         return rtn;
      } // make_value
   };
};
