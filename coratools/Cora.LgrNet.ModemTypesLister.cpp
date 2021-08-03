/* Cora.LgrNet.ModemTypesLister.cpp

   Copyright (C) 2009, 2009 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 14 September 2009
   Last Change: Friday 23 October 2009
   Last Commit: $Date: 2009-10-23 14:41:01 -0600 (Fri, 23 Oct 2009) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.ModemTypesLister.h"
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
            typedef ModemTypesLister::client_type client_type;
            client_type *client;

            ////////////////////////////////////////////////////////////
            // outcome
            ////////////////////////////////////////////////////////////
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            ////////////////////////////////////////////////////////////
            // cpost
            ////////////////////////////////////////////////////////////
            static void cpost(
               ModemTypesLister *lister, client_type *client, outcome_type outcome)
            {
               event_complete *event = new event_complete(lister, client, outcome);
               event->post();
            }

         private:
            ////////////////////////////////////////////////////////////
            // constructor
            ////////////////////////////////////////////////////////////
            event_complete(
               ModemTypesLister *lister, client_type *client_, outcome_type outcome_):
               Event(event_id, lister),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::LgrNet::ModemTypesLister::event_complete"));
      };


      ////////////////////////////////////////////////////////////
      // class ModemTypesLister definitions
      ////////////////////////////////////////////////////////////
      ModemTypesLister::ModemTypesLister():
         client(0),
         state(state_standby)
      { }


      ModemTypesLister::~ModemTypesLister()
      { finish(); }


      void ModemTypesLister::start(
         client_type *client_, ClientBase::router_handle &router)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client object");
         modems.clear();
         client = client_;
         state = state_delegate;
         ClientBase::start(router);
      } // start


      void ModemTypesLister::start(
         client_type *client_, ClientBase *other_component)
      {
         if(state != state_standby)
            throw exc_invalid_state();
         if(!client_type::is_valid_instance(client_))
            throw std::invalid_argument("invalid client object");
         modems.clear();
         client = client_;
         state = state_delegate;
         ClientBase::start(other_component);
      } // start


      void ModemTypesLister::finish()
      {
         client = 0;
         state = state_standby;
         ClientBase::finish();
      } // finish


      void ModemTypesLister::describe_outcome(
         std::ostream &out, client_type::outcome_type outcome)
      {
         switch(outcome)
         {
         case client_type::outcome_invalid_logon:
            describe_failure(out, corabase_failure_logon);
            break;
            
         case client_type::outcome_session_broken:
            describe_failure(out, corabase_failure_session);
            break;
            
         case client_type::outcome_unsupported:
            describe_failure(out, corabase_failure_unsupported);
            break;
            
         default:
            describe_failure(out, corabase_failure_unknown);
            break;
         }
      } // describe_outcome


      void ModemTypesLister::receive(Csi::SharedPtr<Csi::Event> &ev)
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


      void ModemTypesLister::on_corabase_ready()
      {
         Csi::Messaging::Message command(net_session, Messages::modems_enum_cmd);
         command.addUInt4(++last_tran_no);
         state = state_active;
         router->sendMessage(&command);
      } // on_corabase_ready


      void ModemTypesLister::on_corabase_failure(
         corabase_failure_type failure)
      {
         client_type::outcome_type outcome = client_type::outcome_unknown;
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
         }
         event_complete::cpost(this, client, outcome);
      } // on_corabase_failure


      void ModemTypesLister::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::modems_enum_ack)
               on_modems_enum_ack(message);
            else if(message->getMsgType() == Messages::modem_get_ack)
               on_modem_get_ack(message);
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage


      void ModemTypesLister::on_modems_enum_ack(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         uint4 count;
         StrUni name;
         message->readUInt4(tran_no);
         message->readUInt4(count);
         waiting.clear();
         modems.clear();
         for(uint4 i = 0; i < count; ++i)
         {
            message->readWStr(name);
            waiting.push_back(name);
         }
         get_next_modem(false);
      } // on_modems_enum_ack


      void ModemTypesLister::on_modem_get_ack(Csi::Messaging::Message *message)
      {
         uint4 tran_no;
         bool found;
         message->readUInt4(tran_no);
         message->readBool(found);
         if(found && !waiting.empty())
         {
            ModemType modem;
            modem.type_name = waiting.front();
            message->readStr(modem.reset);
            message->readStr(modem.init);
            message->readBool(modem.custom);
            modems.push_back(modem);
         }
         get_next_modem(true);
      } // on_modem_get_ack


      void ModemTypesLister::get_next_modem(bool pop_first)
      {
         if(!waiting.empty() && pop_first)
            waiting.pop_front();
         if(!waiting.empty())
         {
            Csi::Messaging::Message command(net_session, Messages::modem_get_cmd);
            command.addUInt4(++last_tran_no);
            command.addWStr(waiting.front());
            router->sendMessage(&command);
         }
         else
            event_complete::cpost(this, client, client_type::outcome_success);
      } // get_next_modem
   };
};
