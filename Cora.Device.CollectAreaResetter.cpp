/* Cora.Device.CollectAreaResetter.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 09 July 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectAreaResetter.h"


namespace Cora
{
   namespace Device
   {
      namespace
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            ////////////////////////////////////////////////////////////
            // event_id
            ////////////////////////////////////////////////////////////
            static uint4 const event_id;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            typedef CollectAreaResetterClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               CollectAreaResetter *resetter,
               client_type *client,
               outcome_type outcome)
            {
               try {(new event_complete(resetter,client,outcome))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               CollectAreaResetter *resetter,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,resetter),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::CollectAreaResetter::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class CollectAreaResetter definitions
      ////////////////////////////////////////////////////////////
      void CollectAreaResetter::start(
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
               throw std::invalid_argument("invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void CollectAreaResetter::start(
         client_type *client_,
         ClientBase *other)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               DeviceBase::start(other);
            }
            else
               throw std::invalid_argument("invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void CollectAreaResetter::finish()
      {
         client = 0;
         state = state_standby;
         DeviceBase::finish();
      } // finish

      
      void CollectAreaResetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_complete(this,event->outcome);
         }
      } // receive

      
      void CollectAreaResetter::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(
            device_session,
            Messages::collect_area_reset_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addWStr(collect_area_name);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void CollectAreaResetter::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
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

         case devicebase_failure_session:
            outcome = client_type::outcome_session_failed;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::cpost(this,client,outcome);
      } // on_devicebase_failure

      
      void CollectAreaResetter::on_devicebase_session_failure()
      { on_devicebase_failure(devicebase_failure_session); }

      
      void CollectAreaResetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::collect_area_reset_ack)
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
                  outcome = client_type::outcome_invalid_area_name;
                  break;

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

