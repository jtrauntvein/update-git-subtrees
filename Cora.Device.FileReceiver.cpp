/* Cora.Device.FileReceiver.cpp

   Copyright (C) 2000, 2016 Campbell Scientific, Inc.

   Written by: Pin-Wu Kao, Carl Zmola & Jon Trauntvein
   Date Begun: 11 September 2000
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include <assert.h>
#include "Cora.Defs.h"
#include "Cora.Device.FileReceiver.h"
#include "Csi.OsException.h"
#include "coratools.strings.h"
#include "Csi.Utils.h"


namespace Cora
{
   namespace Device
   {
      namespace FileReceiverHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_base
         ////////////////////////////////////////////////////////////
         class event_base: public Csi::Event
         {
         protected:
            FileReceiver *receiver;
            FileReceiverClient *client;
            friend class Cora::Device::FileReceiver;
          
         public:
            event_base(uint4 event_id,
                       FileReceiver *receiver_,
                       FileReceiverClient *client_):
               Event(event_id, receiver_),
               receiver(receiver_),
               client(client_)
            { }
            virtual void notify() = 0;
         };
            

         ////////////////////////////////////////////////////////////
         // class event_on_complete declaration and definitions
         ////////////////////////////////////////////////////////////
         class event_on_complete:public event_base
         {
         public:
            static uint4 const event_id;
            typedef FileReceiverClient::outcome_type outcome_type;
            outcome_type outcome;

            static void create_and_post(FileReceiver *receiver,
                                        FileReceiverClient *client,
                                        outcome_type outcome);
            virtual void notify()
            { client->on_complete(receiver, outcome); }

         private:
            event_on_complete(FileReceiver *receiver,
                             FileReceiverClient *client,
                             outcome_type outcome_):
               event_base(event_id, receiver, client),
               outcome(outcome_)
            { }
         };


         uint4 const event_on_complete::event_id = 
         Csi::Event::registerType("Cora::Device::FileReceiverHelpers::event_on_complete");


         void event_on_complete::create_and_post(FileReceiver *receiver,
                                                FileReceiverClient *client,
                                                outcome_type outcome)
         {
            try { (new event_on_complete(receiver,client,outcome))->post(); }
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

            static void create_and_post(FileReceiver *receiver,
                                        FileReceiverClient *client,
                                        uint4 receivedbytes);
            virtual void notify()
            { client->on_progress(receiver, receivedbytes); }

         private:
            event_on_progress(FileReceiver *receiver,
                              FileReceiverClient *client,
                              uint4 receivedbytes_):
               event_base(event_id, receiver, client),
               receivedbytes(receivedbytes_)
            { }
         };


         uint4 const event_on_progress::event_id = 
         Csi::Event::registerType("Cora::Device::FileReceiverHelpers::event_on_progress");


         void event_on_progress::create_and_post(FileReceiver *receiver,
                                                 FileReceiverClient *client,
                                                 uint4 receivedbytes)
         {
            try { (new event_on_progress(receiver,client,receivedbytes))->post(); }
            catch(Csi::Event::BadPost &) { } 
         } // create_and_post
      };
      

      ////////////////////////////////////////////////////////////
      // class FileReceiver definitions
      //////////////////////////////////////////////////////////// 
      FileReceiver::FileReceiver():
         state(state_standby),
         client(0),
         bytes_received(0)
      { }


      FileReceiver::~FileReceiver()
      { finish(); }


      void FileReceiver::set_logger_file_name(StrAsc const &logger_file_name_)
      {
         if(state == state_standby)
            logger_file_name = logger_file_name_;
         else
            throw exc_invalid_state();
      } // set_logger_file_name


      void FileReceiver::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace FileReceiverHelpers;
         event_base *event = static_cast<event_base *>(ev.get_rep());
         if (ev->getType() == event_on_complete::event_id)
            finish();
         if(FileReceiverClient::is_valid_instance(event->client))
            event->notify();
         else
            finish();
      }// receive


      void FileReceiver::onNetMessage(
         Csi::Messaging::Router *router, 
         Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            switch (message->getMsgType())
            {
               
            case Messages::file_receive_ack:
               on_receive_ack(message);
               break;
               
            default:
               DeviceBase::onNetMessage(router,message);
            }
         }
         else
            DeviceBase::onNetMessage(router,message);
      } // onNetMessage


      void FileReceiver::on_receive_ack(Csi::Messaging::Message *message)
      {
         using namespace FileReceiverHelpers;
         uint4 tran_no;
         uint4 resp_code;
         
         message->readUInt4(tran_no);
         message->readUInt4(resp_code);
         if(resp_code == 1 || resp_code == 2)
         {
            message->readBStr(receive_buffer);
            sink->receive_next_fragment(receive_buffer.getContents(), (uint4)receive_buffer.length());
            bytes_received += (uint4)receive_buffer.length();
            event_on_progress::create_and_post(this,client,bytes_received);
            if(resp_code == 1)
               event_on_complete::create_and_post(this,client,client_type::outcome_success);
            else
            {
               Csi::Messaging::Message cont_cmd(device_session,Messages::file_receive_cont_cmd);
               cont_cmd.addUInt4(tran_no);
               cont_cmd.addBool(false); // no abort
               router->sendMessage(&cont_cmd);
            }
         }
         else
         {
            FileReceiverClient::outcome_type failure;
            switch (resp_code) 
            {
            case 3:
               failure = FileReceiverClient::outcome_communication_disabled;
               break;
               
            case 4:
               failure = FileReceiverClient::outcome_communication_failed;
               break;
               
            case 5:
               failure = FileReceiverClient::outcome_logger_permission_denied;
               break;
               
            case 6:
               failure = FileReceiverClient::outcome_invalid_file_name;
               break;
               
            default:
               failure = FileReceiverClient::outcome_unknown;
               break;
            };
            event_on_complete::create_and_post(this,client,failure);
         }
      } // on_send_ack


      void FileReceiver::on_devicebase_failure(devicebase_failure_type failure)
      {
         // map the device failure into a client failure
         using namespace FileReceiverHelpers;
         FileReceiverClient::outcome_type client_failure;

         switch(failure)
         {
         case devicebase_failure_logon:
            client_failure = FileReceiverClient::outcome_invalid_server_logon;
            break;
            
         case devicebase_failure_session:
            client_failure = FileReceiverClient::outcome_server_connection_failed;
            break;
            
         case devicebase_failure_invalid_device_name:
            client_failure = FileReceiverClient::outcome_invalid_device_name;
            break;

         case devicebase_failure_security:
            client_failure = FileReceiverClient::outcome_server_permission_denied;
            break;
            
         case devicebase_failure_unsupported:
            client_failure = client_type::outcome_unsupported;
            break;
            
         default:
            client_failure = FileReceiverClient::outcome_unknown;
         }
         event_on_complete::create_and_post(this,client,client_failure);
      } // on_devicebase_failure


      void FileReceiver::on_devicebase_session_failure()
      {
         using namespace FileReceiverHelpers;

         event_on_complete::create_and_post(
            this,
            client,
            FileReceiverClient::outcome_server_connection_failed);
      } // on_devicebase_session_failure


      void FileReceiver::on_devicebase_ready()
      {
         Csi::Messaging::Message start_cmd(device_session,Messages::file_receive_cmd);
         start_cmd.addUInt4(++last_tran_no);
         start_cmd.addStr(logger_file_name);
         start_cmd.addUInt4(2048);
         router->sendMessage(&start_cmd);
         state = state_active;
      } // on_devicebase_ready()


      void  FileReceiver::start(
         client_type *client_,
         sink_handle sink_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(FileReceiverClient::is_valid_instance(client_))
            {
               if(sink_ != 0)
               {
                  sink = sink_;
                  bytes_received = 0;
                  state = state_delegate;
                  client = client_;
                  DeviceBase::start(router);
               }
               else
                  throw std::invalid_argument("Invalid file receive sink");
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void  FileReceiver::start(
         FileReceiverClient *client_,
         sink_handle sink_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if (FileReceiverClient::is_valid_instance(client_))
            {
               if(sink_ != 0)
               {
                  sink = sink_;
                  bytes_received = 0;
                  state = state_delegate;
                  client = client_;
                  DeviceBase::start(other_component);
               }
               else
                  throw std::invalid_argument("Invalid receive sink object");
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start


      void FileReceiver::finish()
      {
         sink.clear(); 
         state = state_standby;
         client = 0;
         DeviceBase::finish();
      } // finish


      void FileReceiver::format_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         default:
         case client_type::outcome_unknown:
            format_devicebase_failure(out, devicebase_failure_unknown);
            break;
            
         case client_type::outcome_success:
            out << common_strings[common_success];
            break;
            
         case client_type::outcome_invalid_server_logon:
            format_devicebase_failure(out, devicebase_failure_logon);
            break;

         case client_type::outcome_invalid_device_name:
            format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_server_connection_failed:
            format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_server_permission_denied:
            format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_communication_failed:
            out << common_strings[common_comm_failed];
            break;
            
         case client_type::outcome_communication_disabled:
            out << common_strings[common_comm_disabled];
            break;
            
         case client_type::outcome_logger_permission_denied:
            out << common_strings[common_logger_security_blocked];
            break;
            
         case client_type::outcome_invalid_file_name:
            out << common_strings[common_invalid_file_name];
            break;
            
         case client_type::outcome_unsupported:
            format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
         }
      } // format_outcome


      namespace FileReceiverHelpers
      {
         ////////////////////////////////////////////////////////////
         // class file_sink_type definitions
         ////////////////////////////////////////////////////////////
         file_sink_type::file_sink_type(StrAsc const &file_name):
            file_handle(0)
         {
#pragma warning(disable: 4996)
            file_handle = Csi::open_file(file_name.c_str(),"wb");
#pragma warning(default: 4996)
            if(file_handle == 0)
               throw Csi::OsException("Failed to open output file"); 
         } // constructor


         file_sink_type::~file_sink_type()
         {
            if(file_handle)
            {
               fclose(file_handle);
               file_handle = 0;
            }
         } // destructor


         void file_sink_type::receive_next_fragment(
            void const *buff,
            uint4 buff_len)
         {
            fwrite(buff,1,buff_len,file_handle);
         } // receive_next_fragment
      };
   };
};

