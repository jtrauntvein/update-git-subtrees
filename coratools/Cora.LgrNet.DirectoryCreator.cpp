/* Cora.LgrNet.DirectoryCreator.cpp

   Copyright (C) 2003, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Wednesday 22 October 2003
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.DirectoryCreator.h"
#include "Cora.LgrNet.Defs.h"


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
            typedef DirectoryCreator::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // failure_reason
            ////////////////////////////////////////////////////////////
            StrAsc failure_reason;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               DirectoryCreator *creator,
               client_type *client,
               outcome_type outcome,
               StrAsc const &failure_reason = "")
            {
               try{(new event_complete(creator,client,outcome,failure_reason))->post();}
               catch(Csi::Event::BadPost &) { }
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               DirectoryCreator *creator,
               client_type *client_,
               outcome_type outcome_,
               StrAsc const &failure_reason_):
               Event(event_id,creator),
               client(client_),
               outcome(outcome_),
               failure_reason(failure_reason_)
            { } 
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::LgrNet::DirectoryCreator::event_complete");
      };

      
      ////////////////////////////////////////////////////////////
      // class DirectoryCreator definitions
      ////////////////////////////////////////////////////////////
      void DirectoryCreator::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            if(client == event->client && client_type::is_valid_instance(client))
            {
               finish();
               event->client->on_complete(
                  this,
                  event->outcome,
                  event->failure_reason);
            }
         }
      } // receive

      
      void DirectoryCreator::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(
            net_session,
            Messages::create_directory_cmd);
         cmd.addUInt4(++last_tran_no);
         cmd.addStr(path_name);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      
      void DirectoryCreator::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active &&
            msg->getMsgType() == Messages::create_directory_ack)
         {
            uint4 tran_no;
            uint4 response;
            StrAsc failure_reason;
            client_type::outcome_type outcome = client_type::outcome_success;
            
            msg->readUInt4(tran_no);
            msg->readUInt4(response);
            if(response != 1)
            {
               msg->readStr(failure_reason);
               outcome = client_type::outcome_create_failed;
            }
            event_complete::cpost(this,client,outcome,failure_reason);
         }
         else
            ClientBase::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void DirectoryCreator::on_corabase_failure(corabase_failure_type failure)
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

      
      void DirectoryCreator::on_corabase_session_failure()
      {
         event_complete::cpost(this,client,client_type::outcome_session_broken); 
      } // on_corabase_session_failure
   };
};


