/* Cora.Sec2.AccountDeleter.cpp

   Copyright (C) 2002, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Monday 30 December 2002
   Last Change: Saturday 19 October 2019
   Last Commit: $Date: 2019-10-19 11:15:49 -0600 (Sat, 19 Oct 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Sec2.AccountDeleter.h"
#include "coratools.strings.h"


namespace Cora
{
   namespace Sec2
   {
      namespace
      {
         class event_complete: public Csi::Event
         {
         public:
            static uint4 const event_id;
            typedef AccountDeleterClient client_type;
            client_type *client;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            static void create_and_post(
               AccountDeleter *adder,
               client_type *client,
               outcome_type outcome)
            {
               try{(new event_complete(adder,client,outcome))->post(); }
               catch(Csi::Event::BadPost &) { }
            }

         private:
            event_complete(
               AccountDeleter *adder,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,adder),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Sec2::AccountDeleter::event_complete"); 
      };


      AccountDeleter::AccountDeleter():
         client(0),
         state(state_standby)
      { }

      
      AccountDeleter::~AccountDeleter()
      { finish(); }

      
      void AccountDeleter::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_standby;
               Sec2Base::start(router);
            }
            else
               throw std::invalid_argument("Invalid argument specified");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void AccountDeleter::start(
         client_type *client_,
         ClientBase *other_client)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_standby;
               Sec2Base::start(other_client);
            }
            else
               throw std::invalid_argument("Invalid argument specified");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void AccountDeleter::finish()
      {
         state = state_standby;
         client = 0;
         Sec2Base::finish();
      } // finish


      void AccountDeleter::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace AccountDeleterStrings;
         switch(outcome)
         {
         case client_type::outcome_success:
            out << my_strings[strid_outcome_success];
            break;
            
         case client_type::outcome_connection_failed:
            format_failure(out, sec2base_failure_session);
            break;
            
         case client_type::outcome_invalid_logon:
            format_failure(out, sec2base_failure_logon);
            break;
            
         case client_type::outcome_insufficient_access:
            format_failure(out, sec2base_failure_security);
            break;
            
         case client_type::outcome_unsupported:
            format_failure(out, sec2base_failure_unsupported);
            break;
            
         case client_type::outcome_locked:
            out << my_strings[strid_outcome_locked];
            break;
            
         case client_type::outcome_invalid_account_name:
            out << my_strings[strid_outcome_invalid_account_name];
            break;
            
         case client_type::outcome_account_in_use:
            out << my_strings[strid_outcome_account_in_use];
            break;

         default:
            format_failure(out, sec2base_failure_unknown);
            break;
         }
      }
      
      void AccountDeleter::on_sec2base_ready()
      {
         Csi::Messaging::Message command(sec2_session,Messages::delete_account_cmd);
         command.addUInt4(++last_tran_no);
         command.addWStr(account_name);
         state = state_active;
         router->sendMessage(&command);
      } // on_sec2base_ready

      
      void AccountDeleter::on_sec2base_failure(sec2base_failure_type failure)
      {
         client_type::outcome_type outcome;
         switch(failure)
         {
         case sec2base_failure_logon:
            outcome = client_type::outcome_invalid_logon;
            break;
            
         case sec2base_failure_session:
            outcome = client_type::outcome_connection_failed;
            break;
            
         case sec2base_failure_unsupported:
            outcome = client_type::outcome_unsupported;
            break;
            
         case sec2base_failure_security:
            outcome = client_type::outcome_insufficient_access;
            break;
            
         default:
            outcome = client_type::outcome_unknown;
            break;
                     
         }
         event_complete::create_and_post(this,client,outcome);
      } // on_sec2base_failure

      
      void AccountDeleter::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::delete_account_ack)
            {
               uint4 tran_no;
               uint4 server_outcome;
               client_type::outcome_type client_outcome;

               msg->readUInt4(tran_no);
               msg->readUInt4(server_outcome);
               switch(server_outcome)
               {
               case 1:
                  client_outcome = client_type::outcome_success;
                  break;

               case 3:
                  client_outcome = client_type::outcome_insufficient_access;
                  break;

               case 4:
                  client_outcome = client_type::outcome_locked;
                  break;

               case 5:
                  client_outcome = client_type::outcome_invalid_account_name;
                  break;

               case 6:
                  client_outcome = client_type::outcome_account_in_use;
                  break;
                  
               default:
                  client_outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::create_and_post(this,client,client_outcome);
            }
            else
               Sec2Base::onNetMessage(rtr,msg);
         }
         else
            Sec2Base::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void AccountDeleter::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = event->client;
            finish();
            if(client == event->client && client_type::is_valid_instance(client))
               client->on_complete(this,event->outcome); 
         }
      } // receive 
   };
};
