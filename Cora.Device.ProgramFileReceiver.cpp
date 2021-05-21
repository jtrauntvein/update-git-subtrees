/* Cora.Device.ProgramFileReceiver.cpp

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: 7 September 2000
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include <assert.h>
#include <iostream>
#include "Cora.Defs.h"
#include "Cora.Device.ProgramFileReceiver.h"
#include "coratools.strings.h"
#include "Csi.Utils.h"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            ProgramFileReceiver *receiver;
            ProgramFileReceiverClient *client;
            friend class Cora::Device::ProgramFileReceiver;
          
         public:
            event_base(uint4 event_id,
                       ProgramFileReceiver *receiver_,
                       ProgramFileReceiverClient *client_):
               Event(event_id, receiver_),
               receiver(receiver_),
               client(client_)
            { }
            virtual void notify() = 0;
         };
            

         ////////////////////////////////////////////////////////////
         // class event_complete declaration and definitions
         ////////////////////////////////////////////////////////////
         class event_complete:public event_base
         {
         public:
            static uint4 const event_id;
            typedef ProgramFileReceiverClient::outcome_type outcome_type;
            outcome_type outcome;

            static void create_and_post(ProgramFileReceiver *receiver,
                                        ProgramFileReceiverClient *client,
                                        outcome_type outcome_);
            virtual void notify()
            { client->on_complete(receiver, outcome); }

         private:
            event_complete(ProgramFileReceiver *receiver,
                             ProgramFileReceiverClient *client,
                             outcome_type outcome_):
               event_base(event_id, receiver, client),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id = 
         Csi::Event::registerType("Cora::Device::ProgramFileReceiverHelpers::event_complete");


         void event_complete::create_and_post(ProgramFileReceiver *receiver,
                                                ProgramFileReceiverClient *client,
                                                outcome_type outcome)
         {
            try { (new event_complete(receiver,client,outcome))->post(); }
            catch(Csi::Event::BadPost &) { } 
         } // create_and_post


         ////////////////////////////////////////////////////////////
         // class event_on_progress declarations and definitions
         //////////////////////////////////////////////////////////// 
         class event_on_progress: public event_base
         {
         public:
            static uint4 const event_id;
            uint4 receivedbytes;

            static void create_and_post(ProgramFileReceiver *receiver,
                                        ProgramFileReceiverClient *client,
                                        uint4 receivedbytes);
            virtual void notify()
            { client->on_progress(receiver, receivedbytes); }

         private:
            event_on_progress(ProgramFileReceiver *receiver,
                              ProgramFileReceiverClient *client,
                              uint4 receivedbytes_):
               event_base(event_id, receiver, client),
               receivedbytes(receivedbytes_)
            { }
         };


         uint4 const event_on_progress::event_id = 
         Csi::Event::registerType("Cora::Device::ProgramFileReceiverHelpers::event_on_progress");


         void event_on_progress::create_and_post(ProgramFileReceiver *receiver,
                                                 ProgramFileReceiverClient *client,
                                                 uint4 receivedbytes)
         {
            try { (new event_on_progress(receiver,client,receivedbytes))->post(); }
            catch(Csi::Event::BadPost &) { } 
         } // create_and_post
      };


      ////////////////////////////////////////////////////////////
      // class FileSender definitions
      ////////////////////////////////////////////////////////////
      ProgramFileReceiver::ProgramFileReceiver():
         state(state_standby),
         client(0),
         received_bytes(0),
         output(0)
      { }


      ProgramFileReceiver::~ProgramFileReceiver()
      {  finish(); }


      void ProgramFileReceiver::set_out_file_name(StrAsc const &out_file_name_)
      {
         if(state == state_standby)
            out_file_name = out_file_name_;
         else
            throw exc_invalid_state();
      } // set_out_file_name

      
      void  ProgramFileReceiver::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if (client_type::is_valid_instance(client_))
            {
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


      void  ProgramFileReceiver::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if (client_type::is_valid_instance(client_))
            {
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
      

      bool ProgramFileReceiver::cancel()
      {
         bool rtn = true;
         event_complete::create_and_post(this,client,client_type::outcome_cancelled);
         finish();
         return rtn;
      } // cancel


      void ProgramFileReceiver::finish()
      {
         if(output)
         {
            fclose(output);
            output = 0;
         }
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      void ProgramFileReceiver::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_success:
            out << common_strings[common_success];
            break;
            
         case client_type::outcome_communication_failure:
            out << common_strings[common_comm_failed];
            break;
            
         case client_type::outcome_communication_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case client_type::outcome_logger_security_blocked:
            out << common_strings[common_logger_security_blocked];
            break;
            
         case client_type::outcome_invalid_server_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_server_connection_failure:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_cannot_open_file:
            out << common_strings[common_file_io_failed];
            break;
            
         case client_type::outcome_server_security_blocked:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_not_supported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         default:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break; 
         }
      } // describe_outcome


      void ProgramFileReceiver::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            client_type *client = this->client;
            if(ev->getType() == event_complete::event_id)
               finish();
            if(client == event->client && client_type::is_valid_instance(event->client))
               event->notify();
            else
               finish();
         }
      }// receive


      void ProgramFileReceiver::onNetMessage(
         Csi::Messaging::Router *router, 
         Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::program_file_receive_ack)
            {
               uint4 tran_no;
               uint4 resp_code;

               message->readUInt4(tran_no);
               message->readUInt4(resp_code);
               if(resp_code <= 1)
               {
                  StrBin content;
                  message->readBStr(content);
                  fwrite(content.getContents(),content.length(),1,output);
                  received_bytes += (uint4)content.length();
                  event_on_progress::create_and_post(this,client,received_bytes);
                  if(resp_code == 0)
                     event_complete::create_and_post(this,client,client_type::outcome_success);
                  else
                  {
                     Csi::Messaging::Message cont_cmd(
                        device_session,
                        Messages::program_file_receive_cont_cmd);
                     cont_cmd.addUInt4(tran_no);
                     cont_cmd.addBool(false);
                     router->sendMessage(&cont_cmd);
                  }
               }
               else
               {
                  client_type::outcome_type outcome;
                  switch(resp_code)
                  {
                  case 3:
                     outcome = client_type::outcome_communication_failure;
                     break;
                     
                  case 5:
                     outcome = client_type::outcome_logger_security_blocked;
                     break;
                     
                  case 7:
                     outcome = client_type::outcome_communication_disabled;
                     break;
                     
                  case 8:
                     outcome = client_type::outcome_not_supported;
                     break;
                     
                  default:
                     outcome = client_type::outcome_unknown;
                     break;
                  }
                  event_complete::create_and_post(this,client,outcome);
               }
            }
            else
               DeviceBase::onNetMessage(router,message);
         }
         else
            DeviceBase::onNetMessage(router,message);
      } // onNetMessage


      void ProgramFileReceiver::on_devicebase_failure(devicebase_failure_type failure)
      {
         // map the device failure into a client failure
         ProgramFileReceiverClient::outcome_type outcome;

         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_server_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_server_connection_failure;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
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


      void ProgramFileReceiver::on_devicebase_session_failure()
      {
         event_complete::create_and_post(
            this,
            client,
            client_type::outcome_server_connection_failure);
      } // on_devicebase_session_failure


      void ProgramFileReceiver::on_devicebase_ready()
      {
#pragma warning(disable: 4996)
         if((output = Csi::open_file(out_file_name.c_str(),"wb")) != 0)
#pragma warning(default: 4996)
         {
            Csi::Messaging::Message cmd(device_session,Messages::program_file_receive_cmd);
            cmd.addUInt4(tran_no = ++last_tran_no);
            cmd.addBool(false);
            cmd.addUInt4(UInt4_Max);
            state = state_active;
            received_bytes = 0;
            router->sendMessage(&cmd);
         }
         else
            event_complete::create_and_post(
               this,
               client,client_type::outcome_cannot_open_file);
      } // on_devicebase_ready()
   };
};

