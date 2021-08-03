/* Csi.Posix.Condition.cpp

   Copyright (C) 2005, 2005 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 31 May 2005
   Last Change: Saturday 31 December 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.Condition.h"
#include "Csi.OsException.h"
#include "Csi.Utils.h"
#include <sys/time.h>


namespace Csi
{
   namespace Posix
   {
      Condition::Condition(
         char const *name,
         bool initial_state,
         bool manual_reset_):
         signalled(initial_state),
         manual_reset(manual_reset_)
      {
         int rcd = pthread_mutex_init(&mutex_handle,0);
         if(rcd != 0)
            throw OsException("Mutex create failed");
         rcd = pthread_cond_init(&cond_handle,0);
         if(rcd != 0)
         {
            OsException e("condion create failed");
            pthread_mutex_destroy(&mutex_handle);
            throw e;
         }
      } // constructor


      Condition::~Condition()
      {
         pthread_cond_destroy(&cond_handle);
         pthread_mutex_destroy(&mutex_handle);
      } // destructor


      void Condition::set()
      {
         pthread_mutex_lock(&mutex_handle);
         if(!signalled)
         {
            signalled = true;
            pthread_cond_signal(&cond_handle);
         }
         pthread_mutex_unlock(&mutex_handle);
      } // set


      void Condition::reset()
      {
         pthread_mutex_lock(&mutex_handle);
         signalled = false;
         pthread_mutex_unlock(&mutex_handle);
      }


      bool Condition::wait(uint4 timeout)
      {
         // we will lock the condition mutex to begin with
         pthread_mutex_lock(&mutex_handle);
         bool rtn = signalled;
         uint4 counter_base = counter(0);
         while(!rtn && counter(counter_base) < timeout)
         {
            int rcd;
            if(timeout == 0xFFFFFFFF)
            {
               rcd = pthread_cond_wait(
                  &cond_handle,
                  &mutex_handle);
            }
            else
            {
               struct timeval current_time;
               struct timespec time_to_wait;
               gettimeofday(&current_time,0); 
               time_to_wait.tv_sec = current_time.tv_sec + timeout / 1000;
               time_to_wait.tv_nsec = current_time.tv_usec * 1000 + ((timeout % 1000) * 1000000);
               if(time_to_wait.tv_nsec >= 1000000000)
               {
                  ++time_to_wait.tv_sec;
                  time_to_wait.tv_nsec -= 1000000000;
               }
               rcd = pthread_cond_timedwait(
                  &cond_handle,
                  &mutex_handle,
                  &time_to_wait);
            }
            if(rcd == 0)
               rtn = signalled;
            else if(rcd == ETIMEDOUT)
               break;
            else
            {
               pthread_mutex_unlock(&mutex_handle);
               throw OsException("pthread_cond_wait failed");
            }
         }
         if(rtn && !manual_reset)
            signalled = false;
         pthread_mutex_unlock(&mutex_handle);
         return rtn;
      } // wait
   };
};

