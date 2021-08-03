/* Cora.LgrNet.DeviceRelationsDescriber.cpp

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Andrew Mortenson
   Date Begun: Friday 25 October 2019
   Last Change: Tuesday 29 October 2019
   Last Commit: $Date: 2019-10-29 12:35:46 -0600 (Tue, 29 Oct 2019) $
   Committed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.DeviceRelationsDescriber.h"
#include "Cora.LgrNet.Defs.h"

namespace Cora
{
   namespace LgrNet
   {
      namespace DeviceRelationsDescriberHelpers
      {
         class event_complete:public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef DeviceRelationsDescriberClient::outcome_type outcome_type;
            outcome_type outcome;
            DeviceRelationsDescriberClient *client;
            DeviceRelationsDescriber *describer;
            typedef DeviceRelationsDescriberClient::relations_type relations_type;
            relations_type const device_relations;

         private:
            event_complete(
               DeviceRelationsDescriber *describer_,
               DeviceRelationsDescriberClient *client_,
               outcome_type outcome_,
               relations_type const &relations_):
               Event(event_id, describer_),
               describer(describer_),
               client(client_),
               outcome(outcome_),
               device_relations(relations_)
            {}

         public:
            static void create_and_post(
               DeviceRelationsDescriber *describer,
               DeviceRelationsDescriberClient *client,
               outcome_type outcome,
               relations_type const &relations = relations_type())
            {
               try { (new event_complete(describer, client, outcome, relations))->post(); }
               catch(Event::BadPost &) {}
            }
         };

         uint4 const event_complete::event_id =
            Csi::Event::registerType("Cora::LgrNet::DeviceRelationsDescriber::event_complete");
      };
      

      DeviceRelationsDescriber::DeviceRelationsDescriber():
         client(0),
         state(state_standby)
      {}

      DeviceRelationsDescriber::~DeviceRelationsDescriber()
      { finish(); }

      void DeviceRelationsDescriber::start(client_type * client_, router_handle & router)
      {
         if(state == state_standby)
         {
            if(DeviceRelationsDescriberClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               ClientBase::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer specified");
         }
         else
            throw exc_invalid_state();
      }

      void DeviceRelationsDescriber::start(client_type * client_, ClientBase * other_component)
      {
         if(state == state_standby)
         {
            if(DeviceRelationsDescriberClient::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               ClientBase::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer specified");
         }
         else
            throw exc_invalid_state();
      }

      void DeviceRelationsDescriber::on_corabase_ready()
      {
         Csi::Messaging::Message command(net_session, Messages::describe_device_relations_ex_cmd);
         command.addUInt4(++last_tran_no);
         state = state_active;
         router->sendMessage(&command);
      }

      void DeviceRelationsDescriber::on_describe_device_relations_ack(Csi::Messaging::Message * message)
      {
         using namespace DeviceRelationsDescriberHelpers;
         uint4 tran_no;
         uint4 devicesCount;
         client_type::relations_type deviceRelations;
         
         message->readUInt4(tran_no);
         message->readUInt4(devicesCount);
         for(uint4 count = 0; count < devicesCount; count++)
         {
            device_relations_type deviceRelation;
            message->readUInt4(deviceRelation.deviceType);
            uint4 ddCount;
            message->readUInt4(ddCount);
            for(uint4 DDcount = 0; DDcount < ddCount; DDcount++)
            {
               uint4 ddType;
               message->readUInt4(ddType);
               deviceRelation.disallowedDescendants.push_back(ddType);
            }
            uint4 slotsCount;
            message->readUInt4(slotsCount);
            for(uint4 Scount = 0; Scount < slotsCount; Scount++)
            {
               allowed_child_type allowedChild;
               uint4 allowedChildCount;
               message->readUInt4(allowedChildCount);
               for(uint4 ACcount = 0; ACcount < allowedChildCount; ACcount++)
               {
                  uint4 allowedChildType;
                  message->readUInt4(allowedChildType);
                  allowedChild.allowedTypes.push_back(allowedChildType);
               }
               message->readUInt4(allowedChild.maxCount);
               message->readUInt4(allowedChild.maxDepth);
               deviceRelation.slots.push_back(allowedChild);
            }
            deviceRelations.push_back(deviceRelation);
         }
         event_complete::create_and_post(this, client, client_type::outcome_success, deviceRelations);
      }

      void DeviceRelationsDescriber::onNetMessage(Csi::Messaging::Router * rtr, Csi::Messaging::Message * msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::describe_device_relations_ex_ack)
               on_describe_device_relations_ack(msg);
            else
               ClientBase::onNetMessage(rtr, msg);
         }
         else
            ClientBase::onNetMessage(rtr, msg);
      }

      void DeviceRelationsDescriber::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace DeviceRelationsDescriberHelpers;
         client_type::outcome_type outcome;
         switch(failure)
         {
            case corabase_failure_logon:
               outcome = client_type::outcome_invalid_logon;
               break;
            case corabase_failure_session:
               outcome = client_type::outcome_session_failed;
               break;
            case corabase_failure_unsupported:
               outcome = client_type::outcome_unsupported;
               break;
            case corabase_failure_security:
               outcome = client_type::outcome_server_security_blocked;
               break;
            default:
               outcome = client_type::outcome_unknown;
               break;
         }

         event_complete::create_and_post(this, client, outcome);
      }

      void DeviceRelationsDescriber::finish()
      {
         client = 0;
         state = state_standby;
         ClientBase::finish();
      }


      void DeviceRelationsDescriber::receive(Csi::SharedPtr<Csi::Event> & ev)
      {
         using namespace DeviceRelationsDescriberHelpers;
         if(ev->getType() == event_complete::event_id) 
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this, event->outcome, event->device_relations);
            }
            else
               finish();
         }
      }

      void DeviceRelationsDescriber::describe_outcome(std::ostream & out, client_type::outcome_type outcome)
      {

         switch(outcome)
         {
            case client_type::outcome_success:
               out << "success";
               break;
            case client_type::outcome_session_failed:
               describe_failure(out, corabase_failure_session);
               break;
            case client_type::outcome_invalid_logon:
               describe_failure(out, corabase_failure_logon);
               break;
            case client_type::outcome_server_security_blocked:
               describe_failure(out, corabase_failure_security);
               break;
            case client_type::outcome_unsupported:
               describe_failure(out, corabase_failure_unsupported);
               break;
            default:
               describe_failure(out, corabase_failure_unknown);
               break;
         }
      }
   };
};


