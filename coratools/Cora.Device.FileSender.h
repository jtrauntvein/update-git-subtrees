/* Cora.Device.FileSender.h

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Pin-Wu Kao, Carl Zmola & Jon Trauntvein
   Date Begun: Monday 11 September, 2000
   Last Change: Tuesday 19 September 2017
   Last Commit: $Date: 2017-09-19 16:53:01 -0600 (Tue, 19 Sep 2017) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_FileSender_h
#define Cora_Device_FileSender_h


#include "Cora.Device.DeviceBase.h"
#include "Cora.Device.Defs.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"
#include <fstream>


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class FileSender;
      //@endgroup
      
      namespace FileSenderHelpers
      {
         /**
          * Defines a base class that defines the interface for an object that provides the
          * FileSender component with the contents of the file to send.
          */
         class send_source_type
         {
         public:
            /**
             * Destructor
             */
            virtual ~send_source_type()
            { }

            /**
             * Called by the file sender when it is ready to transmit the next fragment to the
             * server.  The overloaded versiom should copy at most the specified number of bytes to
             * the provided buffer and return the number of bytes that were copied.
             *
             * @return Must return the number of bytes that were copied to the fragment buffer.
             *
             * @param buff Specifies the base address of the fragment buffer.
             *
             * @param max_fragment_len Specifies the maximum number of bytes that can be copied.
             */
            virtual uint4 get_next_fragment(
               void *buff,
               uint4 max_fragment_len) = 0;

            /**
             * @return Must be overloaded to return true if there is no more data to be sent for the
             * file.
             */
            virtual bool at_end() = 0;
         };
      };
      //@endgroup
      
 
      /**
       * Defines the interface that an application object must implement in order to use the
       * FileSender component.
       */
      class FileSenderClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction has been completed.
          *
          * @param sender Specifies the component reporting this event.
          *
          * @param outcome Specifies a code that identifies the outcome of the server transaction.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_communication_disabled = 2,
            outcome_missing_file_name = 3,
            outcome_invalid_file_name = 4,
            outcome_logger_resource_error = 5,
            outcome_logger_compile_error = 6,
            outcome_communication_failed = 7,
            outcome_logger_permission_denied = 8,
            outcome_invalid_logon = 9,
            outcome_server_connection_failed = 10,
            outcome_invalid_device_name = 11,
            outcome_server_permission_denied = 12,
            outcome_network_locked = 13,
            outcome_logger_root_dir_full = 14,
            outcome_logger_incompatible = 15
         };
         virtual void on_complete(
            FileSender *sender, outcome_type outcome) = 0;

         /**
          * Called from time to time when the server has made progress but not yet completed the
          * transaction.
          *
          * @param sender Specifies the component reporting this event.
          *
          * @param sent_bytes Specifies the number of bytes that have been sent to the datalogger.
          */
         virtual void on_progress(
            FileSender *sender,
            uint4 sent_bytes)
         { }
      };

      
      /**
       * Defines a component that can be used to send a file to a datalogger that supports a file
       * system including the CR1000, CR3000, CR300, CR800, CR5000, CR9000, CR6, and CR1000X
       * dataloggers.
       *
       * In order to use this component, an application must provide a client object that is derived
       * from class FileSenderClient.  This object will receive notifications regarding the progress
       * and/or outcome of the server transaction.  The application must then create an instance of
       * this component, call methods to set the appropriate attributes including set_device_name(),
       * set_logger_file_name(), and set_send_source().  It should then invoke one of the start()
       * methods provided to start the transaction.  When the transaction has been completed, the
       * client object's on_complete() method will be called with a parameter that will indicate the
       * outcome of the transaction.
       */
      class FileSender:
         public DeviceBase,
         public Csi::EvReceiver
      {
      public:
         typedef Csi::SharedPtr<FileSenderHelpers::send_source_type> send_source_handle;
         
      private:
         // @group: attributes

         /**
          * Specifies the name and storage device for the file on the datalogger.
          */
         StrAsc logger_file_name;
         
         /**
          * Specifies the source the file to be sent to the datalogger.  This property must be set
          * each time that the component is started because it will be cleared when the component is
          * stopped.
          */
         send_source_handle send_source;

         /**
          * Set to true if the file represents a program that should be compiled and run once the
          * file has been sent.  The default value is false.
          */
         bool run_program_now;

         /**
          * Set to true of the file is a program that should be marked as the program for the
          * datalogger to run on power up.  The default value is false.
          */
         bool run_program_on_power_up;
         
         // @endgroup:
         
      public:
         /**
          * Constructor
          */
         FileSender();

         /**
          * Destructor
          */
         virtual ~FileSender();

         // @group: properties access methods
         
         /**
          * @param logger_file_name Specifies the name and location of the file to be stored on the
          * datalogger.  The location is separated from the name with a colon and, if omitted, will
          * default to the datalogger's CPU: device.
          */
         void set_logger_file_name(StrAsc const &logger_file_name_);

         /**
          * @return Returns the name and location of the file to be stored on the datalogger.
          */
         StrAsc const &get_logger_file_name() const { return logger_file_name; }

         /**
          * @param send_source_ Specifies the source object for the file to be sent to the
          * datalogger.
          */
         void set_send_source(send_source_handle send_source_);

         /**
          * @param run_program_now_ Set to true if the file is a datalogger program that should be
          * run on the datalogger once the file has been sent.
          */
         void set_run_program_now(bool run_program_now_);

         /**
          * @return Returns true if the file is a datalogger program that should be run when the
          * file has been sent.
          */
         bool get_run_program_now() const
         { return run_program_now; }

         /**
          * @param run_program_on_power_up_ Set to true if the file is a datalogger program that
          * should be run when the datalogger powers up.
          */
         void set_run_program_on_power_up(bool run_program_on_power_up_);

         /**
          * @return Returns true if the file is a datalogger program that should be run when the
          * datalogger powers up.
          */
         bool get_run_program_on_power_up() const
         { return run_program_on_power_up; }

         /**
          * Called to start the file send transaction with the LoggerNet server.
          *
          * @param client Specifies the application object that will receive status notifications.
          *
          * @param router Specifies a messaging router that has not been previously connected.
          *
          * @param other_component Specifies another component that is already connected the
          * LoggerNet server.
          */
         typedef FileSenderClient client_type;
         void start(client_type *client, router_handle &router);
         void start(client_type *client, ClientBase *other_component);
         
         /**
          * Called to cancel the server transaction and return this component into a newly
          * initialised state.
          */
         void finish();

         /**
          * @return Returns the explanation string that the datalogger sent when it reported the
          * file to be incompatible.
          */
         StrAsc const &get_explanation() const
         { return explanation; }

         /**
          * Formats the client outcome code to the specified stream.
          *
          * @param out Specifies the stream to which the description will be written.
          *
          * @param outcome Specifies the outcome code to be described.
          *
          * @param explanation Specifies the datalogger string that was supplied with an
          * incompatible outcome.
          */
         static void describe_outcome(
            std::ostream &out, client_type::outcome_type outcome, StrAsc const &explanation = "");
         
      protected:
         /**
          * Overloads the base class version to handle incoming messages from the LoggerNet server.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, 
            Csi::Messaging::Message *message);

         /**
          * Overloads the base class version to handle a loss of connection to the LoggerNet device
          * object.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);
         
         /**
          * Overloads the base class version to handle a loss of the messaging session to the
          * LoggerNet device.
          */
         virtual void on_devicebase_session_failure();

         /**
          * Overloads the base class version to handle the case where the device session has been
          * established.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the base class version to handle asynchronous events.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         /**
          * Transmits the next file fragment to the LoggerNet server.
          */
         void send_next_fragment();

         /**
          * Handles the send acknowledgement message from the LoggerNet server.
          */
         void on_send_ack(Csi::Messaging::Message *message);

         /**
          * Handles the status notification  message from the LoggerNet server.
          */
         void on_status_not(Csi::Messaging::Message *message);
         
      private:
         /**
          * Specifies the application object that will receive status notifications from this
          * component.
          */
         client_type *client;

         /**
          * Specifies the maximum number of bytes that will be sent in any single command to the
          * LoggerNet server.
          */
         static uint4 const max_fragment_len;

         /**
          * Specifies the current state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_sending_file,
            state_waiting_for_result,
         } state;

         /**
          * Specifies the transaction number for the LoggerNet transaction.
          */
         uint4 send_tran;

         /**
          * Specifies the explanation string that the datalogger sent when it reported the file to
          * be incompatible.
          */
         StrAsc explanation;
      };


      namespace FileSenderHelpers
      {
         /**
          * Defines a send source that will specifically send the contents of a file.
          */
         class file_send_source: public send_source_type
         {
         public:
            /**
             * Constructor
             *
             * @param file_name Specifies the name and path for the file.
             */
            file_send_source(StrAsc const &file_name);

            /**
             * Destructor
             */
            virtual ~file_send_source();

            /**
             * Overloads the base class version to fill in the next fragment.
             */
            virtual uint4 get_next_fragment(void *buffer, uint4 max_fragment_len);

            /**
             * @return Overloads the base class version to return true if we have read past the file
             * end.
             */
            virtual bool at_end();

         private:
            /**
             * Specifies the file handle.
             */
            FILE *file_handle; 
         };
      };
   };
};

#endif
