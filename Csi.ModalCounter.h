/* Csi.ModalCounter.h

   Copyright (C) 2009, 2011 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 05 May 2009
   Last Change: Thursday 06 January 2011
   Last Commit: $Date: 2011-01-06 16:16:16 -0600 (Thu, 06 Jan 2011) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_ModalCounter_h
#define Csi_ModalCounter_h


namespace Csi
{
   ////////////////////////////////////////////////////////////
   // class ModalCounter
   //
   // Used to track the number of modal dialogues that an application may have
   // open.   This class uses a static value to keep a reference count.  
   ////////////////////////////////////////////////////////////
   class ModalCounter
   {
   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      ModalCounter();
      
      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      ~ModalCounter();
      
      ////////////////////////////////////////////////////////////
      // has_modals
      ////////////////////////////////////////////////////////////
      static bool has_modals();
      
      ////////////////////////////////////////////////////////////
      // release
      ////////////////////////////////////////////////////////////
      void release();
      
   private:
      ////////////////////////////////////////////////////////////
      // released
      ////////////////////////////////////////////////////////////
      bool released;
   };
};


#endif
