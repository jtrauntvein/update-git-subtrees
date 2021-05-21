/* Csi.OsLoader.DevconfigLoader.h

   Copyright (C) 2008, 2017 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 30 April 2008
   Last Change: Tuesday 19 September 2017
   Last Commit: $Date: 2017-09-19 10:49:29 -0600 (Tue, 19 Sep 2017) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_OsLoader_DevconfigLoader_h
#define Csi_OsLoader_DevconfigLoader_h

#include "Csi.OsLoader.OsLoaderBase.h"
#include "Csi.DevConfig.FileSender.h"
#include "Csi.DevConfig.Session.h"
#include "Csi.PolySharedPtr.h"
#include "Csi.ReadFileMapping.h"


namespace Csi
{
   namespace OsLoader
   {
      namespace DevconfigLoaderHelpers
      {
         class MyDriver;
      };


      /**
       * Defines an OS loader object that uses the devconfug file send protocol to send the
       * operating system.
       */
      class DevconfigLoader:
         public OsLoaderBase,
         public Csi::DevConfig::FileSender
      {
      public:
         /**
          * Constructor
          *
          * @param timer_ Specifies the one shot object that will handle timeouts.
          */
         typedef Csi::SharedPtr<OneShot> timer_handle;
         DevconfigLoader(timer_handle timer_);

         /**
          * Destructor
          */
         virtual ~DevconfigLoader();

         /**
          * @return Overloaded to return the expected file extension pattern.
          */
         char const *get_file_extension()
         {
            char const *rtn(OsLoaderBase::get_file_extension());
            if(*rtn == 0)
               rtn = "OBJ Files (*.obj)|*.obj";
            return rtn;
         } 

         /**
          * Overloaded to open and validate the contents of the file to be sent.
          */
         virtual void open_and_validate(char const *os_file_name);

         /**
          * Overloaded to start the process of sending the file.
          */
         virtual void start_send(driver_handle os_driver, EventReceiver *client);

         /**
          * Overloaded to cancel the operation.
          */
         virtual void cancel_send();

         /**
          * Overloaded to handle an incoming block of data.
          */
         virtual void on_receive(
            OsLoaderDriver *driver, void const *buff, uint4 buff_len);

         /**
          * Overloaded to handle a driver error report.
          */
         virtual void on_driver_error(
            OsLoaderDriver *driver, char const *error_message);

      protected:
         /**
          * Overloaded from  the devconfig file sender to fill the next buffer to be sent.
          */
         virtual void get_next_fragment(
            StrBin &send_buffer,
            bool &last_fragment,
            uint4 max_len,
            uint4 offset);

         /**
          * Overloaded from the devconfig file sender to handle the completion event.
          */
         virtual void on_complete(
            outcome_type outcome,
            bool will_reset,
            uint2 wait_after,
            StrAsc const &explanation = "");

         // @group Methods overloaded from class TransactionClient

         /**
          * Overloaded to handle the completion of a devconfig transaction.
          */
         virtual void on_complete(
            message_handle &command, message_handle &response);

         /**
          * Overloaded to handle the failure of the devconfig transaction.
          */
         virtual void on_failure(
            message_handle &command, failure_type failure);
         
         // @endgroup
         
      protected:
         /**
          * Specifies the one shot timer that is used for generating timeouts.
          */
         timer_handle timer;

         /**
          * Specifies the name of the operating system file.
          */
         StrAsc os_file_name;

         /**
          * Keeps track of the number of bytes that have been sent.
          */
         uint4 bytes_sent;

         /**
          * Keeps track of the number of bytes that need to be sent.
          */
         uint4 bytes_to_send;

         /**
          * Specifies the driver used for communications.
          */
         Csi::PolySharedPtr<Csi::DevConfig::SessionDriverBase, DevconfigLoaderHelpers::MyDriver> driver;

         /**
          * Specifies the devconfig session.
          */
         Csi::PolySharedPtr<Csi::DevConfig::SessionBase, Csi::DevConfig::Session> session;

         /**
          * Specifies the shared memory to the operating system file.
          */
         Csi::SharedPtr<Csi::ReadFileMapping> os_file;

         /**
          * Specifies the pointer to the shared memory for the OS file.
          */
         byte const *os_image;

         /**
          * Specifies the current state of this sender.
          */
         enum state_type
         {
            state_standby,
            state_reset,
            state_send_file
         } state;
      };
   };
};


#endif
