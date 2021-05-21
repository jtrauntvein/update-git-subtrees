/* Cora.LgrNet.ModemTypeAdder.cpp

   Copyright (C) 2009, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 14 September 2009
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.ModemTypeAdder.h"
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
            typedef ModemTypeAdderClient client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type const outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(ModemTypeAdder *adder, client_type *client, outcome_type outcome)
            {
               event_complete *event = new event_complete(adder, client, outcome);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(ModemTypeAdder *adder, client_type *client_, outcome_type outcome_):
               Event(event_id, adder),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora:::LgrNet::ModemTypeAdder::event_complete"));
      };

      
      ////////////////////////////////////////////////////////////
      // class ModemTypeAdder definitions
      ////////////////////////////////////////////////////////////
      void ModemTypeAdder::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(client == event->client && client_type::is_valid_instance(client))
               client->on_complete(this, event->outcome);
         }
      } // receive

      
      void ModemTypeAdder::on_corabase_ready()
      {
         Csi::Messaging::Message command(net_session, Messages::modem_add_cmd);
         command.addUInt4(++last_tran_no);
         command.addWStr(type_name);
         command.addStr(reset);
         command.addStr(init);
         state = state_active;
         router->sendMessage(&command);
      } // on_corabase_ready

      
      void ModemTypeAdder::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_unknown);
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_failure_invalid_logon;
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
         event_complete::cpost(this, client, outcome);
      } // on_corabase_failure

      
      void ModemTypeAdder::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::modem_add_ack)
            {
               uint4 tran_no;
               uint4 response;
               client_type::outcome_type outcome(client_type::outcome_unknown);
               
               message->readUInt4(tran_no);
               message->readUInt4(response);
               switch(response)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 2:
                  outcome = client_type::outcome_failure_invalid_modem_name;
                  break;

               case 3:
                  outcome = client_type::outcome_failure_modem_already_exists;
                  break;

               case 4:
                  outcome = client_type::outcome_failure_network_locked;
                  break;
               }
               event_complete::cpost(this, client, outcome);
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage
   };
};

