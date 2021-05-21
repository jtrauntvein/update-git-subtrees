/* Csi.Posix.Mutex.cpp

   Copyright (C) 2005, Campbell Scientific, Inc

   Written by: Jon Trauntvein
   Date Begun: Wednesday 30 March 2005
   Last Change: Wednesday 30 March 2005
   Last Commit: $Date: 2007-02-01 14:58:26 -0600 (Thu, 01 Feb 2007) $ (UTC)
   Last Changed by: $Author: tmecham $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Csi.Posix.Mutex.h"
#include <fcntl.h>
#include <sys/stat.h>


namespace Csi
{
   namespace Posix
   {
      ////////////////////////////////////////////////////////////
      // class Mutex definitions
      ////////////////////////////////////////////////////////////
      Mutex::Mutex(char const *name):
         sem_handle(0),
         is_locked(false)
      {
         // if the name if non empty or non-null, we will use a posix semaphore to do what is
         // needed.  Otherwise, we will use a pthread mutex object
         if(name != 0 && name[0] != 0)
         {
            sem_handle = sem_open(
               name,
               O_CREAT,         // create, if needed
               S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH,       
               1);              // initial value is one, meaning that the mutex is unlocked
            if(sem_handle == 0)
               throw OsException("Sempahore allocation failure");
         }
         else
         {
            int rcd = pthread_mutex_init(&mutex_handle,0);
            if(rcd != 0)
            {
               errno = rcd;
               throw OsException("Mutex allocation failure");
            }
         }
         trace("Csi::Posix::Mutex constructor -- %p",this);
      } // constructor


      Mutex::~Mutex()
      {
         trace("Csi::Posix::Mutex destructor -- %p",this);
         if(is_locked)
            release();
         if(sem_handle)
         {
            sem_close(sem_handle);
            sem_handle = 0;
         }
         else
            pthread_mutex_destroy(&mutex_handle);
      } // destructor


      void Mutex::lock()
      {
         if(sem_handle)
         {
            if(sem_wait(sem_handle) == 0)
               is_locked = true;
         }
         else
         {
            int rcd = pthread_mutex_lock(&mutex_handle);
            if(rcd == 0)
               is_locked = true;
         }
      } // lock


      bool Mutex::try_lock()
      {
         bool rtn = false;
         if(sem_handle)
         {
            if(sem_trywait(sem_handle) == 0)
               is_locked = rtn = true; 
         }
         else
         {
            int rcd = pthread_mutex_trylock(&mutex_handle);
            if(rcd == 0)
               is_locked = rtn = true;
         }
         return rtn;
      } // try_lock


      void Mutex::release()
      {
         if(sem_handle)
         {
            if(sem_post(sem_handle) == 0)
               is_locked = false;
         }
         else
         {
            if(pthread_mutex_unlock(&mutex_handle) == 0)
               is_locked = false;
         }
      } // release
   };
};

