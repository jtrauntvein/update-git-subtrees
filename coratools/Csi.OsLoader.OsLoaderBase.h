/* Csi.OsLoader.OsLoaderBase.h

   Copyright (C) 2004, 2007 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Saturday 13 March 2004
   Last Change: Tuesday 01 May 2007
   Last Commit: $Date: 2009-10-12 10:19:23 -0600 (Mon, 12 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Csi_OsLoader_OsLoaderBase_h
#define Csi_OsLoader_OsLoaderBase_h

#include "Csi.Events.h"
#include "StrAsc.h"


namespace Csi
{
   namespace OsLoader
   {
      //@group class forward declarations
      class OsLoaderBase;
      //@endgroup
      
      
      ////////////////////////////////////////////////////////////
      // class OsLoaderDriver
      //
      // Defines the communication layer that will be used by specific
      // OsLoaderBase derived objects.
      ////////////////////////////////////////////////////////////
      class OsLoaderDriver
      {
      protected:
         ////////////////////////////////////////////////////////////
         // loader
         ////////////////////////////////////////////////////////////
         OsLoaderBase *loader;
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         OsLoaderDriver(OsLoaderBase *loader_):
            loader(loader_)
         { }
         
         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~OsLoaderDriver()
         { }
         
         ////////////////////////////////////////////////////////////
         // send
         ////////////////////////////////////////////////////////////
         virtual void send(
            void const *buff,
            uint4 buff_len) = 0;
      };
      
      
      ////////////////////////////////////////////////////////////
      // class OsLoaderStatusEvent
      ////////////////////////////////////////////////////////////
      class OsLoaderStatusEvent: public Event
      {
      public:
         ////////////////////////////////////////////////////////////
         // event_id
         ////////////////////////////////////////////////////////////
         static uint4 const event_id;

         ////////////////////////////////////////////////////////////
         // status_message
         ////////////////////////////////////////////////////////////
         StrAsc status_message;

         ////////////////////////////////////////////////////////////
         // bytes_sent
         ////////////////////////////////////////////////////////////
         uint4 bytes_sent;

         ////////////////////////////////////////////////////////////
         // bytes_to_send
         ////////////////////////////////////////////////////////////
         uint4 bytes_to_send;

         ////////////////////////////////////////////////////////////
         // loader
         ////////////////////////////////////////////////////////////
         OsLoaderBase *loader;

         ////////////////////////////////////////////////////////////
         // cpost
         ////////////////////////////////////////////////////////////
         static void cpost(
            EventReceiver *client,
            OsLoaderBase *loader,
            StrAsc const &status_message,
            uint4 bytes_sent,
            uint4 bytes_to_send)
         {
            try
            {
               (new OsLoaderStatusEvent(
                  client,
                  loader,
                  status_message,
                  bytes_sent,
                  bytes_to_send))->post();
            }
            catch(Event::BadPost &)
            { }
         }

      private:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         OsLoaderStatusEvent(
            EventReceiver *client,
            OsLoaderBase *loader_,
            StrAsc const &status_message_,
            uint4 bytes_sent_,
            uint4 bytes_to_send_):
            Event(event_id,client),
            loader(loader_),
            status_message(status_message_),
            bytes_sent(bytes_sent_),
            bytes_to_send(bytes_to_send_)
         { }
      };


      ////////////////////////////////////////////////////////////
      // class OsLoaderCompleteEvent
      ////////////////////////////////////////////////////////////
      class OsLoaderCompleteEvent: public Event
      {
      public:
         ////////////////////////////////////////////////////////////
         // event_id
         ////////////////////////////////////////////////////////////
         static uint4 const event_id;

         ////////////////////////////////////////////////////////////
         // loader
         ////////////////////////////////////////////////////////////
         OsLoaderBase *loader;

         ////////////////////////////////////////////////////////////
         // succeeded
         ////////////////////////////////////////////////////////////
         bool succeeded;

         ////////////////////////////////////////////////////////////
         // message
         ////////////////////////////////////////////////////////////
         StrAsc message;

         ////////////////////////////////////////////////////////////
         // display_elapsed_time
         ////////////////////////////////////////////////////////////
         bool display_elapsed_time;

         ////////////////////////////////////////////////////////////
         // cpost
         ////////////////////////////////////////////////////////////
         static void cpost(
            EventReceiver *receiver,
            OsLoaderBase *loader,
            bool succeeded,
            StrAsc const &message,
            bool display_elapsed_time = false)
         {
            try
            {
               (new OsLoaderCompleteEvent(
                  receiver,
                  loader,
                  succeeded,
                  message,
                  display_elapsed_time))->post();
            }
            catch(Event::BadPost &)
            { }
         }

      private:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         OsLoaderCompleteEvent(
            EventReceiver *receiver,
            OsLoaderBase *loader_,
            bool succeeded_,
            StrAsc const &message_,
            bool display_elapsed_time_):
            Event(event_id,receiver),
            loader(loader_),
            succeeded(succeeded_),
            message(message_),
            display_elapsed_time(display_elapsed_time_)
         { }
      };

   
      ////////////////////////////////////////////////////////////
      // class OsLoaderBase
      //
      // Defines a base class for objects that implement specific operating
      // system download protocols.  The main purpose for this class is to
      // define the relationship between an operating system loader and its
      // driver.  Protocol specific issues must be taken care of in the derived
      // classes.
      ////////////////////////////////////////////////////////////
      class OsLoaderBase: public Csi::InstanceValidator
      {
      protected:
         ////////////////////////////////////////////////////////////
         // driver
         //
         // Specifies the object that will carry out the low level
         // communications.
         ////////////////////////////////////////////////////////////
         SharedPtr<OsLoaderDriver> driver;

         ////////////////////////////////////////////////////////////
         // client
         //
         // Specifies the object that will receive events that describe the
         // progress of the OS send process.
         ////////////////////////////////////////////////////////////
         EventReceiver *client;

         ////////////////////////////////////////////////////////////
         // file_extension
         ////////////////////////////////////////////////////////////
         StrAsc file_extension;

         ////////////////////////////////////////////////////////////
         // admonitions
         ////////////////////////////////////////////////////////////
         StrAsc admonition;

      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         OsLoaderBase()
         { }

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~OsLoaderBase()
         { driver.clear(); }

         ////////////////////////////////////////////////////////////
         // get_file_extension
         //
         // Must be overloaded to return the expected extension for the file
         // name.
         ////////////////////////////////////////////////////////////
         virtual char const *get_file_extension()
         { return file_extension.c_str(); }

         ////////////////////////////////////////////////////////////
         // set_file_extension
         ////////////////////////////////////////////////////////////
         virtual void set_file_extension(StrAsc const &file_extension_)
         { file_extension = file_extension_; }

         ////////////////////////////////////////////////////////////
         // get_extra_admonition
         ////////////////////////////////////////////////////////////
         virtual char const *get_extra_admonition()
         { return admonition.c_str(); }

         ////////////////////////////////////////////////////////////
         // set_extra_admonition
         ////////////////////////////////////////////////////////////
         virtual void set_extra_admonition(StrAsc const &admonition_)
         { admonition = admonition_; }

         ////////////////////////////////////////////////////////////
         // open_and_validate
         //
         // Pure virtual method that must be overloaded to open the specified
         // file and validate its contents.
         ////////////////////////////////////////////////////////////
         virtual void open_and_validate(
            char const *file_name) = 0;

         ////////////////////////////////////////////////////////////
         // start_send
         //
         // Must be overloaded to initiate the process of sending the operating
         // system to the device.
         ////////////////////////////////////////////////////////////
         typedef SharedPtr<OsLoaderDriver> driver_handle;
         virtual void start_send(
            driver_handle driver_,
            EventReceiver *client_)
         {
            driver = driver_;
            client = client_;
         }

         ////////////////////////////////////////////////////////////
         // cancel_send
         //
         // Initiates the process of cancelling the send.  on_complete() will
         // be called when the operation has been cancelled.
         ////////////////////////////////////////////////////////////
         virtual void cancel_send() = 0;

         //@group driver event notification methods
         ////////////////////////////////////////////////////////////
         // on_receive
         ////////////////////////////////////////////////////////////
         virtual void on_receive(
            OsLoaderDriver *driver,
            void const *buff,
            uint4 buff_len) = 0;

         ////////////////////////////////////////////////////////////
         // on_driver_error
         //
         // Handles the event when a low level failure occurs in the driver
         // layer.  The default implementation will map into a call into the
         // on_complete() method.
         ////////////////////////////////////////////////////////////
         virtual void on_driver_error(
            OsLoaderDriver *driver,
            char const *error_message);
         //@endgroup

         ////////////////////////////////////////////////////////////
         // set_client
         ////////////////////////////////////////////////////////////
         void set_client(EventReceiver *client_)
         { client = client_; }

      protected:
         ////////////////////////////////////////////////////////////
         // on_status
         //
         // Provided as a convenience method to post status events to the
         // client.  This default implementation can be overridden in a derived
         // class ifthat is needed in the application.
         ////////////////////////////////////////////////////////////
         virtual void on_status(
            char const *status_message,
            uint4 bytes_sent,
            uint4 bytes_to_send);

         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Provided as a convenience method to post a completion event to the
         // client.
         ////////////////////////////////////////////////////////////
         virtual void on_complete(
            char const *message,
            bool succeeded,
            bool display_elapsed_time = false);
      };
   };
};


#endif
