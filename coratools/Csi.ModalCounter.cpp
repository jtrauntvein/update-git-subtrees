/* Csi.ModalCounter.cpp

   Copyright (C) 2009, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 05 May 2009
   Last Change: Thursday 06 January 2011
   Last Commit: $Date: 2011-01-06 16:16:16 -0600 (Thu, 06 Jan 2011) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.ModalCounter.h"


namespace Csi
{
   namespace
   {
      ////////////////////////////////////////////////////////////
      // instance_count
      ////////////////////////////////////////////////////////////
      int instance_count = 0;
   };

   
   ////////////////////////////////////////////////////////////
   // class ModalCounter definitions
   ////////////////////////////////////////////////////////////
   ModalCounter::ModalCounter():
      released(false)
   { ++instance_count; }
   
   
   ModalCounter::~ModalCounter()
   {
      if(!released)
         --instance_count;
   } // destructor
   
   
   bool ModalCounter::has_modals()
   { return instance_count > 0; }
   
   
   void ModalCounter::release()
   {
      if(!released)
      {
         --instance_count;
         released = true;
      }
   } // release
};
