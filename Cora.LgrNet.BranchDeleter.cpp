/* Cora.LgrNet.BranchDeleter.cpp

   Copyright (C) 2000, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 05 December 2000
   Last Change: Wednesday 16 October 2019
   Last Commit: $Date: 2019-10-29 13:45:27 -0600 (Tue, 29 Oct 2019) $ 
   Committed by: $Author: amortenson $
   
*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.BranchDeleter.h"
#include "Cora.LgrNet.Defs.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace LgrNet
   {
      namespace BranchDeleterHelpers
      {
         ////////////////////////////////////////////////////////////
         // class event_complete
         ////////////////////////////////////////////////////////////
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef BranchDeleterClient::outcome_type outcome_type;
            outcome_type outcome;
            BranchDeleterClient *client;
            BranchDeleter *deleter;

         private:
            event_complete(
               BranchDeleter *deleter_,
               BranchDeleterClient *client_,
               outcome_type outcome_):
               Event(event_id,deleter_),
               deleter(deleter_),
               client(client_),
               outcome(outcome_)
            { }

         public:
            static void create_and_post(
               BranchDeleter *deleter,
               BranchDeleterClient *client,
               outcome_type outcome)
            {
               try {(new event_complete(deleter,client,outcome))->post(); }
               catch(Event::BadPost &) { }
            } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::BranchDeleter::event_complete");
      };


      ////////////////////////////////////////////////////////////
      // class BranchDeleter definitions
      ////////////////////////////////////////////////////////////
      BranchDeleter::BranchDeleter():
         client(0),
         state(state_standby)
      { }


      BranchDeleter::~BranchDeleter()
      { finish(); }
      
         
      void BranchDeleter::set_device_name(StrUni const &device_name_)
      {
         if(state == state_standby)
            device_name = device_name_;
         else
            throw exc_invalid_state();
      } // set_device_name

      
      void BranchDeleter::start(
         BranchDeleterClient *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(BranchDeleterClient::is_valid_instance(client_))
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


      void BranchDeleter::start(
         BranchDeleterClient *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(BranchDeleterClient::is_valid_instance(client_))
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

      
      void BranchDeleter::finish()
      {
         client = 0;
         state = state_standby;
         ClientBase::finish();
      } // finish


      void BranchDeleter::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace DeviceDeleterStrings;
         switch(outcome)
         {
         case client_type::outcome_unknown:
            describe_failure(out, corabase_failure_unknown);
            break;

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
            
         case client_type::outcome_device_online:
            out << my_strings[strid_outcome_device_online];
            break;
            
         case client_type::outcome_network_locked:
            out << my_strings[strid_outcome_network_locked];
            break;
         }
      } // describe_outcome

      
      void BranchDeleter::on_corabase_ready()
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
            start_delete(0);
      } // on_corabase_ready

      
      void BranchDeleter::on_corabase_failure(corabase_failure_type failure)
      {
         using namespace BranchDeleterHelpers;
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

      
      void BranchDeleter::on_corabase_session_failure()
      {
         using namespace BranchDeleterHelpers;
         event_complete::create_and_post(this,client,client_type::outcome_session_broken);
      } // on_corabase_session_failure

      
      void BranchDeleter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         using namespace BranchDeleterHelpers;
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

      
      void BranchDeleter::onNetMessage(
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
               
            case Messages::delete_branch_ack:
               on_delete_branch_ack(msg);
               break;

            default:
               ClientBase::onNetMessage(rtr,msg);
            }
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage


      void BranchDeleter::on_network_map_enum_not(Csi::Messaging::Message *message)
      {
         if(state == state_get_network_map)
         {
            // all we need is the network map version
            uint4 net_map_version;
            message->readUInt4(net_map_version);
            start_delete(net_map_version);
         }
      } // on_network_map_enum_not


      void BranchDeleter::on_delete_branch_ack(Csi::Messaging::Message *message)
      {
         // read the response
         using namespace BranchDeleterHelpers;
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

         case 102:
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


      void BranchDeleter::start_delete(uint4 version)
      {
         Csi::Messaging::Message command(
            net_session,
            Messages::delete_branch_cmd);
         
         command.addUInt4(++last_tran_no);
         command.addUInt4(version);
         command.addWStr(device_name);
         router->sendMessage(&command);
         state = state_active;
      } // start_delete
   };
};
