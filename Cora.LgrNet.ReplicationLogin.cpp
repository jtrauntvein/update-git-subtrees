/* Cora.LgrNet.ReplicationLogin.cpp

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Thursday 15 October 2020
   Last Change: Tuesday 03 November 2020
   Last Commit: $Date: 2020-11-03 11:29:38 -0600 (Tue, 03 Nov 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.ReplicationLogin.h"
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
            typedef ReplicationLoginClient::outcome_type outcome_type;
            outcome_type const outcome;

            static void cpost(ReplicationLogin *sender, outcome_type outcome)
            { (new event_complete(sender, outcome))->post(); }

         private:
            event_complete(ReplicationLogin *sender, outcome_type outcome_):
               Event(event_id, sender),
               outcome(outcome_)
            { }
         };
         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::LgrNet::ReplicationLogin::event_complete"));
      };

      void ReplicationLogin::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event((event_complete *)ev.get_rep());
            client_type *notify(client);
            finish();
            if(client_type::is_valid_instance(notify))
               notify->on_complete(this, event->outcome);
         }
      } // receive

      void ReplicationLogin::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace ReplicationLoginStrings;
         switch(outcome)
         {
         case client_type::outcome_failure_unknown:
         default:
            describe_failure(out, corabase_failure_unknown);
            break;

         case client_type::outcome_success:
            out << "success";
            break;

         case client_type::outcome_failure_logon:
            describe_failure(out, corabase_failure_logon);
            break;

         case client_type::outcome_failure_session:
            describe_failure(out, corabase_failure_session);
            break;

         case client_type::outcome_failure_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;

         case client_type::outcome_failure_security:
            describe_failure(out, corabase_failure_security);
            break;

         case client_type::outcome_failure_cloud_comms:
            out << my_strings[strid_outcome_failure_cloud_comms];
            break;

         case client_type::outcome_failure_cloud:
            out << my_strings[strid_outcome_failure_cloud];
            break;

         case client_type::outcome_failure_disabled:
            out << "replication engine is disabled";
            break;

         case client_type::outcome_failure_busy:
            out << "busy with another login attempt";
            break;
         }
      } // describe_outcome

      void ReplicationLogin::on_corabase_ready()
      {
         Csi::Messaging::Message msg(net_session, Messages::replication_login_cmd);
         msg.addUInt4(++last_tran_no);
         msg.addStr(base_uri);
         msg.addWStr(user_name);
         msg.addWStr(password);
         state = state_active;
         router->sendMessage(&msg);
      } // on_corabase_ready

      void ReplicationLogin::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_failure_unknown);
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_failure_logon;
            break;

         case corabase_failure_session:
            outcome = client_type::outcome_failure_session;
            break;

         case corabase_failure_unsupported:
            outcome = client_type::outcome_failure_unsupported;
            break;

         case corabase_failure_security:
            outcome = client_type::outcome_failure_security;
            break;
         }
         event_complete::cpost(this, outcome);
      } // on_corabase_failure

      void ReplicationLogin::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::replication_login_ack)
            {
               uint4 tran_no, response;
               client_type::outcome_type outcome(client_type::outcome_failure_unknown);
               message->readUInt4(tran_no);
               message->readUInt4(response);
               assert(tran_no == last_tran_no);
               switch(response)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 3:
                  outcome = client_type::outcome_failure_cloud_comms;
                  break;

               case 4:
                  outcome = client_type::outcome_failure_cloud;
                  break;

               case 5:
                  outcome = client_type::outcome_failure_busy;
                  break;
                  
               case 6:
                  outcome = client_type::outcome_failure_disabled;
                  break;
               }
               event_complete::cpost(this, outcome);
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage
   };
};

