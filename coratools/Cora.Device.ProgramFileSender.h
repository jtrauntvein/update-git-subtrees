/* Cora.Device.ProgramFileSender.h

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Pin-Wu Kao, revised by Jon Trauntvein
   Date Begun: Tuesday 29 August 2000
   Last Change: Tuesday 19 September 2017
   Last Commit: $Date: 2017-09-19 16:53:01 -0600 (Tue, 19 Sep 2017) $ 
   Committed by: $Author: jon $
   
*/


#ifndef Cora_Device_ProgramFileSender_h
#define Cora_Device_ProgramFileSender_h


#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include <stdio.h>


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class ProgramFileSender;
      //@endgroup


      /**
       * Defines the interface that must be implemented by application objects that will use the
       * ProgramFileSender component type.
       */
      class ProgramFileSenderClient: public Csi::InstanceValidator
      {
      public:
         /**
          * Called when the server transaction has been completed.
          *
          * @param sender Specifies the component reporting this event.
          *
          * @param outcome Specifies the outcome of the server transaction.
          */
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_in_progress = 2,
            outcome_invalid_program_name = 3,
            outcome_server_resource_error = 4,
            outcome_communication_failed = 5,
            outcome_communication_disabled = 6,
            outcome_logger_compile_error = 7,
            outcome_logger_security_failed = 8,
            outcome_invalid_logon = 9,
            outcome_session_failed = 10,
            outcome_invalid_device_name = 11,
            outcome_cannot_open_file = 12,
            outcome_server_security_failed = 13,
            outcome_logger_buffer_full = 14,
            outcome_network_locked = 15,
            outcome_aborted_by_client = 16,
            outcome_table_defs_failed = 17,
            outcome_logger_file_inaccessible = 18,
            outcome_logger_root_dir_full = 19,
            outcome_logger_incompatible = 20
         }; 
         virtual void on_complete(
            ProgramFileSender *sender, outcome_type outcome) = 0;

         /**
          * Called when the server has made some progress on the transaction.
          *
          * @param sender Specifies the component reporting this event.
          *
          * @param bytes_sent Specifies the number of bytes that have been sent to the datalogger.
          *
          * @param total_bytes Specifies the total number of bytes that need to be sent.
          */
         virtual void on_progress(
            ProgramFileSender *sender, uint4 bytes_sent, uint4 total_bytes)
         { }

         /**
          * Called when the server has sent an extended status notification that indicates that the
          * last program file fragment has been sent.
          *
          * @param sender Specifies the component reporting this event.
          */
         virtual void on_last_fragment_sent(
            ProgramFileSender *sender)
         { }

         /**
          * Called when the server has completed the program file send portion and is starting to
          * get table definitions and compile results from the datalogger.
          *
          * @param sender Specifies the component reporting this event.
          */
         virtual void on_getting_table_definitions(
            ProgramFileSender *sender)
         { }
      };


      /**
       * Defines a component that can be used to send program files and operating systems to the
       * datalogger throught the LoggerNet interface.  In order to use this component, an
       * application must provide an object that extends class ProgramFileSenderClient and should
       * create an instance of this class.  It should then set properties such as device name
       * (set_device_name()) and file name (set_file_name()) and then invoke one of the two versions
       * of start().
       *
       * As the server transaction progresses, the application will be notified of transaction
       * status through calls to its client object's methods.
       */
      class ProgramFileSender:
         public DeviceBase,
         public Csi::EvReceiver
      {
      private:
         // @group: properties
         /**
          * Specifies the name of the program file that is to be sent.
          */
         StrAsc file_name;

         /**
          * Specifies the name and location of the program on the datalogger file systems.  If this
          * property is empty, its default value will be derived from the file_name property and the
          * file will be stored on the datalogger's CPU: (or equivalent) device.
          */
         StrAsc program_name;

         /**
          * Specifies whether we will ask the server to prevent the first datalogger program reset
          * from occurring when we are sending an operating system to the datalogger.  This property
          * will default to false.
          */
         bool prevent_first_stop;
         
         // @endgroup:

         /**
          * Set to true if this component has received an extended status notification message.
          */
         bool received_extended_status_not;

      public:
         /**
          * Constructor
          */
         ProgramFileSender();

         /**
          * Destructor
          */
         virtual ~ProgramFileSender();

         /**
          * @param file_name Specifies the name of the file to send.
          */
         void set_file_name(StrAsc const &file_name);

         /**
          * @return Returns the name of the file to be sent.
          */
         StrAsc const &get_file_name() const { return file_name; }

         /**
          * @param program_name_ Specifies the name of the program on the datalogger.
          */
         void set_program_name(StrAsc const &program_name_);

         /**
          * @return Returns the name of the program on the datalogger.
          */
         StrAsc const &get_program_name() const { return program_name; }

         /**
          * @return Returns true if the server should NOT first stop the current program when
          * sending an operating system.
          */
         bool get_prevent_first_stop() const
         { return prevent_first_stop; }

         /**
          * @param val Set to true if the server should NOT stop the current program when sending an
          * operating system.
          */
         void set_prevent_first_stop(bool val);

         /**
          * @return Returns the explanation string that was sent by the server when it reported that
          * the file is incompatible with the datalogger.
          */
         StrAsc const &get_explanation() const
         { return explanation; }

         /**
          * Responsible for starting the server transaction.
          *
          * @param client_ Specifies the application object that will receive notifications
          * regarding transaction status.
          *
          * @param router Specifies a newly created message router.
          *
          * @param other_component Specifies a client component that already has an active
          * connection to the LoggerNet server.
          */
         typedef ProgramFileSenderClient client_type;
         void start(
            ProgramFileSenderClient *client_, router_handle &router);
         void start(
            ProgramFileSenderClient *client_, ClientBase *other_component);

         /**
          * Cancels the server transaction.
          *
          * @return Returns true if the transaction was in a state where it could be cancelled.
          */
         virtual bool cancel();
         
         /**
          * Restores this component to the state it existed before start() was called.
          */
         void finish();

         /**
          * Formats the transaction outcome to the specified stream.
          *
          * @param out Specifies the stream to which the formatted outcome will be written.
          *
          * @param outcome Specifies the outcome to be formatted.
          *
          * @param explanation Specifies any text that was sent by the datalogger.
          */
         static void describe_outcome(
            std::ostream &out, client_type::outcome_type outcome, StrAsc const &explanation = "");

      protected:
         /**
          * Overloads the base class version to handle a received message.
          */
         virtual void onNetMessage(
            Csi::Messaging::Router *router, Csi::Messaging::Message *message);

         /**
          * Overloads the base class version to handle a failure with the LoggerNet device.
          */
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         /**
          * Overloads the base class version to handle a session failure with the LoggerNet device.
          */
         virtual void on_devicebase_session_failure();

         /**
          * Overloads the base class version to handle the event where the device session has been
          * established.
          */
         virtual void on_devicebase_ready();

         /**
          * Overloads the base class version to handle asynchronouse messages.
          */
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);

      private:
         /**
          * Specifies the application object that will receive event notifications.
          */
         ProgramFileSenderClient *client;

         /**
          * Specifies the file that will be sent.
          */
         FILE *input;

         /**
          * Set to true if the last fragment has been sent.
          */
         bool last_fragment;

         /**
          * Specifies the maximum size fragment that will be sent to the server.
          */
         static const uint4 packet_size;

         /**
          * Specifies the state of this component.
          */
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         /**
          * Specifies the identifier for the loggernet device program file send tranasaction.
          */
         uint4 send_tran;

         /**
          * Specifies the explanation that was returned from the server transaction.
          */
         StrAsc explanation;

         /**
          * Transmits the next fragment to the server.
          */
         void send_one_packet(bool first);

         /**
          * Handles the send acknowledgement message from the server.
          */
         void on_send_ack(Csi::Messaging::Message *message);

         /**
          * Handles the status notification message from the server.
          */
         void on_status_notify(Csi::Messaging::Message *message);

         /**
          * Hand;es the extended status notification message from the server.
          */
         void on_extended_status_not(Csi::Messaging::Message *message);
      };
   };
};

#endif
