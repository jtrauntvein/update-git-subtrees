/* Cora.LgrNet.AccessTokenGetter.cpp

   Copyright (C) 2020, 2020 Campbell Scientific, Inc.

   Written by: Jon Trauntvein 
   Date Begun: Tuesday 15 December 2020
   Last Change: Tuesday 15 December 2020
   Last Commit: $Date: 2020-12-17 12:40:37 -0600 (Thu, 17 Dec 2020) $
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.LgrNet.AccessTokenGetter.h"
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
            typedef AccessTokenGetterClient::outcome_type outcome_type;
            outcome_type const outcome;
            StrAsc const access_token;
            StrAsc const refresh_token;

            static void cpost(
               AccessTokenGetter *sender,
               outcome_type outcome,
               StrAsc const access_token = "",
               StrAsc const refresh_token = "")
            { (new event_complete(sender, outcome, access_token, refresh_token))->post(); }

         private:
            event_complete(
               AccessTokenGetter *sender,
               outcome_type outcome_,
               StrAsc const &access_token_,
               StrAsc const &refresh_token_):
               Event(event_id, sender),
               outcome(outcome_),
               access_token(access_token_),
               refresh_token(refresh_token_)
            { }
         };
         uint4 const event_complete::event_id(
            Csi::Event::registerType("Cora::LgrNet::AccessTokenGetter::event_complete"));
      };

      void AccessTokenGetter::receive(event_handle &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event((event_complete *)ev.get_rep());
            client_type *target(client);
            finish();
            if(client_type::is_valid_instance(target))
               target->on_complete(this, event->outcome, event->access_token, event->refresh_token);
         }
      } // receive

      void AccessTokenGetter::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace AccessTokenGetterStrings;
         switch(outcome)
         {
         default:
            describe_failure(out, corabase_failure_unknown);
            break;

         case client_type::outcome_success:
            out << my_strings[strid_success];
            break;

         case client_type::outcome_failure_no_account:
            out << my_strings[strid_failure_no_account];
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
           
         case client_type::outcome_failure_invalid_access:
            describe_failure(out, corabase_failure_invalid_access);
            break;
            
         case client_type::outcome_failure_access_expired:
            describe_failure(out, corabase_failure_access_expired);
            break;
            
         case client_type::outcome_failure_invalid_refresh:
            out << my_strings[strid_failure_invalid_refresh];
            break;
            
         case client_type::outcome_failure_refresh_expired:
            out << my_strings[strid_failure_refresh_expired];
            break;
         }
      } // describe_outcome

      void AccessTokenGetter::on_corabase_ready()
      {
         Csi::Messaging::Message cmd(net_session, Messages::get_access_token_cmd);
         cmd.addUInt4(++last_tran_no);
         if(refresh_token.length())
            cmd.addStr(refresh_token);
         state = state_active;
         router->sendMessage(&cmd);
      } // on_corabase_ready

      void AccessTokenGetter::on_corabase_failure(corabase_failure_type failure)
      {
         client_type::outcome_type outcome(client_type::outcome_failure_unknown);
         switch(failure)
         {
         case corabase_failure_logon:
            outcome = client_type::outcome_failure_no_account;
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

         case corabase_failure_invalid_access:
            outcome = client_type::outcome_failure_invalid_access;
            break;
            
         case corabase_failure_access_expired:
            outcome = client_type::outcome_failure_access_expired;
            break;
         }
         event_complete::cpost(this, outcome);
      } // on_corabase_failure

      void AccessTokenGetter::start_logon()
      {
         if(refresh_token.length() > 0)
            on_corabase_ready();
         else
            ClientBase::start_logon();
      } // start_logon

      void AccessTokenGetter::onNetMessage(
         Csi::Messaging::Router *router, Csi::Messaging::Message *message)
      {
         if(state == state_active)
         {
            if(message->getMsgType() == Messages::get_access_token_ack)
            {
               uint4 tran_no(0), rcd(0);
               StrAsc access_token, refresh_token;
               client_type::outcome_type outcome(client_type::outcome_failure_unknown);

               if(message->readUInt4(tran_no) && message->readUInt4(rcd))
               {
                  if(rcd == 1 && message->readStr(access_token) && message->readStr(refresh_token))
                     outcome = client_type::outcome_success;
                  else
                  {
                     switch(rcd)
                     {
                     case 3:
                        outcome = client_type::outcome_failure_no_account;
                        break;

                     case 4:
                        outcome = client_type::outcome_failure_invalid_refresh;
                        break;

                     case 5:
                        outcome = client_type::outcome_failure_refresh_expired;
                        break;
                     }
                  }  
               }
               event_complete::cpost(this, outcome, access_token, refresh_token);
            }
            else
               ClientBase::onNetMessage(router, message);
         }
         else
            ClientBase::onNetMessage(router, message);
      } // onNetMessage
   };
};

