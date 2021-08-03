/* Cora.LgrNet.SnapshotRestorer.cpp

   Copyright (C) 2004, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Thursday 23 December 2004
   Last Change: Monday 28 January 2019
   Last Commit: $Date: 2019-01-28 17:52:35 -0600 (Mon, 28 Jan 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.SnapshotRestorer.h"
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
            typedef SnapshotRestorer::client_type client_type;
            client_type *client;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;
            typedef client_type::results_type results_type;
            results_type results;

         private:
            event_complete(
               SnapshotRestorer *restorer,
               client_type *client_,
               outcome_type outcome_,
               results_type const &results_):
               Event(event_id,restorer),
               client(client_),
               outcome(outcome_),
               results(results_)
            { }

         public:
            static void cpost(
               SnapshotRestorer *restorer,
               client_type *client,
               outcome_type outcome,
               results_type const &results = results_type())
            {
               event_complete *event = 0;
               try
               {
                  event = new event_complete(restorer,client,outcome,results);
                  event->post();
               }
               catch(Event::BadPost &)
               { delete event; }
            }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::SnapshotRestorer::event_complete");
      };

      
      void SnapshotRestorer::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(event->client == client && client_type::is_valid_instance(client))
               client->on_complete(this,event->outcome,event->results);
         }
      } // receive


      void SnapshotRestorer::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace Cora::LgrNet::SnapshotRestorerStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << "success";
            break;
            
         case client_type::outcome_partial_success:
            out << my_strings[strid_outcome_partial_success];
            break;
            
         case client_type::outcome_invalid_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_session_broken:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_server_security_blocked:
            describe_failure(out, corabase_failure_security);
            break;
               
         case client_type::outcome_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         case client_type::outcome_invalid_file_name:
            out << my_strings[strid_outcome_invalid_file_name];
            break;
            
         case client_type::outcome_invalid_snapshot_version:
            out << my_strings[strid_outcome_invalid_snapshot_version];
            break;
            
         case client_type::outcome_corrupt_snapshot:
            out << my_strings[strid_outcome_corrupt_snapshot];
            break;
            
         case client_type::outcome_other_transactions:
            out << my_strings[strid_outcome_other_transactions];
            break;
            
         case client_type::outcome_network_locked:
            out << my_strings[strid_outcome_network_locked];
            break;
         }
      } // describe_outcome
      
      
      void SnapshotRestorer::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(
            net_session,
            Messages::restore_snapshot_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addStr(file_name);
         cmd.addBool(clear_before_restore);
         cmd.addInt4(comm_enabled_state);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      
      void SnapshotRestorer::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::restore_snapshot_ack)
            {
               uint4 tran_no;
               uint4 server_outcome;
               client_type::outcome_type outcome;
               uint4 results_count;
               client_type::results_type results;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(server_outcome);
               switch(server_outcome)
               {
               case 1:
               case 2:
                  if(server_outcome == 1)
                     outcome = client_type::outcome_success;
                  else
                     outcome = client_type::outcome_partial_success;
                  msg->readUInt4(results_count);
                  for(uint4 i = 0; i < results_count; ++i)
                  {
                     client_type::result_type result;
                     msg->readStr(result.first);
                     msg->readStr(result.second);
                     results.push_back(result);
                  }
                  break;

               case 4:
                  outcome = client_type::outcome_invalid_file_name;
                  break;
                  
               case 5:
                  outcome = client_type::outcome_invalid_snapshot_version;
                  break;
                  
               case 6:
                  outcome = client_type::outcome_corrupt_snapshot;
                  break;

               case 3:
               case 7:
                  outcome = client_type::outcome_other_transactions;
                  break;
                  
               case 8:
                  outcome = client_type::outcome_network_locked;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::cpost(this,client,outcome,results);
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void SnapshotRestorer::on_corabase_failure(corabase_failure_type failure)
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
         event_complete::cpost(this,client,outcome);
      } // on_corabase_failure
   };
};
