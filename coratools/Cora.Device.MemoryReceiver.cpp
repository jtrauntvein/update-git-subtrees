/* Cora.Device.MemoryReceiver.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 16 April 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $
   CVS $Header: /home/group/cvs2/cora/coratools/Cora.Device.MemoryReceiver.cpp,v 1.1.1.1 2004/04/30 13:18:35 jon Exp $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.MemoryReceiver.h"


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
            // receiver
            ////////////////////////////////////////////////////////////
            typedef MemoryReceiver receiver_type;
            receiver_type *receiver;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef receiver_type::client_type client_type;
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
               receiver_type *receiver_,
               client_type *client_):
               Event(event_id,receiver_),
               receiver(receiver_),
               client(client_)
            { }
         };


         ////////////////////////////////////////////////////////////
         // class event_fragment
         ////////////////////////////////////////////////////////////
         class event_fragment: public event_base
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // fragment
            ////////////////////////////////////////////////////////////
            StrBin fragment;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               receiver_type *receiver,
               client_type *client,
               StrBin const &fragment)
            {
               try{(new event_fragment(receiver,client,fragment))->post();}
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // do_notify
            ////////////////////////////////////////////////////////////
            virtual void do_notify()
            { client->on_fragment(receiver,fragment); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_fragment(
               receiver_type *receiver,
               client_type *client,
               StrBin const &fragment_):
               event_base(event_id,receiver,client),
               fragment(fragment_)
            { }
         };


         uint4 const event_fragment::event_id =
         Csi::Event::registerType("Cora::Device::MemoryReceiver::on_fragment");


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
               receiver_type *receiver,
               client_type *client,
               outcome_type outcome)
            {
               try{(new event_complete(receiver,client,outcome))->post();}
               catch(Csi::Event::BadPost &) { }
            }

            ////////////////////////////////////////////////////////////
            // do_notify
            ////////////////////////////////////////////////////////////
            virtual void do_notify()
            { client->on_complete(receiver,outcome); }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               receiver_type *receiver,
               client_type *client,
               outcome_type outcome_):
               event_base(event_id,receiver,client),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::MemoryReceiver::on_complete");
      };


      ////////////////////////////////////////////////////////////
      // class MemortReceiver definitions
      ////////////////////////////////////////////////////////////
      MemoryReceiver::MemoryReceiver():
         state(state_standby),
         client(0)
      { }

      
      MemoryReceiver::~MemoryReceiver()
      { finish(); }

      
      void MemoryReceiver::start(
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

      
      void MemoryReceiver::start(
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

      
      void MemoryReceiver::finish()
      {
         client = 0;
         state = state_standby;
         DeviceBase::finish();
      } // finish

      
      void MemoryReceiver::receive(Csi::SharedPtr<Csi::Event> &ev)
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

      
      void MemoryReceiver::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::memory_receive_start_cmd);
         cmd.addUInt4(tran_no = ++last_tran_no);
         cmd.addUInt4(address);
         cmd.addUInt4(swath);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void MemoryReceiver::on_devicebase_failure(devicebase_failure_type failure)
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

      
      void MemoryReceiver::on_devicebase_session_failure()
      { event_complete::cpost(this,client,client_type::outcome_session_failed); }

      
      void MemoryReceiver::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::memory_receive_not)
            {
               uint4 tran_no;
               msg->readUInt4(tran_no);
               msg->readBStr(fragment_buffer);
               if(tran_no == this->tran_no)
                  event_fragment::cpost(this,client,fragment_buffer);
            }
            else if(msg->getMsgType() == Messages::memory_receive_stopped_not)
            {
               uint4 tran_no;
               uint4 reason;
               msg->readUInt4(tran_no);
               msg->readUInt4(reason);
               if(tran_no == this->tran_no)
               {
                  client_type::outcome_type outcome;
                  switch(reason)
                  {
                  case 1:
                     outcome = client_type::outcome_success;
                     break;

                  case 4:
                     outcome = client_type::outcome_logger_security_blocked;
                     break;
                     
                  case 5:
                     outcome = client_type::outcome_communication_failed;
                     break;

                  case 6:
                     outcome = client_type::outcome_communication_disabled;
                     break;

                  default:
                     outcome = client_type::outcome_unknown;
                     break;
                  }
                  event_complete::cpost(this,client,outcome);
               }
            }
            else
               DeviceBase::onNetMessage(rtr,msg);
         }
         else
            DeviceBase::onNetMessage(rtr,msg);
      } // onNetMessage 
   }; 
};

