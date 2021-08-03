/* Cora.Sec2.Enabler.cpp

   Copyright (C) 2002, 2019 Campbell Scientific, Inc.

   Written by: Jon Trauntvein
   Date Begun: Tuesday 24 December 2002
   Last Change: Saturday 19 October 2019
   Last Commit: $Date: 2019-10-19 10:35:54 -0600 (Sat, 19 Oct 2019) $ 
   Last Changed by: $Author: jon $

*/

#pragma hdrstop               // stop creation of precompiled header
#include "Cora.Sec2.Enabler.h"
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
            typedef EnablerClient client_type;
            client_type *client;
            typedef client_type::outcome_type outcome_type;
            outcome_type outcome;

            static void create_and_post(
               Enabler *enabler,
               client_type *client,
               outcome_type outcome)
            {
               try{(new event_complete(enabler,client,outcome))->post();}
               catch(Csi::Event::BadPost &) { }
            }

         private:
            event_complete(
               Enabler *enabler,
               client_type *client_,
               outcome_type outcome_):
               Event(event_id,enabler),
               client(client_),
               outcome(outcome_)
            { }
         };


         uint4 const event_complete::event_id =
         Csi::Event::registerType("Cora::Sec2::Enabler::event_complete");
      };


      Enabler::Enabler():
         client(0),
         state(state_standby),
         security_enabled(false)
      { }

      
      Enabler::~Enabler()
      { finish(); }

      
      void Enabler::set_security_enabled(bool security_enabled_)
      {
         if(state == state_standby)
            security_enabled = security_enabled_;
         else
            throw exc_invalid_state();
      } // set_security_enabled

      
      void Enabler::start(
         client_type *client_,
         router_handle &router)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               Sec2Base::start(router);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void Enabler::start(
         client_type *client_,
         ClientBase *other_component)
      {
         if(state == state_standby)
         {
            if(client_type::is_valid_instance(client_))
            {
               client = client_;
               state = state_delegate;
               Sec2Base::start(other_component);
            }
            else
               throw std::invalid_argument("Invalid client pointer");
         }
         else
            throw exc_invalid_state();
      } // start

      
      void Enabler::finish()
      {
         client = 0;
         state = state_standby;
         Sec2Base::finish();
      } // finish


      void Enabler::describe_outcome(std::ostream &out, client_type::outcome_type outcome)
      {
         using namespace EnablerStrings;
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
            
         case client_type::outcome_no_root_account:
            out << my_strings[strid_outcome_no_root_account];
            break;
            
         case client_type::outcome_locked:
            out << my_strings[strid_outcome_locked];
            break;
            
         case client_type::outcome_not_admin:
            out << my_strings[strid_outcome_not_admin];
            break;

         default:
            format_failure(out, sec2base_failure_unknown);
            break;
         }
      } // describe_outcome
      
      
      void Enabler::on_sec2base_ready()
      {
         if(interface_version >= Csi::VersionNumber("1.3.7.8"))
         {
            Csi::Messaging::Message command(sec2_session,Messages::enable_cmd);
            command.addUInt4(++last_tran_no);
            command.addBool(security_enabled);
            state = state_active;
            router->sendMessage(&command);
         }
         else
            event_complete::create_and_post(this,client,client_type::outcome_not_admin);
      } // on_sec2base_ready

      
      void Enabler::on_sec2base_failure(sec2base_failure_type failure)
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

      
      void Enabler::onNetMessage(
         Csi::Messaging::Router *rtr,
         Csi::Messaging::Message *msg)
      {
         if(state == state_active)
         {
            if(msg->getMsgType() == Messages::enable_ack)
            {
               uint4 tran_no;
               uint4 response;
               client_type::outcome_type outcome;
               
               msg->readUInt4(tran_no);
               msg->readUInt4(response);
               switch(response)
               {
               case 1:
                  outcome = client_type::outcome_success;
                  break;

               case 3:
                  outcome = client_type::outcome_insufficient_access;
                  break;

               case 4:
                  outcome = client_type::outcome_no_root_account;
                  break;

               case 5:
                  outcome = client_type::outcome_locked;
                  break;

               case 6:
                  outcome = client_type::outcome_not_admin;
                  break;
                  
               default:
                  outcome = client_type::outcome_unknown;
                  break;
               }
               event_complete::create_and_post(this,client,outcome);
            }
            else
               Sec2Base::onNetMessage(rtr,msg);
         }
         else
            Sec2Base::onNetMessage(rtr,msg);
      } // onNetMessage

      
      void Enabler::receive(Csi::SharedPtr<Csi::Event> &ev)
      {
         if(ev->getType() == event_complete::event_id)
         {
            event_complete *event = static_cast<event_complete *>(ev.get_rep());
            client_type *client = this->client;
            finish();
            if(client == event->client && client_type::is_valid_instance(client))
               client->on_complete(this,event->outcome); 
         }
      } // receive 
   };
};
