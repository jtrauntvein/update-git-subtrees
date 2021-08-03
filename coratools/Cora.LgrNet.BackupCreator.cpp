/* Cora.LgrNet.BackupCreator.cpp

   Copyright (C) 2004, 2016 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 December 2004
   Last Change: Friday 29 April 2016
   Last Commit: $Date: 2019-10-30 16:17:59 -0600 (Wed, 30 Oct 2019) $ 
   Last Changed by: $Author: amortenson $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.BackupCreator.h"
#include "Cora.LgrNet.Defs.h"
#include "coratools.strings.h"
#include <iterator>


namespace Cora
{
   namespace LgrNet
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
            typedef BackupCreator::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // file_name
            ////////////////////////////////////////////////////////////
            StrAsc file_name;

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               BackupCreator *creator,
               client_type *client_,
               outcome_type outcome_,
               StrAsc const &file_name_):
               Event(event_id,creator),
               client(client_),
               outcome(outcome_),
               file_name(file_name_)
            { }

         public:
            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               BackupCreator *creator,
               client_type *client,
               outcome_type outcome,
               StrAsc const &file_name = "")
            {
               event_complete *event = 0;
               try
               {
                  event = new event_complete(creator,client,outcome,file_name);
                  event->post();
               }
               catch(Event::BadPost &)
               { delete event; }
            }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::BackupCreator::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class BackupCreator definitions
      ////////////////////////////////////////////////////////////
      void BackupCreator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(event->client == client && client_type::is_valid_instance(client))
               client->on_complete(this,event->outcome,event->file_name);
         }
      } // receive

      
      void BackupCreator::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(
            net_session,
            Messages::create_backup_file_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addStr(file_name);
         cmd.addBool(include_table_files);
         cmd.addUInt4((uint4)additional_files.size());
         for(additional_files_type::const_iterator adi = additional_files.begin();
             adi != additional_files.end();
             ++adi)
            cmd.addStr(*adi);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      
      void BackupCreator::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::create_backup_file_ack)
            {
               uint4 tran_no;
               uint4 server_outcome;
               StrAsc created_file_name;
               client_type::outcome_type outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(server_outcome);
               switch(server_outcome)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  msg->readStr(created_file_name);
                  break;
                  
               case 2:
                  outcome = client_type::outcome_invalid_file_name;
                  break;
                  
               case 3:
                  outcome = client_type::outcome_no_resources;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::cpost(this,client,outcome,created_file_name);
            }
            else
               ClientBase::onNetMessage(rtr,msg);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage

      void BackupCreator::format_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace Cora::LgrNet::BackupCreatorStrings;
         switch(outcome)
         {
            default:
            case client_type::outcome_unknown:
               ClientBase::describe_failure(out, corabase_failure_unknown);
               break;
            case client_type::outcome_success:
               out << "success";
               break;
            case client_type::outcome_invalid_logon:
               ClientBase::describe_failure(out, corabase_failure_logon);
               break;
            case client_type::outcome_session_broken:
               ClientBase::describe_failure(out, corabase_failure_session);
               break;
            case client_type::outcome_server_security_blocked:
               ClientBase::describe_failure(out, corabase_failure_security);
               break;
            case client_type::outcome_unsupported:
               ClientBase::describe_failure(out, corabase_failure_unsupported);
               break;
            case client_type::outcome_invalid_file_name:
               out << my_strings[strid_outcome_invalid_file_name];
               break;
            case client_type::outcome_no_resources:
               out << my_strings[strid_outcome_no_resources];
         }
      } // format_outcome
      
      void BackupCreator::on_corabase_failure(corabase_failure_type failure)
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



