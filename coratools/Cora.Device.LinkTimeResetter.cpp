/* Cora.Device.LinkTimeResetter.cpp

   Copyright (C) 2008, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 07 January 2008
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Device.LinkTimeResetter.h"
#include "coratools.strings.h"


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
            static const uint4 event_id;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef LinkTimeResetter::client_type client_type;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // client
            ////////////////////////////////////////////////////////////
            client_type *client;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(LinkTimeResetter *resetter, client_type *client, outcome_type outcome)
            {
               event_complete *event = new event_complete(resetter, client, outcome);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(LinkTimeResetter *resetter, client_type *client_, outcome_type outcome_):
               Event(event_id, resetter),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::Device::LinkTimeResetter::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class LinkTimeResetter definitions
      ////////////////////////////////////////////////////////////
      LinkTimeResetter::LinkTimeResetter():
         state(state_standby),
         client(0)
      { }

      
      void LinkTimeResetter::start(client_type *client_, router_handle router)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("Invalid client pointer");
         client = client_;
         state = state_delegate;
         DeviceBase::start(router);
      } // start

      
      void LinkTimeResetter::start(client_type *client_, ClientBase *other_client)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("Invalid client pointer");
         client = client_;
         state = state_delegate;
         DeviceBase::start(other_client);
      } // start


      void LinkTimeResetter::finish()
      {
         state = state_standby;
         DeviceBase::finish();
      } // finish


      void LinkTimeResetter::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace LinkTimeResetterStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_invalid_logon:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_logon);
            break;
            
         case client_type::outcome_invalid_security:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_security);
            break;
            
         case client_type::outcome_invalid_device_name:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_invalid_device_name);
            break;
            
         case client_type::outcome_session_lost:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_session);
            break;
            
         case client_type::outcome_unsupported:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_unsupported);
            break;
            
         default:
            DeviceBase::format_devicebase_failure(out, devicebase_failure_unknown);
            break;
         }
      } // format_outcome

      
      void LinkTimeResetter::on_devicebase_ready()
      {
         Csi::Messaging::Message cmd(device_session, Messages::reset_link_time_remain_cmd);
         cmd.addUInt4(++last_tran_no); 
         state = state_active;
         router->sendMessage(&cmd);
      } // on_devicebase_ready

      
      void LinkTimeResetter::on_devicebase_failure(devicebase_failure_type failure)
      {
         client_type::outcome_type outcome = client_type::outcome_unknown;
         switch(failure)
         {
         case devicebase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case devicebase_failure_session:
            outcome = client_type::outcome_session_lost;
            break;
            
         case devicebase_failure_invalid_device_name:
            outcome = client_type::outcome_invalid_device_name;
            break;

         case devicebase_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case devicebase_failure_security:
            outcome = client_type::outcome_invalid_security;
            break;
         }
         event_complete::cpost(this, client, outcome);
      } // on_devicebase_failure

      
      void LinkTimeResetter::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::reset_link_time_remain_ack)
            {
               uint4 tran_no;
               uint4 response;
               message->readUInt4(tran_no);
               message->readUInt4(response);
               if(response == 1)
                  event_complete::cpost(this, client, client_type::outcome_success);
               else
                  event_complete::cpost(this, client, client_type::outcome_unknown);
            }
            else
               DeviceBase::onNetMessage(router, message);
         }
         else
            DeviceBase::onNetMessage(router, message);
      } // onNetMessage

      
      void LinkTimeResetter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client)
            {
               finish();
               if(client_type::is_valid_instance(event->client))
                  event->client->on_complete(this, event->outcome);
            }
         }
      } // receive
   };
};
