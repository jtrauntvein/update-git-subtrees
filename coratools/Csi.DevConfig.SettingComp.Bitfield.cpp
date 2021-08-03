/* Csi.DevConfig.SettingComp.Bitfield.cpp

   Copyright (C) 2004, 2012 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 24 January 2004
   Last Change: Friday 31 August 2012
   Last Commit: $Date: 2012-09-04 08:14:09 -0600 (Tue, 04 Sep 2012) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Csi.DevConfig.SettingComp.Bitfield.h"


namespace Csi
{
   namespace DevConfig
   {
      namespace SettingComp
      {
         ////////////////////////////////////////////////////////////
         // class Bitfield definitions
         ////////////////////////////////////////////////////////////
         Bitfield::Bitfield(
            SharedPtr<DescBase> &desc_,
            SharedPtr<CompBase> &previous_component):
            CompBase(desc_),
            desc(desc_),
            shift(0)
         {
            if(previous_component != 0 &&
               previous_component->get_component_type() == Components::comp_bitfield)
            {
               // if the previous component is a bitfield, we will share its buffer
               previous_field = previous_component;
               buffer = previous_field->buffer;
               buffer->increase_overall_size(desc->get_size());
                  
               // we need to increment the shift for all of the previous fields by this fields
               // size
               PolySharedPtr<CompBase, Bitfield> prev(previous_field);
               if(!desc->get_reverse())
               {
                  while(prev != 0)
                  {
                     prev->shift += desc->get_size();
                     prev = prev->previous_field;
                  }
               }
               else
                  shift = prev->desc->get_size() + prev->shift;
            }
            else
               buffer.bind(new BitfieldBuffer(desc->get_size()));
         }
      };
   };
};


