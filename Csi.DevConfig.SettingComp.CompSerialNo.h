/* Csi.DevConfig.SettingComp.CompSerialNo.h

   Copyright (C) 2012, Campbell Scientific, Inc

   Written by: Jon Trauntvein 
   Date Begun: Wednesday 08 August 2012
   Last Change: Wednesday 08 August 2012
   Last Commit: $Date: 2012-08-08 11:45:33 -0600 (Wed, 08 Aug 2012) $
   Last Changed by: $Author: jon $

*/

#ifndef Csi_DevConfig_SettingComp_CompSerialNo_h
#define Csi_DevConfig_SettingComp_CompSerialNo_h

#include "Csi.DevConfig.SettingComp.CompScalar.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         ////////////////////////////////////////////////////////////
         // class CompSerialNoDesc
         ////////////////////////////////////////////////////////////
         class CompSerialNoDesc: public CompScalarDesc<uint4>
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            CompSerialNoDesc():
               CompScalarDesc<uint4>(Components::comp_serial_no)
            { }

            ////////////////////////////////////////////////////////////
            // make_component
            ////////////////////////////////////////////////////////////
            virtual CompBase *make_component(
               SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous);
         };


         ////////////////////////////////////////////////////////////
         // class CompSerialNo
         ////////////////////////////////////////////////////////////
         class CompSerialNo: public CompScalar<uint4, CompSerialNoDesc>
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            CompSerialNo(SharedPtr<DescBase> &desc):
               CompScalar<uint4, CompSerialNoDesc>(desc)
            { }

            ////////////////////////////////////////////////////////////
            // output
            ////////////////////////////////////////////////////////////
            virtual void output(std::ostream &out, bool translate);
            virtual void output(std::wostream &out, bool translate);

            ////////////////////////////////////////////////////////////
            // input
            ////////////////////////////////////////////////////////////
            virtual void input(std::istream &in, bool translate);
         };


         inline CompBase *CompSerialNoDesc::make_component(
            SharedPtr<DescBase> &desc, SharedPtr<CompBase> &previous)
         {
            return new CompSerialNo(desc);
         }
      };
   };
};


#endif

