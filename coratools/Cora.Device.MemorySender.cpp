/* Cora.Device.MemorySender.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 16 April 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2016-05-03 16:18:03 -0600 (Tue, 03 May 2016) $ 
   Last Changed by: $Author: jon $
   CVS $Header: /home/group/cvs2/cora/coratools/Cora.Device.MemorySender.cpp,v 1.1.1.1 2004/04/30 13:18:35 jon Exp $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.MemorySender.h"


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
         public:
            ////////////////////////////////////////////////////////////
            // sender
            ////////////////////////////////////////////////////////////
            typedef MemorySender sender_type;
            sender_type *sender;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef sender_type::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // notify
            ////////////////////////////////////////////////////////////
            void notify()
            {
               if(client_type::is_valid_instance(client))
                  do_notify();
            }
            
            ////////////////////////////////////////////////////////////
            // do_notify
            ////////////////////////////////////////////////////////////
            virtual void do_notify() = 0;
            
         protected:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_base(
               uint4 event_id,
               sender_type *sender_,
               client_type *client_):
               Event(event_id,sender_),
               sender(sender_),
               client(client_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;
            
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               sender_type *sender,
               client_type *client,
               outcome_type outcome)
            {
               try{(new event_complete(sender,client,outcome))->post();}
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // do_notify
            ////////////////////////////////////////////////////////////
            virtual void do_notify()
            { client->on_complete(sender,outcome); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               sender_type *sender,
               client_type *client,
               outcome_type outcome_):
               event_base(event_id,sender,client),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::MemorySender::on_complete");
      };


      ////////////////////////////////////////////////////////////
      // class MemortSender definitions
      ////////////////////////////////////////////////////////////
      MemorySender::MemorySender():
         state(state_standby),
         client(0)
      { }

      
      MemorySender::~MemorySender()
      { finish(); }

      
      void MemorySender::start(
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

      
      void MemorySender::start(
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

      
      void MemorySender::finish()
      {
         client = 0;
         state = state_standby;
         DeviceBase::finish();
      } // finish

      
      void MemorySender::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         event_base *event = dynamic_cast<event_base *>(ev.get_rep());
         if(event)
         {
            client_type *client = event->client;
            if(event->getType() == event_complete::event_id)
               finish();
            event->notify();
         }
      } // receive

      
      void MemorySender::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::memory_send_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addUInt4(address);
         cmd.addBytes(image.getContents(), (uint4)image.length());
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void MemorySender::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_session_failed;
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
         event_complete::cpost(this,client,outcome);
      } // on_devicebase_failure

      
      void MemorySender::on_devicebase_session_failure()
      { event_complete::cpost(this,client,client_type::outcome_session_failed); }

      
      void MemorySender::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::memory_send_ack)
            {
               uint4 tran_no;
               uint4 server_outcome;
               client_type::outcome_type outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(server_outcome);
               switch(server_outcome)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 3:
                  outcome = client_type::outcome_image_too_large;
                  break;

               case 4:
                  outcome = client_type::outcome_logger_security_blocked;
                  break;

               case 5:
                  outcome = client_type::outcome_communication_failed;
                  break;

               case 6:
                  outcome = client_type::outcome_communication_disabled;

               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::cpost(this,client,outcome);
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage 
   }; 
};

