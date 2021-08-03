/* Csi.Mutex.h

   Copyright (C) 2005, 2010 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 30 March 2005
   Last Change: Thursday 16 December 2010
   Last Commit: $Date: 2010-12-17 17:56:53 -0600 (Fri, 17 Dec 2010) $ 
   Last Changed by: $Author: jon $

*/

#ifndef Csi_Mutex_h
#define Csi_Mutex_h

#ifdef _WIN32
#include "Csi.Win32.Mutex.h"
#else
#include "Csi.Posix.Mutex.h"
#endif


namespace Csi
{
#ifdef _WIN32
   using Win32::Mutex;
#else
   using Posix::Mutex;
#endif


   ////////////////////////////////////////////////////////////
   // class MutexLocker
   //
   // Defines an object that will lock a mutex in its contructor and release
   // that mutex in its destructor.
   ////////////////////////////////////////////////////////////
   class MutexLocker
   {
   private:
      ////////////////////////////////////////////////////////////
      // mutex
      ////////////////////////////////////////////////////////////
      Mutex *mutex;

   public:
      ////////////////////////////////////////////////////////////
      // constructor
      ////////////////////////////////////////////////////////////
      MutexLocker(Mutex *mutex_):
         mutex(mutex_)
      { mutex->lock(); }

      ////////////////////////////////////////////////////////////
      // destructor
      ////////////////////////////////////////////////////////////
      ~MutexLocker()
      { mutex->release(); }
   };
};


#endif
