/* Cora.Device.FileReceiver.h

   Copyright (C) 2000, 2009 Campbell Scientific, Inc.

   Written by: Pin-Wu Kao, Carl Zmola & Jon Trauntvein
   Date Begun: Monday 11 September 2000
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2012-09-17 14:17:52 -0600 (Mon, 17 Sep 2012) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_FileReceiver_h
#define Cora_Device_FileReceiver_h
 
#include "Cora.Device.DeviceBase.h"
#include "Cora.Device.Defs.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class FileReceiver;
      namespace FileReceiverHelpers
      {
         class receive_sink_type;
      };
      //@endgroup
      
 
      ////////////////////////////////////////////////////////////
      // class FileReceiverClient
      ////////////////////////////////////////////////////////////
      class FileReceiverClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         //
         // Called when the component has completed the file transfer and has made the
         // transition to the standby state.
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_server_logon = 2,
            outcome_invalid_device_name = 3,
            outcome_server_connection_failed = 4,
            outcome_server_permission_denied = 5,
            outcome_communication_failed = 6,
            outcome_communication_disabled = 7,
            outcome_logger_permission_denied = 8,
            outcome_invalid_file_name = 9,
            outcome_unsupported = 10,
         };
         virtual void on_complete(
            FileReceiver *receiver,
            outcome_type complete) = 0;

         
         ////////////////////////////////////////////////////////////
         // on_progress
         //
         // Called when some bytes of the file have been received.
         ////////////////////////////////////////////////////////////
         virtual void on_progress(
            FileReceiver *receiver,
            uint4 total_bytes_received)
         { }
      };

      
      ////////////////////////////////////////////////////////////
      // class FileReceiver
      //
      // Defines a component that reads a file from a datalogger file system using the server's
      // Device File Receive transaction.
      //
      // In order to use this component, the application must provide a client object that is
      // derived from class FileReceiverClient as well as an object derived from
      // FileReceiverHelpers::receive_sink_type.  The application should create the component and
      // set the appropriate attributes including set_device_name() and set_logger_file_name().  It
      // can then call start() to initiate the transfer.
      //
      // As the component carries out the transfer, it will, from time to time, send progress
      // notifications to the client indicating the number of bytes that have been received.  It
      // will also send any bytes received from the server to the supplied sink object.  When the
      // file transfer is complete, the component will call the client's on_complete() notification
      // and place itself back into a standby mode.  It will also close its reference to the sink
      // object. 
      ////////////////////////////////////////////////////////////
      class FileReceiver:
         public DeviceBase,
         public Csi::EventReceiver
      {
      private:
         //@group properties
         ////////////////////////////////////////////////////////////
         // logger_file_name
         //
         // The name of the logger file that should be retrieved including its storage device name.
         // This string should have the following format:
         //
         //   logger_file_name := [ storage_device_name ":" ] file_name.
         ////////////////////////////////////////////////////////////
         StrAsc logger_file_name;
         //@endgroup
         
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         FileReceiver();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~FileReceiver();

         ////////////////////////////////////////////////////////////
         // set_logger_file_name
         ////////////////////////////////////////////////////////////
         void set_logger_file_name(StrAsc const &logger_file_name_);

         ////////////////////////////////////////////////////////////
         // get_logger_file_name
         ////////////////////////////////////////////////////////////
         StrAsc const &get_logger_file_name() const { return logger_file_name; }

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef FileReceiverClient client_type;
         typedef FileReceiverHelpers::receive_sink_type sink_type;
         typedef Csi::SharedPtr<sink_type> sink_handle;
         void start(
            client_type *client_,
            sink_handle sink_,
            router_handle &router);
         void start(
            client_type *client_,
            sink_handle sink_,
            ClientBase *other_component);
         
         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         //  format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);
         
      protected: 
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *router, 
            Csi::Messaging::Message *message);

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure();

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         ////////////////////////////////////////////////////////////
         // on_receive_ack
         ////////////////////////////////////////////////////////////
         void on_receive_ack(Csi::Messaging::Message *message);
         
      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // sink
         ////////////////////////////////////////////////////////////
         sink_handle sink;

         ////////////////////////////////////////////////////////////
         // bytes_received
         ////////////////////////////////////////////////////////////
         uint4 bytes_received;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby, 
            state_delegate,
            state_active,
         } state;

         ////////////////////////////////////////////////////////////
         // receive_buffer
         ////////////////////////////////////////////////////////////
         StrBin receive_buffer;
      };


      namespace FileReceiverHelpers
      {
         ////////////////////////////////////////////////////////////
         // class receive_sink_type
         //
         // Defines an object that is responsible for handling the data received from a file
         // sender. 
         ////////////////////////////////////////////////////////////
         class receive_sink_type
         {
         public:
            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~receive_sink_type() { }

            ////////////////////////////////////////////////////////////
            // receive_next_fragment
            ////////////////////////////////////////////////////////////
            virtual void receive_next_fragment(
               void const *buff,
               uint4 buff_len) = 0;
         };


         ////////////////////////////////////////////////////////////
         // class file_sink_type
         //
         // Defines a type of sink that will save the recieved data to a file.  This class is
         // defined here because this is, by far, the most common way for applications to deal with
         // file receive data.
         ////////////////////////////////////////////////////////////
         class file_sink_type: public receive_sink_type
         {
         public:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            file_sink_type(StrAsc const &file_name);

            ////////////////////////////////////////////////////////////
            // destructor
            ////////////////////////////////////////////////////////////
            virtual ~file_sink_type();

            ////////////////////////////////////////////////////////////
            // receive_next_fragment
            ////////////////////////////////////////////////////////////
            virtual void receive_next_fragment(
               void const *buff,
               uint4 buff_len);

         private:
            ////////////////////////////////////////////////////////////
            // file_handle
            //
            // Reference to the file that the received bytes will be written to.
            ////////////////////////////////////////////////////////////
            FILE *file_handle;
         };
      };
   };
};

#endif
