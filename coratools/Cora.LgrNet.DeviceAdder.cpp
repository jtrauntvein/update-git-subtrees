/* Cora.LgrNet.DeviceAdder.cpp

   Copyright (C) 2000, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 13 November 2000
   Last Change: Wednesday 15 January 2020
   Last Commit: $Date: 2020-01-15 13:59:18 -0600 (Wed, 15 Jan 2020) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.DeviceAdder.h"
#include "Cora.LgrNet.Defs.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            DeviceAdder *adder;
            DeviceAdderClient *client;
            DeviceAdderClient::outcome_type outcome;

            void notify()
            { client->on_complete(adder,outcome); }

         private:
            event_complete(
               DeviceAdder *adder_,
               DeviceAdderClient *client_,
               DeviceAdderClient::outcome_type outcome_):
               Event(event_id,adder_),
               adder(adder_),
               client(client_),
               outcome(outcome_)
            { }

         public:
            static void create_and_post(
               DeviceAdder *adder,
               DeviceAdderClient *client,
               DeviceAdderClient::outcome_type outcome)
            {
               try {(new event_complete(adder,client,outcome))->post(); }
               catch(Event::BadPost &) { }
            }
         };


         uint4 const event_complete::event_id  =
         Csi::Event::registerType("Cora::LgrNet::event_complete");
      };


      DeviceAdder::DeviceAdder():
         anchor_code(anchor_before),
         device_type(DeviceTypes::unknown),
         client(0),
         state(state_standby)
      { }

      
      DeviceAdder::~DeviceAdder()
      { finish(); }

      
      void DeviceAdder::start(client_type *client_, router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
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
      } // start


      void DeviceAdder::start(client_type *client_, ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
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
      } // start

      
      void DeviceAdder::finish()
      {
         client = 0;
         state = state_standby;
         mapper.clear();
         ClientBase::finish();
      } // finish


      void DeviceAdder::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace Cora::LgrNet::DeviceAdderStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << "success";
            break;
            
         case client_type::outcome_invalid_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_session_broken:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::outcome_invalid_device_name:
            out << my_strings[strid_outcome_invalid_device_name];
            break;
            
         case client_type::outcome_unattachable:
            out << my_strings[strid_outcome_unattachable];
            break;
            
         case client_type::outcome_unsupported_device_type:
            out << my_strings[strid_outcome_unsupported_device_type];
            break;
            
         case client_type::outcome_network_locked:
            out << my_strings[strid_outcome_network_locked];
            break;

         case client_type::outcome_too_many_stations:
            out << my_strings[strid_outcome_too_many_stations];
            break;
            
         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_outcome

      
      void DeviceAdder::on_corabase_ready()
      {
         state = state_active;
         if(interface_version < Csi::VersionNumber("1.3.1.31"))
         {
            mapper.bind(new NetworkMapper);
            mapper->start(this,this);
         }
         else
            start_add(0);
      } // on_corabase_ready

      
      void DeviceAdder::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;

         case corabase_failure_session:
            outcome = client_type::outcome_session_broken;
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
         event_complete::create_and_post(this,client,outcome);
      } // on_corabase_failure

      
      void DeviceAdder::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete*>(ev.get_rep());
            finish();
            event->notify();
         }
      } // receive

      
      void DeviceAdder::onNetMessage(Csi::Messaging::Router *rtr, Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::add_device_ack)
            {
               uint4 tran_no;
               uint4 resp_code;
               client_type::outcome_type outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(resp_code);
               switch(resp_code)
               {
               case 0:
                  outcome = client_type::outcome_success;
                  break;
                  
               case 102:
                  outcome = client_type::outcome_invalid_device_name;
                  break;

               case 108:
                  outcome = client_type::outcome_unattachable;
                  break;
                  
               case 109:
                  outcome = client_type::outcome_unsupported_device_type;
                  break;

               case 110:
                  outcome = client_type::outcome_network_locked;
                  break;

               case 111:
                  outcome = client_type::outcome_too_many_stations;
                  break;

               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::create_and_post(this,client,outcome);
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void DeviceAdder::on_failure(
         NetworkMapper *mapper,
         failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case failure_invalid_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case failure_session_broken:
            outcome = client_type::outcome_session_broken;
            break;
            
         case failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case failure_server_security:
            outcome = client_type::outcome_server_security_blocked;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_failure

      
      void DeviceAdder::on_notify(
         NetworkMapper *mapper,
         uint4 network_map_version,
         uint4 agent_transaction_id,
         bool first_notification,
         uint4 device_count)
      {
         if(first_notification)
            start_add(network_map_version);
      } // on_notify


      void DeviceAdder::start_add(uint4 version)
      {
         Csi::Messaging::Message command(net_session, Messages::add_device_cmd);
         
         command.addUInt4(++last_tran_no);
         command.addUInt4(version);
         command.addWStr(anchor_name);
         command.addUInt4(anchor_code);
         command.addUInt4(device_type);
         command.addWStr(device_name);
         router->sendMessage(&command);
      } // start_add
   };
};
