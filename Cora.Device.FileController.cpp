/* Cora.Device.FileController.cpp

   Copyright (C) 2002, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: 23 April 2002
   Last Change: Monday 22 February 2016
   Last Commit: $Date: 2016-02-22 11:53:52 -0600 (Mon, 22 Feb 2016) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.FileController.h"
#include "coratools.strings.h"
#include <iostream>


namespace Cora
{
   namespace Device
   {
      namespace FileControllerHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef FileControllerClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // create_and_post
            ////////////////////////////////////////////////////////////
            static void create_and_post(
               FileController *controller,
               client_type *client,
               outcome_type outcome)
            {
               try { (new event_complete(controller,client,outcome))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               FileController *controller,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,controller),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id = Csi::Event::registerType("Cora::Device::FileController::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class FileController definitions
      ////////////////////////////////////////////////////////////
      
      FileController::FileController():
         client(0),
         state(state_standby),
         file_transaction(0)
      { }

      
      FileController::~FileController()
      { finish(); }

      
      void FileController::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void FileController::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void FileController::finish()
      {
         state = state_standby;
         client = 0;
         file_transaction = 0;
         DeviceBase::finish();
      } // finish

      
      void FileController::set_file_argument(StrAsc const &argument)
      {
         if( state == state_standby )
            file_argument = argument;
         else
            throw exc_invalid_state();
      } // set_file_argument


      void FileController::set_file_argument2(StrAsc const &argument2)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         file_argument2 = argument2;
      } // set_file_argument2


      void FileController::set_file_command(file_command_type command)
      {
         if( state == state_standby )
            file_command = command;
         else
            throw exc_invalid_state();
      }


      void FileController::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace FileControllerStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_server_session_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_logger_communication_failed:
            out << my_strings[strid_outcome_logger_communication_failed];
            break;
            
         case client_type::outcome_communication_disabled:
            out << my_strings[strid_outcome_logger_communication_disabled];
            break;
            
         case client_type::outcome_logger_security_blocked:
            out << my_strings[strid_outcome_logger_security_blocked];
            break;
            
         case client_type::outcome_insufficient_logger_resources:
            out << my_strings[strid_outcome_insufficient_logger_resources];
            break;
            
         case client_type::outcome_invalid_file_name:
            out << my_strings[strid_outcome_invalid_file_name];
            break;
            
         case client_type::outcome_unsupported_command:
            out << my_strings[strid_outcome_unsupported_command];
            break;
            
         case client_type::outcome_logger_locked:
            out << my_strings[strid_outcome_logger_locked];
            break;
            
         case client_type::outcome_network_locked:
            out << my_strings[strid_outcome_network_locked];
            break;
            
         case client_type::outcome_logger_root_dir_full:
            out << my_strings[strid_outcome_logger_root_dir_full];
            break;

         case client_type::outcome_logger_file_busy:
            out << my_strings[strid_outcome_logger_file_busy];
            break;

         case client_type::outcome_logger_drive_busy:
            out << my_strings[strid_outcome_logger_drive_busy];
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_outcome


      void FileController::format_command(std::ostream &out)
      {
         switch(file_command)
         {
         case command_compile_and_run:
            out << "compile and run \"" << file_argument << "\", make power up";
            break;
            
         case command_set_run_on_power_up:
            out << "make \"" << file_argument << "\" power up";
            break;
            
         case command_make_hidden:
            out << "hide \"" << file_argument << "\"";
            break;
            
         case command_delete_file:
            out << "delete \"" << file_argument << "\"";
            break;
            
         case command_format_device:
            out << "format " << file_argument;
            break;
            
         case command_compile_and_run_leave_tables:
            out << "compile and run \"" << file_argument << "\", make power up, and preserve data";
            break;
            
         case command_stop_program:
            out << "stop the program";
            break;
            
         case command_stop_program_and_delete:
            out << "stop and delete the program";
            break;
            
         case command_make_os:
            break;
            
         case command_compile_and_run_no_power_up:
            out << "compile and run \"" << file_argument << "\"";
            break;
            
         case command_pause:
            out << "pause the program";
            break;
            
         case command_resume:
            out << "resume the program";
            break;
            
         case command_stop_delete_and_run:
            out << "stop the program, delete its data, compile and run \"" << file_argument << "\", and set run on power up";
            break;
            
         case command_stop_delete_and_run_no_power:
            out << "stop the program, delete its data, compile and run \"" << file_argument << "\"";
            break;
            
         case command_move_file:
            out << "move \"" << file_argument << "\" to \"" << file_argument2 << "\"";
            break;
            
         case command_move_stop_delete_run_power_up:
            out << "move \"" << file_argument << "\" to \"" << file_argument2 << "\", compile and run \""
                << file_argument2 << "\", and set run on power up";
            break;
            
         case command_move_stop_delete_run:
            out << "move \"" << file_argument << "\" to \"" << file_argument2 << "\", compile and run \""
                << file_argument2 << "\" while preserving data";
            break;
            
         case command_copy_file:
            out << "copy \"" << file_argument << "\" to \"" << file_argument2 << "\"";
            break;
            
         case command_copy_stop_delete_run_power_up:
            out << "copy \"" << file_argument << "\" to \"" << file_argument2 << "\", compile and run \""
                << file_argument2 << "\", and set run on power up";
            break;
            
         case command_copy_stop_delete_run:
            out << "copy \"" << file_argument << "\" to \"" << file_argument2 << "\", compile and run \""
                << file_argument2 << "\" while preserving data";
            break;

         case command_compile_pause_reset_tables:
            out << "compile \"" << file_argument << "\", pause, and reset data tables";
            break;

         default:
            out << "unrecognised command";
            break;
         }
      } // format_outcome

      
      void FileController::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace FileControllerHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this,event->outcome);
            }
            else
               finish();
         }
      } // receive

      
      void FileController::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::file_control_ack)
            {
               using namespace FileControllerHelpers;
               uint4 tran_no;
               uint4 resp_code;
               client_type::outcome_type outcome;

               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 2:
                  outcome = client_type::outcome_logger_communication_failed;
                  break;

               case 3:
                  outcome = client_type::outcome_communication_disabled;
                  break;

               case 4:
                  outcome = client_type::outcome_logger_security_blocked;
                  break;

               case 5:
                  outcome = client_type::outcome_insufficient_logger_resources;
                  break;

               case 6:
                  outcome = client_type::outcome_invalid_file_name;
                  break;

               case 7:
                  outcome = client_type::outcome_invalid_tran_number;
                  break;

               case 8:
                  outcome = client_type::outcome_unsupported_command;
                  break;
                  
               case 9:
                  outcome = client_type::outcome_logger_locked;
                  break;

               case 10:
                  outcome = client_type::outcome_network_locked;
                  break;

               case 11:
                  outcome = client_type::outcome_logger_root_dir_full;
                  break;

               case 12:
                  outcome = client_type::outcome_logger_file_busy;
                  break;

               case 13:
                  outcome = client_type::outcome_logger_drive_busy;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::create_and_post(this,client,outcome);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void FileController::on_devicebase_ready()
      {
         if(client_type::is_valid_instance(client))
         {
            Csi::Messaging::Message command(
               device_session,
               Messages::file_control_cmd);
            file_transaction = ++last_tran_no;
            command.addUInt4(file_transaction);
            command.addUInt4(file_command);
            command.addStr(file_argument);
            command.addStr(file_argument2);
            router->sendMessage(&command);
            state = state_active;
         }
         else
            finish();
      } // on_devicebase_ready

      
      void FileController::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace FileControllerHelpers;
         client_type::outcome_type outcome;

         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_server_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
            break;
            
         case devicebase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case devicebase_failure_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_devicebase_failure

      
      void FileController::on_devicebase_session_failure()
      {
         using namespace FileControllerHelpers;
         event_complete::create_and_post(this,client,client_type::outcome_server_session_failed);
      } // on_devicebase_session_failure 
   };
};
