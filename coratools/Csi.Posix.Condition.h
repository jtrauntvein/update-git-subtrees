/* Csi.Posix.Condition.h

   Copyright (C) 2005, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 14 April 2005
   Last Change: Saturday 31 December 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#ifndef Csi_Posix_Condition_h
#define Csi_Posix_Condition_h

#include "CsiTypeDefs.h"
#include <pthread.h>


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class Condition
      ////////////////////////////////////////////////////////////
      class Condition
      {
      private:
         ////////////////////////////////////////////////////////////
         // signalled
         ////////////////////////////////////////////////////////////
         bool signalled;

         ////////////////////////////////////////////////////////////
         // manual_reset
         ////////////////////////////////////////////////////////////
         bool manual_reset;

         ////////////////////////////////////////////////////////////
         // cond_handle
         ////////////////////////////////////////////////////////////
         pthread_cond_t cond_handle;

         ////////////////////////////////////////////////////////////
         // mutex_handle
         ////////////////////////////////////////////////////////////
         pthread_mutex_t mutex_handle;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         Condition(
            char const *name = 0,
            bool initial_state = false,
            bool manual_reset = true);

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~Condition();

         ////////////////////////////////////////////////////////////
         // set
         ////////////////////////////////////////////////////////////
         void set();

         ////////////////////////////////////////////////////////////
         // reset
         ////////////////////////////////////////////////////////////
         void reset();

         ////////////////////////////////////////////////////////////
         // wait
         ////////////////////////////////////////////////////////////
         bool wait(uint4 timeout = 0xFFFFFFFF);
      };
   };
};


#endif
