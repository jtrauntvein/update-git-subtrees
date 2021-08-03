/* Cora.Device.FileSender.cpp

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Pin-Wu Kao, Carl Zmola & Jon Trauntvein
   Date Begun: Monday 11 September, 2000
   Last Change: Wednesday 04 October 2017
   Last Commit: $Date: 2017-10-04 13:46:10 -0600 (Wed, 04 Oct 2017) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Defs.h"
#include "Cora.Device.FileSender.h"
#include "Csi.OsException.h"
#include "coratools.strings.h"
#include "Csi.Utils.h"
#include <assert.h>
#include <stdio.h>
#include <iostream>


namespace Cora
{
   namespace Device
   {
      namespace FileSenderHelpers
      {
         class event_base: public Csi::Event
         {
         protected:
            FileSender *sender;
            FileSenderClient *client;
            friend class Cora::Device::FileSender;
          
         public:
            event_base(uint4 event_id,
                       FileSender *sender_,
                       FileSenderClient *client_):
               Event(event_id, sender_),
               sender(sender_),
               client(client_)
            { }
            virtual void notify() = 0;
         };
            

         class event_on_complete:public event_base
         {
         public:
            static uint4 const event_id;
            typedef FileSenderClient::outcome_type outcome_type;
            outcome_type complete;

            static void create_and_post(FileSender *sender,
                                        FileSenderClient *client,
                                        outcome_type complete);
            virtual void notify()
            { client->on_complete(sender, complete); }

         private:
            event_on_complete(FileSender *sender,
                             FileSenderClient *client,
                             outcome_type complete_):
               event_base(event_id, sender, client),
               complete(complete_)
            { }
         };


         uint4 const event_on_complete::event_id = 
         Csi::Event::registerType("Cora::Device::FileSenderHelpers::event_on_complete");


         void event_on_complete::create_and_post(FileSender *sender,
                                                FileSenderClient *client,
                                                outcome_type complete)
         {
            try { (new event_on_complete(sender,client,complete))->post(); }
            catch(Csi::Event::BadPost &) { } 
         } // create_and_post


         class event_on_progress: public event_base
         {
         public:
            static uint4 const event_id;
            uint4 sentbytes;

            static void create_and_post(
               FileSender *sender,
               FileSenderClient *client,
               uint4 sentbytes);

            virtual void notify()
            { client->on_progress(sender, sentbytes); }

         private:
            event_on_progress(FileSender *sender,
                              FileSenderClient *client,
                              uint4 sentbytes_):
               event_base(event_id, sender, client),
               sentbytes(sentbytes_)
            { }
         };


         uint4 const event_on_progress::event_id = 
         Csi::Event::registerType("Cora::Device::FileSenderHelpers::event_on_progress");


         void event_on_progress::create_and_post(
            FileSender *sender,
            FileSenderClient *client,
            uint4 sentbytes)
         {
            try { (new event_on_progress(sender,client,sentbytes))->post(); }
            catch(Csi::Event::BadPost &) { } 
         } // create_and_post
      };

      
      uint4 const FileSender::max_fragment_len = 2048;

      
      FileSender::FileSender():
         run_program_now(false),
         run_program_on_power_up(false),
         state(state_standby),
         send_tran(0)
      { } 


      FileSender::~FileSender()
      { finish(); }


      void FileSender::set_logger_file_name(StrAsc const &logger_file_name_)
      {
         if(state == state_standby)
            logger_file_name = logger_file_name_;
         else
            throw exc_invalid_state();
      } // set_file_name


      void FileSender::set_send_source(send_source_handle send_source_)
      {
         if(state == state_standby)
            send_source = send_source_;
         else
            throw exc_invalid_state();
      } // set_send_source
      
      
      void FileSender::set_run_program_now(bool run_program_now_)
      {
         if(state == state_standby)
            run_program_now = run_program_now_;
         else
            throw exc_invalid_state();
      } // set_run_program_now
      

      void FileSender::set_run_program_on_power_up(bool run_program_on_power_up_)
      {
         if(state == state_standby)
            run_program_on_power_up = run_program_on_power_up_;
         else
            throw exc_invalid_state();
      } // set_run_program_on_power_up


      void FileSender::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace FileSenderHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         if(ev->getType() == event_on_complete::event_id)
            finish();
         if(client_type::is_valid_instance(event->client))
            event->notify();
         else
            finish();
      }// receive


      void FileSender::onNetMessage(
         Csi::Messaging::Router *router, 
         Csi::Messaging::Message *message)
      {
         if(state >= state_sending_file)
         {
            switch(message->getMsgType())
            {
               
            case Messages::file_send_ack:
               on_send_ack(message);
               break;
               
            case Messages::file_send_status_not:
               on_status_not(message);
               break;
               
            default:
               DeviceBase::onNetMessage(router,message);
            }
         }
         else
            DeviceBase::onNetMessage(router,message);
      } // onNetMessage


      void FileSender::on_send_ack(Csi::Messaging::Message *message)
      {
         using namespace FileSenderHelpers;
         uint4 tran_no;
         uint4 resp_code;
         
         message->readUInt4(tran_no);
         message->readUInt4(resp_code);

         if(resp_code == 1)
         {
            if(state == state_sending_file)
               send_next_fragment();
         }
         else
         {
            FileSenderClient::outcome_type failure;
            switch(resp_code) 
            {
            case 2:
               failure = FileSenderClient::outcome_communication_disabled;
               break;
               
            case 3:
               failure = FileSenderClient::outcome_missing_file_name;
               break;
               
            case 4:
               failure = FileSenderClient::outcome_invalid_file_name;
               break;

            case 5:
               failure = FileSenderClient::outcome_network_locked;
               break;
               
            default:
               failure = FileSenderClient::outcome_unknown;
               break;
            };
            event_on_complete::create_and_post(this,client,failure);
         }
      } // on_send_ack


      void FileSender::on_status_not(Csi::Messaging::Message *message)
      {
         // read the message
         using namespace FileSenderHelpers;
         uint4 tran_no, resp_code, sent_bytes;
         
         message->readUInt4(tran_no);
         message->readUInt4(resp_code);
         message->readUInt4(sent_bytes);
         if(resp_code == 1)
         {
            event_on_progress::create_and_post(this,client,sent_bytes);
            event_on_complete::create_and_post(this,client,FileSenderClient::outcome_success);
         }
         else if(resp_code == 2)
            event_on_progress::create_and_post(this,client,sent_bytes);
         else
         {
            FileSenderClient::outcome_type failure;
            switch(resp_code) 
            {
            case 3:
               failure = client_type::outcome_communication_failed;
               break;
               
            case 4:
               failure = client_type::outcome_logger_permission_denied;
               break;
               
            case 5:
               failure = client_type::outcome_logger_resource_error;
               break;
               
            case 6:
               failure = client_type::outcome_invalid_file_name;
               break;

            case 7:
               failure = client_type::outcome_logger_root_dir_full;
               break;

            case 8:
               failure = client_type::outcome_logger_incompatible;
               if(message->whatsLeft())
                  message->readStr(explanation);
               break;
               
            default:
               failure = client_type::outcome_unknown;
               break;
            };
            event_on_complete::create_and_post(this,client,failure);
         }
      }  // on_status_notification


      void FileSender::on_devicebase_failure(devicebase_failure_type failure)
      {
         // map the device failure into a client failure
         using namespace FileSenderHelpers;
         client_type::outcome_type outcome;

         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_server_connection_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
            break;

         default:
            outcome = client_type::outcome_unknown;
         }
         event_on_complete::create_and_post(this,client,outcome);
      } // on_devicebase_failure


      void FileSender::on_devicebase_session_failure()
      {
         using namespace FileSenderHelpers;
         event_on_complete::create_and_post(
            this,
            client,
            client_type::outcome_server_connection_failed);
      } // on_devicebase_session_failure


      void FileSender::on_devicebase_ready()
      {
         state = state_sending_file;
         send_next_fragment();
      } // on_devicebase_ready


      void FileSender::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            if(send_source == 0)
               throw std::invalid_argument("Invalid send source");
            state = state_delegate;
            client = client_;
            explanation.cut(0);
            DeviceBase::start(router);
         }
         else
            throw exc_invalid_state();
      } // start


      void FileSender::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(!client_type::is_valid_instance(client_))
               throw std::invalid_argument("Invalid client pointer");
            if(send_source == 0)
               throw std::invalid_argument("invalid send source");
            state = state_delegate;
            client = client_;
            explanation.cut(0);
            DeviceBase::start(other_component);
         }
         else
            throw exc_invalid_state();
      } // start


      void FileSender::finish()
      {
         state = state_standby;
         send_source.clear();
         send_tran = 0;
         client = 0;
         DeviceBase::finish();
      } // finish


      void FileSender::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome, StrAsc const &explanation)
      {
         using namespace FileSenderStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_communication_disabled:
            out << my_strings[strid_outcome_communication_disabled];
            break;
            
         case client_type::outcome_missing_file_name:
            out << my_strings[strid_outcome_missing_file_name];
            break;
            
         case client_type::outcome_invalid_file_name:
            out << my_strings[strid_outcome_invalid_file_name];
            break;
            
         case client_type::outcome_logger_resource_error:
            out << my_strings[strid_outcome_logger_resource_error];
            break;
            
         case client_type::outcome_logger_compile_error:
            out << my_strings[strid_outcome_logger_compile_error];
            break;
            
         case client_type::outcome_communication_failed:
            out << my_strings[strid_outcome_communication_failed];
            break;
            
         case client_type::outcome_logger_permission_denied:
            out << my_strings[strid_outcome_logger_permission_denied];
            break;
            
         case client_type::outcome_invalid_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_server_connection_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_server_permission_denied:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_network_locked:
            out << my_strings[strid_outcome_network_locked];
            break;
            
         case client_type::outcome_logger_root_dir_full:
            out << my_strings[strid_outcome_logger_root_dir_full];
            break;

         case client_type::outcome_logger_incompatible:
            if(explanation.length() > 0)
               out << explanation;
            else
               out << my_strings[strid_outcome_logger_incompatible];
            break;

         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // describe_outcome


      void FileSender::send_next_fragment()
      {
         // the first time that this method will be called after the component is started, the file
         // will not yet be open.  We need to take care of this initialisation
         bool first_time_called = false;
         if(send_tran == 0)
         {
            first_time_called = true;
            send_tran = ++last_tran_no;
         }
         
         // we need to read the next fragment from the file.
         byte fragment[max_fragment_len];
         uint4 fragment_len = send_source->get_next_fragment(fragment,max_fragment_len);
         
         // we can now send the next fragment to the server
         Csi::Messaging::Message cmd(device_session,Messages::file_send_cmd);
         cmd.addUInt4(send_tran);
         cmd.addBool(false); // don't abort
         cmd.addBytes(fragment,fragment_len);
         cmd.addBool(send_source->at_end());
         if(first_time_called)
         {
            cmd.addStr(logger_file_name.c_str());
            cmd.addBool(run_program_now);
            cmd.addBool(run_program_on_power_up);
         }
         router->sendMessage(&cmd);
         if(send_source->at_end())
            state = state_waiting_for_result;
      } // send_next_fragment


      namespace FileSenderHelpers
      {
         ////////////////////////////////////////////////////////////
         // class file_send_source definitions
         ////////////////////////////////////////////////////////////
         file_send_source::file_send_source(StrAsc const &file_name):
            file_handle(0)
         {
#pragma warning(disable: 4996)
            file_handle = Csi::open_file(file_name.c_str(),"rb");
#pragma warning(default: 4996)
            if(file_handle == 0)
               throw Csi::OsException("failed to open file");
         } // constructor


         file_send_source::~file_send_source()
         {
            if(file_handle)
            {
               fclose(file_handle);
               file_handle = 0;
            }
         } // destructor


         uint4 file_send_source::get_next_fragment(
            void *buffer,
            uint4 max_fragment_len)
         { return (uint4)fread(buffer, 1, max_fragment_len, file_handle); }


         bool file_send_source::at_end()
         { return feof(file_handle) != 0; }
      };
   };
};

