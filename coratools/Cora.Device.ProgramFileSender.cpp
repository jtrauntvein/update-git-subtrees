/* Cora.Device.ProgramFileSender.cpp

   Copyright (C) 2000, 2017 Campbell Scientific, Inc.

   Written by: Pin-Wu Kao, revised by Jon Trauntvein
   Date Begun: 29 August 2000
   Last Change: Wednesday 04 October 2017
   Last Commit: $Date: 2017-10-04 13:46:10 -0600 (Wed, 04 Oct 2017) $ 
   Committed by: $Author: jon $
   
*/


#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.ProgramFileSender.h"
#include "coratools.strings.h"
#include "Csi.Utils.h"
#include <assert.h>


namespace Cora
{
   namespace Device
   {
      namespace ProgramFileSenderHelpers
      {
         class event_base: public Csi::Event
         {
         protected:
            ProgramFileSender *sender;
            ProgramFileSenderClient *client;
            friend class Cora::Device::ProgramFileSender;
          
         public:
            event_base(uint4 event_id,
                       ProgramFileSender *sender_,
                       ProgramFileSenderClient *client_):
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
            typedef ProgramFileSenderClient::outcome_type outcome_type;
            outcome_type outcome;

            static void create_and_post(
               ProgramFileSender *sender,
               ProgramFileSenderClient *client,
               outcome_type outcome_);
            
            virtual void notify()
            { client->on_complete(sender, outcome); }

         private:
            event_on_complete(
               ProgramFileSender *sender,
               ProgramFileSenderClient *client,
               outcome_type outcome_):
               event_base(event_id, sender, client),
               outcome(outcome_)
            { }
         };


         uint4 const event_on_complete::event_id = 
         Csi::Event::registerType("Cora::Device::ProgramFileSenderHelpers::event_on_complete");


         void event_on_complete::create_and_post(
            ProgramFileSender *sender,
            ProgramFileSenderClient *client,
            outcome_type outcome)
         {
            try { (new event_on_complete(sender,client,outcome))->post(); }
            catch(Csi::Event::BadPost &) { } 
         } // create_and_post


         class event_on_progress: public event_base
         {
         public:
            static uint4 const event_id;
            uint4 sentbytes, totalbytes;

            static void create_and_post(
               ProgramFileSender *sender,
               ProgramFileSenderClient *client,
               uint4 sentbytes,
               uint4 totalbytes);
            
            virtual void notify()
            { client->on_progress(sender, sentbytes, totalbytes); }

         private:
            event_on_progress(
               ProgramFileSender *sender,
               ProgramFileSenderClient *client,
               uint4 sentbytes_,
               uint4 totalbytes_):
               event_base(event_id, sender, client),
               sentbytes(sentbytes_),
               totalbytes(totalbytes_)
            { }
         };


         uint4 const event_on_progress::event_id = 
         Csi::Event::registerType("Cora::Device::ProgramFileSenderHelpers::event_on_progress");


         void event_on_progress::create_and_post(
            ProgramFileSender *sender,
            ProgramFileSenderClient *client,
            uint4 sentbytes,
            uint4 totalbytes)
         {
            try { (new event_on_progress(sender,client,sentbytes,totalbytes))->post(); }
            catch(Csi::Event::BadPost &) { } 
         } // create_and_post


         class event_extended_status: public event_base
         {
         public:
            static uint4 const event_id;
            uint4 event;
            
         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_extended_status(
               ProgramFileSender *sender,
               ProgramFileSenderClient *client,
               uint4 event_):
               event_base(event_id,sender,client),
               event(event_)
            { }

         public:
            static void create_and_post(
               ProgramFileSender *sender,
               ProgramFileSenderClient *client,
               uint4 event)
            {
               try { (new event_extended_status(sender,client,event))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

            virtual void notify()
            {
               switch(event)
               {
               case 1:
                  client->on_last_fragment_sent(sender);
                  break;

               case 2:
                  client->on_getting_table_definitions(sender);
                  break;
               }
            }
         };


         uint4 const event_extended_status::event_id =
         Csi::Event::registerType("Cora::Device::ProgramFileSender::event_extended_status");
      };


