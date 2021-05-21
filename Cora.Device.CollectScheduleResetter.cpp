/* Cora.Device.CollectScheduleResetter.cpp

   Copyright (C) 2001, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 09 January 2001
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 

*/

#pragma warning(disable:4786) // disable overlong symbols warning
#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.CollectScheduleResetter.h"

namespace Cora
{
   namespace Device
   {
      namespace CollectScheduleResetterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            CollectScheduleResetter *resetter;
            typedef CollectScheduleResetterClient client_type;
            client_type *client;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

         private:
            event_complete(
               CollectScheduleResetter *resetter_,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,resetter_),
               resetter(resetter_),
               client(client_),
               outcome(outcome_)
            { }

         public:
            static void create_and_post(
               CollectScheduleResetter *resetter_,
               client_type *client_,
               outcome_type outcome_)
            {
               try{ (new event_complete(resetter_,client_,outcome_))->post(); }
               catch(BadPost &) { }
            }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Device::CollectScheduleResetter::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class CollectScheduleResetter definitions
      ////////////////////////////////////////////////////////////
      CollectScheduleResetter::CollectScheduleResetter():
         client(0),
         state(state_standby)
      { }


      CollectScheduleResetter::~CollectScheduleResetter()
      { finish(); }


      void CollectScheduleResetter::set_start_now(bool start_now_)
      {
         if(state == state_standby)
            start_now = start_now_;
         else
            throw exc_invalid_state();
      } // set_start_now


      void CollectScheduleResetter::start(
         CollectScheduleResetterClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(CollectScheduleResetterClient::is_valid_instance(client_))
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


      void CollectScheduleResetter::start(
         CollectScheduleResetterClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(CollectScheduleResetterClient::is_valid_instance(client_))
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


      void CollectScheduleResetter::finish()
      {
         client = 0;
         state = state_standby;
         DeviceBase::finish();
      } // finish


      void CollectScheduleResetter::on_devicebase_ready()
      {
         Csi::Messaging::Message command(
            device_session,
            Cora::Device::Messages::collect_schedule_reset_cmd);

         command.addUInt4(++last_tran_no);
         command.addBool(start_now);
         router->sendMessage(&command);
         state = state_active;
      } // on_devicebase_ready


      void CollectScheduleResetter::on_devicebase_failure(devicebase_failure_type failure)
      {
         using namespace CollectScheduleResetterHelpers;
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

         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_devicebase_failure


      void CollectScheduleResetter::on_devicebase_session_failure()
      {
         using namespace CollectScheduleResetterHelpers;
         event_complete::create_and_post(this,client,client_type::outcome_session_failed);
      } // on_devicebase_session_failure


      void CollectScheduleResetter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::collect_schedule_reset_ack)
            {
               // read the message
               using namespace CollectScheduleResetterHelpers;
               uint4 tran_no;
               uint4 resp_code;

               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);

               // process the outcome
               client_type::outcome_type outcome;
               switch(resp_code)
               {
               case 1:
                  outcome = client_type::outcome_reset;
                  break;
                  
               case 2:
                  outcome = client_type::outcome_reset_but_disabled;
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


      void CollectScheduleResetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace CollectScheduleResetterHelpers; 
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            finish();
            if(client_type::is_valid_instance(event->client))
               event->client->on_complete(event->resetter,event->outcome);
         }
      } // receive 
   };
};
