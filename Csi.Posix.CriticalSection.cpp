/* Csi.Posix.CriticalSection.cpp

   Copyright (C) 2005, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 09 August 2005
   Last Change: Friday 30 December 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.CriticalSection.h"


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class CriticalSection definitions
      ////////////////////////////////////////////////////////////
      CriticalSection::CriticalSection()
      {
         pthread_mutexattr_t attr;
         int rcd = pthread_mutexattr_init(&attr);
         if(rcd != 0)
         {
            errno = rcd;
            throw OsException("Mutex Attributes Alloc failure");
         }
         rcd = pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE);
         if(rcd != 0)
         {
            pthread_mutexattr_destroy(&attr);
            errno = rcd;
            throw OsException("Mutex attributes type set failure");
         }
         rcd = pthread_mutex_init(&os_handle,&attr);
         pthread_mutexattr_destroy(&attr);
         if(rcd != 0)
         {
            errno = rcd;
            throw OsException("Mutex Allocation Failure");
         }
      } // constructor


      CriticalSection::~CriticalSection()
      {
         pthread_mutex_destroy(&os_handle);
      } // destructor


      void CriticalSection::lock()
      {
         int rcd = pthread_mutex_lock(&os_handle);
         if(rcd != 0)
         {
            errno = rcd;
            throw OsException("Mutex Lock Failure");
         }
      } // lock


      void CriticalSection::unlock()
      {
         int rcd;
         
         rcd = pthread_mutex_unlock(&os_handle);
         if(rcd != 0)
         {
            errno = rcd;
            throw OsException("Mutex Unlock Failure");
         }
      } // unlock
   };
};