      ProgramFileSender::ProgramFileSender():
         state(state_standby),
         client(0),
         send_tran(0),
         input(0),
         received_extended_status_not(false),
         prevent_first_stop(false)
      { }  


      ProgramFileSender::~ProgramFileSender()
      { finish(); }


      void ProgramFileSender::set_file_name(StrAsc const &file_name_)
      {
         if(state == state_standby)
            file_name = file_name_;
         else
            throw exc_invalid_state();
      } // set_file_name


      void ProgramFileSender::set_program_name(StrAsc const &program_name_)
      {
         if(state == state_standby)
            program_name = program_name_;
         else
            throw exc_invalid_state();
      } // set_program_name


      void ProgramFileSender::set_prevent_first_stop(bool val)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         prevent_first_stop = val;
      } // set_prevent_first_stop


      void  ProgramFileSender::start(
         ProgramFileSenderClient *client_, router_handle &router)
      {
         if(state == state_standby)
         {
            last_fragment = false;
            explanation.cut(0);
            if (ProgramFileSenderClient::is_valid_instance(client_))
            {
               // check to make sure that the file_name property was properly specified
               if(file_name.length() == 0)
                  throw std::invalid_argument("File name not specified");
               
               // if the program_name property is empty (not specified), we need to derive its value
               // from the file_name
               if(program_name.length() == 0)
               {
                  size_t last_slash_pos = file_name.findRev("\\");
                  if(last_slash_pos >= file_name.length())
                     last_slash_pos = UInt4_Max;
                  program_name = file_name.c_str() + (last_slash_pos + 1);
               }

               // start the process
               state = state_delegate;
               client = client_;
               DeviceBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void  ProgramFileSender::start(
         ProgramFileSenderClient *client_, ClientBase *other_component)
      {
         if(state == state_standby)
         {
            last_fragment = false;
            explanation.cut(0);
            if (ProgramFileSenderClient::is_valid_instance(client_))
            {
               // check to make sure that the file_name property was properly specified
               if(file_name.length() == 0)
                  throw std::invalid_argument("File name not specified");
               
               // if the program_name property is empty (not specified), we need to derive its value
               // from the file_name
               if(program_name.length() == 0)
               {
                  size_t last_slash_pos = file_name.findRev("\\");
                  if(last_slash_pos >= file_name.length())
                     last_slash_pos = UInt4_Max;
                  program_name = file_name.c_str() + (last_slash_pos + 1);
               }

               // start the process
               state = state_delegate;
               client = client_;
               DeviceBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      bool ProgramFileSender::cancel()
      {
         bool rtn = false;
         if(state == state_active &&
            get_interface_version() >= Csi::VersionNumber("1.3.6.8"))
         {
            Csi::Messaging::Message abort_cmd(
               device_session,
               Messages::program_file_send_abort_cmd);
            abort_cmd.addUInt4(send_tran);
            router->sendMessage(&abort_cmd);
            rtn = true;
         }
         return rtn;
      } // cancel

      
      void ProgramFileSender::finish()
      {
         if(input)
         {
            fclose(input);
            input = 0;
         }
         state = state_standby;
         client = 0;
         received_extended_status_not = false;
         DeviceBase::finish();
      } // finish


      void ProgramFileSender::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome, StrAsc const &explanation)
      {
         using namespace ProgramFileSenderStrings;
         switch(outcome)
         {
         default:
         case client_type::outcome_unknown:
            ClientBase::describe_failure(out, corabase_failure_unknown); 
            break;
            
         case client_type::outcome_success:
            out << my_strings[strid_success];
            break;
            
         case client_type::outcome_in_progress:
            out << my_strings[strid_other_in_progress];
            break;
            
         case client_type::outcome_invalid_program_name:
            out << my_strings[strid_invalid_program_name];
            break;
            
         case client_type::outcome_server_resource_error:
            out << my_strings[strid_server_resource_error]; 
            break;
            
         case client_type::outcome_communication_failed:
            out << my_strings[strid_communication_failed];
            break;
            
         case client_type::outcome_communication_disabled:
            break;
            
         case client_type::outcome_logger_compile_error:
            out << my_strings[strid_logger_compile_error];
            break;
            
         case client_type::outcome_logger_security_failed:
            out << my_strings[strid_logger_security_failed];
            break;
            
         case client_type::outcome_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_session_failed:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_invalid_device_name:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_cannot_open_file:
            out << my_strings[strid_cannot_open_file];
            break;
            
         case client_type::outcome_server_security_failed:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::outcome_logger_buffer_full:
            out << my_strings[strid_logger_buffer_full];
            break;
            
         case client_type::outcome_network_locked:
            out << my_strings[strid_network_locked];
            break;
            
         case client_type::outcome_aborted_by_client:
            out << my_strings[strid_aborted];
            break;
            
         case client_type::outcome_table_defs_failed:
            out << my_strings[strid_table_defs_failed];
            break;

         case client_type::outcome_logger_file_inaccessible:
            out << my_strings[strid_logger_file_inaccessible];
            break;

         case client_type::outcome_logger_root_dir_full:
            out << my_strings[strid_logger_root_dir_full];
            break;

         case client_type::outcome_logger_incompatible:
            if(explanation.length() == 0)
               out << my_strings[strid_logger_incompatible];
            else
               out << explanation;
            break;
         }
      } // describe_outcome


      void ProgramFileSender::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace ProgramFileSenderHelpers;
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         assert(event != 0);
         if(event->client == client)
         {
            if(ev->getType() == event_on_complete::event_id)
               finish();
            if(ProgramFileSenderClient::is_valid_instance(event->client))
               event->notify();
            else
               finish();
         }
      }// receive


      void ProgramFileSender::onNetMessage(
         Csi::Messaging::Router *router, 
         Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            switch (message->getMsgType())
            {
            case Messages::program_file_send_ack:
               on_send_ack(message);
               break;
               
            case Messages::program_file_send_status_not:
               on_status_notify(message);
               break;

            case Messages::program_file_send_extended_status_not:
               on_extended_status_not(message);
               break;
               
            default:
               DeviceBase::onNetMessage(router,message);
            }
         }
         else
            DeviceBase::onNetMessage(router,message);
      } // onNetMessage


      void ProgramFileSender::on_send_ack(Csi::Messaging::Message *message)
      {
         using namespace ProgramFileSenderHelpers;
         uint4 tran_no;
         uint4 resp_code;
         
         message->readUInt4(tran_no);
         message->readUInt4(resp_code);

         if(resp_code == 0)
         {
            if(last_fragment == false)
               send_one_packet(false);
         }
         else
         {
            ProgramFileSenderClient::outcome_type outcome;
            switch (resp_code) 
            {
            case 1:
               outcome = ProgramFileSenderClient::outcome_in_progress;
               break;
               
            case 4:
               outcome = ProgramFileSenderClient::outcome_invalid_program_name;
               break;
               
            case 5:
               outcome = ProgramFileSenderClient::outcome_server_resource_error;
               break;
               
            case 6:
               outcome = ProgramFileSenderClient::outcome_communication_disabled;
               break;

            case 8:
               outcome = ProgramFileSenderClient::outcome_network_locked;
               break;

            case 9:
               outcome = ProgramFileSenderClient::outcome_aborted_by_client;
               break;

            case 10:
               outcome = ProgramFileSenderClient::outcome_logger_file_inaccessible;
               break;
               
            default:
               outcome = ProgramFileSenderClient::outcome_unknown;
               break;
            };
            event_on_complete::create_and_post(this,client,outcome);
         }
      } // on_send_ack


      void ProgramFileSender::on_status_notify(Csi::Messaging::Message *message)
      {
         // read the message
         using namespace ProgramFileSenderHelpers;
         uint4 tran_no, resp_code, bytes_sent, bytes_total;
         
         message->readUInt4(tran_no);
         message->readUInt4(resp_code);
         message->readUInt4(bytes_sent);
         message->readUInt4(bytes_total);
         if(resp_code <= 1)
         {
            if(!received_extended_status_not)
               event_on_progress::create_and_post(this,client,bytes_sent,bytes_total);
            if (resp_code == 0)
                event_on_complete::create_and_post(
                   this,
                   client,
                   ProgramFileSenderClient::outcome_success);
         }
         else
         {
            ProgramFileSenderClient::outcome_type outcome;
            switch(resp_code) 
            {
            case 2:
               outcome = ProgramFileSenderClient::outcome_logger_compile_error;
               break;
               
            case 3:
               outcome = ProgramFileSenderClient::outcome_communication_failed;
               break;
               
            case 4:
               outcome = ProgramFileSenderClient::outcome_logger_security_failed;
               break;
               
            case 5:
               outcome = ProgramFileSenderClient::outcome_logger_buffer_full;
               break;
               
            case 6:
               outcome = ProgramFileSenderClient::outcome_communication_disabled;
               break;

            case 7:
               outcome = ProgramFileSenderClient::outcome_table_defs_failed;
               break;

            case 8:
               outcome = ProgramFileSenderClient::outcome_aborted_by_client;
               break;

            case 9:
               outcome = ProgramFileSenderClient::outcome_invalid_program_name;
               break;

            case 10:
               outcome = client_type::outcome_logger_file_inaccessible;
               break;
               
            case 11:
               outcome = ProgramFileSenderClient::outcome_logger_root_dir_full;
               break;

            case 12:
               outcome = client_type::outcome_logger_incompatible;
               if(message->whatsLeft())
                  message->readStr(explanation);
               break;
               
            default:
               outcome = ProgramFileSenderClient::outcome_unknown;
               break;
            };
            event_on_complete::create_and_post(this,client,outcome);
         }
      }  // on_status_notification


      void ProgramFileSender::on_extended_status_not(
         Csi::Messaging::Message *message)
      {
         received_extended_status_not = true;
         
         using namespace ProgramFileSenderHelpers;
         uint4 tran_no;
         uint4 event;

         message->readUInt4(tran_no);
         message->readUInt4(event);
         event_extended_status::create_and_post(this,client,event);
      } // on_extended_status_not


      void ProgramFileSender::on_devicebase_failure(devicebase_failure_type failure)
      {
         // map the device failure into a client failure
         using namespace ProgramFileSenderHelpers;
         ProgramFileSenderClient::outcome_type outcome;

         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = ProgramFileSenderClient::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = ProgramFileSenderClient::outcome_session_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = ProgramFileSenderClient::outcome_invalid_device_name;
            break;

         case devicebase_failure_security:
            outcome = ProgramFileSenderClient::outcome_server_security_failed;
            break;

         default:
            outcome = ProgramFileSenderClient::outcome_unknown;
         }
         event_on_complete::create_and_post(this,client,outcome);
      } // on_devicebase_failure


      void ProgramFileSender::on_devicebase_session_failure()
      {
         using namespace ProgramFileSenderHelpers;
         event_on_complete::create_and_post(
            this,
            client,
            ProgramFileSenderClient::outcome_session_failed);
      } // on_devicebase_session_failure


      void ProgramFileSender::on_devicebase_ready()
      {
         using namespace ProgramFileSenderHelpers;
         send_tran = ++last_tran_no;

         // Open the specified file
#pragma warning(disable: 4996)
         input = Csi::open_file(file_name.c_str(),"rb");
#pragma warning(default: 4996)
         if(input)
         {
            state = state_active;
            last_fragment = false;
            send_one_packet(true);
         }
         else
            event_on_complete::create_and_post(
               this,
               client,
               ProgramFileSenderClient::outcome_cannot_open_file);
      } // on_devicebase_ready()


      const uint4 ProgramFileSender::packet_size = 2048;


      void ProgramFileSender::send_one_packet(bool first)
      {
         // read the next fragment to be sent
         byte buffer[packet_size];
         size_t bytes_read = fread(buffer,1,sizeof(buffer),input);

         if(bytes_read != sizeof(buffer))
         {
            last_fragment = true;
            fclose(input);
            input = 0;
         }

         // prepare the fragment to be sent
         Csi::Messaging::Message command(device_session, Messages::program_file_send_cmd);
         command.addUInt4(send_tran);
         command.addBool(last_fragment);
         command.addBytes(buffer, (uint4)bytes_read);
         command.addBool(false);
         if (first)
         {
            // add the file name to the message
            command.addStr(program_name.c_str());
            command.addByte(0); // reserved
            command.addBool(true); // send extended status if available
            command.addBool(prevent_first_stop);
         }

         router->sendMessage(&command);
      } // send_one_packet
   };
};

