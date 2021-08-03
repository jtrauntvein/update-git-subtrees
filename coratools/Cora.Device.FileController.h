/* Cora.Device.FileController.h

   Copyright (C) 2002, 2016 Campbell Scientific, Inc.

   Written by: Tyler Mecham
   Date Begun: 23 April 2002
   Last Change: Monday 22 February 2016
   Last Commit: $Date: 2016-02-22 11:53:52 -0600 (Mon, 22 Feb 2016) $
   Last Changed by: $Author: jon $

*/

#pragma once
#ifndef Cora_Device_FileController_h
#define Cora_Device_FileController_h

#include "Cora.Device.DeviceBase.h"
#include "Csi.InstanceValidator.h"
#include "CsiEvents.h"


namespace Cora
{
   namespace Device
   {
      //@group class forward declarations
      class FileController;
      //@endgroup


      ////////////////////////////////////////////////////////////
      // class FileControllerClient
      ////////////////////////////////////////////////////////////
      class FileControllerClient: public Csi::InstanceValidator
      {
      public:
         ////////////////////////////////////////////////////////////
         // on_complete
         ////////////////////////////////////////////////////////////
         enum outcome_type
         {
            outcome_unknown = 0,
            outcome_success = 1,
            outcome_invalid_logon = 2,
            outcome_server_session_failed = 3,
            outcome_invalid_device_name = 4,
            outcome_unsupported = 5,
            outcome_server_security_blocked = 6,
            outcome_logger_communication_failed = 7,
            outcome_communication_disabled = 8,
            outcome_logger_security_blocked = 9,
            outcome_insufficient_logger_resources = 10,
            outcome_invalid_file_name = 11,
            outcome_invalid_tran_number = 12,
            outcome_unsupported_command = 13,
            outcome_logger_locked = 14,
            outcome_network_locked = 15,
            outcome_logger_root_dir_full = 16,
            outcome_logger_file_busy = 17,
            outcome_logger_drive_busy = 18
         };
         virtual void on_complete(FileController *controller, outcome_type outcome) = 0;
      };


      ////////////////////////////////////////////////////////////
      // class FileController
      //
      // This class defines a component that can be used to drive a file
      // control transaction with a server device.
      //
      // An application can use this component by deriving a class from
      // FileControllerClient and instantiating an instance of this
      // class. Once the component is created, the application should call
      // the appropriate set property methods including set_device_name()
      // and then invoke the start() method. The application will receive
      // notice of the completion event through the on_complete() method
      // provided in its client object.
      //
      // If the application invokes the finish() method before receiving
      // the completion notification, the file control operation will be
      // aborted and the application will not receive any notice regarding
      // the outcome.
      ////////////////////////////////////////////////////////////
      class FileController:
         public DeviceBase,
         public Csi::EventReceiver
      {
      public:
         ////////////////////////////////////////////////////////////
         // constructor
         ////////////////////////////////////////////////////////////
         FileController();

         ////////////////////////////////////////////////////////////
         // destructor
         ////////////////////////////////////////////////////////////
         virtual ~FileController();

         ////////////////////////////////////////////////////////////
         // start
         ////////////////////////////////////////////////////////////
         typedef FileControllerClient client_type;
         void start(
            client_type *client_,
            router_handle &router);
         void start(
            client_type *client_,
            ClientBase *other_component);

         ////////////////////////////////////////////////////////////
         // finish
         ////////////////////////////////////////////////////////////
         void finish();

         ////////////////////////////////////////////////////////////
         // set_file_command
         //
         // Identifies the operation that should take place in the logger
         ////////////////////////////////////////////////////////////
         enum file_command_type
         {
            command_compile_and_run = 1,
            command_set_run_on_power_up = 2,
            command_make_hidden = 3,
            command_delete_file = 4,
            command_format_device = 5,
            command_compile_and_run_leave_tables = 6,
            command_stop_program = 7,
            command_stop_program_and_delete = 8,
            command_make_os = 9,
            command_compile_and_run_no_power_up = 10,
            command_pause = 11,
            command_resume = 12,
            command_stop_delete_and_run = 13,
            command_stop_delete_and_run_no_power = 14,
            command_move_file = 15,
            command_move_stop_delete_run_power_up = 16,
            command_move_stop_delete_run = 17,
            command_copy_file = 18,
            command_copy_stop_delete_run_power_up = 19,
            command_copy_stop_delete_run = 20,
            command_compile_pause_reset_tables = 21
         };
         void set_file_command(file_command_type command);

         ////////////////////////////////////////////////////////////
         // get_file_command
         ////////////////////////////////////////////////////////////
         file_command_type get_file_command() const {return file_command;}

         ////////////////////////////////////////////////////////////
         // set_file_argument
         //
         // Specifies the argument for the command.  This can be empty
         // depending on the command specified.
         ////////////////////////////////////////////////////////////
         void set_file_argument(StrAsc const &argument);

         ////////////////////////////////////////////////////////////
         // get_file_argument
         ////////////////////////////////////////////////////////////
         StrAsc const &get_file_argument() const {return file_argument;}

         ////////////////////////////////////////////////////////////
         // set_file_argument2
         ////////////////////////////////////////////////////////////
         void set_file_argument2(StrAsc const &argument2);

         ////////////////////////////////////////////////////////////
         // get_file_argument2
         ////////////////////////////////////////////////////////////
         StrAsc const &get_file_argument2() const
         { return file_argument2; }

         ////////////////////////////////////////////////////////////
         // format_outcome
         ////////////////////////////////////////////////////////////
         static void format_outcome(std::ostream &out, client_type::outcome_type outcome);

         ////////////////////////////////////////////////////////////
         // format_command
         //
         // Formats the operation that this control operation will complete.
         ////////////////////////////////////////////////////////////
         void format_command(std::ostream &out);

      protected:
         ////////////////////////////////////////////////////////////
         // receive
         ////////////////////////////////////////////////////////////
         virtual void receive(Csi::SharedPtr<Csi::Event> &ev);
         
         ////////////////////////////////////////////////////////////
         // onNetMessage
         ////////////////////////////////////////////////////////////
         virtual void onNetMessage(
            Csi::Messaging::Router *rtr,
            Csi::Messaging::Message *msg);

         ////////////////////////////////////////////////////////////
         // on_devicebase_ready
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_ready();

         ////////////////////////////////////////////////////////////
         // on_devicebase_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_failure(devicebase_failure_type failure);

         ////////////////////////////////////////////////////////////
         // on_devicebase_session_failure
         ////////////////////////////////////////////////////////////
         virtual void on_devicebase_session_failure();

      private:
         ////////////////////////////////////////////////////////////
         // client
         ////////////////////////////////////////////////////////////
         client_type *client;

         ////////////////////////////////////////////////////////////
         // state
         ////////////////////////////////////////////////////////////
         enum state_type
         {
            state_standby,
            state_delegate,
            state_active
         } state;

         ////////////////////////////////////////////////////////////
         // file_transaction
         ////////////////////////////////////////////////////////////
         uint4 file_transaction;

         ////////////////////////////////////////////////////////////
         // file_command
         ////////////////////////////////////////////////////////////
         file_command_type file_command;

         ////////////////////////////////////////////////////////////
         // file_argument
         ////////////////////////////////////////////////////////////
         StrAsc file_argument;

         ////////////////////////////////////////////////////////////
         // file_argument2
         ////////////////////////////////////////////////////////////
         StrAsc file_argument2;
      };
   };
};


#endif
