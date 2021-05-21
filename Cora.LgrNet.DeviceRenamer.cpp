/* Cora.LgrNet.DeviceRenamer.cpp

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 13 June 2001
   Last Change: Tuesday 08 October 2019
   Last Commit: $Date: 2019-10-16 11:44:34 -0600 (Wed, 16 Oct 2019) $ 
   Committed by: $Author: jon $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.DeviceRenamer.h"
#include "Cora.LgrNet.Defs.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace DeviceRenamerHelpers
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef DeviceRenamerClient::outcome_type outcome_type;
            outcome_type outcome;
            DeviceRenamerClient *client;

         private:
            event_complete(
               DeviceRenamer *renamer_,
               DeviceRenamerClient *client_,
               outcome_type outcome_):
               Event(event_id,renamer_),
               client(client_),
               outcome(outcome_)
            { }

         public:
            static void create_and_post(
               DeviceRenamer *renamer,
               DeviceRenamerClient *client,
               outcome_type outcome)
            {
               try {(new event_complete(renamer,client,outcome))->post(); }
               catch(Event::BadPost &) { }
            } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::DeviceRenamer::event_complete");
      };


      DeviceRenamer::DeviceRenamer():
         client(0),
         state(state_standby)
      { }


      DeviceRenamer::~DeviceRenamer()
      { finish(); }
      
         
      void DeviceRenamer::set_device_name(StrUni const &device_name_)
      {
         if(state == state_standby)
            device_name = device_name_;
         else
            throw exc_invalid_state();
      } // set_device_name


      void DeviceRenamer::set_new_device_name(StrUni const &new_device_name_)
      {
         if(state == state_standby)
            new_device_name = new_device_name_;
         else
            throw exc_invalid_state();
      } // set_new_device_name

      
      void DeviceRenamer::start(
         DeviceRenamerClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(DeviceRenamerClient::is_valid_instance(client_))
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


      void DeviceRenamer::start(
         DeviceRenamerClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(DeviceRenamerClient::is_valid_instance(client_))
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

      
      void DeviceRenamer::finish()
      {
         client = 0;
         state = state_standby;
         ClientBase::finish();
      } // finish


      void DeviceRenamer::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace DeviceRenamerStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_invalid_logon:
            ClientBase::describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_session_broken:
            ClientBase::describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_unsupported:
            ClientBase::describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_server_security_blocked:
            ClientBase::describe_failure(out, corabase_failure_security);
            break;
            
         case client_type::outcome_invalid_device_name:
            out << my_strings[strid_outcome_failure_invalid_device_name];
            break;
            
         case client_type::outcome_device_online:
            out << my_strings[strid_outcome_failure_device_online];
            break;
            
         case client_type::outcome_network_locked:
            out << my_strings[strid_outcome_failure_locked];
            break;

         default:
            ClientBase::describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_outcome

      
      void DeviceRenamer::on_corabase_ready()
      {
         if(interface_version < Csi::VersionNumber("1.3.1.31"))
         {
            Csi::Messaging::Message command(
               net_session,
            Messages::network_map_enum_cmd);
            router->sendMessage(&command);
            state = state_get_network_map;
         }
         else
            start_rename(0);
      } // on_corabase_ready

      
      void DeviceRenamer::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace DeviceRenamerHelpers;
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

      
      void DeviceRenamer::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace DeviceRenamerHelpers;
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(event->client == client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(this,event->outcome);
            }
            else
               finish();
         }
      } // receive

      
      void DeviceRenamer::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active || state == state_get_network_map)
         {
            switch(msg->getMsgType())
            {
            case Messages::network_map_advise_not:
               on_network_map_enum_not(msg);
               break;
               
            case Messages::rename_device_ack:
               on_rename_device_ack(msg);
               break;

            default:
               ClientBase::onNetMessage(rtr,msg);
            }
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void DeviceRenamer::on_network_map_enum_not(Csi::Messaging::Message *message)
      {
         if(state == state_get_network_map)
         {
            // all we need is the network map version
            uint4 net_map_version;
            message->readUInt4(net_map_version);
            start_rename(net_map_version);
         }
      } // on_network_map_enum_not


      void DeviceRenamer::on_rename_device_ack(Csi::Messaging::Message *message)
      {
         // read the response
         using namespace DeviceRenamerHelpers;
         uint4 tran_no;
         uint4 server_outcome;

         message->readUInt4(tran_no);
         message->readUInt4(server_outcome);

         // relay the outcome to the client
         client_type::outcome_type client_outcome;
         switch(server_outcome)
         {
         case 0:
            client_outcome = client_type::outcome_success;
            break;

         case 103:
            client_outcome = client_type::outcome_invalid_device_name;
            break;

         case 104:
            client_outcome = client_type::outcome_device_online;
            break;

         case 105:
            client_outcome = client_type::outcome_network_locked;
            break;
            
         default:
            client_outcome = client_type::outcome_unknown;
            break;
         }
         event_complete::create_and_post(this,client,client_outcome);
      } // on_delete_branch_ack


      void DeviceRenamer::start_rename(uint4 version)
      {
         Csi::Messaging::Message command(
            net_session,
            Messages::rename_device_cmd);
         
         command.addUInt4(++last_tran_no);
         command.addUInt4(version);
         command.addWStr(device_name);
         command.addWStr(new_device_name);
         router->sendMessage(&command);
         state = state_active;
      } // start_rename
   };
};
